#pragma once

#ifndef MGMK_LOWER_CONTEXT_IMPL_HXX
#define MGMK_LOWER_CONTEXT_IMPL_HXX

#include "context.hxx"
#include "objects.hxx"
#ifdef MGMK_ENABLE_EXT_CMAKE
#include "cmake/provider.hxx"
#endif // MGMK_ENABLE_EXT_CMAKE
#include "../build/artifact_names.hxx"
#include "../build/target.hxx"
#include "../detail/assert.hxx"
#include "../spec/project.hxx"
#include "../sys/command_line.hxx"
#include "../sys/file_command.hxx"

#include <algorithm>
#include <filesystem>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// Lowering is demand-driven: libraries are lowered when linked, executables are lowered explicitly, and provider targets become DAG dependencies.

namespace mgmake::lower {
	inline context::context(
		dag::graph& graph,
		const build::request& req,
		const spec::project& project,
		const prep::result& prep,
		dep::database& deps
	)
		: m_req{req}
		, m_project{project}
		, m_prep{prep}
		, m_deps{deps}
		, m_emit{graph}
		, m_libraries(project.m_libraries.size()) {}


	[[nodiscard]] inline std::string render_link_input(
		const dag::artifact& artifact,
		const build::toolchain& tc
	) {
		auto name = artifact.path().string();

		if (!artifact.is_system()) {
			return name;
		}

		if (name.starts_with("-") || name.starts_with("/")) {
			return name;
		}

		if (tc.dialect() == build::toolchain::dialect::msvc) {
			if (name.ends_with(".lib")) {
				return name;
			}

			return name + ".lib";
		}

		if (name.ends_with(".lib")) {
			name = name.substr(0, name.size() - 4);
		}

		return "-l" + name;
	}

	inline lower::usage context::use_libraries(
		const std::set<std::string>& libraries,
		std::string_view owner_name
	) {
		lower::usage result{};

		// Usage propagation turns named library edges into include paths, link inputs, and DAG target dependencies.
		for (const auto& library_name : libraries) {
			const auto linked_id = m_project.find_library(library_name);
			
			const lower::target& dep = [&] {
				if (linked_id.has_value()) {
					return lower_library(linked_id.value());
				} else {
					// lowering with just the name assumes system/external
					return lower_library(library_name);
				}
			}();

			/*
			mgmkassert(
				linked_id.has_value(),
				"mgmake lower: target '" + std::string{owner_name} +
					"' links unknown library '" + library_name + "'"
			);
			*/

			if (dep.m_dag_target.has_value()) {
				result.m_dag_dependencies.emplace(dep.m_dag_target.value());
			}

			result.m_link_inputs.insert(
				result.m_link_inputs.end(),
				dep.m_linkable_artifacts.begin(),
				dep.m_linkable_artifacts.end()
			);

			result.m_include_dirs.insert(dep.m_include_dirs.begin(), dep.m_include_dirs.end());
			result.m_usage_inputs.insert(
				result.m_usage_inputs.end(),
				dep.m_usage_inputs.begin(),
				dep.m_usage_inputs.end()
			);
		}

		return result;
	}

	inline const lower::target& context::lower_library(spec::library::id id) {
		mgmkassert(
			id < m_libraries.size(),
			"mgmake lower: invalid library id"
		);

		// Libraries are memoized because multiple dependents can request the same lowered library.
		if (m_libraries.at(id).has_value()) {
			return m_libraries.at(id).value();
		}

		const auto& lib = m_project.m_libraries.at(id);

		mgmkassert(not lib.m_name.empty(), "mgmake lower: library target has no name");

		mgmkassert(
			not m_active_libraries.contains(id),
			"mgmake lower: cyclic library dependency involving '" + lib.m_name + "'"
		);

		m_active_libraries.emplace(id);

		lower::usage usage = use_libraries(
			lib.linked_libraries(),
			lib.m_name
		);

#ifdef MGMK_ENABLE_EXT_CMAKE
		if (lib.provider_backed()) {
			m_libraries.at(id) = lower::cmake::lower_provider_library(*this, lib, std::move(usage));
			m_active_libraries.erase(id);
			return m_libraries.at(id).value();
		}
#endif // MGMK_ENABLE_EXT_CMAKE

		switch (lib.m_kind) {
			case spec::library::kind::interface:
				m_libraries.at(id) = lower_interface_library(lib, std::move(usage));
				break;

			case spec::library::kind::static_lib:
				m_libraries.at(id) = lower_static_library(lib, std::move(usage));
				break;

			case spec::library::kind::shared_lib:
				m_libraries.at(id) = lower_shared_library(lib, std::move(usage));
				break;
		}

		m_active_libraries.erase(id);

		return m_libraries.at(id).value();
	}

	// When we only have a string for a library, it's probably a system library
	inline const lower::target& context::lower_library(std::string_view lib) {
		// assert it is NOT part of the project
		mgmkassert(not m_project.find_library(lib).has_value(), "mgmake target passed as external/system library? This should never happen.");

		// Append external/system libs AFTER project libraries
		return m_libraries.emplace_back(lower_system_library(lib)).value();
	}

