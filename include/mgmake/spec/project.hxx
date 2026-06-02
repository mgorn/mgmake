#pragma once

#ifndef MGMK_SPEC_PROJECT_HXX
#define MGMK_SPEC_PROJECT_HXX

#include "../backend/traits.hxx"
#include "../build/request.hxx"
#include "../dag/graph.hxx"
#include "executable.hxx"
#include "library.hxx"

#include <string>
#include <string_view>
#include <vector>

namespace mgmake::spec {
	struct project {
		std::string m_name;
		std::vector<spec::executable> m_executables;
		std::vector<spec::library> m_libraries;

		inline constexpr project& add_target(const spec::executable& exe) {
			mgmkassert(not exe.m_name.empty(), "mgmake spec: executable target has no name");
            mgmkassert(find_library(exe.m_name) == nullptr, "mgmake spec: target name conflict '" + exe.m_name + "'");

			m_executables.emplace_back(exe);
			return *this;
		}
		inline constexpr project& add_target(const spec::library& lib) {
			mgmkassert(not lib.m_name.empty(), "mgmake spec: library target has no name");
            mgmkassert(find_executable(lib.m_name) == nullptr, "mgmake spec: target name conflict '" + lib.m_name + "'");

			// Skip if the library was already added
			if (find_library(lib.m_name) != nullptr) {
                return *this;
            }

			m_libraries.emplace_back(lib);
			return *this;
		}

		const spec::library* find_library(std::string_view name) const {
            for (const auto& lib : m_libraries) {
                if (lib.m_name == name) {
                    return &lib;
                }
            }

            return nullptr;
        }

		const spec::executable* find_executable(std::string_view name) const {
            for (const auto& exe : m_executables) {
                if (exe.m_name == name) {
                    return &exe;
                }
            }

            return nullptr;
        }

		// Generate the graph from all project info
		inline dag::graph graph(const build::request& req) const {
			const auto& tc = req.toolchain();

			dag::graph result{};

			auto append_unique_include_dir = [](
				std::vector<std::filesystem::path>& dirs,
				const std::filesystem::path& dir
			) {
				for (const auto& existing : dirs) {
					if (existing == dir) {
						return;
					}
				}

				dirs.emplace_back(dir);
			};

			auto append_unique_target_dep = [](
				std::vector<dag::target::id>& deps,
				dag::target::id dep
			) {
				for (const auto existing : deps) {
					if (existing == dep) {
						return;
					}
				}

				deps.emplace_back(dep);
			};

			auto find_library_index = [&](std::string_view name) -> std::vector<spec::library>::size_type {
				for (std::vector<spec::library>::size_type i = 0; i < m_libraries.size(); ++i) {
					if (m_libraries[i].m_name == name) {
						return i;
					}
				}

				mgmkassert(
					false,
					std::string{"mgmake spec: unknown linked library '"} + std::string{name} + "'"
				);

				return 0;
			};

			auto collect_library_usage = [&](
				auto& self,
				const spec::library& lib,
				std::vector<std::filesystem::path>& include_dirs,
				std::vector<std::string_view>& seen
			) -> void {
				for (const auto seen_name : seen) {
					if (seen_name == lib.m_name) {
						return;
					}
				}

				seen.emplace_back(lib.m_name);

				mgmkassert(
					lib.m_kind == spec::library::kind::interface,
					std::string{"mgmake spec: lowering non-interface library '"} + lib.m_name + "' is not implemented yet"
				);

				for (const auto& include_dir : lib.m_include_dirs) {
					append_unique_include_dir(include_dirs, include_dir);
				}

				for (const auto linked_library : lib.m_linked_libraries) {
					const auto linked_index = find_library_index(linked_library);
					self(self, m_libraries[linked_index], include_dirs, seen);
				}
			};

			// Create all library DAG targets first. Interface libraries produce no artifact,
			// but they should still exist as named build targets.
			std::vector<dag::target::id> library_target_ids{};
			library_target_ids.reserve(m_libraries.size());

			for (const auto& lib : m_libraries) {
				mgmkassert(not lib.m_name.empty(), "mgmake spec: library target has no name");

				mgmkassert(
					lib.m_kind == spec::library::kind::interface,
					std::string{"mgmake spec: lowering non-interface library '"} + lib.m_name + "' is not implemented yet"
				);

				library_target_ids.emplace_back(result.create_target(
					lib.m_name,
					std::vector<dag::artifact::id>{},
					std::vector<dag::target::id>{}
				));
			}

			// Add library-to-library DAG target dependencies.
			for (std::vector<spec::library>::size_type i = 0; i < m_libraries.size(); ++i) {
				const auto& lib = m_libraries[i];
				auto& dag_target = result.target(library_target_ids[i]);

				for (const auto linked_library : lib.m_linked_libraries) {
					const auto linked_index = find_library_index(linked_library);
					append_unique_target_dep(
						dag_target.m_dependencies,
						library_target_ids[linked_index]
					);
				}
			}

			// Lower executables.
			for (const auto& exe : m_executables) {
				mgmkassert(not exe.m_name.empty(), "mgmake spec: executable target has no name");
				mgmkassert(not exe.m_sources.empty(), "mgmake spec: executable target '" + exe.m_name + "' has no sources");

				std::vector<dag::artifact::id> inputs{};
				inputs.reserve(exe.m_sources.size());

				for (const auto& source : exe.m_sources) {
					inputs.emplace_back(result.create_artifact(
						dag::artifact::kind::source,
						source
					));
				}

				std::filesystem::path output = req.build_dir() / exe.m_name;

		#if defined(MGMK_PLATFORM_WINDOWS)
				output += ".exe";
		#endif

				auto output_id = result.create_artifact(
					dag::artifact::kind::generated,
					output
				);

				std::vector<std::filesystem::path> include_dirs{};

				for (const auto& include_dir : exe.m_include_dirs) {
					append_unique_include_dir(include_dirs, include_dir);
				}

				std::vector<std::string_view> seen_libraries{};

				for (const auto linked_library : exe.m_linked_libraries) {
					const auto linked_index = find_library_index(linked_library);
					collect_library_usage(
						collect_library_usage,
						m_libraries[linked_index],
						include_dirs,
						seen_libraries
					);
				}

				sys::command_line command{};
				command.m_args.emplace_back(tc.cxx());

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

				for (const auto& source : exe.m_sources) {
					command.m_args.emplace_back(source.string());
				}

				command.m_args.emplace_back("-o");
				command.m_args.emplace_back(output.string());

				result.create_action(
					std::string{"Build executable "} + exe.m_name,
					std::string{"Builds executable target '"} + exe.m_name + "'.",
					std::move(inputs),
					std::vector<dag::artifact::id>{output_id},
					false,
					std::move(command),
					std::filesystem::path{}
				);

				std::vector<dag::target::id> executable_dependencies{};

				for (const auto linked_library : exe.m_linked_libraries) {
					const auto linked_index = find_library_index(linked_library);
					append_unique_target_dep(
						executable_dependencies,
						library_target_ids[linked_index]
					);
				}

				result.create_target(
					exe.m_name,
					std::vector<dag::artifact::id>{output_id},
					std::move(executable_dependencies)
				);
			}

			return result;
		}
	};
}

#endif