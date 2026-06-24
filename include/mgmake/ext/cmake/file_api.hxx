#pragma once

#ifndef MGMK_EXT_CMAKE_FILE_API_HXX
#define MGMK_EXT_CMAKE_FILE_API_HXX

#include "../json.hxx"
#include "../provider_ref.hxx"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// CMake File API helpers request and parse codemodel replies so mgmake can find external target artifacts.

namespace mgmake::ext::cmake_file_api {
	struct target {
		std::string m_name;
		std::string m_type;
		std::filesystem::path m_artifact;
		std::vector<std::filesystem::path> m_artifacts;
	};

	struct project {
		std::filesystem::path m_source_dir;
		std::filesystem::path m_build_dir;
		std::filesystem::path m_install_dir;
		ext::output_root m_usage_root = ext::output_root::install_dir;
		std::map<std::string, target> m_targets;

		[[nodiscard]] const target* find_target(std::string_view name) const {
			const auto found = m_targets.find(std::string{name});
			return found == m_targets.end() ? nullptr : &found->second;
		}
	};

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
		return R"({"requests":[{"kind":"codemodel","version":2}]})";
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

	// Only the codemodel fields needed for provider artifact lookup are materialized.
	[[nodiscard]] inline std::optional<target> parse_target_file(
		const std::filesystem::path& file,
		const std::filesystem::path& build_dir
	) {
		const auto content = read_file(file);

		if (!content.has_value()) {
			return std::nullopt;
		}

		const auto parsed = ext::json::parse(*content);

		if (!parsed.has_value()) {
			return std::nullopt;
		}

		const auto name = parsed->get("name");

		if (!name.has_value()) {
			return std::nullopt;
		}

		const auto name_text = name->as_string();

		if (!name_text.has_value()) {
			return std::nullopt;
		}

		target result{};
		result.m_name = *name_text;

		if (const auto type = parsed->get("type")) {
			if (const auto type_text = type->as_string()) {
				result.m_type = *type_text;
			}
		}

		for (const auto& artifact : parsed->array("artifacts")) {
			const auto path = artifact.get("path");

			if (!path.has_value()) {
				continue;
			}

			const auto path_text = path->as_string();

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

		if (!result.m_artifacts.empty()) {
			result.m_artifact = result.m_artifacts.front();
		}

		return result;
	}

	inline void load_reply_targets(project& project) {
		const auto dir = reply_dir(project.m_build_dir);

		if (!std::filesystem::exists(dir)) {
			return;
		}

		for (const auto& entry : std::filesystem::directory_iterator{dir}) {
			if (!entry.is_regular_file()) {
				continue;
			}

			const auto filename = entry.path().filename().string();

			if (!filename.starts_with("target-") || entry.path().extension() != ".json") {
				continue;
			}

			auto target = parse_target_file(entry.path(), project.m_build_dir);

			if (!target.has_value() || target->m_name.empty()) {
				continue;
			}

			project.m_targets.insert_or_assign(target->m_name, std::move(*target));
		}
	}
}

#endif // MGMK_EXT_CMAKE_FILE_API_HXX
