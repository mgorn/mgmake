#pragma once

#ifndef MGMAKE_DISCOVERY_TOOL_ROLE_HXX
#define MGMAKE_DISCOVERY_TOOL_ROLE_HXX

#include "../detail/enum_string.hxx"

#include <string_view>

namespace mgmake::discovery {
	enum struct tool_role {
		c_compiler,
		cxx_compiler,
		objc_compiler,
		objcxx_compiler,
		assembler,
		cuda_compiler,
		hip_compiler,
		resource_compiler,
		midl_compiler,
		archiver,
		ranlib,
		librarian,
		linker,
		shared_linker,
		dll_tool,
		manifest_tool,
		strip,
		objcopy,
		objdump,
		nm,
		readelf,
		debug_symbol_tool,
		lipo,
		install_name_tool,
		codesign,
		generator_ninja,
		generator_make,
		generator_msbuild,
		generator_xcode,
		cmake,
		pkg_config,
		git,
		curl,
		wget,
		unzip,
		tar,
		exe_wrapper,
		emulator,
		count
	};

	using tool_role_names = detail::enum_table<
		tool_role,
		detail::enum_entry<tool_role::c_compiler, "c compiler">,
		detail::enum_entry<tool_role::cxx_compiler, "c++ compiler">,
		detail::enum_entry<tool_role::objc_compiler, "objective-c compiler">,
		detail::enum_entry<tool_role::objcxx_compiler, "objective-c++ compiler">,
		detail::enum_entry<tool_role::assembler, "assembler">,
		detail::enum_entry<tool_role::cuda_compiler, "cuda compiler">,
		detail::enum_entry<tool_role::hip_compiler, "hip compiler">,
		detail::enum_entry<tool_role::resource_compiler, "resource compiler">,
		detail::enum_entry<tool_role::midl_compiler, "midl compiler">,
		detail::enum_entry<tool_role::archiver, "archiver">,
		detail::enum_entry<tool_role::ranlib, "ranlib">,
		detail::enum_entry<tool_role::librarian, "librarian">,
		detail::enum_entry<tool_role::linker, "linker">,
		detail::enum_entry<tool_role::shared_linker, "shared linker">,
		detail::enum_entry<tool_role::dll_tool, "dll tool">,
		detail::enum_entry<tool_role::manifest_tool, "manifest tool">,
		detail::enum_entry<tool_role::strip, "strip">,
		detail::enum_entry<tool_role::objcopy, "objcopy">,
		detail::enum_entry<tool_role::objdump, "objdump">,
		detail::enum_entry<tool_role::nm, "nm">,
		detail::enum_entry<tool_role::readelf, "readelf">,
		detail::enum_entry<tool_role::debug_symbol_tool, "debug symbol tool">,
		detail::enum_entry<tool_role::lipo, "lipo">,
		detail::enum_entry<tool_role::install_name_tool, "install name tool">,
		detail::enum_entry<tool_role::codesign, "codesign">,
		detail::enum_entry<tool_role::generator_ninja, "ninja">,
		detail::enum_entry<tool_role::generator_make, "make">,
		detail::enum_entry<tool_role::generator_msbuild, "msbuild">,
		detail::enum_entry<tool_role::generator_xcode, "xcodebuild">,
		detail::enum_entry<tool_role::cmake, "cmake">,
		detail::enum_entry<tool_role::pkg_config, "pkg-config">,
		detail::enum_entry<tool_role::git, "git">,
		detail::enum_entry<tool_role::curl, "curl">,
		detail::enum_entry<tool_role::wget, "wget">,
		detail::enum_entry<tool_role::unzip, "unzip">,
		detail::enum_entry<tool_role::tar, "tar">,
		detail::enum_entry<tool_role::exe_wrapper, "exe wrapper">,
		detail::enum_entry<tool_role::emulator, "emulator">
	>;

	static_assert(tool_role_names::is_zero_based_count_canonical(tool_role::count));

	[[nodiscard]] inline constexpr std::string_view name(tool_role role) noexcept {
		return tool_role_names::to_string(role);
	}
}

#endif // MGMAKE_DISCOVERY_TOOL_ROLE_HXX