	inline lower::target context::lower_system_library(std::string_view lib) {
		auto artifact = m_emit.file_artifact(dag::artifact::kind::system, lib);
		dag::target dag_target{ 
			.m_name = std::string{ lib },
			.m_outputs = { artifact }
		};
		return lower::target{
			.m_dag_target = m_emit.target(dag_target),
			.m_linkable_artifacts = { artifact }
		};
	}

	inline lower::target context::lower_interface_library(
		const spec::library& lib,
		lower::usage usage
	) {
		mgmkassert(
			lib.m_sources.empty(),
			"mgmake lower: interface library '" + lib.m_name + "' cannot have sources"
		);

		auto include_dirs = lib.include_dirs();
		include_dirs.insert(usage.m_include_dirs.begin(), usage.m_include_dirs.end());
		auto link_inputs = std::move(usage.m_link_inputs);

		dag::target dag_target{
			lib.m_name,
			{},
			std::move(usage.m_dag_dependencies)
		};

		lower::target lowered{};
		lowered.m_dag_target = m_emit.target(dag_target);
		lowered.m_linkable_artifacts = std::move(link_inputs);
		lowered.m_include_dirs = std::move(include_dirs);
		lowered.m_usage_inputs = std::move(usage.m_usage_inputs);
		return lowered;
	}

	inline lower::target context::lower_static_library(
		const spec::library& lib,
		lower::usage usage
	) {
		const auto& tc = toolchain();

		mgmkassert(
			not lib.m_sources.empty(),
			"mgmake lower: static library '" + lib.m_name + "' has no sources"
		);

		auto include_dirs = lib.include_dirs();
		include_dirs.insert(usage.m_include_dirs.begin(), usage.m_include_dirs.end());

		auto object_ids = lower_objects(lib, include_dirs, usage.m_usage_inputs);

		std::filesystem::path archive_path;

		switch (tc.dialect()) {
			case build::toolchain::dialect::gcc:
				archive_path = request().build_dir() / "lib" / ("lib" + lib.m_name + ".a");
				break;

			case build::toolchain::dialect::msvc:
				archive_path = request().build_dir() / "lib" / (lib.m_name + ".lib");
				break;
		}

		auto archive_id = m_emit.generated(archive_path);

		sys::command_line command{};
		command.m_args.emplace_back(tc.ar());

		switch (tc.dialect()) {
			case build::toolchain::dialect::gcc: {
				if (tc.archive_flags().empty()) {
					command.m_args.emplace_back("rcs");
				}
				else {
					for (const auto& flag : tc.archive_flags()) {
						command.m_args.emplace_back(flag);
					}
				}

				command.m_args.emplace_back(archive_path.string());

				for (auto object_id : object_ids) {
					command.m_args.emplace_back(m_emit.path(object_id).string());
				}

				break;
			}

			case build::toolchain::dialect::msvc: {
				for (const auto& flag : tc.archive_flags()) {
					command.m_args.emplace_back(flag);
				}

				command.m_args.emplace_back(std::string{"/OUT:"} + archive_path.string());

				for (auto object_id : object_ids) {
					command.m_args.emplace_back(m_emit.path(object_id).string());
				}

				break;
			}
		}

		std::vector<dag::artifact::id> inputs = object_ids;
		inputs.insert(inputs.end(), usage.m_usage_inputs.begin(), usage.m_usage_inputs.end());

		m_emit.action(
			std::string{"Build static library "} + lib.m_name,
			std::string{"Builds static library target '"} + lib.m_name + "'.",
			inputs,
			{ archive_id },
			command
		);

		dag::target dag_target{
			lib.m_name,
			{ archive_id },
			std::move(usage.m_dag_dependencies)
		};

		lower::target lowered{};
		lowered.m_dag_target = m_emit.target(dag_target);
		lowered.m_linkable_artifacts.emplace_back(archive_id);
		lowered.m_linkable_artifacts.insert(
			lowered.m_linkable_artifacts.end(),
			usage.m_link_inputs.begin(),
			usage.m_link_inputs.end()
		);
		lowered.m_include_dirs = std::move(include_dirs);
		lowered.m_usage_inputs = std::move(usage.m_usage_inputs);
		return lowered;
	}

