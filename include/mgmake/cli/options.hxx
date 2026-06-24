#pragma once

#ifndef MGMAKE_CLI_OPTIONS_HXX
#define MGMAKE_CLI_OPTIONS_HXX

#include "action.hxx"
#include "backend.hxx"
#include "../discovery/mode.hxx"
#include "../sys/platform.hxx"

#include <string>
#include <vector>

// cli::options is the raw parsed user intent; build::request is the normalized form used by build phases.

namespace mgmake::cli {
	struct options {
		action_kind m_action = action_kind::build;
		backend_kind m_backend = backend_kind::automatic;

		std::string m_toolchain = "clang-mg";
		std::string m_build_dir = ".build";

		std::vector<std::string> m_targets;
		std::vector<std::string> m_passthrough_args;
		std::vector<std::string> m_run_args;

		int m_jobs = 0;

		sys::arch m_target_arch = sys::g_host_arch;
		sys::platform m_target_platform = sys::g_host_platform;
		sys::abi m_target_abi = sys::g_host_abi;
		std::string m_target_triple{};

		bool m_verbose = false;
		bool m_dry_run = false;
		bool m_show_help = false;
		bool m_show_version = false;
		bool m_refresh_tools = false;
		bool m_no_tool_cache = false;
		bool m_show_tool_search = false;

		discovery::mode m_tool_discovery = discovery::mode::automatic;

		std::string m_toolchain_root{};
		std::string m_sdk_root{};
		std::string m_sysroot{};
		std::string m_package_toolchain_root{};

		std::string m_cc{};
		std::string m_cxx{};
		std::string m_ar{};
		std::string m_ranlib{};
		std::string m_librarian{};
		std::string m_linker{};
		std::string m_shared_linker{};
		std::string m_rc{};
		std::string m_mt{};
		std::string m_dll_tool{};
		std::string m_strip{};
		std::string m_objcopy{};
		std::string m_objdump{};
		std::string m_nm{};
		std::string m_readelf{};
		std::string m_ninja{};
		std::string m_cmake{};
		std::string m_pkg_config{};
		std::string m_exe_wrapper{};
		std::string m_emulator{};

		std::string m_toolchain_version{};

		std::string m_android_ndk{};
		std::string m_android_abi{};
		int m_android_api = 0;

		std::string m_apple_sdk{};

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

#endif // MGMAKE_CLI_OPTIONS_HXX
