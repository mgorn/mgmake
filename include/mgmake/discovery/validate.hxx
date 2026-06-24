#pragma once

#ifndef MGMAKE_DISCOVERY_VALIDATE_HXX
#define MGMAKE_DISCOVERY_VALIDATE_HXX

#include "../sys/command_line.hxx"
#include "../sys/util.hxx"
#include "context.hxx"
#include "filesystem.hxx"

#include <chrono>
#include <expected>
#include <filesystem>
#include <fstream>
#include <string>

// Candidate validation checks executable existence and gathers version/details before accepting a tool.

namespace mgmake::discovery {
	[[nodiscard]] inline bool is_ninja_name(std::string_view stem) noexcept {
		return stem == "ninja" || stem == "ninja-build";
	}

	[[nodiscard]] inline std::expected<std::string, std::string> capture_command(
		sys::command_line command
	) {
		const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
		auto output_path = std::filesystem::temp_directory_path()
			/ ("mgmake-tool-probe-" + std::to_string(now) + ".txt");

		std::string shell = command.full_command();
		shell += " > ";
		shell += sys::shell_escape(output_path.string());
		shell += " 2>&1";

#if defined(MGMK_PLATFORM_WINDOWS)
		std::string shell_command;
		shell_command.reserve(shell.size() + 2);
		shell_command += '"';
		shell_command += shell;
		shell_command += '"';
		const int exit_code = std::system(shell_command.c_str());
#else
		const int exit_code = std::system(shell.c_str());
#endif // defined(MGMK_PLATFORM_WINDOWS)

		std::ifstream in(output_path);
		std::string text;

		if (in.is_open()) {
			text.assign(
				std::istreambuf_iterator<char>{in},
				std::istreambuf_iterator<char>{}
			);
		}

		std::error_code ec;
		std::filesystem::remove(output_path, ec);

		if (exit_code != 0 && text.empty()) {
			return std::unexpected{"version probe failed"};
		}

		return text;
	}

	[[nodiscard]] inline std::expected<std::string, std::string> probe_version(
		const tool_candidate& candidate
	) {
		for (std::string flag : {"--version", "-v", "/?"}) {
			sys::command_line command;
			command.m_args.emplace_back(candidate.m_path.string());
			command.m_args.emplace_back(std::move(flag));

			auto result = capture_command(std::move(command));

			if (result && !result->empty()) {
				return result;
			}
		}

		return std::string{};
	}

	[[nodiscard]] inline tool_family classify_tool_family(
		const tool_requirement& req,
		const tool_candidate& candidate,
		std::string_view version_output
	) {
		const auto filename = candidate.m_path.filename().string();
		const auto text = std::string{version_output};

		if (filename.find("clang-cl") != std::string::npos) return tool_family::clang_cl;
		if (filename == "cl.exe" || filename == "cl") return tool_family::msvc;
		if (filename.find("llvm-") != std::string::npos) return tool_family::llvm_binutils;
		if (filename == "lib.exe" || filename == "lib") return tool_family::msvc_binutils;
		if (text.find("Apple clang") != std::string::npos) return tool_family::apple_clang;
		if (text.find("clang") != std::string::npos || filename.find("clang") != std::string::npos) return tool_family::clang;
		if (text.find("gcc") != std::string::npos || filename.find("gcc") != std::string::npos || filename.find("g++") != std::string::npos) return tool_family::gcc;
		if (filename.find("windres") != std::string::npos || filename.find("mingw") != std::string::npos) return tool_family::mingw;

		return req.m_expected_family;
	}

	[[nodiscard]] inline bool is_binutils_role(tool_role role) noexcept {
		switch (role) {
			case tool_role::archiver:
			case tool_role::ranlib:
			case tool_role::strip:
			case tool_role::objcopy:
			case tool_role::objdump:
			case tool_role::nm:
			case tool_role::readelf:
				return true;

			default:
				return false;
		}
	}

	[[nodiscard]] inline bool is_compatible_family(
		const build::toolchain& tc,
		tool_role role,
		tool_family family
	) {
		if (family == tool_family::unknown) {
			return true;
		}

		const auto tc_name = tc.name();

		if (tc_name.find("clang") != std::string_view::npos) {
			if (is_binutils_role(role)) {
				return family == tool_family::llvm_binutils
					|| family == tool_family::gnu_binutils
					|| family == tool_family::mingw;
			}

			return family == tool_family::clang
				|| family == tool_family::apple_clang
				|| family == tool_family::clang_cl
				|| family == tool_family::llvm_binutils
				|| family == tool_family::mingw;
		}

		if (tc.dialect() == build::toolchain::dialect::msvc) {
			return family == tool_family::msvc
				|| family == tool_family::clang_cl
				|| family == tool_family::msvc_binutils
				|| family == tool_family::llvm_binutils;
		}

		if (tc_name.find("gcc") != std::string_view::npos
			|| tc.dialect() == build::toolchain::dialect::gcc) {
			if (is_binutils_role(role)) {
				return family == tool_family::gnu_binutils
					|| family == tool_family::llvm_binutils
					|| family == tool_family::mingw;
			}

			return family == tool_family::gcc
				|| family == tool_family::clang
				|| family == tool_family::apple_clang
				|| family == tool_family::mingw;
		}

		return true;
	}

	[[nodiscard]] inline std::expected<resolved_tool, std::string> validate_candidate(
		context& ctx,
		const tool_requirement& req,
		const tool_candidate& candidate
	) {
		if (!is_launchable_file(candidate.m_path)) {
			return std::unexpected{"path is not a launchable file"};
		}

		if (req.m_role == tool_role::generator_ninja) {
			const auto stem = candidate.m_path.stem().string();

			if (!is_ninja_name(stem)) {
				auto version_probe = probe_version(candidate);

				if (!version_probe || version_probe->empty()) {
					return std::unexpected{"candidate is not a recognized Ninja executable"};
				}
			}
		}

		std::string version;
#if !defined(_WIN32)

		if (auto probe = probe_version(candidate)) {
			version = *probe;
		}
#endif // !defined(_WIN32)

		resolved_tool result{};
		result.m_role = candidate.m_role;
		result.m_logical_name = candidate.m_logical_name;
		result.m_path = candidate.m_path;
		result.m_provider = candidate.m_provider;
		result.m_reason = candidate.m_reason;
		result.m_version = version;
		result.m_family = classify_tool_family(req, candidate, version);
		result.m_target_triple = req.m_target_triple;
		result.m_provider_root = candidate.m_provider_root;

		if (ctx.m_mode == mode::family_fallback
			&& !is_compatible_family(ctx.toolchain(), req.m_role, result.m_family)) {
			return std::unexpected{"candidate belongs to an incompatible tool family"};
		}

		return result;
	}
}

#endif // MGMAKE_DISCOVERY_VALIDATE_HXX
