#pragma once

#ifndef MGMAKE_DISCOVERY_TOOL_REQUIREMENT_HXX
#define MGMAKE_DISCOVERY_TOOL_REQUIREMENT_HXX

#include "tool_family.hxx"
#include "tool_role.hxx"
#include "../build/request.hxx"
#include "../cli/options.hxx"
#include "../spec/project.hxx"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace mgmake::discovery {
	enum struct requirement_strength {
		required,
		optional,
		probe_only
	};

	struct tool_requirement {
		tool_role m_role{};
		requirement_strength m_strength = requirement_strength::required;
		std::string m_logical_name{};
		std::string m_needed_because{};
		tool_family m_expected_family = tool_family::unknown;
		object_format m_expected_object_format = object_format::unknown;
		std::string m_target_triple{};

		[[nodiscard]] inline bool required() const noexcept {
			return m_strength == requirement_strength::required;
		}
	};

	[[nodiscard]] inline std::vector<tool_requirement> required_tools(
		const cli::options&,
		const build::request& req,
		const spec::project& project
	) {
		std::vector<tool_requirement> result;
		const auto& tc = req.toolchain();

		bool has_c_sources = false;
		bool has_cxx_sources = false;
		bool has_asm_sources = false;
		bool has_rc_sources = false;
		bool has_idl_sources = false;
		bool has_static_library = false;
		bool has_shared_library = false;
		bool has_executable = false;

		auto inspect_sources = [&](const auto& target) {
			for (const auto& source : target.m_sources) {
				const auto ext = source.extension().string();

				if (ext == ".c") {
					has_c_sources = true;
				} else if (ext == ".cc" || ext == ".cpp" || ext == ".cxx" || ext == ".C") {
					has_cxx_sources = true;
				} else if (ext == ".s" || ext == ".S" || ext == ".asm") {
					has_asm_sources = true;
				} else if (ext == ".rc") {
					has_rc_sources = true;
				} else if (ext == ".idl") {
					has_idl_sources = true;
				} else {
					has_cxx_sources = true;
				}
			}
		};

		for (const auto& lib : project.m_libraries) {
			inspect_sources(lib);

			switch (lib.m_kind) {
				case spec::library::kind::interface:
					break;

				case spec::library::kind::static_lib:
					has_static_library = true;
					break;

				case spec::library::kind::shared_lib:
					has_shared_library = true;
					break;
			}
		}

		for (const auto& exe : project.m_executables) {
			inspect_sources(exe);
			has_executable = true;
		}

		if (has_c_sources) {
			result.push_back({tool_role::c_compiler, requirement_strength::required, std::string{tc.tool(tool_role::c_compiler)}, "the project has C sources"});
		}

		if (has_cxx_sources) {
			result.push_back({tool_role::cxx_compiler, requirement_strength::required, std::string{tc.tool(tool_role::cxx_compiler)}, "the project has C++ sources"});
		}

		if (has_asm_sources) {
			result.push_back({tool_role::assembler, requirement_strength::required, std::string{tc.tool(tool_role::assembler)}, "the project has assembly sources"});
		}

		if (has_rc_sources) {
			result.push_back({tool_role::resource_compiler, requirement_strength::required, std::string{tc.tool(tool_role::resource_compiler)}, "the project has Windows resource sources"});
		}

		if (has_idl_sources) {
			result.push_back({tool_role::midl_compiler, requirement_strength::required, std::string{tc.tool(tool_role::midl_compiler)}, "the project has IDL sources"});
		}

		if (has_static_library) {
			const auto role = tc.dialect() == build::toolchain::dialect::msvc
				? tool_role::librarian
				: tool_role::archiver;

			result.push_back({role, requirement_strength::required, std::string{tc.tool(role)}, "the project builds at least one static library"});

			if (!tc.tool(tool_role::ranlib).empty()) {
				result.push_back({tool_role::ranlib, requirement_strength::optional, std::string{tc.tool(tool_role::ranlib)}, "the selected toolchain declares ranlib"});
			}
		}

		if (has_shared_library) {
			const auto shared = tc.tool(tool_role::shared_linker);
			result.push_back({
				shared.empty() ? tool_role::linker : tool_role::shared_linker,
				requirement_strength::required,
				std::string{shared.empty() ? tc.tool(tool_role::linker) : shared},
				"the project builds at least one shared library"
			});
		}

		if (has_executable) {
			result.push_back({tool_role::linker, requirement_strength::required, std::string{tc.tool(tool_role::linker)}, "the project builds at least one executable"});
		}

		if (req.target_platform() == sys::platform::p_windows && tc.dialect() == build::toolchain::dialect::msvc) {
			if (!tc.tool(tool_role::manifest_tool).empty()) {
				result.push_back({tool_role::manifest_tool, requirement_strength::optional, std::string{tc.tool(tool_role::manifest_tool)}, "MSVC-style Windows builds may require manifest handling"});
			}
		}

		return result;
	}
}

#endif
