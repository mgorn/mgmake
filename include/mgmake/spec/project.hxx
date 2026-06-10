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

			std::vector<std::optional<dag::target::id>> library_target_ids(m_libraries.size());
			std::vector<std::vector<dag::artifact::id>> library_link_outputs(m_libraries.size());
			std::vector<unsigned char> library_states(m_libraries.size(), 0);

			for (const auto& lib : m_libraries) {
				lib.lower(
					result,
					req,
					*this,
					library_target_ids,
					library_link_outputs,
					library_states
				);
			}

			// Lower executables.
			for (const auto& exe : m_executables) {
				std::set<dag::target::id> executable_dependencies{};
				std::vector<dag::artifact::id> link_inputs{};

				for (const auto& linked_library : exe.linked_libraries()) {
					const auto linked_id = find_library(linked_library);

					mgmkassert(
						linked_id.has_value(),
						"mgmake spec: executable '" + exe.m_name +
							"' links unknown library '" + linked_library + "'"
					);

					const auto* lib = get_library(linked_id.value());

					mgmkassert(
						lib != nullptr,
						"mgmake spec: failed to fetch linked library '" + linked_library + "'"
					);

					auto lib_target_id = lib->lower(
						result,
						req,
						*this,
						library_target_ids,
						library_link_outputs,
						library_states
					);

					executable_dependencies.emplace(lib_target_id);

					for (auto artifact_id : library_link_outputs.at(linked_id.value())) {
						link_inputs.emplace_back(artifact_id);
					}
				}

				auto target = exe.graph(
					result,
					req,
					*this,
					link_inputs,
					std::move(executable_dependencies)
				);

				result.create_target(std::move(target));
			}

			return result;
		}
	};
}

#endif
