#pragma once

#ifndef MGMAKE_DISCOVERY_TOOL_PROVIDER_HXX
#define MGMAKE_DISCOVERY_TOOL_PROVIDER_HXX

#include "../detail/enum_string.hxx"

#include <string_view>

namespace mgmake::discovery {
	enum struct tool_provider {
		explicit_path,
		cli_override,
		environment_override,
		cache,
		sibling,
		target_prefix,
		path,
		known_install_root,
		toolchain_root,
		sdk_root,
		sysroot_root,
		project_toolchain_root,
		package_toolchain,
		windows_registry,
		vswhere,
		visual_studio,
		visual_studio_llvm,
		visual_studio_environment,
		windows_sdk,
		standalone_llvm,
		msys2,
		mingw,
		cygwin,
		xcrun,
		xcode,
		homebrew,
		macports,
		unix_system,
		distro_llvm,
		distro_gcc,
		android_ndk,
		emscripten_sdk,
		embedded_sdk,
		compatible_fallback,
		auto_fallback,
		count
	};

	using tool_provider_names = detail::enum_table<
		tool_provider,
		detail::enum_entry<tool_provider::explicit_path, "explicit path">,
		detail::enum_entry<tool_provider::cli_override, "CLI override">,
		detail::enum_entry<tool_provider::environment_override, "environment override">,
		detail::enum_entry<tool_provider::cache, "cache">,
		detail::enum_entry<tool_provider::sibling, "sibling">,
		detail::enum_entry<tool_provider::target_prefix, "target prefix">,
		detail::enum_entry<tool_provider::path, "PATH">,
		detail::enum_entry<tool_provider::known_install_root, "known install root">,
		detail::enum_entry<tool_provider::toolchain_root, "toolchain root">,
		detail::enum_entry<tool_provider::sdk_root, "SDK root">,
		detail::enum_entry<tool_provider::sysroot_root, "sysroot root">,
		detail::enum_entry<tool_provider::project_toolchain_root, "project toolchain root">,
		detail::enum_entry<tool_provider::package_toolchain, "package toolchain">,
		detail::enum_entry<tool_provider::windows_registry, "Windows registry">,
		detail::enum_entry<tool_provider::vswhere, "vswhere">,
		detail::enum_entry<tool_provider::visual_studio, "Visual Studio">,
		detail::enum_entry<tool_provider::visual_studio_llvm, "Visual Studio LLVM">,
		detail::enum_entry<tool_provider::visual_studio_environment, "Visual Studio environment">,
		detail::enum_entry<tool_provider::windows_sdk, "Windows SDK">,
		detail::enum_entry<tool_provider::standalone_llvm, "standalone LLVM">,
		detail::enum_entry<tool_provider::msys2, "MSYS2">,
		detail::enum_entry<tool_provider::mingw, "MinGW">,
		detail::enum_entry<tool_provider::cygwin, "Cygwin">,
		detail::enum_entry<tool_provider::xcrun, "xcrun">,
		detail::enum_entry<tool_provider::xcode, "Xcode">,
		detail::enum_entry<tool_provider::homebrew, "Homebrew">,
		detail::enum_entry<tool_provider::macports, "MacPorts">,
		detail::enum_entry<tool_provider::unix_system, "Unix system">,
		detail::enum_entry<tool_provider::distro_llvm, "distro LLVM">,
		detail::enum_entry<tool_provider::distro_gcc, "distro GCC">,
		detail::enum_entry<tool_provider::android_ndk, "Android NDK">,
		detail::enum_entry<tool_provider::emscripten_sdk, "Emscripten SDK">,
		detail::enum_entry<tool_provider::embedded_sdk, "embedded SDK">,
		detail::enum_entry<tool_provider::compatible_fallback, "compatible fallback">,
		detail::enum_entry<tool_provider::auto_fallback, "auto fallback">
	>;

	static_assert(tool_provider_names::is_zero_based_count_canonical(tool_provider::count));

	[[nodiscard]] inline constexpr std::string_view name(tool_provider provider) noexcept {
		return tool_provider_names::to_string(provider);
	}
}

#endif
