#pragma once

#ifndef MGMK_LOWER_CONTEXT_IMPL_HXX
#define MGMK_LOWER_CONTEXT_IMPL_HXX

#include "context.hxx"
#include "objects.hxx"
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
#include <array>
#include <vector>

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


#ifdef MGMK_ENABLE_EXT_CMAKE
	[[nodiscard]] inline std::filesystem::path provider_root(
		const prep::cmake_project& cmake_project,
		ext::output_root root
	) {
		switch (root) {
			case ext::output_root::source_dir: return cmake_project.m_source_dir;
			case ext::output_root::build_dir: return cmake_project.m_build_dir;
			case ext::output_root::install_dir: return cmake_project.m_install_dir;
		}

		return cmake_project.m_install_dir;
	}

	[[nodiscard]] inline std::filesystem::path resolve_rooted_path(
		const prep::cmake_project& cmake_project,
		const ext::rooted_path& path
	) {
		if (path.m_path.is_absolute()) {
			return path.m_path;
		}

		return provider_root(cmake_project, path.m_root) / path.m_path;
	}

	[[nodiscard]] inline std::filesystem::path conventional_provider_artifact(
		const build::request& req,
		const prep::cmake_project& cmake_project,
		const ext::provider_ref& provider,
		spec::library::kind kind
	) {
		const auto root = provider_root(cmake_project, provider.m_usage_root);

		switch (kind) {
			case spec::library::kind::static_lib:
				return root / "lib" /
					(
						std::string{build::static_library_prefix(req.target_platform())} +
						provider.m_target +
						std::string{build::static_library_extension(req.target_platform())}
					);

			case spec::library::kind::shared_lib:
				return root / "lib" /
					(
						std::string{build::shared_library_prefix(req.target_platform())} +
						provider.m_target +
						std::string{build::shared_library_extension(req.target_platform())}
					);

			case spec::library::kind::interface:
				return {};
		}

		return {};
	}

	[[nodiscard]] inline std::filesystem::path conventional_provider_executable(
		const build::request& req,
		const prep::cmake_project& cmake_project,
		const ext::provider_ref& provider
	) {
		const auto root = provider_root(cmake_project, provider.m_usage_root);
		return root / "bin" / (provider.m_target + std::string{build::executable_extension(req.target_platform())});
	}

	[[nodiscard]] inline std::filesystem::path provider_target_stamp(
		const build::request& req,
		const ext::provider_ref& provider
	) {
		return req.build_dir() / "ext" / "stamp" /
			(provider.m_project + ".cmake.build." + provider.m_target);
	}

	[[nodiscard]] inline sys::command_line cmake_build_command(
		const build::request& req,
		const ext::cmake& cmake_project,
		const prep::cmake_project& prepared,
		const ext::provider_ref& provider
	) {
		sys::command_line command;
		command.m_args.emplace_back(req.tool_path(discovery::tool_role::cmake, "cmake").string());
		command.m_args.emplace_back("--build");
		command.m_args.emplace_back(prepared.m_build_dir.string());

		const auto target = cmake_project.m_install
			? cmake_project.m_install_target
			: provider.m_target;

		if (!target.empty()) {
			command.m_args.emplace_back("--target");
			command.m_args.emplace_back(target);
		}

		if (!cmake_project.m_build_config.empty()) {
			command.m_args.emplace_back("--config");
			command.m_args.emplace_back(cmake_project.m_build_config);
		}

		return command;
	}

	[[nodiscard]] inline sys::command_line cmake_touch_command(
		const build::request& req,
		const std::filesystem::path& path
	) {
		sys::command_line command;
		command.m_args.emplace_back(req.tool_path(discovery::tool_role::cmake, "cmake").string());
		command.m_args.emplace_back("-E");
		command.m_args.emplace_back("touch");
		command.m_args.emplace_back(path.string());
		return command;
	}
