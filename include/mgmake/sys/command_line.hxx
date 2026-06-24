#pragma once

#ifndef MGMAKE_SYS_COMMAND_LINE_HXX
#define MGMAKE_SYS_COMMAND_LINE_HXX

#include "../detail/convert.hxx"
#include "util.hxx"

#include <cstdlib>
#include <print>
#include <span>
#include <string_view>
#include <vector>

// command_line stores argv-like tokens and is responsible for producing a shell-safe display/invocation string.

namespace mgmake::sys {
	struct command_run_options {
		bool m_verbose = false;
		bool m_dry_run = false;
	};

	struct command_line {
		std::vector<std::string> m_args;

		inline constexpr std::string_view program_name() const {
			return m_args.empty() ? std::string_view{} : std::string_view(m_args[0]);
		}

		inline constexpr std::span<const std::string> user_args() const {
			if (m_args.size() <= 1) {
				return {};
			}

			return std::span<const std::string>(m_args).subspan(1);
		}

		// full_command() is used both for actual process invocation and for generated backend command text.
		inline constexpr std::string full_command() const {
			std::string result;

            for (std::size_t i = 0; i < m_args.size(); ++i) {
                if (i != 0) {
                    result += ' ';
                }

                result += sys::shell_escape(m_args[i]);
            }

            return result;
		}

		auto invoke(command_run_options opts = {}) const {
			const auto command = full_command();

			if (opts.m_verbose || opts.m_dry_run) {
				std::println("{}", command);
			}

			if (opts.m_dry_run) {
				return 0;
			}

#if defined(MGMK_PLATFORM_WINDOWS)
			std::string shell_command;
			shell_command.reserve(command.size() + 2);
			shell_command += '"';
			shell_command += command;
			shell_command += '"';
			return std::system(shell_command.c_str());
#else
			return std::system(command.c_str());
#endif // defined(MGMK_PLATFORM_WINDOWS)
		}
	};

	inline constexpr command_line args_from_utf8(int argc, const char* const* argv) {
		command_line result;

		if (argc <= 0 || argv == nullptr) {
			return result;
		}

		result.m_args.reserve(static_cast<std::size_t>(argc));

		for (int i = 0; i < argc; ++i) {
			result.m_args.emplace_back(argv[i] ? argv[i] : "");
		}

		return result;
	}

#ifdef MGMK_INCLUDED_WINDOWS
	inline constexpr command_line args_from_wide(int argc, const wchar_t* const* argv) {
		command_line result;

		if (argc <= 0 || argv == nullptr) {
			return result;
		}

		result.m_args.reserve(static_cast<std::size_t>(argc));

		for (int i = 0; i < argc; ++i) {
			if (argv[i] == nullptr) {
				result.m_args.emplace_back();
				continue;
			}

			result.m_args.emplace_back(detail::wide_to_utf8(argv[i]));
		}

		return result;
	}
#endif // MGMK_INCLUDED_WINDOWS
}

#endif // MGMAKE_SYS_COMMAND_LINE_HXX
