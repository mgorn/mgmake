#pragma once

#ifndef MGMAKE_DISCOVERY_TOOL_ROLE_HXX
#define MGMAKE_DISCOVERY_TOOL_ROLE_HXX

#include "../cli/option.hxx"
#include "../meta/static_string.hxx"
#include "../meta/type_builder.hxx"
#include "../meta/value_list.hxx"
#include "../meta/type_map.hxx"

namespace mgmake::discovery {
	template<typename storage_t = meta::type_map<>>
	struct tool_role_impl : public meta::type_builder<tool_role_impl, storage_t>, public meta::named<tool_role_impl<storage_t>> {
		using builder_type = meta::type_builder<tool_role_impl, storage_t>;

		// Logical name, for the override option ("cc", "cxx")
		template<meta::static_string logical_v>
		[[nodiscard]] static consteval auto logical() {
			return builder_type::template set_str<"logical", logical_v>();
		}
		static consteval auto logical() {
			return builder_type::template get_str<"logical">();
		}

		// Environment variable override ("MGMK_CC", "MGMK_CXX", etc)
		template<meta::static_string env_v>
		[[nodiscard]] static consteval auto env() {
			return builder_type::template set_str<"env", env_v>();
		}
		static consteval auto env() {
			return builder_type::template get_str<"env">();
		}

		[[nodiscard]] static consteval auto option() {
			constexpr auto description_v = meta::static_string{ "Override the " } + builder_type::template get_str<"name">() + meta::static_string{ " tool" };
			return cli::option.name<logical()>().template description<description_v>();
		}
		using option_type = decltype(option());
	};
	static constexpr auto tool_role = tool_role_impl<>{};

	static constexpr auto cc_role = tool_role.logical<"cc">().name<"C Compiler">().env<"MGMK_CC">();
	static constexpr auto cxx_role = tool_role.logical<"cxx">().name<"C++ Compiler">().env<"MGMK_CXX">();
	static constexpr auto objc_role = tool_role.logical<"objcc">().name<"Objective-C Compiler">().env<"MGMK_OBJCC">();
	static constexpr auto objcxx_role = tool_role.logical<"objcxx">().name<"Objective-CXX Compiler">().env<"MGMK_OBJCXX">();
	static constexpr auto assembler_role = tool_role.logical<"asm">().name<"Assembler">().env<"MGMK_ASM">();
	static constexpr auto cuda_compiler_role = tool_role.logical<"cuda">().name<"CUDA Compiler">();
	static constexpr auto hip_compiler_role = tool_role.logical<"hip">().name<"HIP Compiler">();
	static constexpr auto resource_compiler_role = tool_role.logical<"rc">().name<"Resource Compiler">().env<"MGMK_RC">();
	static constexpr auto midl_compiler_role = tool_role.logical<"midl">().name<"MIDL Compiler">();
	static constexpr auto archiver_role = tool_role.logical<"ar">().name<"Archiver">().env<"MGMK_AR">();
	static constexpr auto ranlib_role = tool_role.logical<"ranlib">().name<"Ranlib">().env<"MGMK_RANLIB">();
	static constexpr auto librarian_role = tool_role.logical<"lib">().name<"Librarian">().env<"MGMK_LIB">();
	static constexpr auto linker_role = tool_role.logical<"linker">().name<"Linker">().env<"MGMK_LINKER">();
	static constexpr auto shared_linker_role = tool_role.logical<"shared-linker">().name<"Shared Linker">().env<"MGMK_SHARED_LINKER">();
	static constexpr auto dll_tool_role = tool_role.logical<"dlltool">().name<"DLL Tool">().env<"MGMK_DLLTOOL">();
	static constexpr auto manifest_tool_role = tool_role.logical<"mt">().name<"Manifest Tool">().env<"MGMK_MT">();
	static constexpr auto strip_role = tool_role.logical<"strip">().name<"Strip">().env<"MGMK_STRIP">();
	static constexpr auto objcopy_role = tool_role.logical<"objcopy">().name<"Objcopy">().env<"MGMK_OBJCOPY">();
	static constexpr auto objdump_role = tool_role.logical<"objdump">().name<"Objdump">().env<"MGMK_OBJDUMP">();
	static constexpr auto nm_role = tool_role.logical<"nm">().name<"NM">().env<"MGMK_NM">();
	static constexpr auto readelf_role = tool_role.logical<"readelf">().name<"Readelf">().env<"MGMK_READELF">();
	static constexpr auto debug_symbol_tool_role = tool_role.logical<"debug-symbol-tool">().name<"Debug Symbol Tool">();
	static constexpr auto lipo_role = tool_role.logical<"lipo">().name<"Lipo">();
	static constexpr auto install_name_tool_role = tool_role.logical<"install-name-tool">().name<"Install Name Tool">();
	static constexpr auto codesign_role = tool_role.logical<"codesign">().name<"Codesign">();
	static constexpr auto ninja_role = tool_role.logical<"ninja">().name<"Ninja">().env<"MGMK_NINJA">();
	static constexpr auto make_role = tool_role.logical<"make">().name<"Make">();
	static constexpr auto msbuild_role = tool_role.logical<"msbuild">().name<"MSBuild">();
	static constexpr auto xcodebuild_role = tool_role.logical<"xcodebuild">().name<"Xcodebuild">();
	static constexpr auto cmake_role = tool_role.logical<"cmake">().name<"CMake">().env<"MGMK_CMAKE">();
	static constexpr auto pkg_config_role = tool_role.logical<"pkg-config">().name<"pkg-config">().env<"MGMK_PKG_CONFIG">();
	static constexpr auto git_role = tool_role.logical<"git">().name<"Git">().env<"MGMK_GIT">();
	static constexpr auto curl_role = tool_role.logical<"curl">().name<"Curl">().env<"MGMK_CURL">();
	static constexpr auto wget_role = tool_role.logical<"wget">().name<"Wget">().env<"MGMK_WGET">();
	static constexpr auto unzip_role = tool_role.logical<"unzip">().name<"Unzip">().env<"MGMK_UNZIP">();
	static constexpr auto tar_role = tool_role.logical<"tar">().name<"Tar">().env<"MGMK_TAR">();
	static constexpr auto exe_wrapper_role = tool_role.logical<"exe-wrapper">().name<"Executable Wrapper">().env<"MGMK_EXE_WRAPPER">();
	static constexpr auto emulator_role = tool_role.logical<"emulator">().name<"Emulator">().env<"MGMK_EMULATOR">();

	using default_tool_roles = meta::value_list<
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