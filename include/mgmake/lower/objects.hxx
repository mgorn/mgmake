#pragma once

#ifndef MGMK_LOWER_OBJECTS_HXX
#define MGMK_LOWER_OBJECTS_HXX

#include "context.hxx"
#include "../build/target.hxx"
#include "../build/toolchain.hxx"
#include "../sys/command_line.hxx"

#include <cstddef>
#include <filesystem>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace mgmake::lower {
	template<typename target_t>
	inline std::vector<dag::artifact::id> context::lower_objects(
		const target_t& target,
		const std::set<std::filesystem::path>& include_dirs
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

			sys::command_line command{};
			const bool is_c_source = source.extension().string() == ".c";
			command.m_args.emplace_back(is_c_source ? tc.cc() : tc.cxx());

			build::append_target_args(command, tc, request());

			for (const auto& flag : tc.compile_flags()) {
				command.m_args.emplace_back(flag);
			}

			if (is_c_source) {
				for (const auto& flag : tc.c_flags()) {
					command.m_args.emplace_back(flag);
				}
			} else {
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
				{ source_id },
				{ object_id },
				std::move(command)
			);

			object_ids.emplace_back(object_id);
		}

		return object_ids;
	}
}

#endif
