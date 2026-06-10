#pragma once

#ifndef MGMK_SPEC_PROJECT_HXX
#define MGMK_SPEC_PROJECT_HXX

#include "../backend/traits.hxx"
#include "../build/request.hxx"
#include "../dag/graph.hxx"
#include "../sys/command_line.hxx"
#include "../sys/platform.hxx"
#include "executable.hxx"
#include "library.hxx"

#include <filesystem>
#include <optional>
#include <set>
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
            mgmkassert(not find_library(exe.m_name).has_value(), "mgmake spec: target name conflict '" + exe.m_name + "'");

			m_executables.emplace_back(exe);
			return *this;
		}
		inline constexpr project& add_target(const spec::library& lib) {
			mgmkassert(not lib.m_name.empty(), "mgmake spec: library target has no name");
            mgmkassert(not find_executable(lib.m_name).has_value(), "mgmake spec: target name conflict '" + lib.m_name + "'");

			// Skip if the library was already added
			if (find_library(lib.m_name).has_value()) {
                return *this;
            }

			m_libraries.emplace_back(lib);
			return *this;
		}

		const std::optional<spec::library::id> find_library(std::string_view name) const {
            for (spec::library::id idx = 0; idx < m_libraries.size(); idx++) {
				const auto& lib = m_libraries.at(idx);
                if (lib.m_name == name) {
                    return idx;
                }
            }
            return std::nullopt;
        }
		const spec::library* get_library(const spec::library::id idx) const {
			if (idx > m_libraries.size())
				return nullptr;
			return &m_libraries.at(idx);
		}

		const std::optional<spec::executable::id> find_executable(std::string_view name) const {
            for (spec::library::id idx = 0; idx < m_executables.size(); idx++) {
				const auto& exe = m_executables.at(idx);
                if (exe.m_name == name) {
                    return idx;
                }
            }
            return std::nullopt;
        }
		const spec::executable* get_executable(const spec::executable::id idx) const {
			if (idx > m_executables.size())
				return nullptr;
			return &m_executables.at(idx);
		}

		const std::set<std::filesystem::path> collect_includes(const spec::library::id idx) const {
			const auto& lib = m_libraries.at(idx);
			std::set<std::filesystem::path> result;
			for (const auto linked_library : lib.m_linked_libraries) {
				const auto linked_index = find_library(linked_library);
				if (linked_index.has_value())
					result.insert_range(collect_includes(linked_index.value()));
			}

			result.insert_range(lib.include_dirs());

			return result;
		}
		const std::set<std::filesystem::path> collect_includes(const spec::library& lib) const {
			auto lib_idx = find_library(lib);
			if (lib_idx.has_value())
				return collect_includes(lib_idx.value());
			return {};
		}
		const std::set<std::filesystem::path> collect_includes(const spec::executable& exe) const {
			std::set<std::filesystem::path> include_dirs = exe.m_include_dirs;
			for (auto& lib_name : exe.linked_libraries()) {
				auto lib_idx = find_library(lib_name);
				if (lib_idx.has_value())
					include_dirs.insert_range(collect_includes(lib_idx.value()));
			}
			return include_dirs;
		}

		// Generate the graph from all project info
		inline dag::graph graph(const build::request& req) const {
			dag::graph result{};

			// Create all library DAG targets first. Interface libraries produce no artifact,
			// but they should still exist as named build targets.
			std::vector<dag::target::id> library_target_ids{};
			library_target_ids.reserve(m_libraries.size());

			for (const auto& lib : m_libraries) {
				auto target = lib.graph(
					result,
					req,
					*this,
					{}
				);

				library_target_ids.emplace_back(result.create_target(std::move(target)));
			}

			// Add library-to-library DAG target dependencies.
			for (spec::library::id i = 0; i < m_libraries.size(); ++i) {
				const auto& lib = m_libraries.at(i);
				auto& dag_target = result.target(library_target_ids.at(i));

				for (const auto linked_library : lib.m_linked_libraries) {
					const auto linked_index = find_library(linked_library);
					if (linked_index.has_value()) {
						dag_target.add_dependency(library_target_ids.at(linked_index.value()));
					}
				}
			}

			// Lower executables.
			for (const auto& exe : m_executables) {
				std::set<dag::target::id> executable_dependencies{};

				for (const auto linked_library : exe.m_linked_libraries) {
					const auto linked_index = find_library(linked_library);
					if (linked_index.has_value()) {
						executable_dependencies.emplace(library_target_ids.at(linked_index.value()));
					}
				}

				auto target = exe.graph(
					result,
					req,
					*this,
					std::move(executable_dependencies)
				);

				result.create_target(std::move(target));
			}

			return result;
		}
	};
}

#endif
