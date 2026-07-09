#pragma once

#ifndef MGMAKE_CLI_PARSE_HXX
#define MGMAKE_CLI_PARSE_HXX

#include "option_parser.hxx"
#include "../sys/command_line.hxx"

// Top-level CLI parsing wires action aliases, options, passthrough handling, and positional targets together.

namespace mgmake::cli {
	inline void apply_help(options& opts) {
		opts.m_action = action_kind::help;
		opts.m_show_help = true;
	}

	inline void apply_version(options& opts) {
		opts.m_action = action_kind::version;
		opts.m_show_version = true;
	}

	using help_option = callback_option<"help", 'h', apply_help>
		::description<"Show help.">;

	using version_option = callback_option<"version", '\0', apply_version>
		::description<"Show version information.">;

	using verbose_option = flag_option<&options::m_verbose, "verbose", 'v'>
		::description<"Print commands before executing them.">;

	using dry_run_option = flag_option<&options::m_dry_run, "dry-run">
		::description<"Print commands without executing them.">;

	using backend_option = value_option<&options::m_backend, "backend">
		::value_name<"name">
		::description<"Select a build backend to use.">;

	using toolchain_option = value_option<&options::m_toolchain, "toolchain">
		::value_name<"name">
		::description<"Select a compiler toolchain preset.">;

	using build_dir_option = value_option<&options::m_build_dir, "build-dir">
		::value_name<"path">
		::description<"Set the build directory.">;

	using jobs_option =
		value_option<&options::m_jobs, "jobs", 'j'>
			::value_name<"count">
			::description<"Set the maximum number of parallel jobs.">;

	using target_option =
		append_option<&options::m_targets, "target">
			::value_name<"name">
			::description<"Build a specific target. May be passed multiple times.">;

	using platform_option =
		value_option<&options::m_target_platform, "platform">
			::value_name<"platform">
			::description<"Set the target platform used for artifact naming.">;

	using arch_option =
		value_option<&options::m_target_arch, "arch">
			::value_name<"arch">
			::description<"Set the target architecture.">;

	using abi_option =
		value_option<&options::m_target_abi, "abi">
			::value_name<"abi">
			::description<"Set the target ABI.">;

	using target_triple_option =
		value_option<&options::m_target_triple, "target-triple">
			::value_name<"triple">
			::description<"Set an explicit compiler target triple for target-aware toolchains.">;

	using refresh_tools_option =
		flag_option<&options::m_refresh_tools, "refresh-tools">
			::description<"Ignore cached tool paths and rediscover tools.">;

	using no_tool_cache_option =
		flag_option<&options::m_no_tool_cache, "no-tool-cache">
			::description<"Do not read or write the tool discovery cache.">;

	using show_tool_search_option =
		flag_option<&options::m_show_tool_search, "show-tool-search">
			::description<"Print detailed tool discovery search diagnostics.">;

	using discover_source_dependencies_option =
		flag_option<&options::m_discover_source_dependencies, "discover-source-dependencies">
			::description<"Discover source dependencies (typically included headers) from depfiles and populate them as artifacts in the build graph.">;

	using tool_discovery_option =
		value_option<&options::m_tool_discovery, "tool-discovery">
			::value_name<"mode">
			::description<"Set tool discovery mode: automatic, exact, family-fallback, auto-fallback, disabled.">;

	using toolchain_root_option =
		value_option<&options::m_toolchain_root, "toolchain-root">
			::value_name<"path">
			::description<"Add a toolchain root to tool discovery.">;

	using sdk_root_option =
		value_option<&options::m_sdk_root, "sdk">
			::value_name<"path">
			::description<"Set an SDK root for tool discovery.">;

	using sysroot_option =
		value_option<&options::m_sysroot, "sysroot">
			::value_name<"path">
			::description<"Set a target sysroot.">;

	using package_toolchain_root_option =
		value_option<&options::m_package_toolchain_root, "package-toolchain-root">
			::value_name<"path">
			::description<"Add a package-provided toolchain root.">;

	using cc_option =
		value_option<&options::m_cc, "cc">
			::value_name<"path-or-name">
			::description<"Override the C compiler.">;

	using cxx_option =
		value_option<&options::m_cxx, "cxx">
			::value_name<"path-or-name">
			::description<"Override the C++ compiler.">;

	using ar_option =
		value_option<&options::m_ar, "ar">
			::value_name<"path-or-name">
			::description<"Override the archiver.">;

	using ranlib_option =
		value_option<&options::m_ranlib, "ranlib">
			::value_name<"path-or-name">
			::description<"Override ranlib.">;

