#pragma once

#ifndef MGMK_CONFIGURE_CMAKE_COMMAND_HXX
#define MGMK_CONFIGURE_CMAKE_COMMAND_HXX

#include "../../build/request.hxx"
#include "../../cli/backend.hxx"
#include "../../discovery/tool_role.hxx"
#include "../../ext/cmake/project.hxx"
#include "../../sys/command_line.hxx"

#include <filesystem>
#include <string>
#include <string_view>

namespace mgmake::configure::cmake {
	inline void append_tool_if_discovered(
		sys::command_line& command,
		const build::request& req,
		discovery::tool_role role,
		std::string_view cmake_variable
	) {
		const auto* tool = req.discovered_tool(role);

		if (tool == nullptr) {
			return;
		}

		command.m_args.emplace_back(
			"-D" + std::string{cmake_variable} + "=" + tool->path_string()
		);
	}

	inline void append_archive_tool_if_discovered(
		sys::command_line& command,
		const build::request& req
	) {
		if (const auto* tool = req.discovered_tool(discovery::tool_role::archiver)) {
			command.m_args.emplace_back(
				"-DCMAKE_AR=" + tool->path_string()
			);
			return;
		}

		if (const auto* tool = req.discovered_tool(discovery::tool_role::librarian)) {
			command.m_args.emplace_back(
				"-DCMAKE_AR=" + tool->path_string()
			);
		}
	}

	[[nodiscard]] inline std::string_view generator_for_backend(
		cli::backend_kind backend
	) noexcept {
		switch (backend) {
			case cli::backend_kind::automatic:
			case cli::backend_kind::ninja:
				return "Ninja";

			case cli::backend_kind::make:
			case cli::backend_kind::direct:
			case cli::backend_kind::count:
				return {};
		}

		return {};
	}

	inline void append_backend_generator_args(
		sys::command_line& command,
		const build::request& req
	) {
		const auto generator = generator_for_backend(req.backend());

		if (generator.empty()) {
			return;
		}

		command.m_args.emplace_back("-G");
		command.m_args.emplace_back(std::string{generator});

		if (generator == "Ninja") {
			if (const auto* ninja = req.discovered_tool(discovery::tool_role::generator_ninja)) {
				command.m_args.emplace_back(
					"-DCMAKE_MAKE_PROGRAM=" + ninja->path_string()
				);
			}
		}
	}

	inline void append_discovered_toolchain_args(
		sys::command_line& command,
		const build::request& req
	) {
		append_tool_if_discovered(
			command,
			req,
			discovery::tool_role::c_compiler,
			"CMAKE_C_COMPILER"
		);

		append_tool_if_discovered(
			command,
			req,
			discovery::tool_role::cxx_compiler,
			"CMAKE_CXX_COMPILER"
		);

		append_archive_tool_if_discovered(command, req);

		append_tool_if_discovered(
			command,
			req,
			discovery::tool_role::ranlib,
			"CMAKE_RANLIB"
		);

		append_tool_if_discovered(
			command,
			req,
			discovery::tool_role::resource_compiler,
			"CMAKE_RC_COMPILER"
		);
	}

	[[nodiscard]] inline sys::command_line configure_command(
		const build::request& req,
		const ext::cmake::project& cmake_project,
		const std::filesystem::path& source_dir,
		const std::filesystem::path& build_dir,
		const std::filesystem::path& install_dir
	) {
		sys::command_line command;
		command.m_args.emplace_back(req.tool_path(discovery::tool_role::cmake, "cmake").string());
		command.m_args.emplace_back("-S");
		command.m_args.emplace_back(source_dir.string());
		command.m_args.emplace_back("-B");
		command.m_args.emplace_back(build_dir.string());
		command.m_args.emplace_back("-DCMAKE_INSTALL_PREFIX=" + install_dir.string());

		if (!cmake_project.m_generator.empty()) {
			command.m_args.emplace_back("-G");
			command.m_args.emplace_back(cmake_project.m_generator);
		} else {
			append_backend_generator_args(command, req);
		}

		append_discovered_toolchain_args(command, req);

		if (!cmake_project.m_build_config.empty()) {
			command.m_args.emplace_back("-DCMAKE_BUILD_TYPE=" + cmake_project.m_build_config);
		}

		for (const auto& [key, value] : cmake_project.m_defines) {
			command.m_args.emplace_back("-D" + key + "=" + value);
		}

		for (const auto& arg : cmake_project.m_args) {
			command.m_args.emplace_back(arg);
		}

		return command;
	}
}

#endif // MGMK_CONFIGURE_CMAKE_COMMAND_HXX
