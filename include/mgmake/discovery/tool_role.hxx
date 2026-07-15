#pragma once

#ifndef MGMAKE_DISCOVERY_TOOL_ROLE_HXX
#define MGMAKE_DISCOVERY_TOOL_ROLE_HXX

#include "../cli/option.hxx"
#include "../meta/static_string.hxx"
#include "../meta/type_builder.hxx"
#include "../meta/type_list.hxx"
#include "../meta/type_map.hxx"

namespace mgmake::discovery {
	template<typename storage_t = meta::type_map<>>
	struct tool_role_impl {
		MGMAKE_TYPE_CONSUMER_VALUE_FIELD(logical, meta::static_string{ "" });
		MGMAKE_TYPE_CONSUMER_VALUE_FIELD(name, meta::static_string{ "" });
		MGMAKE_TYPE_CONSUMER_VALUE_FIELD(env, meta::static_string{ "" });

		using option_type = option
			::name<logical_value>
			::description<std::format("Override the {} tool", name_value.view())>
			::build;
	};

	template<typename builder_t = meta::type_builder<>>
	struct tool_role_builder {
		using builder_type = builder_t;

		// Logical name, for the override option ("cc", "cxx")
		MGMAKE_TYPE_BUILDER_VALUE_FIELD(tool_role_builder, logical, meta::static_string);
		// Role name ("C Compiler", "C++ Compiler", etc)
		MGMAKE_TYPE_BUILDER_VALUE_FIELD(tool_role_builder, name, meta::static_string);
		// Environment variable override ("MGMK_CC", "MGMK_CXX", etc)
		MGMAKE_TYPE_BUILDER_VALUE_FIELD(tool_role_builder, env, meta::static_string);

		using build = typename builder_type::template build<tool_role_impl>;
	};
	using tool_role = tool_role_builder<>;

	using cc_role = tool_role::name<"C Compiler">::env<"MGMK_CC">::build;
	using cxx_role = tool_role::name<"C++ Compiler">::env<"MGMK_CXX">::build;
	using objc_role = tool_role::name<"Objective-C Compiler">::env<"MGMK_OBJCC">::build;
	using objcxx_role = tool_role::name<"Objective-CXX Compiler">::env<"MGMK_OBJCXX">::build;
	using assembler_role = tool_role::name<"Assembler">::env<"MGMK_ASM">::build;
	using cuda_compiler_role = tool_role::name<"CUDA Compiler">::build;
	using hip_compiler_role = tool_role::name<"HIP Compiler">::build;
	using resource_compiler_role = tool_role::name<"Resource Compiler">::env<"MGMK_RC">::build;
	using midl_compiler_role = tool_role::name<"MIDL Compiler">::build;
	using archiver_role = tool_role::name<"Archiver">::env<"MGMK_AR">::build;
	using ranlib_role = tool_role::name<"Ranlib">::env<"MGMK_RANLIB">::build;
	using librarian_role = tool_role::name<"Librarian">::env<"MGMK_LIB">::build;
	using linker_role = tool_role::name<"Linker">::env<"MGMK_LINKER">::build;
	using shared_linker_role = tool_role::name<"Shared Linker">::env<"MGMK_SHARED_LINKER">::build;
	using dll_tool_role = tool_role::name<"DLL Tool">::env<"MGMK_DLLTOOL">::build;
	using manifest_tool_role = tool_role::name<"Manifest Tool">::env<"MGMK_MT">::build;
	using strip_role = tool_role::name<"Strip">::env<"MGMK_STRIP">::build;
	using objcopy_role = tool_role::name<"Objcopy">::env<"MGMK_OBJCOPY">::build;
	using objdump_role = tool_role::name<"Objdump">::env<"MGMK_OBJDUMP">::build;
	using nm_role = tool_role::name<"NM">::env<"MGMK_NM">::build;
	using readelf_role = tool_role::name<"Readelf">::env<"MGMK_READELF">::build;
	using debug_symbol_tool_role = tool_role::name<"Debug Symbol Tool">::build;
	using lipo_role = tool_role::name<"Lipo">::build;
	using install_name_tool_role = tool_role::name<"Install Name Tool">::build;
	using codesign_role = tool_role::name<"Codesign">::build;
	using ninja_role = tool_role::name<"Ninja">::env<"MGMK_NINJA">::build;
	using make_role = tool_role::name<"Make">::build;
	using msbuild_role = tool_role::name<"MSBuild">::build;
	using xcodebuild_role = tool_role::name<"Xcodebuild">::build;
	using cmake_role = tool_role::name<"CMake">::env<"MGMK_CMAKE">::build;
	using pkg_config_role = tool_role::name<"pkg-config">::env<"MGMK_PKG_CONFIG">::build;
	using git_role = tool_role::name<"Git">::env<"MGMK_GIT">::build;
	using curl_role = tool_role::name<"Curl">::env<"MGMK_CURL">::build;
	using wget_role = tool_role::name<"Wget">::env<"MGMK_WGET">::build;
	using unzip_role = tool_role::name<"Unzip">::env<"MGMK_UNZIP">::build;
	using tar_role = tool_role::name<"Tar">::env<"MGMK_TAR">::build;
	using exe_wrapper_role = tool_role::name<"Executable Wrapper">::env<"MGMK_EXE_WRAPPER">::build;
	using emulator_role = tool_role::name<"Emulator">::env<"MGMK_EMULATOR">::build;

	using default_tool_roles = meta::type_list<
		cc_role,
		cxx_role,
		objc_role,
		objcxx_role,
		assembler_role,
		cuda_compiler_role,
		hip_compiler_role,
		resource_compiler_role,
		midl_compiler_role,
		archiver_role,
		ranlib_role,
		librarian_role,
		linker_role,
		shared_linker_role,
		dll_tool_role,
		manifest_tool_role,
		strip_role,
		objcopy_role,
		objdump_role,
		nm_role,
		readelf_role,
		debug_symbol_tool_role,
		lipo_role,
		install_name_tool_role,
		codesign_role,
		ninja_role,
		make_role,
		msbuild_role,
		xcodebuild_role,
		cmake_role,
		pkg_config_role,
		git_role,
		curl_role,
		wget_role,
		unzip_role,
		tar_role,
		exe_wrapper_role,
		emulator_role
	>;
}

#endif // MGMAKE_DISCOVERY_TOOL_ROLE_HXX