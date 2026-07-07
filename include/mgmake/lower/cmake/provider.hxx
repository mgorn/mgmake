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

#include <algorithm>
#include <array>
#include <filesystem>
#include <set>
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

	inline void append_artifact_once(
		std::vector<dag::artifact::id>& artifacts,
		dag::artifact::id artifact
	) {
		if (std::find(artifacts.begin(), artifacts.end(), artifact) == artifacts.end()) {
			artifacts.emplace_back(artifact);
		}
	}

	[[nodiscard]] inline bool target_is_static(
		const ext::cmake::target& target,
		spec::library::kind fallback_kind
	) {
		if (target.m_type == "STATIC_LIBRARY") {
			return true;
		}

		return target.m_type.empty() && fallback_kind == spec::library::kind::static_lib;
	}

	[[nodiscard]] inline bool link_fragment_is_path(std::string_view fragment) {
		if (fragment.empty() || fragment.starts_with("-")) {
			return false;
		}

		if (fragment.starts_with("/") && fragment.find('/', 1) == std::string_view::npos) {
			return false;
		}

		return fragment.find('/') != std::string_view::npos ||
			fragment.find('\\') != std::string_view::npos;
	}

	[[nodiscard]] inline std::filesystem::path resolve_link_fragment_path(
		const prep::cmake::project& prepared,
		std::string_view fragment
	) {
		auto path = std::filesystem::path{std::string{fragment}};

		if (path.is_relative()) {
			path = prepared.m_build_dir / path;
		}

		return path;
	}

	inline void append_target_link_usage(
		dag::emitter& emit,
		const prep::cmake::project& prepared,
		const ext::cmake::target& target,
		spec::library::kind fallback_kind,
		std::vector<dag::artifact::id>& linkable_artifacts,
		std::vector<dag::artifact::id>& provider_outputs,
		std::set<std::string>& visited_targets
	);

	inline void append_link_fragment(
		dag::emitter& emit,
		const prep::cmake::project& prepared,
		std::string_view fragment,
		std::vector<dag::artifact::id>& linkable_artifacts,
		std::vector<dag::artifact::id>& provider_outputs
	) {
		if (fragment.empty()) {
			return;
		}

		if (link_fragment_is_path(fragment)) {
			const auto artifact = emit.generated(resolve_link_fragment_path(prepared, fragment));
			append_artifact_once(linkable_artifacts, artifact);
			append_artifact_once(provider_outputs, artifact);
			return;
		}

		const auto artifact = emit.file_artifact(
			dag::artifact::kind::system,
			std::filesystem::path{std::string{fragment}}
		);

		append_artifact_once(linkable_artifacts, artifact);
	}

	inline void append_target_ref(
		dag::emitter& emit,
		const prep::cmake::project& prepared,
		std::string_view target_id,
		std::vector<dag::artifact::id>& linkable_artifacts,
		std::vector<dag::artifact::id>& provider_outputs,
		std::set<std::string>& visited_targets
	) {
		if (target_id.empty()) {
			return;
		}

		const auto target_id_text = std::string{target_id};

		if (visited_targets.contains(target_id_text)) {
			return;
		}

		visited_targets.emplace(target_id_text);

		const auto* target = prepared.find_target_id(target_id_text);

		if (target == nullptr) {
			return;
		}

		if (prepared.m_usage_root == ext::path_root::build) {
			const auto artifact_path = target->primary_artifact();

			if (!artifact_path.empty()) {
				const auto artifact = emit.generated(artifact_path);
				append_artifact_once(linkable_artifacts, artifact);
				append_artifact_once(provider_outputs, artifact);
			}
		}

		append_target_link_usage(
			emit,
			prepared,
			*target,
			spec::library::kind::interface,
			linkable_artifacts,
			provider_outputs,
			visited_targets
		);
	}

	inline void append_link_entries(
		dag::emitter& emit,
		const prep::cmake::project& prepared,
		const std::vector<ext::cmake::link_entry>& entries,
		std::vector<dag::artifact::id>& linkable_artifacts,
		std::vector<dag::artifact::id>& provider_outputs,
		std::set<std::string>& visited_targets
	) {
		for (const auto& entry : entries) {
			switch (entry.m_kind) {
				case ext::cmake::link_entry_kind::fragment:
					append_link_fragment(
						emit,
						prepared,
						entry.m_value,
						linkable_artifacts,
						provider_outputs
					);
					break;

				case ext::cmake::link_entry_kind::target_id:
					append_target_ref(
						emit,
						prepared,
						entry.m_value,
						linkable_artifacts,
						provider_outputs,
						visited_targets
					);
					break;
			}
		}
	}

	inline void append_target_link_usage(
		dag::emitter& emit,
		const prep::cmake::project& prepared,
		const ext::cmake::target& target,
		spec::library::kind fallback_kind,
		std::vector<dag::artifact::id>& linkable_artifacts,
		std::vector<dag::artifact::id>& provider_outputs,
		std::set<std::string>& visited_targets
	) {
		if (target_is_static(target, fallback_kind)) {
			append_link_entries(
				emit,
				prepared,
				target.m_link_entries,
				linkable_artifacts,
				provider_outputs,
				visited_targets
			);
		}

		append_link_entries(
			emit,
			prepared,
			target.m_interface_link_entries,
			linkable_artifacts,
			provider_outputs,
			visited_targets
		);
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
			"ext:cmake:" + provider.m_project + ":" + provider.m_target,
			{stamp_id},
			{}
		};

		return lower::provider_build{
			.m_dag_target = ctx.m_emit.target(dag_target),
			.m_ready_stamp = stamp_id
		};
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

		lower::target lowered{};
		std::vector<dag::artifact::id> provider_outputs{};
		std::filesystem::path artifact_path;

		if (lib.m_kind != spec::library::kind::interface) {
			artifact_path = resolve_library_artifact(
				ctx.request(),
				*prepared,
				provider,
				lib
			);

			mgmkassert(!artifact_path.empty(), "mgmake lower: unable to resolve artifact for provider-backed library '" + lib.m_name + "'");
			const auto artifact_id = ctx.m_emit.generated(artifact_path);
			provider_outputs.emplace_back(artifact_id);
			lowered.m_linkable_artifacts.emplace_back(artifact_id);
		}

		const ext::cmake::target* cmake_target = prepared->find_target(provider.m_target);

		if (cmake_target == nullptr && !artifact_path.empty()) {
			cmake_target = prepared->find_target_artifact(artifact_path);
		}

		mgmkassert(
			cmake_target != nullptr,
			"mgmake lower: CMake provider target '" + provider.m_target +
				"' from project '" + provider.m_project +
				"' was not found in the CMake File API codemodel"
		);

		const auto old_linkable_count = lowered.m_linkable_artifacts.size();
		std::set<std::string> visited_cmake_targets;

		if (!cmake_target->m_id.empty()) {
			visited_cmake_targets.emplace(cmake_target->m_id);
		}

		append_target_link_usage(
			ctx.m_emit,
			*prepared,
			*cmake_target,
			lib.m_kind,
			lowered.m_linkable_artifacts,
			provider_outputs,
			visited_cmake_targets
		);

		mgmkassert(
			!cmake_target->has_link_usage() || lowered.m_linkable_artifacts.size() != old_linkable_count,
			"mgmake lower: CMake provider target '" + provider.m_target +
				"' was found and has link usage, but no CMake link inputs were imported"
		);

		// The provider build stamp becomes a usage input so dependents wait for the CMake target.
		auto provider_target = lower_provider_build(ctx, provider, provider_outputs);
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

		lowered.m_dag_target = ctx.m_emit.target(dag_target);
		return lowered;
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

		ctx.m_emit.target(dag_target);
	}
}

#endif // MGMK_ENABLE_EXT_CMAKE

#endif // MGMK_LOWER_CMAKE_PROVIDER_HXX
