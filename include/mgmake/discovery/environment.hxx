#pragma once

#ifndef MGMAKE_DISCOVERY_ENVIRONMENT_HXX
#define MGMAKE_DISCOVERY_ENVIRONMENT_HXX

#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace mgmake::discovery {
	[[nodiscard]] inline std::optional<std::string> getenv_string(std::string_view name) {
		std::string key{name};
		const char* value = std::getenv(key.c_str());

		if (value == nullptr || value[0] == '\0') {
			return std::nullopt;
		}

		return std::string{value};
	}

	[[nodiscard]] inline std::optional<std::filesystem::path> getenv_path(
		std::string_view name
	) {
		if (auto value = getenv_string(name)) {
			return std::filesystem::path{*value};
		}

		return std::nullopt;
	}
}

#endif
