#pragma once

#ifndef MGMK_EXT_CMAKE_FILE_API_HXX
#define MGMK_EXT_CMAKE_FILE_API_HXX

#include "codemodel.hxx"
#include "../json.hxx"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <string>
#include <utility>

// CMake File API helpers request and parse codemodel replies so mgmake can find external target artifacts.

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

		return result;
	}

	inline void load_reply_targets(
		codemodel& model,
		const std::filesystem::path& build_dir
	) {
		const auto dir = reply_dir(build_dir);

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

			auto target = parse_target_file(entry.path(), build_dir);

			if (!target.has_value() || target->m_name.empty()) {
				continue;
			}

			model.m_targets.insert_or_assign(target->m_name, std::move(*target));
		}
	}
}

#endif // MGMK_EXT_CMAKE_FILE_API_HXX
