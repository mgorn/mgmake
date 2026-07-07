#pragma once

#ifndef MGMK_EXT_CMAKE_FILE_API_HXX
#define MGMK_EXT_CMAKE_FILE_API_HXX

#include "codemodel.hxx"
#include "../json.hxx"

#include <algorithm>
#include <expected>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// CMake File API helpers request and locate codemodel replies. Parsed CMake data is owned by target/codemodel.

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

	[[nodiscard]] inline std::expected<std::string, std::string> read_file(
		const std::filesystem::path& path
	) {
		std::ifstream in(path, std::ios::binary);

		if (!in.is_open()) {
			return std::unexpected{"failed to open file '" + path.string() + "'"};
		}

		return std::string{
			std::istreambuf_iterator<char>{in},
			std::istreambuf_iterator<char>{}
		};
	}

	[[nodiscard]] inline std::expected<ext::json, std::string> parse_json_file(
		const std::filesystem::path& path
	) {
		const auto content = read_file(path);

		if (!content.has_value()) {
			return std::unexpected{content.error()};
		}

		auto parsed = ext::json::parse(*content);

		if (!parsed.has_value()) {
			return std::unexpected{"failed to parse JSON file '" + path.string() + "'"};
		}

		return std::move(*parsed);
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

	[[nodiscard]] inline bool kind_is(
		const ext::json& value,
		std::string_view expected_kind
	) {
		if (!value.has("kind")) {
			return false;
		}

		const auto kind = value.get("kind").as_string();
		return kind.has_value() && *kind == expected_kind;
	}

	[[nodiscard]] inline std::expected<std::filesystem::path, std::string> codemodel_file_from_index(
		const ext::json& index,
		const std::filesystem::path& dir,
		const std::filesystem::path& index_file
	) {
		if (index.has("reply")) {
			const auto reply = index.get("reply");

			if (reply.has("client-mgmake")) {
				const auto client = reply.get("client-mgmake");
				if (client.has("query.json")) {
					const auto query = client.get("query.json");
					mgmkassert(query.has("responses"), "Query doesn't have responses");
					auto responses = query.get("responses");
					for (const auto& response : query.array("responses")) {
						if (!kind_is(response, "codemodel")) {
							continue;
						}

						if (!response.has("jsonFile")) {
							return std::unexpected{"codemodel response in '" + index_file.string() + "' is missing 'jsonFile'"};
						}

						const auto json_file = response.get("jsonFile");

						const auto json_file_text = json_file.as_string();

						if (!json_file_text.has_value() || json_file_text->empty()) {
							return std::unexpected{"codemodel response in '" + index_file.string() + "' has non-string 'jsonFile'"};
						}

						const auto path = dir / *json_file_text;

						if (!std::filesystem::exists(path)) {
							return std::unexpected{"codemodel response in '" + index_file.string() + "' references missing file '" + path.string() + "'"};
						}

						return path;
					}
				}
			}
		}

		for (const auto& object : index.array("objects")) {
			if (!kind_is(object, "codemodel")) {
				continue;
			}

			if (!object.has("jsonFile")) {
				return std::unexpected{"codemodel object in '" + index_file.string() + "' is missing 'jsonFile'"};
			}

			const auto json_file = object.get("jsonFile");

			const auto json_file_text = json_file.as_string();

			if (!json_file_text.has_value() || json_file_text->empty()) {
				return std::unexpected{"codemodel object in '" + index_file.string() + "' has non-string 'jsonFile'"};
			}

			const auto path = dir / *json_file_text;

			if (!std::filesystem::exists(path)) {
				return std::unexpected{"codemodel object in '" + index_file.string() + "' references missing file '" + path.string() + "'"};
			}

			return path;
		}

		return std::unexpected{"index file '" + index_file.string() + "' does not contain a codemodel response"};
	}

	[[nodiscard]] inline std::expected<std::filesystem::path, std::string> codemodel_file_from_reply(
		const std::filesystem::path& dir
	) {
		if (!std::filesystem::exists(dir)) {
			return std::unexpected{"CMake File API reply directory does not exist: '" + dir.string() + "'"};
		}

		for (const auto& index_path : index_files_newest_first(dir)) {
			const auto index = parse_json_file(index_path);

			if (!index.has_value()) {
				return std::unexpected{index.error()};
			}

			auto codemodel_file = codemodel_file_from_index(*index, dir, index_path);

			if (!codemodel_file.has_value()) {
				return std::unexpected{codemodel_file.error()};
			}

			return *codemodel_file;
		}

		for (const auto& path : codemodel_files_newest_first(dir)) {
			return path;
		}

		return std::unexpected{"CMake File API reply directory contains no codemodel reply: '" + dir.string() + "'"};
	}

	[[nodiscard]] inline std::expected<std::vector<std::filesystem::path>, std::string> target_files_from_codemodel(
		const ext::json& codemodel_json,
		const std::filesystem::path& codemodel_file
	) {
		std::vector<std::filesystem::path> result;
		std::set<std::string> seen;

		for (const auto& configuration : codemodel_json.array("configurations")) {
			for (std::string_view key : {std::string_view{"targets"}, std::string_view{"abstractTargets"}}) {
				for (const auto& target_ref : configuration.array(std::string{key})) {

					if (!target_ref.has("jsonFile")) {
						return std::unexpected{"target reference in '" + codemodel_file.string() + "' is missing 'jsonFile'"};
					}

					const auto json_file = target_ref.get("jsonFile");
					const auto json_file_text = json_file.as_string();

					if (!json_file_text.has_value() || json_file_text->empty()) {
						return std::unexpected{"target reference in '" + codemodel_file.string() + "' has non-string 'jsonFile'"};
					}

					if (seen.emplace(*json_file_text).second) {
						result.emplace_back(*json_file_text);
					}
				}
			}
		}

		if (result.empty()) {
			return std::unexpected{"codemodel '" + codemodel_file.string() + "' contains no target jsonFile references"};
		}

		return result;
	}
}

