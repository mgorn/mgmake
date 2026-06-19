#pragma once

#ifndef MGMAKE_BUILD_CLEAN_HXX
#define MGMAKE_BUILD_CLEAN_HXX

#include "request.hxx"
#include "../sys/command_line.hxx"

#include <expected>
#include <print>
#include <filesystem>
#include <string>

namespace mgmake::build {
	inline void print_clean_command(const build::request& req) {
#if defined(MGMK_PLATFORM_WINDOWS)
		std::println("rmdir /s /q {}", sys::shell_escape(req.build_dir().string()));
#else
		std::println("rm -rf {}", sys::shell_escape(req.build_dir().string()));
#endif
	}

	[[nodiscard]] inline std::expected<void, std::string> clean(
		const build::request& req,
		sys::command_run_options run_options = {}
	) {
		if (run_options.m_verbose || run_options.m_dry_run) {
			print_clean_command(req);
		}

		if (run_options.m_dry_run) {
			return {};
		}

		std::error_code ec;
		std::filesystem::remove_all(req.build_dir(), ec);

		if (ec) {
			return std::unexpected{
				"mgmake: failed to clean build directory '" +
				req.build_dir().string() +
				"': " +
				ec.message()
			};
		}

		return {};
	}
}

#endif
