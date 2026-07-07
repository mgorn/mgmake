#pragma once

#ifndef MGMK_EXT_CMAKE_FILE_API_HXX
#define MGMK_EXT_CMAKE_FILE_API_HXX

#include "codemodel.hxx"
#include "../json.hxx"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// CMake File API helpers request and parse codemodel replies so mgmake can import external target artifacts and link usage.

namespace mgmake::ext::cmake::file_api {
	[[nodiscard]] inline std::filesystem::path query_file(
		const std::filesystem::path& build_dir
	) {
		return build_dir / ".cmake" / "api" / "v1" / "query" / "client-mgmake" / "query.json";
	}

	[[nodiscard]] inline std::filesystem::path reply_dir(
		const std::filesystem::path& build_dir
	) {
		return build_dir / ".cmake" / "api" / "v1" / "reply";
	}

	[[nodiscard]] inline std::string codemodel_query_text() {
		return R"({"requests":[{"kind":"codemodel","version":{"major":2,"minor":9}}]})";
	}

	[[nodiscard]] inline bool write_query_file(
		const std::filesystem::path& build_dir
	) {
		const auto path = query_file(build_dir);
		const auto dir = path.parent_path();

		if (!dir.empty()) {
			std::filesystem::create_directories(dir);
		}

		std::ofstream out{path, std::ios::binary | std::ios::trunc};

		if (!out.is_open()) {
			return false;
		}

		out << codemodel_query_text();
		return out.good();
	}

	[[nodiscard]] inline std::optional<std::string> read_file(
		const std::filesystem::path& path
	) {
		std::ifstream in(path, std::ios::binary);

		if (!in.is_open()) {
			return std::nullopt;
		}

		return std::string{
			std::istreambuf_iterator<char>{in},
			std::istreambuf_iterator<char>{}
		};
	}

	[[nodiscard]] inline std::optional<ext::json> parse_json_file(
		const std::filesystem::path& path
	) {
		const auto content = read_file(path);

		if (!content.has_value()) {
			return std::nullopt;
		}

		return ext::json::parse(*content);
	}

	[[nodiscard]] inline std::vector<std::filesystem::path> index_files_newest_first(
		const std::filesystem::path& dir
	) {
		std::vector<std::filesystem::path> result;

		if (!std::filesystem::exists(dir)) {
			return result;
		}

		for (const auto& entry : std::filesystem::directory_iterator{dir}) {
			if (!entry.is_regular_file()) {
				continue;
			}

			const auto filename = entry.path().filename().string();

			if (!filename.starts_with("index-") || entry.path().extension() != ".json") {
				continue;
			}

			result.emplace_back(entry.path());
		}

		std::ranges::sort(result, [](const auto& lhs, const auto& rhs) {
			// CMake embeds an ISO-like timestamp in File API reply filenames. Prefer
			// that stable timestamp over filesystem mtimes so copied/restored build
			// directories still select the newest reply deterministically.
			return lhs.filename().string() > rhs.filename().string();
		});

		return result;
	}

	[[nodiscard]] inline std::vector<std::filesystem::path> codemodel_files_newest_first(
		const std::filesystem::path& dir
	) {
		std::vector<std::filesystem::path> result;

		if (!std::filesystem::exists(dir)) {
			return result;
		}

		for (const auto& entry : std::filesystem::directory_iterator{dir}) {
			if (!entry.is_regular_file()) {
				continue;
			}

			const auto filename = entry.path().filename().string();

			if (!filename.starts_with("codemodel-v2") || entry.path().extension() != ".json") {
				continue;
			}

			result.emplace_back(entry.path());
		}

		std::ranges::sort(result, [](const auto& lhs, const auto& rhs) {
			const auto lhs_time = std::filesystem::last_write_time(lhs);
			const auto rhs_time = std::filesystem::last_write_time(rhs);

			if (lhs_time != rhs_time) {
				return lhs_time > rhs_time;
			}

			return lhs.filename().string() > rhs.filename().string();
		});

		return result;
	}

	[[nodiscard]] inline std::optional<std::string> json_string_member(
		const ext::json& value,
		std::string_view key
	) {
		const auto member = value.get(key);

		if (!member.has_value()) {
			return std::nullopt;
		}

		return member->as_string();
	}

	[[nodiscard]] inline bool json_kind_is(
		const ext::json& value,
		std::string_view expected_kind
	) {
		const auto kind = json_string_member(value, "kind");
		return kind.has_value() && *kind == expected_kind;
	}

	[[nodiscard]] inline std::optional<std::string> codemodel_file_from_client_reply(
		const ext::json& index
	) {
		const auto reply = index.get("reply");

		if (!reply.has_value()) {
			return std::nullopt;
		}

		const auto client = reply->get("client-mgmake");

		if (!client.has_value()) {
			return std::nullopt;
		}

		const auto query = client->get("query.json");

		if (!query.has_value()) {
			return std::nullopt;
		}

		for (const auto& response : query->array("responses")) {
			if (!json_kind_is(response, "codemodel")) {
				continue;
			}

			const auto json_file = json_string_member(response, "jsonFile");

			if (json_file.has_value()) {
				return json_file;
			}
		}

		return std::nullopt;
	}

	[[nodiscard]] inline std::optional<std::string> codemodel_file_from_objects(
		const ext::json& index
	) {
		for (const auto& object : index.array("objects")) {
			if (!json_kind_is(object, "codemodel")) {
				continue;
			}

			const auto json_file = json_string_member(object, "jsonFile");

			if (json_file.has_value()) {
				return json_file;
			}
		}

		return std::nullopt;
	}

