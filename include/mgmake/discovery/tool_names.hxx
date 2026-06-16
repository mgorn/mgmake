#pragma once

#ifndef MGMAKE_DISCOVERY_TOOL_NAMES_HXX
#define MGMAKE_DISCOVERY_TOOL_NAMES_HXX

#include "../build/request.hxx"
#include "../build/toolchain.hxx"
#include "../cli/options.hxx"
#include "mode.hxx"
#include "tool_role.hxx"

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

namespace mgmake::discovery {
	[[nodiscard]] inline std::string environment_variable_for(tool_role role) {
		switch (role) {
			case tool_role::c_compiler: return "MGMK_CC";
			case tool_role::cxx_compiler: return "MGMK_CXX";
			case tool_role::archiver: return "MGMK_AR";
			case tool_role::ranlib: return "MGMK_RANLIB";
			case tool_role::librarian: return "MGMK_LIB";
			case tool_role::linker: return "MGMK_LINKER";
			case tool_role::shared_linker: return "MGMK_SHARED_LINKER";
			case tool_role::resource_compiler: return "MGMK_RC";
			case tool_role::manifest_tool: return "MGMK_MT";
			case tool_role::dll_tool: return "MGMK_DLLTOOL";
			case tool_role::strip: return "MGMK_STRIP";
			case tool_role::objcopy: return "MGMK_OBJCOPY";
			case tool_role::objdump: return "MGMK_OBJDUMP";
			case tool_role::nm: return "MGMK_NM";
			case tool_role::readelf: return "MGMK_READELF";
			case tool_role::cmake: return "MGMK_CMAKE";
			case tool_role::pkg_config: return "MGMK_PKG_CONFIG";
			case tool_role::generator_ninja: return "MGMK_NINJA";
			case tool_role::exe_wrapper: return "MGMK_EXE_WRAPPER";
			case tool_role::emulator: return "MGMK_EMULATOR";
			default: return {};
		}
	}

	[[nodiscard]] inline std::string cli_override_for(const cli::options& opts, tool_role role) {
		switch (role) {
			case tool_role::c_compiler: return opts.m_cc;
			case tool_role::cxx_compiler: return opts.m_cxx;
			case tool_role::archiver: return opts.m_ar;
			case tool_role::ranlib: return opts.m_ranlib;
			case tool_role::librarian: return opts.m_librarian;
			case tool_role::linker: return opts.m_linker;
			case tool_role::shared_linker: return opts.m_shared_linker;
			case tool_role::resource_compiler: return opts.m_rc;
			case tool_role::manifest_tool: return opts.m_mt;
			case tool_role::dll_tool: return opts.m_dll_tool;
			case tool_role::strip: return opts.m_strip;
			case tool_role::objcopy: return opts.m_objcopy;
			case tool_role::objdump: return opts.m_objdump;
			case tool_role::nm: return opts.m_nm;
			case tool_role::readelf: return opts.m_readelf;
			case tool_role::generator_ninja: return opts.m_ninja;
			case tool_role::cmake: return opts.m_cmake;
			case tool_role::pkg_config: return opts.m_pkg_config;
			case tool_role::exe_wrapper: return opts.m_exe_wrapper;
			case tool_role::emulator: return opts.m_emulator;
			default: return {};
		}
	}

	[[nodiscard]] inline std::vector<std::string> logical_names_for(
		const build::toolchain& tc,
		tool_role role
	) {
		std::vector<std::string> result;

		if (auto value = tc.tool(role); !value.empty()) {
			result.emplace_back(value);
		}

		return result;
	}

	[[nodiscard]] inline std::vector<std::string> fallback_names_for(
		const build::toolchain& tc,
		const build::request&,
		tool_role role
	) {
		if (tc.dialect() == build::toolchain::dialect::msvc) {
			switch (role) {
				case tool_role::c_compiler:
				case tool_role::cxx_compiler: return {"cl"};
				case tool_role::archiver:
				case tool_role::librarian: return {"lib"};
				case tool_role::linker:
				case tool_role::shared_linker: return {"link"};
				case tool_role::resource_compiler: return {"rc", "llvm-rc"};
				case tool_role::manifest_tool: return {"mt"};
				default: break;
			}
		}

		const bool clang_like = tc.name().find("clang") != std::string_view::npos;

		if (clang_like) {
			switch (role) {
				case tool_role::c_compiler: return {"clang"};
				case tool_role::cxx_compiler: return {"clang++"};
				case tool_role::archiver: return {"llvm-ar", "ar"};
				case tool_role::ranlib: return {"llvm-ranlib", "ranlib"};
				case tool_role::linker:
				case tool_role::shared_linker: return {"clang++", "clang"};
				case tool_role::resource_compiler: return {"llvm-rc", "windres"};
				case tool_role::strip: return {"llvm-strip", "strip"};
				case tool_role::objcopy: return {"llvm-objcopy", "objcopy"};
				case tool_role::objdump: return {"llvm-objdump", "objdump"};
				case tool_role::nm: return {"llvm-nm", "nm"};
				case tool_role::readelf: return {"llvm-readelf", "readelf"};
				default: break;
			}
		}

		switch (role) {
			case tool_role::c_compiler: return {"gcc", "cc"};
			case tool_role::cxx_compiler: return {"g++", "c++"};
			case tool_role::archiver: return {"gcc-ar", "ar"};
			case tool_role::ranlib: return {"gcc-ranlib", "ranlib"};
			case tool_role::linker:
			case tool_role::shared_linker: return {"g++", "gcc"};
			case tool_role::resource_compiler: return {"windres"};
			case tool_role::strip: return {"strip"};
			case tool_role::objcopy: return {"objcopy"};
			case tool_role::objdump: return {"objdump"};
			case tool_role::nm: return {"nm"};
			case tool_role::readelf: return {"readelf"};
			default: break;
		}

		return {};
	}

	[[nodiscard]] inline std::vector<std::string> candidate_names_for(
		const build::request& req,
		tool_role role,
		mode discovery_mode
	) {
		std::vector<std::string> result = logical_names_for(req.toolchain(), role);

		if (discovery_mode == mode::exact) {
			return result;
		}

		for (auto fallback : fallback_names_for(req.toolchain(), req, role)) {
			if (std::ranges::find(result, fallback) == result.end()) {
				result.emplace_back(std::move(fallback));
			}
		}

		return result;
	}

	[[nodiscard]] inline std::vector<std::string> target_prefixed_names_for(
		const build::request& req,
		tool_role role,
		mode discovery_mode
	) {
		std::vector<std::string> result;
		std::string triple;

		if (req.toolchain().target_triple().has_value()) {
			triple = *req.toolchain().target_triple();
		} else {
			triple = req.target().m_triple;
		}

		if (triple.empty()) {
			return result;
		}

		for (const auto& name : candidate_names_for(req, role, discovery_mode)) {
			if (!name.starts_with(triple + "-")) {
				result.emplace_back(triple + "-" + name);
			}
		}

		return result;
	}
}

#endif
