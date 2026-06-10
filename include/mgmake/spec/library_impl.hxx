#pragma once

#ifndef MGMK_SPEC_LIBRARY_IMPL_HXX
#define MGMK_SPEC_LIBRARY_IMPL_HXX

#include "library.hxx"
#include "project.hxx"

#include <filesystem>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace mgmake::spec {
	template<typename target_t>
	inline std::vector<dag::artifact::id> graph_objects(
		dag::graph& result,
		const build::request& req,
		const target_t& target,
		const std::set<std::filesystem::path>& include_dirs
	) {
		const auto& tc = req.toolchain();

		std::vector<dag::artifact::id> object_ids{};
		object_ids.reserve(target.m_sources.size());

		std::size_t source_index = 0;

		for (const auto& source : target.m_sources) {
			auto source_id = result.create_artifact(
				dag::artifact::kind::source,
				source
			);

			std::filesystem::path object_path =
				req.build_dir() /
				"obj" /
				target.m_name /
				(std::to_string(source_index++) +
#if defined(MGMK_PLATFORM_WINDOWS)
					".obj"
#else
					".o"
#endif
				);

			auto object_id = result.create_artifact(
				dag::artifact::kind::generated,
				object_path
			);

			sys::command_line command{};
			command.m_args.emplace_back(tc.cxx());

			for (const auto& flag : tc.compile_flags()) {
				command.m_args.emplace_back(flag);
			}

			for (const auto& flag : tc.cxx_flags()) {
				command.m_args.emplace_back(flag);
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

			result.create_action(
				std::string{"Compile "} + source.string(),
				std::string{"Compiles source file '"} + source.string() + "' for target '" + target.m_name + "'.",
				std::vector<dag::artifact::id>{ source_id },
				std::vector<dag::artifact::id>{ object_id },
				false,
				std::move(command),
				std::filesystem::path{}
			);

			object_ids.emplace_back(object_id);
		}

		return object_ids;
	}

	inline dag::target::id spec::library::lower(
		dag::graph& result,
		const build::request& req,
		const spec::project& proj,
		std::vector<std::optional<dag::target::id>>& library_target_ids,
		std::vector<std::vector<dag::artifact::id>>& library_link_outputs,
		std::vector<unsigned char>& library_states
	) const {
		const auto this_id = proj.find_library(m_name);

		mgmkassert(
			this_id.has_value(),
			"mgmake spec: cannot lower library '" + m_name + "' because it is not part of the project"
		);

		const auto id = this_id.value();

		mgmkassert(
			id < library_target_ids.size(),
			"mgmake spec: invalid library target cache size"
		);

		mgmkassert(
			id < library_link_outputs.size(),
			"mgmake spec: invalid library link output cache size"
		);

		mgmkassert(
			id < library_states.size(),
			"mgmake spec: invalid library state cache size"
		);

		if (library_states.at(id) == 2) {
			return library_target_ids.at(id).value();
		}

		mgmkassert(
			library_states.at(id) != 1,
			"mgmake spec: cyclic library dependency involving '" + m_name + "'"
		);

		library_states.at(id) = 1;

		std::set<dag::target::id> target_dependencies{};
		std::vector<dag::artifact::id> link_inputs{};

		for (const auto& linked_library : linked_libraries()) {
			const auto linked_id = proj.find_library(linked_library);

			mgmkassert(
				linked_id.has_value(),
				"mgmake spec: library '" + m_name +
					"' links unknown library '" + linked_library + "'"
			);

			const auto* dep = proj.get_library(linked_id.value());

			mgmkassert(
				dep != nullptr,
				"mgmake spec: failed to fetch linked library '" + linked_library + "'"
			);

			auto dep_target_id = dep->lower(
				result,
				req,
				proj,
				library_target_ids,
				library_link_outputs,
				library_states
			);

			target_dependencies.emplace(dep_target_id);

			for (auto artifact_id : library_link_outputs.at(linked_id.value())) {
				link_inputs.emplace_back(artifact_id);
			}
		}

		auto target = graph(
			result,
			req,
			proj,
			link_inputs,
			std::move(target_dependencies)
		);

		auto own_outputs = target.m_outputs;

		auto target_id = result.create_target(std::move(target));

		library_target_ids.at(id) = target_id;

		for (auto artifact_id : own_outputs) {
			library_link_outputs.at(id).emplace_back(artifact_id);
		}

		for (auto artifact_id : link_inputs) {
			library_link_outputs.at(id).emplace_back(artifact_id);
		}

		library_states.at(id) = 2;

		return target_id;
	}

	inline dag::target spec::library::graph(
		dag::graph& result,
		const build::request& req,
		const spec::project& proj,
		const std::vector<dag::artifact::id>& link_inputs,
		std::set<dag::target::id> target_dependencies
	) const {
		const auto& tc = req.toolchain();

		mgmkassert(not m_name.empty(), "mgmake spec: library target has no name");

		switch (m_kind) {
			case spec::library::kind::interface: {
				(void)result;
				(void)req;
				(void)proj;
				(void)link_inputs;

				mgmkassert(
					m_sources.empty(),
					"mgmake spec: interface library '" + m_name + "' cannot have sources"
				);

				return dag::target{
					m_name,
					{},
					std::move(target_dependencies)
				};
			}

			case spec::library::kind::static_lib: {
				(void)link_inputs;

				mgmkassert(
					not m_sources.empty(),
					"mgmake spec: static library '" + m_name + "' has no sources"
				);

				auto object_ids = graph_objects(
					result,
					req,
					*this,
					proj.collect_includes(*this)
				);

				std::filesystem::path archive_path;

				switch (tc.dialect()) {
					case build::toolchain::dialect::gcc:
						archive_path = req.build_dir() / "lib" / ("lib" + m_name + ".a");
						break;

					case build::toolchain::dialect::msvc:
						archive_path = req.build_dir() / "lib" / (m_name + ".lib");
						break;
				}

				auto archive_id = result.create_artifact(
					dag::artifact::kind::generated,
					archive_path
				);

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
							command.m_args.emplace_back(result.artifact(object_id).m_path.string());
						}

						break;
					}

					case build::toolchain::dialect::msvc: {
						for (const auto& flag : tc.archive_flags()) {
							command.m_args.emplace_back(flag);
						}

						command.m_args.emplace_back(std::string{"/OUT:"} + archive_path.string());

						for (auto object_id : object_ids) {
							command.m_args.emplace_back(result.artifact(object_id).m_path.string());
						}

						break;
					}
				}

				result.create_action(
					std::string{"Build static library "} + m_name,
					std::string{"Builds static library target '"} + m_name + "'.",
					object_ids,
					std::vector<dag::artifact::id>{ archive_id },
					false,
					std::move(command),
					std::filesystem::path{}
				);

				return dag::target{
					m_name,
					{ archive_id },
					std::move(target_dependencies)
				};
			}

			case spec::library::kind::shared_lib: {
				mgmkassert(
					not m_sources.empty(),
					"mgmake spec: shared library '" + m_name + "' has no sources"
				);

				mgmkassert(
					tc.dialect() == build::toolchain::dialect::gcc,
					"mgmake spec: shared library lowering is currently only implemented for GCC-like toolchains"
				);

				auto object_ids = graph_objects(
					result,
					req,
					*this,
					proj.collect_includes(*this)
				);

				std::filesystem::path shared_path;

#if defined(__APPLE__)
				shared_path = req.build_dir() / "lib" / ("lib" + m_name + ".dylib");
#else
				shared_path = req.build_dir() / "lib" / ("lib" + m_name + ".so");
#endif

				auto shared_id = result.create_artifact(
					dag::artifact::kind::generated,
					shared_path
				);

				sys::command_line command{};
				command.m_args.emplace_back(tc.linker());

#if defined(__APPLE__)
				command.m_args.emplace_back("-dynamiclib");
#else
				command.m_args.emplace_back("-shared");
#endif

				for (auto object_id : object_ids) {
					command.m_args.emplace_back(result.artifact(object_id).m_path.string());
				}

				for (auto link_input : link_inputs) {
					command.m_args.emplace_back(result.artifact(link_input).m_path.string());
				}

				for (const auto& flag : tc.link_flags()) {
					command.m_args.emplace_back(flag);
				}

				command.m_args.emplace_back("-o");
				command.m_args.emplace_back(shared_path.string());

				std::vector<dag::artifact::id> inputs = object_ids;
				inputs.insert(inputs.end(), link_inputs.begin(), link_inputs.end());

				result.create_action(
					std::string{"Build shared library "} + m_name,
					std::string{"Builds shared library target '"} + m_name + "'.",
					std::move(inputs),
					std::vector<dag::artifact::id>{ shared_id },
					false,
					std::move(command),
					std::filesystem::path{}
				);

				return dag::target{
					m_name,
					{ shared_id },
					std::move(target_dependencies)
				};
			}
		}

		std::unreachable();
	}
}

#endif
