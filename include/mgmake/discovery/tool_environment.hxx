#pragma once

#ifndef MGMAKE_DISCOVERY_TOOL_ENVIRONMENT_HXX
#define MGMAKE_DISCOVERY_TOOL_ENVIRONMENT_HXX

#include "../sys/util.hxx"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

// Tool environment stores command-prefix setup needed by toolchains such as Visual Studio developer shells.

namespace mgmake::discovery {
#if defined(_WIN32)
	[[nodiscard]] inline std::string cmd_call_quote(std::string_view text) {
		std::string result;
		result += "\"\"";

		for (const char ch : text) {
			if (ch == '"') {
				result += "\"\"";
			} else {
				result += ch;
			}
		}

		result += "\"\"";
		return result;
	}
#endif // defined(_WIN32)

	struct environment_variable {
		std::string m_name{};
		std::string m_value{};
	};

	struct tool_environment {
		std::vector<std::filesystem::path> m_path_prepend{};
		std::vector<std::filesystem::path> m_path_append{};
		std::vector<environment_variable> m_variables{};
		std::optional<std::filesystem::path> m_setup_script{};
		std::vector<std::string> m_setup_args{};
		std::string m_fingerprint{};

		[[nodiscard]] inline bool empty() const noexcept {
			return m_path_prepend.empty()
				&& m_path_append.empty()
				&& m_variables.empty()
				&& !m_setup_script.has_value();
		}
	};

	[[nodiscard]] inline std::string wrap_command_for_environment(
		const tool_environment& env,
		std::string command
	) {
		if (env.empty()) {
			return command;
		}

		if (env.m_setup_script.has_value()) {
			std::string result;
#if defined(_WIN32)
			result += "cmd /s /c \"call ";
			result += cmd_call_quote(env.m_setup_script->string());
#else
			result += ". ";
			result += sys::shell_escape(env.m_setup_script->string());
#endif // defined(_WIN32)
			for (const auto& arg : env.m_setup_args) {
				result += ' ';
				result += sys::shell_escape(arg);
			}

#if defined(_WIN32)
			result += " >nul && ";
			result += command;
			result += "\"";
#else
			result += " && ";
			result += command;
#endif // defined(_WIN32)
			return result;
		}

		std::string prefix;
#if !defined(_WIN32)
		for (const auto& variable : env.m_variables) {
			prefix += variable.m_name;
			prefix += '=';
			prefix += sys::shell_escape(variable.m_value);
			prefix += ' ';
		}
#endif // !defined(_WIN32)
		return prefix + command;
	}
}

#endif // MGMAKE_DISCOVERY_TOOL_ENVIRONMENT_HXX