	using librarian_option =
		value_option<&options::m_librarian, "librarian">
			::value_name<"path-or-name">
			::description<"Override the librarian.">;

	using linker_option =
		value_option<&options::m_linker, "linker">
			::value_name<"path-or-name">
			::description<"Override the linker.">;

	using shared_linker_option =
		value_option<&options::m_shared_linker, "shared-linker">
			::value_name<"path-or-name">
			::description<"Override the shared-library linker.">;

	using rc_option =
		value_option<&options::m_rc, "rc">
			::value_name<"path-or-name">
			::description<"Override the resource compiler.">;

	using mt_option =
		value_option<&options::m_mt, "mt">
			::value_name<"path-or-name">
			::description<"Override the manifest tool.">;

	using dll_tool_option =
		value_option<&options::m_dll_tool, "dll-tool">
			::value_name<"path-or-name">
			::description<"Override dlltool.">;

	using strip_option =
		value_option<&options::m_strip, "strip">
			::value_name<"path-or-name">
			::description<"Override strip.">;

	using objcopy_option =
		value_option<&options::m_objcopy, "objcopy">
			::value_name<"path-or-name">
			::description<"Override objcopy.">;

	using objdump_option =
		value_option<&options::m_objdump, "objdump">
			::value_name<"path-or-name">
			::description<"Override objdump.">;

	using nm_option =
		value_option<&options::m_nm, "nm">
			::value_name<"path-or-name">
			::description<"Override nm.">;

	using readelf_option =
		value_option<&options::m_readelf, "readelf">
			::value_name<"path-or-name">
			::description<"Override readelf.">;

	using ninja_option =
		value_option<&options::m_ninja, "ninja">
			::value_name<"path-or-name">
			::description<"Override the Ninja executable.">;

	using cmake_option =
		value_option<&options::m_cmake, "cmake">
			::value_name<"path-or-name">
			::description<"Override CMake.">;

	using pkg_config_option =
		value_option<&options::m_pkg_config, "pkg-config">
			::value_name<"path-or-name">
			::description<"Override pkg-config.">;

	using exe_wrapper_option =
		value_option<&options::m_exe_wrapper, "exe-wrapper">
			::value_name<"path-or-name">
			::description<"Override the executable wrapper.">;

	using emulator_option =
		value_option<&options::m_emulator, "emulator">
			::value_name<"path-or-name">
			::description<"Override the emulator.">;

	using toolchain_version_option =
		value_option<&options::m_toolchain_version, "toolchain-version">
			::value_name<"version">
			::description<"Request a specific toolchain version.">;

	using android_ndk_option =
		value_option<&options::m_android_ndk, "android-ndk">
			::value_name<"path">
			::description<"Set the Android NDK root.">;

	using android_abi_option =
		value_option<&options::m_android_abi, "android-abi">
			::value_name<"abi">
			::description<"Set the Android ABI.">;

	using android_api_option =
		value_option<&options::m_android_api, "android-api">
			::value_name<"level">
			::description<"Set the Android API level.">;

	using apple_sdk_option =
		value_option<&options::m_apple_sdk, "apple-sdk">
			::value_name<"sdk">
			::description<"Set the Apple SDK name for xcrun discovery.">;

	using default_parser = option_parser<
		help_option,
		version_option,
		verbose_option,
		dry_run_option,
		backend_option,
		toolchain_option,
		build_dir_option,
		jobs_option,
		target_option,
		platform_option,
		arch_option,
		abi_option,
		target_triple_option,
		refresh_tools_option,
		no_tool_cache_option,
		show_tool_search_option,
		discover_source_dependencies_option,
		tool_discovery_option,
		toolchain_root_option,
		sdk_root_option,
		sysroot_option,
		package_toolchain_root_option,
		cc_option,
		cxx_option,
		ar_option,
		ranlib_option,
		librarian_option,
		linker_option,
		shared_linker_option,
		rc_option,
		mt_option,
		dll_tool_option,
		strip_option,
		objcopy_option,
		objdump_option,
		nm_option,
		readelf_option,
		ninja_option,
		cmake_option,
		pkg_config_option,
		exe_wrapper_option,
		emulator_option,
		toolchain_version_option,
		android_ndk_option,
		android_abi_option,
		android_api_option,
		apple_sdk_option
	>;

	[[nodiscard]] inline std::expected<cli::options,std::string> parse(std::span<const std::string> args) {
		return default_parser::parse(args);
	}

	inline auto parse(const sys::command_line& cmd) {
		return parse(cmd.user_args());
	}

	inline void print_help(std::string_view program_name) {
		default_parser::print_help(program_name);
	}
}

#endif // MGMAKE_CLI_PARSE_HXX
