#pragma once

#ifndef MGMAKE_CLI_OPTIONS_HXX
#define MGMAKE_CLI_OPTIONS_HXX

#include "action.hxx"
#include "backend.hxx"
#include "../sys/platform.hxx"

#include <string>
#include <vector>

namespace mgmake::cli {
	struct options {
		action_kind m_action = action_kind::build;
		backend_kind m_backend = backend_kind::automatic;

		std::string m_build_dir = ".build";

		std::vector<std::string> m_targets;
		std::vector<std::string> m_passthrough_args;

		int m_jobs = 0;

		sys::arch m_target_arch = sys::g_host_arch;
		sys::platform m_target_platform = sys::g_host_platform;
		sys::abi m_target_abi = sys::g_host_abi;
		std::string m_target_triple{};

		bool m_verbose = false;
		bool m_dry_run = false;
		bool m_show_help = false;
		bool m_show_version = false;

		inline constexpr const std::vector<std::string>& targets() const {
			return m_targets;
		}
		
		[[nodiscard]] inline sys::target target_platform() const {
			return sys::target{
				m_target_arch,
				m_target_platform,
				m_target_abi,
				m_target_triple
			};
		}
	};
}

#endif
