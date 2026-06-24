#pragma once

#ifndef MGMAKE_DISCOVERY_DIAGNOSTIC_HXX
#define MGMAKE_DISCOVERY_DIAGNOSTIC_HXX

#include "tool_requirement.hxx"
#include "tool_role.hxx"

#include <string>
#include <vector>

// Discovery diagnostics collect searched candidates, rejection reasons, and suggested fixes for missing tools.

namespace mgmake::discovery {
	struct diagnostic {
		tool_role m_role{};
		std::string m_toolchain{};
		std::string m_logical_name{};
		std::string m_needed_because{};
		std::vector<std::string> m_searched{};
		std::vector<std::string> m_rejected{};
		std::vector<std::string> m_notes{};
		std::vector<std::string> m_fixes{};

		[[nodiscard]] std::string format_missing_tool() const {
			std::string result;
			result += "mgmake: required tool not found\n\n";
			result += "toolchain:\n  " + m_toolchain + "\n\n";
			result += "required tool:\n  " + std::string{name(m_role)} + "\n\n";

			if (!m_logical_name.empty()) {
				result += "logical name:\n  " + m_logical_name + "\n\n";
			}

			if (!m_needed_because.empty()) {
				result += "needed because:\n  " + m_needed_because + "\n\n";
			}

			if (!m_searched.empty()) {
				result += "searched:\n";
				for (const auto& item : m_searched) {
					result += "  " + item + "\n";
				}
				result += "\n";
			}

			if (!m_rejected.empty()) {
				result += "rejected:\n";
				for (const auto& item : m_rejected) {
					result += "  " + item + "\n";
				}
				result += "\n";
			}

			if (!m_notes.empty()) {
				result += "notes:\n";
				for (const auto& item : m_notes) {
					result += "  " + item + "\n";
				}
				result += "\n";
			}

			if (!m_fixes.empty()) {
				result += "fixes:\n";
				for (const auto& item : m_fixes) {
					result += "  " + item + "\n";
				}
			}

			return result;
		}
	};

	[[nodiscard]] inline std::vector<std::string> fixes_for(const tool_requirement& req) {
		switch (req.m_role) {
			case tool_role::archiver:
			case tool_role::ranlib:
				return {
					"install LLVM/GNU binutils for the selected toolchain",
					"set the matching MGMK_* environment variable",
					"pass an explicit --ar or --ranlib path",
					"add the tool directory to PATH",
					"run './build tools --refresh'"
				};

			case tool_role::generator_ninja:
				return {
					"install Ninja",
					"set MGMK_NINJA",
					"pass --ninja <path>",
					"add Ninja to PATH",
					"run './build tools --refresh'"
				};

			default:
				return {
					"set the matching MGMK_* environment variable",
					"pass an explicit tool override",
					"add the tool directory to PATH",
					"run './build tools --refresh'"
				};
		}
	}
}

#endif // MGMAKE_DISCOVERY_DIAGNOSTIC_HXX
