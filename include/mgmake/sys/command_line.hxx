#pragma once

#ifndef MGMAKE_SYS_COMMAND_LINE_HXX
#define MGMAKE_SYS_COMMAND_LINE_HXX

#include "util.hxx"

#include <cstdlib>
#include <span>
#include <string_view>
#include <vector>

namespace mgmake::sys {
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

		inline constexpr std::string full_command() const {
			std::string result;

            for (std::size_t i = 0; i < command.m_args.size(); ++i) {
                if (i != 0) {
                    result += ' ';
                }

                result += sys::shell_escape(command.m_args[i]);
            }

            return result;
		}

		auto invoke() const {
			return std::system(full_command());
		}
	};

	inline constexpr command_line args_from_utf8(int argc, char** argv) {
		command_line result;

		for (int i = 0; i < argc; ++i) {
			result.m_args.emplace_back(argv[i] ? argv[i] : "");
		}

		return result;
	}

#ifdef MGMK_PLATFORM_WINDOWS
	inline constexpr command_line args_from_wide(int argc, wchar_t** argv) {
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
#endif
}

#endif