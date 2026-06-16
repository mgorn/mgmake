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

namespace mgmake::discovery {
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

		const int exit_code = std::system(shell.c_str());

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

	[[nodiscard]] inline std::expected<resolved_tool, std::string> validate_candidate(
		context&,
		const tool_requirement& req,
		const tool_candidate& candidate
	) {
		if (!is_launchable_file(candidate.m_path)) {
			return std::unexpected{"path is not a launchable file"};
		}

		if (req.m_role == tool_role::generator_ninja) {
			const auto stem = candidate.m_path.stem().string();

			if (stem != "ninja") {
				return std::unexpected{"cached or discovered path is not ninja"};
			}
		}

		std::string version;
#if !defined(_WIN32)

		if (auto probe = probe_version(candidate)) {
			version = *probe;
		}
#endif

		resolved_tool result{};
		result.m_role = candidate.m_role;
		result.m_logical_name = candidate.m_logical_name;
		result.m_path = candidate.m_path;
		result.m_provider = candidate.m_provider;
		result.m_reason = candidate.m_reason;
		result.m_version = version;
		result.m_family = classify_tool_family(req, candidate, version);
		result.m_target_triple = req.m_target_triple;
		return result;
	}
}

#endif