	[[nodiscard]] inline std::optional<std::string> codemodel_file_from_index(
		const ext::json& index
	) {
		if (const auto client_reply = codemodel_file_from_client_reply(index)) {
			return client_reply;
		}

		return codemodel_file_from_objects(index);
	}

	inline void append_target_files_from_array(
		const ext::json& object,
		std::string_view key,
		std::vector<std::string>& out,
		std::set<std::string>& seen
	) {
		for (const auto& target_ref : object.array(key)) {
			const auto json_file = json_string_member(target_ref, "jsonFile");

			if (!json_file.has_value() || json_file->empty()) {
				continue;
			}

			if (seen.emplace(*json_file).second) {
				out.emplace_back(*json_file);
			}
		}
	}

	[[nodiscard]] inline std::vector<std::string> target_files_from_codemodel(
		const ext::json& codemodel_json
	) {
		std::vector<std::string> result;
		std::set<std::string> seen;

		for (const auto& configuration : codemodel_json.array("configurations")) {
			append_target_files_from_array(configuration, "targets", result, seen);
			append_target_files_from_array(configuration, "abstractTargets", result, seen);
		}

		return result;
	}

	[[nodiscard]] inline std::vector<std::string> target_files_from_reply_dir(
		const std::filesystem::path& dir
	) {
		std::vector<std::string> result;

		if (!std::filesystem::exists(dir)) {
			return result;
		}

		for (const auto& entry : std::filesystem::directory_iterator{dir}) {
			if (!entry.is_regular_file()) {
				continue;
			}

			const auto filename = entry.path().filename().string();

			if (filename.starts_with("target-") && entry.path().extension() == ".json") {
				result.emplace_back(filename);
			}
		}

		std::ranges::sort(result);
		return result;
	}

	inline void parse_link_entries(
		const ext::json& parsed,
		std::string_view key,
		std::vector<link_entry>& out
	) {
		for (const auto& item : parsed.array(key)) {
			if (const auto fragment = item.get("fragment")) {
				if (const auto text = fragment->as_string()) {
					out.emplace_back(link_entry{
						.m_kind = link_entry_kind::fragment,
						.m_value = *text
					});
				}
			}

			if (const auto id = item.get("id")) {
				if (const auto text = id->as_string()) {
					out.emplace_back(link_entry{
						.m_kind = link_entry_kind::target_id,
						.m_value = *text
					});
				}
			}
		}
	}

	// Only the codemodel fields needed for provider artifact and link-usage lookup are materialized.
	[[nodiscard]] inline std::optional<target> parse_target_file(
		const std::filesystem::path& file,
		const std::filesystem::path& build_dir
	) {
		const auto parsed = parse_json_file(file);

		if (!parsed.has_value()) {
			return std::nullopt;
		}

		const auto name_text = json_string_member(*parsed, "name");

		if (!name_text.has_value()) {
			return std::nullopt;
		}

		target result{};
		result.m_name = *name_text;

		if (const auto id_text = json_string_member(*parsed, "id")) {
			result.m_id = *id_text;
		}

		if (const auto type_text = json_string_member(*parsed, "type")) {
			result.m_type = *type_text;
		}

		for (const auto& artifact : parsed->array("artifacts")) {
			const auto path_text = json_string_member(artifact, "path");

			if (!path_text.has_value()) {
				continue;
			}

			result.m_artifacts.emplace_back(*path_text);
		}

		for (auto& artifact : result.m_artifacts) {
			if (artifact.is_relative()) {
				artifact = build_dir / artifact;
			}
		}

		parse_link_entries(*parsed, "linkLibraries", result.m_link_entries);
		parse_link_entries(*parsed, "interfaceLinkLibraries", result.m_interface_link_entries);

		return result;
	}

	[[nodiscard]] inline std::optional<std::filesystem::path> codemodel_file_from_reply(
		const std::filesystem::path& dir
	) {
		for (const auto& index_path : index_files_newest_first(dir)) {
			const auto index = parse_json_file(index_path);

			if (!index.has_value()) {
				continue;
			}

			const auto codemodel_file = codemodel_file_from_index(*index);

			if (!codemodel_file.has_value() || codemodel_file->empty()) {
				continue;
			}

			const auto path = dir / *codemodel_file;

			if (std::filesystem::exists(path)) {
				return path;
			}
		}

		for (const auto& path : codemodel_files_newest_first(dir)) {
			return path;
		}

		return std::nullopt;
	}

	inline std::size_t load_reply_targets(
		codemodel& model,
		const std::filesystem::path& build_dir
	) {
		const auto dir = reply_dir(build_dir);
		const auto codemodel_file = codemodel_file_from_reply(dir);

		if (!codemodel_file.has_value()) {
			model = codemodel{};
			return 0;
		}

		const auto codemodel_json = parse_json_file(*codemodel_file);

		if (!codemodel_json.has_value()) {
			model = codemodel{};
			return 0;
		}

		auto load_target_files = [&](const std::vector<std::string>& target_files) {
			codemodel loaded{};

			for (const auto& target_file : target_files) {
				auto target = parse_target_file(dir / target_file, build_dir);

				if (!target.has_value() || target->m_name.empty()) {
					continue;
				}

				loaded.add_target(std::move(*target));
			}

			return loaded;
		};

		auto loaded = load_target_files(target_files_from_codemodel(*codemodel_json));

		if (loaded.empty()) {
			// If the codemodel-to-target reference path fails, still try the reply
			// directory before reporting zero targets. The index/codemodel path remains
			// primary so stale target files are only considered when loading would fail.
			loaded = load_target_files(target_files_from_reply_dir(dir));
		}

		model = std::move(loaded);
		return model.size();
	}
}

#endif // MGMK_EXT_CMAKE_FILE_API_HXX