	inline lower::target context::lower_shared_library(
		const spec::library& lib,
		lower::usage usage
	) {
		const auto& tc = toolchain();

		mgmkassert(
			not lib.m_sources.empty(),
			"mgmake lower: shared library '" + lib.m_name + "' has no sources"
		);

		mgmkassert(
			tc.dialect() == build::toolchain::dialect::gcc,
			"mgmake lower: shared library lowering is currently only implemented for GCC-like toolchains"
		);

		auto include_dirs = lib.include_dirs();
		include_dirs.insert(usage.m_include_dirs.begin(), usage.m_include_dirs.end());

		auto object_ids = lower_objects(lib, include_dirs, usage.m_usage_inputs);

		const auto platform = request().target_platform();
		std::filesystem::path shared_path =
			request().build_dir() /
			"lib" /
			(
				std::string{ build::shared_library_prefix(platform) } +
				lib.m_name +
				std::string{ build::shared_library_extension(platform) }
			);

		auto shared_id = m_emit.generated(shared_path);

		sys::command_line command{};
		command.m_args.emplace_back(tc.linker());

		build::append_target_args(command, tc, request());

		for (const auto& arg : request().link_prefix_args()) {
			command.m_args.emplace_back(arg);
		}

		const auto shared_flag = build::shared_library_link_flag(platform);

		mgmkassert(
			!shared_flag.empty(),
			"mgmake lower: shared library lowering is not supported for requested target platform"
		);

		command.m_args.emplace_back(shared_flag);

		for (auto object_id : object_ids) {
			command.m_args.emplace_back(m_emit.path(object_id).string());
		}

		for (auto link_input : usage.m_link_inputs) {
			command.m_args.emplace_back(
				render_link_input(m_emit.graph().artifact(link_input), tc)
			);
		}

		for (const auto& flag : tc.link_flags()) {
			command.m_args.emplace_back(flag);
		}

		command.m_args.emplace_back("-o");
		command.m_args.emplace_back(shared_path.string());

		std::vector<dag::artifact::id> inputs = object_ids;
		for (auto input : usage.m_link_inputs) {
			inputs.emplace_back(input);
		}
		inputs.insert(inputs.end(), usage.m_usage_inputs.begin(), usage.m_usage_inputs.end());

		m_emit.action(
			std::string{"Build shared library "} + lib.m_name,
			std::string{"Builds shared library target '"} + lib.m_name + "'.",
			inputs,
			{ shared_id },
			command
		);

		dag::target dag_target{
			lib.m_name,
			{ shared_id },
			std::move(usage.m_dag_dependencies)
		};

		lower::target lowered{};
		lowered.m_dag_target = m_emit.target(dag_target);
		lowered.m_linkable_artifacts.emplace_back(shared_id);
		lowered.m_linkable_artifacts.insert(
			lowered.m_linkable_artifacts.end(),
			usage.m_link_inputs.begin(),
			usage.m_link_inputs.end()
		);
		lowered.m_include_dirs = std::move(include_dirs);
		lowered.m_usage_inputs = std::move(usage.m_usage_inputs);
		return lowered;
	}

	inline void context::lower_executable(spec::executable::id id) {
		mgmkassert(
			id < m_project.m_executables.size(),
			"mgmake lower: invalid executable id"
		);

		const auto& exe = m_project.m_executables.at(id);
		const auto& tc = toolchain();

		mgmkassert(not exe.m_name.empty(), "mgmake lower: executable target has no name");

		lower::usage usage = use_libraries(
			exe.linked_libraries(),
			exe.m_name
		);

#ifdef MGMK_ENABLE_EXT_CMAKE
		if (exe.provider_backed()) {
			lower::cmake::lower_provider_executable(*this, exe, std::move(usage));
			return;
		}
#endif // MGMK_ENABLE_EXT_CMAKE

		mgmkassert(
			not exe.m_sources.empty(),
			"mgmake lower: executable target '" + exe.m_name + "' has no sources"
		);

		auto include_dirs = exe.include_dirs();
		include_dirs.insert(usage.m_include_dirs.begin(), usage.m_include_dirs.end());

		auto object_ids = lower_objects(exe, include_dirs, usage.m_usage_inputs);
		std::vector<dag::artifact::id> inputs = object_ids;
		for (auto input : usage.m_link_inputs) {
			inputs.emplace_back(input);
		}
		inputs.insert(inputs.end(), usage.m_usage_inputs.begin(), usage.m_usage_inputs.end());

		std::filesystem::path output =
			request().build_dir() /
			(
				exe.m_name +
				std::string{ build::executable_extension(request().target_platform()) }
			);

		auto output_id = m_emit.generated(output);

		sys::command_line command{};
		command.m_args.emplace_back(tc.cxx());

		build::append_target_args(command, tc, request());

		for (const auto& arg : request().link_prefix_args()) {
			command.m_args.emplace_back(arg);
		}

		for (auto object_id : object_ids) {
			command.m_args.emplace_back(m_emit.path(object_id).string());
		}

		for (auto link_input : usage.m_link_inputs) {
			command.m_args.emplace_back(
				render_link_input(m_emit.graph().artifact(link_input), tc)
			);
		}

		for (const auto& flag : tc.link_flags()) {
			command.m_args.emplace_back(flag);
		}

		switch (tc.dialect()) {
			case build::toolchain::dialect::gcc:
				command.m_args.emplace_back("-o");
				command.m_args.emplace_back(output.string());
				break;

			case build::toolchain::dialect::msvc:
				command.m_args.emplace_back(std::string{"/Fe"} + output.string());
				break;
		}

		m_emit.action(
			std::string{"Build executable "} + exe.m_name,
			std::string{"Builds executable target '"} + exe.m_name + "'.",
			inputs,
			{ output_id },
			command
		);

		dag::target dag_target{
			exe.m_name,
			{ output_id },
			std::move(usage.m_dag_dependencies)
		};

		m_emit.target(dag_target);
	}

}

#endif // MGMK_LOWER_CONTEXT_IMPL_HXX
