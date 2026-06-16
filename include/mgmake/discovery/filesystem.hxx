#pragma once

#ifndef MGMAKE_DISCOVERY_FILESYSTEM_HXX
#define MGMAKE_DISCOVERY_FILESYSTEM_HXX

#include "environment.hxx"

#include <cctype>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#if defined(MGMK_PLATFORM_POSIX)
	#include <unistd.h>
#endif

namespace mgmake::discovery {
	[[nodiscard]] inline bool has_windows_drive_prefix(std::string_view text) noexcept {
		return text.size() >= 2
			&& std::isalpha(static_cast<unsigned char>(text[0]))
			&& text[1] == ':';
	}

	[[nodiscard]] inline bool is_path_like(std::string_view text) {
		return text.find('/') != std::string_view::npos
			|| text.find('\\') != std::string_view::npos
			|| has_windows_drive_prefix(text);
	}

	[[nodiscard]] inline bool is_explicit_path(std::string_view text) {
		return is_path_like(text);
	}

	[[nodiscard]] inline bool is_launchable_file(const std::filesystem::path& path) {
		std::error_code ec;

		if (!std::filesystem::exists(path, ec) || ec) {
			return false;
		}

		if (!std::filesystem::is_regular_file(path, ec) || ec) {
			return false;
		}

#if defined(_WIN32)
		return true;
#else
		return ::access(path.c_str(), X_OK) == 0;
#endif
	}

	[[nodiscard]] inline std::vector<std::string> executable_suffixes() {
#if defined(_WIN32)
		return {".com", ".exe", ".bat", ".cmd", ""};
#else
		return {""};
#endif
	}

	[[nodiscard]] inline std::optional<std::filesystem::path> find_in_directory(
		const std::filesystem::path& dir,
		std::string_view name
	) {
		if (dir.empty() || name.empty()) {
			return std::nullopt;
		}

		std::filesystem::path requested{name};

		if (requested.has_extension()) {
			auto candidate = dir / requested;

			if (is_launchable_file(candidate)) {
				return std::filesystem::absolute(candidate);
			}
		}

		for (const auto& suffix : executable_suffixes()) {
			auto candidate = dir / (std::string{name} + suffix);

			if (is_launchable_file(candidate)) {
				return std::filesystem::absolute(candidate);
			}
		}

		return std::nullopt;
	}

	[[nodiscard]] inline std::vector<std::filesystem::path> path_entries() {
		std::vector<std::filesystem::path> result;
		auto path = getenv_string("PATH");

		if (!path) {
			return result;
		}

#if defined(_WIN32)
		constexpr char separator = ';';
#else
		constexpr char separator = ':';
#endif

		std::string_view view{*path};
		std::size_t start = 0;

		while (start <= view.size()) {
			const auto end = view.find(separator, start);
			const auto part = view.substr(
				start,
				end == std::string_view::npos ? std::string_view::npos : end - start
			);

			if (!part.empty()) {
				result.emplace_back(part);
			}

			if (end == std::string_view::npos) {
				break;
			}

			start = end + 1;
		}

		return result;
	}

	[[nodiscard]] inline std::optional<std::filesystem::path> find_on_path(
		std::string_view name
	) {
		if (is_explicit_path(name)) {
			std::filesystem::path path{name};

			if (is_launchable_file(path)) {
				return std::filesystem::absolute(path);
			}

			return std::nullopt;
		}

		for (const auto& dir : path_entries()) {
			if (auto found = find_in_directory(dir, name)) {
				return found;
			}
		}

		return std::nullopt;
	}
}

#endif
