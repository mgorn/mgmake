#pragma once

#ifndef MGMK_LOWER_OBJECTS_HXX
#define MGMK_LOWER_OBJECTS_HXX

#include "context.hxx"
#include "../build/target.hxx"
#include "../build/toolchain.hxx"
#include "../detail/assert.hxx"
#include "../discovery/source_role.hxx"
#include "../sys/command_line.hxx"

#include <cstddef>
#include <filesystem>
#include <set>
#include <span>
#include <string>
#include <utility>
#include <vector>

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

			m_emit.action(
				std::string{"Compile "} + source.string(),
				std::string{"Compiles source file '"} + source.string() + "' for target '" + target.m_name + "'.",
				compile_inputs,
				{ object_id },
				command
			);

			object_ids.emplace_back(object_id);
		}

		return object_ids;
	}
}

#endif // MGMK_LOWER_OBJECTS_HXX