#endif // MGMK_ENABLE_EXT_CMAKE

	inline lower::usage context::use_libraries(
		const std::set<std::string>& libraries,
		std::string_view owner_name
	) {
		lower::usage result{};

		for (const auto& library_name : libraries) {
			const auto linked_id = m_project.find_library(library_name);

			mgmkassert(
				linked_id.has_value(),
				"mgmake lower: target '" + std::string{owner_name} +
					"' links unknown library '" + library_name + "'"
			);

			const lower::target& dep = lower_library(linked_id.value());

			if (dep.m_dag_target.has_value()) {
				result.m_dag_dependencies.emplace(dep.m_dag_target.value());
			}

			result.m_link_inputs.insert(
				result.m_link_inputs.end(),
				dep.m_linkable_artifacts.begin(),
				dep.m_linkable_artifacts.end()
			);

			result.m_include_dirs.insert_range(dep.m_include_dirs);
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
			m_libraries.at(id) = lower_provider_library(lib, std::move(usage));
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

	inline lower::target context::lower_interface_library(
		const spec::library& lib,
		lower::usage usage
	) {
		mgmkassert(
			lib.m_sources.empty(),
			"mgmake lower: interface library '" + lib.m_name + "' cannot have sources"
		);

		auto include_dirs = lib.include_dirs();
		include_dirs.insert_range(usage.m_include_dirs);
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
		include_dirs.insert_range(usage.m_include_dirs);

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
		include_dirs.insert_range(usage.m_include_dirs);

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
			command.m_args.emplace_back(m_emit.path(link_input).string());
		}

		for (const auto& flag : tc.link_flags()) {
			command.m_args.emplace_back(flag);
		}

		command.m_args.emplace_back("-o");
		command.m_args.emplace_back(shared_path.string());

		std::vector<dag::artifact::id> inputs = object_ids;
		inputs.insert(inputs.end(), usage.m_link_inputs.begin(), usage.m_link_inputs.end());
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
			lower_provider_executable(exe, std::move(usage));
			return;
		}
#endif // MGMK_ENABLE_EXT_CMAKE

		mgmkassert(
			not exe.m_sources.empty(),
			"mgmake lower: executable target '" + exe.m_name + "' has no sources"
		);

		auto include_dirs = exe.include_dirs();
		include_dirs.insert_range(usage.m_include_dirs);

		auto object_ids = lower_objects(exe, include_dirs, usage.m_usage_inputs);
		std::vector<dag::artifact::id> inputs = object_ids;
		inputs.insert(inputs.end(), usage.m_link_inputs.begin(), usage.m_link_inputs.end());
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
			command.m_args.emplace_back(m_emit.path(link_input).string());
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

#ifdef MGMK_ENABLE_EXT_CMAKE
	inline lower::cmake_target context::lower_cmake_target(
		const ext::provider_ref& provider,
		std::span<const dag::artifact::id> extra_outputs
	) {
		mgmkassert(provider.m_kind == ext::provider_kind::cmake, "mgmake lower: unsupported external provider kind");
		const auto* cmake_spec = m_project.get_cmake(m_project.find_cmake(provider.m_project).value());
		mgmkassert(cmake_spec != nullptr, "mgmake lower: unknown CMake project '" + provider.m_project + "'");

		const auto* prepared = m_prep.find_cmake_project(provider.m_project);
		mgmkassert(prepared != nullptr, "mgmake lower: CMake project '" + provider.m_project + "' was not prepared");

		const auto stamp = provider_target_stamp(request(), provider);
		const auto stamp_id = m_emit.generated(stamp);

		std::vector<dag::artifact::id> build_outputs{extra_outputs.begin(), extra_outputs.end()};

		if (build_outputs.empty()) {
			build_outputs.emplace_back(stamp_id);
		}

		m_emit.action(
			"Build CMake target " + provider.m_project + ":" + provider.m_target,
			"Builds external CMake target '" + provider.m_target + "' from project '" + provider.m_project + "'.",
			{},
			build_outputs,
			cmake_build_command(request(), *cmake_spec, *prepared, provider)
		);

		if (!extra_outputs.empty()) {
			m_emit.action(
				"Stamp CMake target " + provider.m_project + ":" + provider.m_target,
				"Marks external CMake target '" + provider.m_target + "' from project '" + provider.m_project + "' as ready.",
				build_outputs,
				{stamp_id},
				cmake_touch_command(request(), stamp)
			);
		}

		dag::target dag_target{
			"ext:cmake:" + provider.m_project + ":" + provider.m_target,
			{stamp_id},
			{}
		};

		return lower::cmake_target{
			.m_dag_target = m_emit.target(dag_target),
			.m_ready_stamp = stamp_id
		};
	}

	inline lower::target context::lower_provider_library(
		const spec::library& lib,
		lower::usage usage
	) {
		mgmkassert(lib.m_provider.has_value(), "mgmake lower: provider library has no provider");
		const auto& provider = lib.m_provider.value();
		const auto* prepared = m_prep.find_cmake_project(provider.m_project);
		mgmkassert(prepared != nullptr, "mgmake lower: CMake project '" + provider.m_project + "' was not prepared");

		auto include_dirs = lib.include_dirs();
		include_dirs.insert_range(usage.m_include_dirs);

		for (const auto& include_dir : lib.m_external_include_dirs) {
			include_dirs.emplace(resolve_rooted_path(*prepared, include_dir));
		}

		lower::target lowered{};
		std::vector<dag::artifact::id> provider_outputs{};

		if (lib.m_kind != spec::library::kind::interface) {
			std::filesystem::path artifact_path;

			if (lib.m_artifact.has_value()) {
				artifact_path = resolve_rooted_path(*prepared, lib.m_artifact.value());
			} else if (const auto* target = prepared->find_target(provider.m_target); target != nullptr && !target->m_artifact.empty()) {
				artifact_path = target->m_artifact;
			} else {
				artifact_path = conventional_provider_artifact(request(), *prepared, provider, lib.m_kind);
			}

			mgmkassert(!artifact_path.empty(), "mgmake lower: unable to resolve artifact for provider-backed library '" + lib.m_name + "'");
			const auto artifact_id = m_emit.generated(artifact_path);
			provider_outputs.emplace_back(artifact_id);
			lowered.m_linkable_artifacts.emplace_back(artifact_id);
		}

		auto provider_target = lower_cmake_target(provider, provider_outputs);
		usage.m_dag_dependencies.emplace(provider_target.m_dag_target);
		usage.m_usage_inputs.emplace_back(provider_target.m_ready_stamp);

		lowered.m_linkable_artifacts.insert(
			lowered.m_linkable_artifacts.end(),
			usage.m_link_inputs.begin(),
			usage.m_link_inputs.end()
		);
		lowered.m_include_dirs = std::move(include_dirs);
		lowered.m_usage_inputs = std::move(usage.m_usage_inputs);

		dag::target dag_target{
			lib.m_name,
			{},
			std::move(usage.m_dag_dependencies)
		};

		for (const auto artifact_id : lowered.m_linkable_artifacts) {
			dag_target.m_outputs.emplace(artifact_id);
		}

		lowered.m_dag_target = m_emit.target(dag_target);
		return lowered;
	}

	inline void context::lower_provider_executable(
		const spec::executable& exe,
		lower::usage usage
	) {
		mgmkassert(exe.m_provider.has_value(), "mgmake lower: provider executable has no provider");
		const auto& provider = exe.m_provider.value();
		const auto* prepared = m_prep.find_cmake_project(provider.m_project);
		mgmkassert(prepared != nullptr, "mgmake lower: CMake project '" + provider.m_project + "' was not prepared");

		std::filesystem::path artifact_path;

		if (exe.m_artifact.has_value()) {
			artifact_path = resolve_rooted_path(*prepared, exe.m_artifact.value());
		} else if (const auto* target = prepared->find_target(provider.m_target); target != nullptr && !target->m_artifact.empty()) {
			artifact_path = target->m_artifact;
		} else {
			artifact_path = conventional_provider_executable(request(), *prepared, provider);
		}

		mgmkassert(!artifact_path.empty(), "mgmake lower: unable to resolve artifact for provider-backed executable '" + exe.m_name + "'");
		const auto artifact_id = m_emit.generated(artifact_path);
		const std::array provider_outputs{artifact_id};
		auto provider_target = lower_cmake_target(provider, provider_outputs);
		usage.m_dag_dependencies.emplace(provider_target.m_dag_target);
		usage.m_usage_inputs.emplace_back(provider_target.m_ready_stamp);

		dag::target dag_target{
			exe.m_name,
			{artifact_id},
			std::move(usage.m_dag_dependencies)
		};

		m_emit.target(dag_target);
	}
#endif // MGMK_ENABLE_EXT_CMAKE
}

#endif // MGMK_LOWER_CONTEXT_IMPL_HXX
