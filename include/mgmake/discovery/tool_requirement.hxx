#pragma once

#ifndef MGMAKE_DISCOVERY_TOOL_REQUIREMENT_HXX
#define MGMAKE_DISCOVERY_TOOL_REQUIREMENT_HXX

#include "source_role.hxx"
#include "tool_family.hxx"
#include "tool_role.hxx"
#include "../build/request.hxx"
#include "../cli/options.hxx"
#include "../ext/fetch.hxx"
#include "../spec/project.hxx"

#include <filesystem>
#include <string>
#include <string_view>
#include <variant>
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
		std::vector<tool_role> m_any_of{};

		[[nodiscard]] inline bool required() const noexcept {
			return m_strength == requirement_strength::required;
		}

		[[nodiscard]] inline bool is_any_of() const noexcept {
			return !m_any_of.empty();
		}
	};

	struct project_tool_usage {
		bool m_has_c_sources = false;
		bool m_has_cxx_sources = false;
		bool m_has_asm_sources = false;
		bool m_has_rc_sources = false;
		bool m_has_idl_sources = false;

		bool m_has_static_library = false;
		bool m_has_shared_library = false;
		bool m_has_executable = false;

		bool m_has_git_fetch = false;
		bool m_has_archive_fetch = false;
		bool m_has_zip_fetch = false;
		bool m_has_tar_fetch = false;
	};

	inline void record_source_role(
		project_tool_usage& usage,
		tool_role role
	) noexcept {
		switch (role) {
			case tool_role::c_compiler:
				usage.m_has_c_sources = true;
				return;

			case tool_role::cxx_compiler:
				usage.m_has_cxx_sources = true;
				return;

			case tool_role::assembler:
				usage.m_has_asm_sources = true;
				return;

			case tool_role::resource_compiler:
				usage.m_has_rc_sources = true;
				return;

			case tool_role::midl_compiler:
				usage.m_has_idl_sources = true;
				return;

			default:
				usage.m_has_cxx_sources = true;
				return;
		}
	}

	template <typename Source>
	inline void record_source_file(
		project_tool_usage& usage,
		const Source& source
	) {
		record_source_role(usage, source_tool_role(source));
	}

	template <typename Target>
	inline void record_target_sources(
		project_tool_usage& usage,
		const Target& target
	) {
		for (const auto& source : target.m_sources) {
			record_source_file(usage, source);
		}
	}

	inline void record_library_kind(
		project_tool_usage& usage,
		spec::library::kind kind
	) noexcept {
		switch (kind) {
			case spec::library::kind::interface:
				return;

			case spec::library::kind::static_lib:
				usage.m_has_static_library = true;
				return;

			case spec::library::kind::shared_lib:
				usage.m_has_shared_library = true;
				return;
		}
	}

	inline void record_fetch_tools(
		project_tool_usage& usage,
		const ext::fetch& fetch
	) {
		if (std::holds_alternative<ext::git_fetch>(fetch.m_data)) {
			usage.m_has_git_fetch = true;
			return;
		}

		if (const auto* archive = std::get_if<ext::archive_fetch>(&fetch.m_data)) {
			usage.m_has_archive_fetch = true;

			switch (archive->m_format) {
				case ext::archive_format::zip:
					usage.m_has_zip_fetch = true;
					break;

				case ext::archive_format::tar:
				case ext::archive_format::tar_gz:
				case ext::archive_format::tar_xz:
					usage.m_has_tar_fetch = true;
					break;

				case ext::archive_format::auto_detect:
					usage.m_has_zip_fetch = true;
					usage.m_has_tar_fetch = true;
					break;
			}
		}
	}

	[[nodiscard]] inline project_tool_usage collect_project_tool_usage(
		const spec::project& project
	) {
		project_tool_usage usage{};

		for (const auto& lib : project.m_libraries) {
			record_target_sources(usage, lib);
			record_library_kind(usage, lib.m_kind);
		}

		for (const auto& exe : project.m_executables) {
			record_target_sources(usage, exe);
			usage.m_has_executable = true;
		}

		for (const auto& fetch : project.m_fetches) {
			record_fetch_tools(usage, fetch);
		}

		return usage;
	}

	[[nodiscard]] inline std::vector<tool_requirement> required_tools(
		const cli::options&,
		const build::request& req,
		const spec::project& project
	) {
		std::vector<tool_requirement> result;
		const auto& tc = req.toolchain();
		const auto usage = collect_project_tool_usage(project);

		if (usage.m_has_c_sources) {
			result.push_back({tool_role::c_compiler, requirement_strength::required, std::string{tc.tool(tool_role::c_compiler)}, "the project has C sources"});
		}

		if (usage.m_has_cxx_sources) {
			result.push_back({tool_role::cxx_compiler, requirement_strength::required, std::string{tc.tool(tool_role::cxx_compiler)}, "the project has C++ sources"});
		}

		if (usage.m_has_asm_sources) {
			result.push_back({tool_role::assembler, requirement_strength::required, std::string{tc.tool(tool_role::assembler)}, "the project has assembly sources"});
		}

		if (usage.m_has_rc_sources) {
			result.push_back({tool_role::resource_compiler, requirement_strength::required, std::string{tc.tool(tool_role::resource_compiler)}, "the project has Windows resource sources"});
		}

		if (usage.m_has_idl_sources) {
			result.push_back({tool_role::midl_compiler, requirement_strength::required, std::string{tc.tool(tool_role::midl_compiler)}, "the project has IDL sources"});
		}

		if (usage.m_has_static_library) {
			const auto role = tc.dialect() == build::toolchain::dialect::msvc
				? tool_role::librarian
				: tool_role::archiver;

			result.push_back({role, requirement_strength::required, std::string{tc.tool(role)}, "the project builds at least one static library"});

			if (!tc.tool(tool_role::ranlib).empty()) {
				result.push_back({tool_role::ranlib, requirement_strength::optional, std::string{tc.tool(tool_role::ranlib)}, "the selected toolchain declares ranlib"});
			}
		}

		if (usage.m_has_shared_library) {
			const auto shared = tc.tool(tool_role::shared_linker);
			const auto role = shared.empty()
				? tool_role::linker
				: tool_role::shared_linker;

			result.push_back({
				role,
				requirement_strength::required,
				std::string{shared.empty() ? tc.tool(tool_role::linker) : shared},
				"the project builds at least one shared library"
			});
		}

		if (usage.m_has_executable) {
			result.push_back({tool_role::linker, requirement_strength::required, std::string{tc.tool(tool_role::linker)}, "the project builds at least one executable"});
		}

		if (usage.m_has_git_fetch) {
			result.push_back({
				.m_role = tool_role::git,
				.m_strength = requirement_strength::required,
				.m_needed_because = "the project fetches external git sources"
			});
		}

		if (usage.m_has_archive_fetch) {
			tool_requirement requirement{};
			requirement.m_any_of = {tool_role::curl, tool_role::wget};
			requirement.m_strength = requirement_strength::required;
			requirement.m_needed_because = "the project downloads external archives";
			result.push_back(std::move(requirement));
		}

		if (usage.m_has_zip_fetch) {
			result.push_back({
				.m_role = tool_role::unzip,
				.m_strength = requirement_strength::required,
				.m_needed_because = "the project extracts zip external archives"
			});
		}

		if (usage.m_has_tar_fetch) {
			result.push_back({
				.m_role = tool_role::tar,
				.m_strength = requirement_strength::required,
				.m_needed_because = "the project extracts tar external archives"
			});
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
