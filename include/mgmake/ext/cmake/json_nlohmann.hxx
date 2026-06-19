#pragma once

#ifndef MGMK_EXT_CMAKE_JSON_NLOHMANN_HXX
#define MGMK_EXT_CMAKE_JSON_NLOHMANN_HXX

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mgmake::ext::cmake_json_nlohmann {
	using json = nlohmann::json;

	[[nodiscard]] inline std::optional<json> parse(std::string_view text) {
		try {
			return json::parse(text.begin(), text.end());
		} catch (...) {
			return std::nullopt;
		}
	}

	[[nodiscard]] inline std::optional<std::string> object_string(
		const json& object,
		std::string_view key
	) {
		if (!object.is_object()) {
			return std::nullopt;
		}

		const auto found = object.find(std::string{key});

		if (found == object.end() || !found->is_string()) {
			return std::nullopt;
		}

		return found->get<std::string>();
	}

	[[nodiscard]] inline std::vector<std::filesystem::path> artifact_paths(
		const json& object
	) {
		std::vector<std::filesystem::path> result;

		if (!object.is_object()) {
			return result;
		}

		const auto artifacts = object.find("artifacts");

		if (artifacts == object.end() || !artifacts->is_array()) {
			return result;
		}

		for (const auto& artifact : *artifacts) {
			const auto path = object_string(artifact, "path");

			if (path.has_value()) {
				result.emplace_back(*path);
			}
		}

		return result;
	}
}

#endif
