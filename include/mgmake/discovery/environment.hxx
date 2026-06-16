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
		const std::string key{name};

#if defined(_WIN32) && defined(_MSC_VER)
		char* value = nullptr;
		std::size_t size = 0;

		const errno_t err = _dupenv_s(&value, &size, key.c_str());

		if (err != 0 || value == nullptr || value[0] == '\0') {
			if (value != nullptr) {
				std::free(value);
			}

			return std::nullopt;
		}

		std::string result{value};
		std::free(value);
		return result;
#else
		const char* value = std::getenv(key.c_str());

		if (value == nullptr || value[0] == '\0') {
			return std::nullopt;
		}

		return std::string{value};
#endif
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
