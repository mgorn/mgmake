#pragma once

#ifndef MGMAKE_DISCOVERY_RESOLVED_TOOLCHAIN_HXX
#define MGMAKE_DISCOVERY_RESOLVED_TOOLCHAIN_HXX

#include "../sys/platform.hxx"
#include "mode.hxx"
#include "resolved_tool.hxx"
#include "tool_environment.hxx"

#include <string>
#include <vector>
#include <initializer_list>

namespace mgmake::discovery {
	struct resolved_toolchain {
		std::string m_name{};
		std::string m_requested_name{};
		mode m_mode = mode::exact;

		sys::target m_host = sys::g_host_target;
		sys::target m_target = sys::g_host_target;

		std::vector<resolved_tool> m_tools{};
		tool_environment m_environment{};

		std::vector<searched_tool_candidate> m_searched{};
		std::vector<rejected_tool_candidate> m_rejected{};
		std::vector<std::string> m_notes{};

		std::vector<std::string> m_compile_prefix_args{};
		std::vector<std::string> m_link_prefix_args{};

		[[nodiscard]] inline const resolved_tool* find(
			tool_role role
		) const noexcept {
			for (const auto& tool : m_tools) {
				if (tool.m_role == role) {
					return &tool;
				}
			}

			return nullptr;
		}

		[[nodiscard]] inline const resolved_tool* find_any(
			std::initializer_list<tool_role> roles
		) const noexcept {
			for (const auto role : roles) {
				if (const auto* tool = find(role)) {
					return tool;
				}
			}

			return nullptr;
		}
	};
}

#endif // MGMAKE_DISCOVERY_RESOLVED_TOOLCHAIN_HXX