namespace mgmake::ext::cmake {
	inline std::expected<codemodel, std::string> codemodel::from_target_files(
		const std::filesystem::path& reply_dir,
		const std::filesystem::path& build_dir,
		const std::vector<std::filesystem::path>& target_files
	) {
		if (target_files.empty()) {
			return std::unexpected{"cannot create CMake codemodel from an empty target file list"};
		}

		codemodel result{};

		for (const auto& target_file : target_files) {
			const auto path = target_file.is_absolute()
				? target_file
				: reply_dir / target_file;

			const auto parsed = file_api::parse_json_file(path);

			if (!parsed.has_value()) {
				return std::unexpected{parsed.error()};
			}

			auto target = target::from_file_api_json(*parsed, build_dir, path);

			if (!target.has_value()) {
				return std::unexpected{target.error()};
			}

			result.add_target(std::move(*target));
		}

		if (result.empty()) {
			return std::unexpected{"CMake target files were parsed but produced an empty codemodel"};
		}

		return result;
	}

	inline std::expected<codemodel, std::string> codemodel::from_file_api_reply(
		const std::filesystem::path& build_dir
	) {
		const auto dir = file_api::reply_dir(build_dir);
		const auto codemodel_file = file_api::codemodel_file_from_reply(dir);

		if (!codemodel_file.has_value()) {
			return std::unexpected{codemodel_file.error()};
		}

		const auto codemodel_json = file_api::parse_json_file(*codemodel_file);

		if (!codemodel_json.has_value()) {
			return std::unexpected{codemodel_json.error()};
		}

		const auto target_files = file_api::target_files_from_codemodel(*codemodel_json, *codemodel_file);

		if (!target_files.has_value()) {
			return std::unexpected{target_files.error()};
		}

		return codemodel::from_target_files(dir, build_dir, *target_files);
	}
}

#endif // MGMK_EXT_CMAKE_FILE_API_HXX
