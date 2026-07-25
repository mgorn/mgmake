#pragma once

#ifndef MGMAKE_FIND_DEFAULT_TOOLS_HXX
#define MGMAKE_FIND_DEFAULT_TOOLS_HXX

#include "tool.hxx"

namespace mgmake::find {
	static constexpr auto cc_tool = tool.logical<"cc">().name<"C Compiler">().env<"MGMK_CC">();
	static constexpr auto cxx_tool = tool.logical<"cxx">().name<"C++ Compiler">().env<"MGMK_CXX">();
	static constexpr auto objc_tool = tool.logical<"objcc">().name<"Objective-C Compiler">().env<"MGMK_OBJCC">();
	static constexpr auto objcxx_tool = tool.logical<"objcxx">().name<"Objective-CXX Compiler">().env<"MGMK_OBJCXX">();
	static constexpr auto assembler_tool = tool.logical<"asm">().name<"Assembler">().env<"MGMK_ASM">();
	static constexpr auto cuda_compiler_tool = tool.logical<"cuda">().name<"CUDA Compiler">();
	static constexpr auto hip_compiler_tool = tool.logical<"hip">().name<"HIP Compiler">();
	static constexpr auto resource_compiler_tool = tool.logical<"rc">().name<"Resource Compiler">().env<"MGMK_RC">();
	static constexpr auto midl_compiler_tool = tool.logical<"midl">().name<"MIDL Compiler">();
	static constexpr auto archiver_tool = tool.logical<"ar">().name<"Archiver">().env<"MGMK_AR">();
	static constexpr auto ranlib_tool = tool.logical<"ranlib">().name<"Ranlib">().env<"MGMK_RANLIB">();
	static constexpr auto librarian_tool = tool.logical<"lib">().name<"Librarian">().env<"MGMK_LIB">();
	static constexpr auto linker_tool = tool.logical<"linker">().name<"Linker">().env<"MGMK_LINKER">();
	static constexpr auto shared_linker_tool = tool.logical<"shared-linker">().name<"Shared Linker">().env<"MGMK_SHARED_LINKER">();
	static constexpr auto dll_tool = tool.logical<"dlltool">().name<"DLL Tool">().env<"MGMK_DLLTOOL">();
	static constexpr auto manifest_tool = tool.logical<"mt">().name<"Manifest Tool">().env<"MGMK_MT">();
	static constexpr auto strip_tool = tool.logical<"strip">().name<"Strip">().env<"MGMK_STRIP">();
	static constexpr auto objcopy_tool = tool.logical<"objcopy">().name<"Objcopy">().env<"MGMK_OBJCOPY">();
	static constexpr auto objdump_tool = tool.logical<"objdump">().name<"Objdump">().env<"MGMK_OBJDUMP">();
	static constexpr auto nm_tool = tool.logical<"nm">().name<"NM">().env<"MGMK_NM">();
	static constexpr auto readelf_tool = tool.logical<"readelf">().name<"Readelf">().env<"MGMK_READELF">();
	static constexpr auto debug_symbol_tool = tool.logical<"debug-symbol-tool">().name<"Debug Symbol Tool">();
	static constexpr auto lipo_tool = tool.logical<"lipo">().name<"Lipo">();
	static constexpr auto install_name_tool = tool.logical<"install-name-tool">().name<"Install Name Tool">();
	static constexpr auto codesign_tool = tool.logical<"codesign">().name<"Codesign">();
	static constexpr auto ninja_tool = tool.logical<"ninja">().name<"Ninja">().env<"MGMK_NINJA">();
	static constexpr auto make_tool = tool.logical<"make">().name<"Make">();
	static constexpr auto msbuild_tool = tool.logical<"msbuild">().name<"MSBuild">();
	static constexpr auto xcodebuild_tool = tool.logical<"xcodebuild">().name<"Xcodebuild">();
	static constexpr auto cmake_tool = tool.logical<"cmake">().name<"CMake">().env<"MGMK_CMAKE">();
	static constexpr auto pkg_config_tool = tool.logical<"pkg-config">().name<"pkg-config">().env<"MGMK_PKG_CONFIG">();
	static constexpr auto git_tool = tool.logical<"git">().name<"Git">().env<"MGMK_GIT">();
	static constexpr auto curl_tool = tool.logical<"curl">().name<"Curl">().env<"MGMK_CURL">();
	static constexpr auto wget_tool = tool.logical<"wget">().name<"Wget">().env<"MGMK_WGET">();
	static constexpr auto unzip_tool = tool.logical<"unzip">().name<"Unzip">().env<"MGMK_UNZIP">();
	static constexpr auto tar_tool = tool.logical<"tar">().name<"Tar">().env<"MGMK_TAR">();
	static constexpr auto exe_wrapper_tool = tool.logical<"exe-wrapper">().name<"Executable Wrapper">().env<"MGMK_EXE_WRAPPER">();
	static constexpr auto emulator_tool = tool.logical<"emulator">().name<"Emulator">().env<"MGMK_EMULATOR">();

	using default_tools = meta::value_list<
		cc_tool,
		cxx_tool,
		objc_tool,
		objcxx_tool,
		assembler_tool,
		cuda_compiler_tool,
		hip_compiler_tool,
		resource_compiler_tool,
		midl_compiler_tool,
		archiver_tool,
		ranlib_tool,
		librarian_tool,
		linker_tool,
		shared_linker_tool,
		dll_tool,
		manifest_tool,
		strip_tool,
		objcopy_tool,
		objdump_tool,
		nm_tool,
		readelf_tool,
		debug_symbol_tool,
		lipo_tool,
		install_name_tool,
		codesign_tool,
		ninja_tool,
		make_tool,
		msbuild_tool,
		xcodebuild_tool,
		cmake_tool,
		pkg_config_tool,
		git_tool,
		curl_tool,
		wget_tool,
		unzip_tool,
		tar_tool,
		exe_wrapper_tool,
		emulator_tool
	>;
}

#endif // MGMAKE_FIND_DEFAULT_TOOLS_HXX
