#pragma once

#ifndef MGMK_EXT_CMAKE_FILE_API_HXX
#define MGMK_EXT_CMAKE_FILE_API_HXX

#include "../provider_ref.hxx"

#if defined(MGMK_JSON_BACKEND_HEADER)
	#include MGMK_JSON_BACKEND_HEADER
#elif defined(__cxxmg_urlinclude)
	#urlinclude <https://raw.githubusercontent.com/nlohmann/json/refs/tags/v3.12.0/single_include/nlohmann/json.hpp>
#endif

#if defined(INCLUDE_NLOHMANN_JSON_HPP_) || defined(NLOHMANN_JSON_VERSION_MAJOR)
	#define MGMK_EXT_CMAKE_HAS_JSON_BACKEND 1
	#include "json_nlohmann.hxx"
#endif

#ifndef MGMK_EXT_CMAKE_HAS_JSON_BACKEND
	#if !defined(MGMK_JSON_BACKEND_HEADER) && !defined(__cxxmg_urlinclude)
		#pragma message("mgmake: A JSON backend is needed for CMake File API support. Define MGMK_JSON_BACKEND_HEADER with your preferred JSON library header.")
	#else
		#pragma message("mgmake: The selected JSON backend is not recognized by mgmake CMake File API support.")
	#endif
#endif

#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

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

	[[nodiscard]] inline std::optional<target> parse_target_file(
		const std::filesystem::path& file,
		const std::filesystem::path& build_dir
	) {
#ifndef MGMK_EXT_CMAKE_HAS_JSON_BACKEND
		(void)file;
		(void)build_dir;
		return std::nullopt;
#else
		const auto content = read_file(file);

		if (!content.has_value()) {
			return std::nullopt;
		}

		const auto json = ext::cmake_json_nlohmann::parse(*content);

		if (!json.has_value()) {
			return std::nullopt;
		}

		const auto name = ext::cmake_json_nlohmann::object_string(*json, "name");
		const auto type = ext::cmake_json_nlohmann::object_string(*json, "type");

		if (!name.has_value()) {
			return std::nullopt;
		}

		target result{};
		result.m_name = *name;
		result.m_type = type.value_or(std::string{});
		result.m_artifacts = ext::cmake_json_nlohmann::artifact_paths(*json);

		for (auto& artifact : result.m_artifacts) {
			if (artifact.is_relative()) {
				artifact = build_dir / artifact;
			}
		}

		if (!result.m_artifacts.empty()) {
			result.m_artifact = result.m_artifacts.front();
		}

		return result;
#endif
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

#endif
