#pragma once

#ifndef MGMAKE_TOOL_DEFAULT_TOOLS_HXX
#define MGMAKE_TOOL_DEFAULT_TOOLS_HXX

#include "tool.hxx"

namespace mgmake::tool {
	static constexpr auto cc_tool = tool
		.cli<"cc">()
		.name<"C Compiler">()
		.exe<"cc">()
		.env<"MGMK_CC">();

	static constexpr auto cxx_tool = tool
		.cli<"cxx">()
		.name<"C++ Compiler">()
		.exe<"c++">()
		.env<"MGMK_CXX">();

	static constexpr auto objc_tool = tool
		.cli<"objcc">()
		.name<"Objective-C Compiler">()
		.exe<"cc">()
		.env<"MGMK_OBJCC">();

	static constexpr auto objcxx_tool = tool
		.cli<"objcxx">()
		.name<"Objective-C++ Compiler">()
		.exe<"c++">()
		.env<"MGMK_OBJCXX">();

	static constexpr auto assembler_tool = tool
		.cli<"asm">()
		.name<"Assembler">()
		.exe<"as">()
		.env<"MGMK_ASM">();

	static constexpr auto cuda_compiler_tool = tool
		.cli<"cuda">()
		.name<"CUDA Compiler">()
		.exe<"nvcc">()
		.env<"MGMK_NVCC">();

	static constexpr auto hip_compiler_tool = tool
		.cli<"hip">()
		.name<"HIP Compiler">()
		.exe<"hipcc">()
		.env<"MGMK_HIPCC">();

	static constexpr auto resource_compiler_tool = tool
		.cli<"rc">()
		.name<"Resource Compiler">()
		.exe<"rc">()
		.env<"MGMK_RC">();

	static constexpr auto midl_compiler_tool = tool
		.cli<"midl">()
		.name<"MIDL Compiler">()
		.exe<"midl">()
		.env<"MGMK_MIDL">();

	static constexpr auto archiver_tool = tool
		.cli<"ar">()
		.name<"Archiver">()
		.exe<"ar">()
		.env<"MGMK_AR">();

	static constexpr auto ranlib_tool = tool
		.cli<"ranlib">()
		.name<"Ranlib">()
		.exe<"ranlib">()
		.env<"MGMK_RANLIB">();

	static constexpr auto librarian_tool = tool
		.cli<"lib">()
		.name<"Librarian">()
		.exe<"lib">()
		.env<"MGMK_LIB">();

	static constexpr auto linker_tool = tool
		.cli<"linker">()
		.name<"Linker">()
		.exe<"ld">()
		.env<"MGMK_LINKER">();

	static constexpr auto shared_linker_tool = tool
		.cli<"shared-linker">()
		.name<"Shared Linker">()
		.exe<"ld">()
		.env<"MGMK_SHARED_LINKER">();

	static constexpr auto dll_tool = tool
		.cli<"dlltool">()
		.name<"DLL Tool">()
		.exe<"dlltool">()
		.env<"MGMK_DLLTOOL">();

	static constexpr auto manifest_tool = tool
		.cli<"mt">()
		.name<"Manifest Tool">()
		.exe<"mt">()
		.env<"MGMK_MT">();

	static constexpr auto strip_tool = tool
		.cli<"strip">()
		.name<"Strip">()
		.exe<"strip">()
		.env<"MGMK_STRIP">();

	static constexpr auto objcopy_tool = tool
		.cli<"objcopy">()
		.name<"Objcopy">()
		.exe<"objcopy">()
		.env<"MGMK_OBJCOPY">();

	static constexpr auto objdump_tool = tool
		.cli<"objdump">()
		.name<"Objdump">()
		.exe<"objdump">()
		.env<"MGMK_OBJDUMP">();

	static constexpr auto nm_tool = tool
		.cli<"nm">()
		.name<"NM">()
		.exe<"nm">()
		.env<"MGMK_NM">();

	static constexpr auto readelf_tool = tool
		.cli<"readelf">()
		.name<"Readelf">()
		.exe<"readelf">()
		.env<"MGMK_READELF">();

	static constexpr auto debug_symbol_tool = tool
		.cli<"debug-symbol-tool">()
		.name<"Debug Symbol Tool">()
		.exe<"dsymutil">()
		.env<"MGMK_DSYMUTIL">();

	static constexpr auto lipo_tool = tool
		.cli<"lipo">()
		.name<"Lipo">()
		.exe<"lipo">()
		.env<"MGMK_LIPO">();

	static constexpr auto install_name_tool = tool
		.cli<"install-name-tool">()
		.name<"Install Name Tool">()
		.exe<"install_name_tool">()
		.env<"MGMK_INSTALLNAMETOOL">();

	static constexpr auto codesign_tool = tool
		.cli<"codesign">()
		.name<"Codesign">()
		.exe<"codesign">()
		.env<"MGMK_CODESIGN">();

	static constexpr auto ninja_tool = tool
		.cli<"ninja">()
		.name<"Ninja">()
		.exe<"ninja">()
		.env<"MGMK_NINJA">();

	static constexpr auto make_tool = tool
		.cli<"make">()
		.name<"Make">()
		.exe<"make">()
		.env<"MGMK_MAKE">();

	static constexpr auto msbuild_tool = tool
		.cli<"msbuild">()
		.name<"MSBuild">()
		.exe<"msbuild">()
		.env<"MGMK_MSBUILD">();

	static constexpr auto xcodebuild_tool = tool
		.cli<"xcodebuild">()
		.name<"Xcodebuild">()
		.exe<"xcodebuild">()
		.env<"MGMK_XCODEBUILD">();

	static constexpr auto cmake_tool = tool
		.cli<"cmake">()
		.name<"CMake">()
		.exe<"cmake">()
		.env<"MGMK_CMAKE">();

	static constexpr auto pkg_config_tool = tool
		.cli<"pkg-config">()
		.name<"pkg-config">()
		.exe<"pkg-config">()
		.env<"MGMK_PKG_CONFIG">();

	static constexpr auto git_tool = tool
		.cli<"git">()
		.name<"Git">()
		.exe<"git">()
		.env<"MGMK_GIT">();

	static constexpr auto curl_tool = tool
		.cli<"curl">()
		.name<"Curl">()
		.exe<"curl">()
		.env<"MGMK_CURL">();

	static constexpr auto wget_tool = tool
		.cli<"wget">()
		.name<"Wget">()
		.exe<"wget">()
		.env<"MGMK_WGET">();

	static constexpr auto unzip_tool = tool
		.cli<"unzip">()
		.name<"Unzip">()
		.exe<"unzip">()
		.env<"MGMK_UNZIP">();

	static constexpr auto tar_tool = tool
		.cli<"tar">()
		.name<"Tar">()
		.exe<"tar">()
		.env<"MGMK_TAR">();

	static constexpr auto exe_wrapper_tool = tool
		.cli<"exe-wrapper">()
		.name<"Executable Wrapper">()
		.env<"MGMK_EXE_WRAPPER">();

	static constexpr auto emulator_tool = tool
		.cli<"emulator">()
		.name<"Emulator">()
		.env<"MGMK_EMULATOR">();

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

#endif // MGMAKE_TOOL_DEFAULT_TOOLS_HXX