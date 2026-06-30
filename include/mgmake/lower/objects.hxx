#pragma once

#ifndef MGMK_LOWER_OBJECTS_HXX
#define MGMK_LOWER_OBJECTS_HXX

#include "context.hxx"
#include "../build/target.hxx"
#include "../build/toolchain.hxx"
#include "../dep/file.hxx"
#include "../detail/assert.hxx"
#include "../discovery/source_role.hxx"
#include "../sys/command_line.hxx"

#include <cstddef>
#include <filesystem>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <utility>
#include <vector>

// Object lowering turns each source file into a compile action and wires depfile-discovered headers into the DAG.

namespace mgmake::lower {
	template<typename target_t>
	inline std::vector<dag::artifact::id> context::lower_objects(
		const target_t& target,
		const std::set<std::filesystem::path>& include_dirs,
		std::span<const dag::artifact::id> usage_inputs
	) {
		const auto& tc = toolchain();

		std::vector<dag::artifact::id> object_ids{};
		object_ids.reserve(target.m_sources.size());

		std::size_t source_index = 0;

		// Object paths are index-based to avoid leaking source-tree layout into build artifacts.
		for (const auto& source : target.m_sources) {
			auto source_id = m_emit.source(source);
			const std::string_view object_extension =
				tc.dialect() == build::toolchain::dialect::msvc ? ".obj" : ".o";

			std::filesystem::path object_path =
				request().build_dir() /
				"obj" /
				target.m_name /
				(std::to_string(source_index++) + std::string{ object_extension });

			auto object_id = m_emit.generated(object_path);

			std::vector<dag::artifact::id> compile_inputs{source_id};
			compile_inputs.insert(
				compile_inputs.end(),
				usage_inputs.begin(),
				usage_inputs.end()
			);

			sys::command_line command{};
			const auto role = discovery::source_tool_role(source);
			const bool is_c_source = role == discovery::tool_role::c_compiler;
			const bool is_cxx_source = role == discovery::tool_role::cxx_compiler;
			const bool is_resource_source = role == discovery::tool_role::resource_compiler;
			const bool is_compiled_source = is_c_source || is_cxx_source;
			std::optional<dep::file> depfile{};

			mgmkassert(
				role != discovery::tool_role::midl_compiler,
				"mgmake lower: IDL source lowering requires a generated-source model and is not implemented yet"
			);

			auto tool_path = request().tool_path(role);

			if (tool_path.empty()) {
				if (is_c_source) {
					tool_path = tc.cc();
				} else if (is_resource_source) {
					tool_path = tc.tool(discovery::tool_role::resource_compiler);
				} else {
					tool_path = tc.cxx();
				}
			}

			command.m_args.emplace_back(tool_path.string());

			if (is_resource_source) {
				switch (tc.dialect()) {
					case build::toolchain::dialect::gcc:
						command.m_args.emplace_back(source.string());
						command.m_args.emplace_back(object_path.string());
						break;

					case build::toolchain::dialect::msvc:
						command.m_args.emplace_back(std::string{"/fo"} + object_path.string());
						command.m_args.emplace_back(source.string());
						break;
				}

				m_emit.action(
					std::string{"Compile "} + source.string(),
					std::string{"Compiles source file '"} + source.string() + "' for target '" + target.m_name + "'.",
					compile_inputs,
					{ object_id },
					command
				);

				object_ids.emplace_back(object_id);
				continue;
			}

			build::append_target_args(command, tc, request());

			for (const auto& arg : request().compile_prefix_args()) {
				command.m_args.emplace_back(arg);
			}

			for (const auto& flag : tc.compile_flags()) {
				command.m_args.emplace_back(flag);
			}

			if (is_c_source) {
				for (const auto& flag : tc.c_flags()) {
					command.m_args.emplace_back(flag);
				}
			} else if (is_cxx_source) {
				for (const auto& flag : tc.cxx_flags()) {
					command.m_args.emplace_back(flag);
				}
			}

			for (const auto& include_dir : include_dirs) {
				switch (tc.dialect()) {
					case build::toolchain::dialect::gcc:
						command.m_args.emplace_back(std::string{"-I"} + include_dir.string());
						break;

					case build::toolchain::dialect::msvc:
						command.m_args.emplace_back(std::string{"/I"} + include_dir.string());
						break;
				}
			}

			if (is_compiled_source) {
				switch (tc.dialect()) {
					case build::toolchain::dialect::gcc:
						depfile = dep::file{};
						depfile->m_format = dep::format::gcc;
						depfile->m_path = object_path;
						depfile->m_path += ".d";
						depfile->m_target = object_path;

						// Ask GCC/Clang to emit a Make-style depfile beside the object.
						// Ninja consumes this for rebuild correctness, and mgmake consumes
						// the previous invocation's file during graph lowering.
						command.m_args.emplace_back("-MMD");
						command.m_args.emplace_back("-MP");
						command.m_args.emplace_back("-MF");
						command.m_args.emplace_back(depfile->m_path.string());
						command.m_args.emplace_back("-MT");
						command.m_args.emplace_back(object_path.string());
						break;

					case build::toolchain::dialect::msvc:
						depfile = dep::file{};
						depfile->m_format = dep::format::msvc_all;
						depfile->m_path = object_path;
						depfile->m_path += ".json";
						depfile->m_target = object_path;

						// /showIncludes is for Ninja-native rebuild correctness.
						// /sourceDependencies writes JSON that mgmake can consume later.
						command.m_args.emplace_back("/showIncludes");
						command.m_args.emplace_back("/sourceDependencies");
						command.m_args.emplace_back(depfile->m_path.string());
						break;
				}
			}

			switch (tc.dialect()) {
				case build::toolchain::dialect::gcc:
					command.m_args.emplace_back("-c");
					command.m_args.emplace_back(source.string());
					command.m_args.emplace_back("-o");
					command.m_args.emplace_back(object_path.string());
					break;

				case build::toolchain::dialect::msvc:
					command.m_args.emplace_back("/c");
					command.m_args.emplace_back(source.string());
					command.m_args.emplace_back(std::string{"/Fo"} + object_path.string());
					break;
			}

			// Discovered headers are kept separate from explicit inputs so visualizers can show them differently.
			std::vector<dag::artifact::id> discovered_dependencies{};
			std::set<dag::artifact::id> discovered_dependency_ids{};

			if (request().m_discover_source_dependencies) {
				if (depfile.has_value()) {
					// This consumes dependency information from a previous build invocation.
					// A clean build will usually have no depfile yet; the compiler emits it
					// during the build, and the next graph/build invocation can materialize it.
					m_deps.consume(*depfile);

					for (const auto& dependency : m_deps.dependencies_for(depfile->m_target)) {
						const auto dependency_id = m_emit.header(dependency);

						bool already_input = false;

						for (const auto input : compile_inputs) {
							if (input == dependency_id) {
								already_input = true;
								break;
							}
						}

						if (already_input) {
							continue;
						}

						if (!discovered_dependency_ids.emplace(dependency_id).second) {
							continue;
						}

						discovered_dependencies.emplace_back(dependency_id);
					}
				}
			}
			

			const auto action_id = m_emit.action(
				std::string{"Compile "} + source.string(),
				std::string{"Compiles source file '"} + source.string() + "' for target '" + target.m_name + "'.",
				compile_inputs,
				{ object_id },
				command
			);

			auto& action = m_emit.m_graph.action(action_id);
			action.m_discovered_dependencies = std::move(discovered_dependencies);
			action.m_depfile = std::move(depfile);

			object_ids.emplace_back(object_id);
		}

		return object_ids;
	}
}

#endif // MGMK_LOWER_OBJECTS_HXX
