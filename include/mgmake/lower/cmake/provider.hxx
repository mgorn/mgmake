#pragma once

#ifndef MGMK_LOWER_CMAKE_PROVIDER_HXX
#define MGMK_LOWER_CMAKE_PROVIDER_HXX

#include "../context.hxx"
#include "../provider_build.hxx"
#include "../../build/artifact_names.hxx"
#include "../../build/target.hxx"
#include "../../detail/assert.hxx"
#include "../../spec/project.hxx"
#include "../../sys/command_line.hxx"

#include <array>
#include <cstdio>
#include <filesystem>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#ifdef MGMK_ENABLE_EXT_CMAKE

namespace mgmake::lower::cmake {
	[[nodiscard]] inline std::filesystem::path conventional_library_artifact(
		const build::request& req,
		const prep::cmake::project& cmake_project,
		const ext::provided_target_ref& provider,
		spec::library::kind kind
	) {
		const auto root = cmake_project.root(ext::path_root::usage);

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

	[[nodiscard]] inline std::filesystem::path conventional_executable_artifact(
		const build::request& req,
		const prep::cmake::project& cmake_project,
		const ext::provided_target_ref& provider
	) {
		const auto root = cmake_project.root(ext::path_root::usage);
		return root / "bin" / (provider.m_target + std::string{build::executable_extension(req.target_platform())});
	}

	[[nodiscard]] inline std::filesystem::path resolve_library_artifact(
		const build::request& req,
		const prep::cmake::project& cmake_project,
		const ext::provided_target_ref& provider,
		const spec::library& lib
	) {
		if (lib.m_artifact.has_value()) {
			return cmake_project.resolve(lib.m_artifact.value());
		}

		if (cmake_project.m_usage_root == ext::path_root::build) {
			const auto* target = cmake_project.find_target(provider.m_target);

			if (target != nullptr) {
				const auto artifact = target->primary_artifact();

				if (!artifact.empty()) {
					return artifact;
				}
			}
		}

		return conventional_library_artifact(req, cmake_project, provider, lib.m_kind);
	}

	[[nodiscard]] inline std::filesystem::path resolve_executable_artifact(
		const build::request& req,
		const prep::cmake::project& cmake_project,
		const ext::provided_target_ref& provider,
		const spec::executable& exe
	) {
		if (exe.m_artifact.has_value()) {
			return cmake_project.resolve(exe.m_artifact.value());
		}

		if (cmake_project.m_usage_root == ext::path_root::build) {
			const auto* target = cmake_project.find_target(provider.m_target);

			if (target != nullptr) {
				const auto artifact = target->primary_artifact();

				if (!artifact.empty()) {
					return artifact;
				}
			}
		}

		return conventional_executable_artifact(req, cmake_project, provider);
	}

	[[nodiscard]] inline std::filesystem::path provider_stamp(
		const build::request& req,
		const ext::provided_target_ref& provider
	) {
		return req.build_dir() / "ext" / "stamp" /
			(provider.m_project + ".cmake.build." + provider.m_target);
	}

	[[nodiscard]] inline sys::command_line build_command(
		const build::request& req,
		const ext::cmake::project& cmake_project,
		const prep::cmake::project& prepared,
		const ext::provided_target_ref& provider
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

	[[nodiscard]] inline sys::command_line touch_command(
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

	[[nodiscard]] inline lower::provider_build lower_provider_build(
		lower::context& ctx,
		const ext::provided_target_ref& provider,
		std::span<const dag::artifact::id> extra_outputs
	) {
		mgmkassert(provider.m_kind == ext::provider_kind::cmake, "mgmake lower: unsupported external provider kind");
		const auto cmake_id = ctx.m_project.find_cmake(provider.m_project);
		mgmkassert(cmake_id.has_value(), "mgmake lower: unknown CMake project '" + provider.m_project + "'");
		const auto* cmake_spec = ctx.m_project.get_cmake(cmake_id.value());
		mgmkassert(cmake_spec != nullptr, "mgmake lower: unknown CMake project '" + provider.m_project + "'");

		const auto* prepared = ctx.m_prep.find_cmake_project(provider.m_project);
		mgmkassert(prepared != nullptr, "mgmake lower: CMake project '" + provider.m_project + "' was not prepared");

		const auto stamp = provider_stamp(ctx.request(), provider);
		const auto stamp_id = ctx.m_emit.generated(stamp);
		const auto target_name = "ext:cmake:" + provider.m_project + ":" + provider.m_target;

		if (const auto existing = ctx.m_emit.graph().find_target(target_name)) {
			return lower::provider_build{
				.m_dag_target = existing.value(),
				.m_ready_stamp = stamp_id
			};
		}

		std::vector<dag::artifact::id> build_outputs{extra_outputs.begin(), extra_outputs.end()};

		if (build_outputs.empty()) {
			build_outputs.emplace_back(stamp_id);
		}

		ctx.m_emit.action(
			"Build CMake target " + provider.m_project + ":" + provider.m_target,
			"Builds CMake target '" + provider.m_target + "' from project '" + provider.m_project + "'.",
			{},
			build_outputs,
			build_command(ctx.request(), *cmake_spec, *prepared, provider)
		);

		if (!extra_outputs.empty()) {
			ctx.m_emit.action(
				"Stamp CMake target " + provider.m_project + ":" + provider.m_target,
				"Marks CMake target '" + provider.m_target + "' from project '" + provider.m_project + "' as ready.",
				build_outputs,
				{stamp_id},
				touch_command(ctx.request(), stamp)
			);
		}

		dag::target dag_target{
			target_name,
			{stamp_id},
			{}
		};

		return lower::provider_build{
			.m_dag_target = ctx.m_emit.create_target(dag_target),
			.m_ready_stamp = stamp_id
		};
	}

	[[nodiscard]] inline lower::target lower_provider_library(
		lower::context& ctx,
		const ext::provided_target_ref& provider,
		const prep::cmake::project& prepared,
		const ext::cmake::target& cmake_target,
		std::filesystem::path artifact_path,
		lower::usage usage,
		std::string_view target_name
	) {
		lower::target lowered{};
		std::vector<dag::artifact::id> provider_outputs{};

		if (!artifact_path.empty()) {
			const auto artifact = ctx.m_emit.generated(artifact_path);
			lowered.m_linkable_artifacts.emplace_back(artifact);
			provider_outputs.emplace_back(artifact);
		}

		for (const auto* entries : {&cmake_target.m_link_entries, &cmake_target.m_interface_link_entries}) {
			for (const auto& entry : *entries) {
				if (entry.m_value.empty()) {
					continue;
				}

				if (entry.m_kind == ext::cmake::link_entry_kind::target_id) {
					const auto* dependency = prepared.find_target_id(entry.m_value);

					if (dependency == nullptr) {
						std::println(
							stderr,
							"mgmake lower: warning: CMake target '{}' from project '{}' references unresolved target id '{}'",
							cmake_target.m_name,
							prepared.m_name,
							entry.m_value
						);
						continue;
					}

					auto dependency_provider = provider;
					dependency_provider.m_target = dependency->m_name;

					auto dependency_target = lower_provider_library(
						ctx,
						dependency_provider,
						prepared,
						*dependency,
						dependency->primary_artifact(),
						lower::usage{},
						{}
					);

					if (dependency_target.m_dag_target.has_value()) {
						usage.m_dag_dependencies.emplace(dependency_target.m_dag_target.value());
					}

					usage.m_link_inputs.insert(
						usage.m_link_inputs.end(),
						dependency_target.m_linkable_artifacts.begin(),
						dependency_target.m_linkable_artifacts.end()
					);
					usage.m_include_dirs.insert(
						dependency_target.m_include_dirs.begin(),
						dependency_target.m_include_dirs.end()
					);
					usage.m_usage_inputs.insert(
						usage.m_usage_inputs.end(),
						dependency_target.m_usage_inputs.begin(),
						dependency_target.m_usage_inputs.end()
					);
					continue;
				}

				auto path = std::filesystem::path{entry.m_value};

				if (!path.has_parent_path()) {
					auto system = ctx.lower_system_library(entry.m_value);

					if (system.m_dag_target.has_value()) {
						usage.m_dag_dependencies.emplace(system.m_dag_target.value());
					}

					usage.m_link_inputs.insert(
						usage.m_link_inputs.end(),
						system.m_linkable_artifacts.begin(),
						system.m_linkable_artifacts.end()
					);
					usage.m_include_dirs.insert(system.m_include_dirs.begin(), system.m_include_dirs.end());
					usage.m_usage_inputs.insert(
						usage.m_usage_inputs.end(),
						system.m_usage_inputs.begin(),
						system.m_usage_inputs.end()
					);
					continue;
				}

				if (path.is_relative()) {
					path = prepared.m_build_dir / path;
				}

				const auto artifact = ctx.m_emit.generated(path);
				usage.m_link_inputs.emplace_back(artifact);
				provider_outputs.emplace_back(artifact);
			}
		}

		if (!provider_outputs.empty()) {
			auto provider_target = lower_provider_build(ctx, provider, provider_outputs);
			usage.m_dag_dependencies.emplace(provider_target.m_dag_target);
			usage.m_usage_inputs.emplace_back(provider_target.m_ready_stamp);

			if (target_name.empty()) {
				lowered.m_dag_target = provider_target.m_dag_target;
			}
		}

		lowered.m_linkable_artifacts.insert(
			lowered.m_linkable_artifacts.end(),
			usage.m_link_inputs.begin(),
			usage.m_link_inputs.end()
		);
		lowered.m_include_dirs = std::move(usage.m_include_dirs);
		lowered.m_usage_inputs = std::move(usage.m_usage_inputs);

		if (target_name.empty()) {
			return lowered;
		}

		dag::target dag_target{
			std::string{target_name},
			{},
			std::move(usage.m_dag_dependencies)
		};

		for (const auto artifact : provider_outputs) {
			dag_target.m_outputs.emplace(artifact);
		}

		if (const auto existing = ctx.m_emit.graph().find_target(dag_target.m_name)) {
			lowered.m_dag_target = existing.value();
		} else {
			lowered.m_dag_target = ctx.m_emit.create_target(dag_target);
		}

		return lowered;
	}

	[[nodiscard]] inline lower::target lower_provider_library(
		lower::context& ctx,
		const spec::library& lib,
		lower::usage usage
	) {
		mgmkassert(lib.m_provider.has_value(), "mgmake lower: provider library has no provider");
		const auto& provider = lib.m_provider.value();
		const auto* prepared = ctx.m_prep.find_cmake_project(provider.m_project);
		mgmkassert(prepared != nullptr, "mgmake lower: CMake project '" + provider.m_project + "' was not prepared");

		auto include_dirs = lib.include_dirs();
		include_dirs.insert(usage.m_include_dirs.begin(), usage.m_include_dirs.end());

		for (const auto& include_dir : lib.m_external_include_dirs) {
			include_dirs.emplace(prepared->resolve(include_dir));
		}

		usage.m_include_dirs = std::move(include_dirs);

		std::filesystem::path artifact_path;

		if (lib.m_kind != spec::library::kind::interface) {
			artifact_path = resolve_library_artifact(
				ctx.request(),
				*prepared,
				provider,
				lib
			);

			mgmkassert(!artifact_path.empty(), "mgmake lower: unable to resolve artifact for provider-backed library '" + lib.m_name + "'");
		}

		const ext::cmake::target* cmake_target = prepared->find_target(provider.m_target);

		if (cmake_target == nullptr) {
			cmake_target = prepared->find_target_name_or_unqualified_alias(provider.m_target);
		}

		if (cmake_target == nullptr && !artifact_path.empty()) {
			cmake_target = prepared->find_target_artifact(artifact_path);
		}

		mgmkassert(
			cmake_target != nullptr,
			"mgmake lower: CMake provider target '" + provider.m_target +
				"' from project '" + provider.m_project +
				"' was not found in the CMake File API codemodel"
		);

		return lower_provider_library(
			ctx,
			provider,
			*prepared,
			*cmake_target,
			std::move(artifact_path),
			std::move(usage),
			lib.m_name
		);
	}

	inline void lower_provider_executable(
		lower::context& ctx,
		const spec::executable& exe,
		lower::usage usage
	) {
		mgmkassert(exe.m_provider.has_value(), "mgmake lower: provider executable has no provider");
		const auto& provider = exe.m_provider.value();
		const auto* prepared = ctx.m_prep.find_cmake_project(provider.m_project);
		mgmkassert(prepared != nullptr, "mgmake lower: CMake project '" + provider.m_project + "' was not prepared");

		const auto artifact_path = resolve_executable_artifact(
			ctx.request(),
			*prepared,
			provider,
			exe
		);

		mgmkassert(!artifact_path.empty(), "mgmake lower: unable to resolve artifact for provider-backed executable '" + exe.m_name + "'");
		const auto artifact_id = ctx.m_emit.generated(artifact_path);
		const std::array provider_outputs{artifact_id};
		auto provider_target = lower_provider_build(ctx, provider, provider_outputs);
		usage.m_dag_dependencies.emplace(provider_target.m_dag_target);
		usage.m_usage_inputs.emplace_back(provider_target.m_ready_stamp);

		dag::target dag_target{
			exe.m_name,
			{artifact_id},
			std::move(usage.m_dag_dependencies)
		};

		ctx.m_emit.create_target(dag_target);
	}
}

#endif // MGMK_ENABLE_EXT_CMAKE

#endif // MGMK_LOWER_CMAKE_PROVIDER_HXX
