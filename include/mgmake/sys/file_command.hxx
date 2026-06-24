#pragma once

#ifndef MGMAKE_SYS_FILE_COMMAND_HXX
#define MGMAKE_SYS_FILE_COMMAND_HXX

#include "command_line.hxx"
#include "util.hxx"

#include <filesystem>
#include <string>

// File commands are represented as command_line objects so prep/clean actions can share dry-run and verbose behavior.

namespace mgmake::sys {
	[[nodiscard]] inline command_line shell_command(std::string command) {
#if defined(MGMK_PLATFORM_WINDOWS)
		return command_line{{"cmd", "/C", std::move(command)}};
#else
		return command_line{{"/bin/sh", "-c", std::move(command)}};
#endif // defined(MGMK_PLATFORM_WINDOWS)
	}

	[[nodiscard]] inline std::string shell_path(const std::filesystem::path& path) {
		return shell_escape(path.string());
	}

	[[nodiscard]] inline command_line remove_path_command(const std::filesystem::path& path) {
#if defined(MGMK_PLATFORM_WINDOWS)
		return shell_command("if exist " + shell_path(path) + " rmdir /S /Q " + shell_path(path) + " & if exist " + shell_path(path) + " del /F /Q " + shell_path(path));
#else
		return shell_command("rm -rf " + shell_path(path));
#endif // defined(MGMK_PLATFORM_WINDOWS)
	}

	[[nodiscard]] inline command_line make_directory_command(const std::filesystem::path& path) {
#if defined(MGMK_PLATFORM_WINDOWS)
		return shell_command("if not exist " + shell_path(path) + " mkdir " + shell_path(path));
#else
		return shell_command("mkdir -p " + shell_path(path));
#endif // defined(MGMK_PLATFORM_WINDOWS)
	}

	[[nodiscard]] inline command_line move_command(
		const std::filesystem::path& from,
		const std::filesystem::path& to
	) {
#if defined(MGMK_PLATFORM_WINDOWS)
		return shell_command("move /Y " + shell_path(from) + " " + shell_path(to));
#else
		return shell_command("mv " + shell_path(from) + " " + shell_path(to));
#endif // defined(MGMK_PLATFORM_WINDOWS)
	}

	[[nodiscard]] inline command_line copy_directory_command(
		const std::filesystem::path& from,
		const std::filesystem::path& to
	) {
#if defined(MGMK_PLATFORM_WINDOWS)
		return shell_command("xcopy /E /I /Y " + shell_path(from) + " " + shell_path(to));
#else
		return shell_command("cp -R " + shell_path(from) + " " + shell_path(to));
#endif // defined(MGMK_PLATFORM_WINDOWS)
	}

	[[nodiscard]] inline command_line touch_command(const std::filesystem::path& path) {
#if defined(MGMK_PLATFORM_WINDOWS)
		return shell_command("type nul > " + shell_path(path));
#else
		return shell_command("touch " + shell_path(path));
#endif // defined(MGMK_PLATFORM_WINDOWS)
	}

	[[nodiscard]] inline command_line validate_path_command(
		const std::filesystem::path& path,
		const std::filesystem::path& stamp
	) {
#if defined(MGMK_PLATFORM_WINDOWS)
		return shell_command("if not exist " + shell_path(path) + " exit /B 1 & type nul > " + shell_path(stamp));
#else
		return shell_command("test -e " + shell_path(path) + " && touch " + shell_path(stamp));
#endif // defined(MGMK_PLATFORM_WINDOWS)
	}

	[[nodiscard]] inline command_line reset_directory_stamp_command(
		const std::filesystem::path& directory,
		const std::filesystem::path& stamp
	) {
#if defined(MGMK_PLATFORM_WINDOWS)
		return shell_command("if exist " + shell_path(directory) + " rmdir /S /Q " + shell_path(directory) + " & mkdir " + shell_path(directory) + " & type nul > " + shell_path(stamp));
#else
		return shell_command("rm -rf " + shell_path(directory) + " && mkdir -p " + shell_path(directory) + " && touch " + shell_path(stamp));
#endif // defined(MGMK_PLATFORM_WINDOWS)
	}

	[[nodiscard]] inline command_line normalize_directory_stamp_command(
		const std::filesystem::path& from,
		const std::filesystem::path& to,
		const std::filesystem::path& stamp
	) {
#if defined(MGMK_PLATFORM_WINDOWS)
		return shell_command("if exist " + shell_path(to) + " rmdir /S /Q " + shell_path(to) + " & move /Y " + shell_path(from) + " " + shell_path(to) + " & type nul > " + shell_path(stamp));
#else
		return shell_command("rm -rf " + shell_path(to) + " && mv " + shell_path(from) + " " + shell_path(to) + " && touch " + shell_path(stamp));
#endif // defined(MGMK_PLATFORM_WINDOWS)
	}
}

#endif // MGMAKE_SYS_FILE_COMMAND_HXX
