#pragma once

#ifndef MGMK_SPEC_PROJECT_HXX
#define MGMK_SPEC_PROJECT_HXX

#include "../build/request.hxx"
#include "../dag/graph.hxx"
#include "../dep/database.hxx"
#include "../detail/assert.hxx"
#include "../prep/result.hxx"
#include "executable.hxx"
#include "library.hxx"

#include "../ext/fetch.hxx"
#include "../ext/provided_target_ref.hxx"
#include "../ext/rooted_path.hxx"
#ifdef MGMK_ENABLE_EXT_CMAKE
#include "../ext/cmake/project.hxx"
#endif // MGMK_ENABLE_EXT_CMAKE

#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

// Project owns the validated spec graph: executables, libraries, fetches, and optional external providers.

namespace mgmake::spec {
	struct project {
		std::string m_name;
		std::vector<spec::executable> m_executables;
		std::vector<spec::library> m_libraries;
		std::vector<ext::fetch> m_fetches;
#ifdef MGMK_ENABLE_EXT_CMAKE
		std::vector<ext::cmake::project> m_cmake_projects;
#endif // MGMK_ENABLE_EXT_CMAKE

		inline constexpr project(std::string_view name)
			: m_name{name} {
			mgmkassert(not m_name.empty(), "mgmake spec: project has no name");
		}

		// Validation stays close to the mutation that adds user-declared targets to the project.
		inline project& add_target(const spec::executable& exe) {
			mgmkassert(not exe.m_name.empty(), "mgmake spec: executable target has no name");
            mgmkassert(not find_library(exe.m_name).has_value(), "mgmake spec: target name conflict '" + exe.m_name + "'");
            mgmkassert(not find_executable(exe.m_name).has_value(), "mgmake spec: target name conflict '" + exe.m_name + "'");
			if (!exe.provider_backed()) {
				mgmkassert(not exe.m_sources.empty(), "mgmake spec: executable target '" + exe.m_name + "' has no sources");
			}
#ifdef MGMK_ENABLE_EXT_CMAKE
			assert_known_provider_for(exe.m_provider, exe.m_name);
#endif // MGMK_ENABLE_EXT_CMAKE

			//assert_known_libraries_for(exe.m_linked_libraries, exe.m_name);

			m_executables.emplace_back(exe);
			return *this;
		}
		inline project& add_target(const spec::library& lib) {
			mgmkassert(not lib.m_name.empty(), "mgmake spec: library target has no name");
            mgmkassert(not find_executable(lib.m_name).has_value(), "mgmake spec: target name conflict '" + lib.m_name + "'");
			mgmkassert(not find_library(lib.m_name).has_value(), "mgmake spec: target name conflict '" + lib.m_name + "'");

			switch (lib.m_kind) {
				case spec::library::kind::interface:
					mgmkassert(
						lib.m_sources.empty(),
						"mgmake spec: interface library '" + lib.m_name + "' cannot have sources"
					);
					break;

				case spec::library::kind::static_lib:
					if (!lib.provider_backed()) {
						mgmkassert(
							not lib.m_sources.empty(),
							"mgmake spec: static library '" + lib.m_name + "' has no sources"
						);
					}
					break;

				case spec::library::kind::shared_lib:
					if (!lib.provider_backed()) {
						mgmkassert(
							not lib.m_sources.empty(),
							"mgmake spec: shared library '" + lib.m_name + "' has no sources"
						);
					}
					break;

				default:
					mgmkassert(false, "mgmake spec: invalid library kind for target '" + lib.m_name + "'");
					break;
			}

#ifdef MGMK_ENABLE_EXT_CMAKE
			assert_known_provider_for(lib.m_provider, lib.m_name);
#endif // MGMK_ENABLE_EXT_CMAKE
			//assert_known_libraries_for(lib.m_linked_libraries, lib.m_name);
			assert_library_link_closure_is_acyclic(lib);

			m_libraries.emplace_back(lib);
			return *this;
		}

		inline project& add_fetch(const ext::fetch& fetch) {
			mgmkassert(!fetch.m_name.empty(), "mgmake spec: external fetch has no name");
			mgmkassert(!find_fetch(fetch.m_name).has_value(), "mgmake spec: external fetch name conflict '" + fetch.m_name + "'");

			m_fetches.emplace_back(fetch);
			return *this;
		}

#ifdef MGMK_ENABLE_EXT_CMAKE
		inline project& add_ext(const ext::cmake::project& cmake_project) {
			mgmkassert(!cmake_project.m_name.empty(), "mgmake spec: external CMake project has no name");
			mgmkassert(!find_cmake(cmake_project.m_name).has_value(), "mgmake spec: external CMake project name conflict '" + cmake_project.m_name + "'");
			m_cmake_projects.emplace_back(cmake_project);
			return *this;
		}
#endif // MGMK_ENABLE_EXT_CMAKE

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
			if (idx >= m_libraries.size())
				return nullptr;
			return &m_libraries.at(idx);
		}

		const std::optional<spec::executable::id> find_executable(std::string_view name) const {
            for (spec::executable::id idx = 0; idx < m_executables.size(); idx++) {
				const auto& exe = m_executables.at(idx);
                if (exe.m_name == name) {
                    return idx;
                }
            }
            return std::nullopt;
        }
		const spec::executable* get_executable(const spec::executable::id idx) const {
			if (idx >= m_executables.size())
				return nullptr;
			return &m_executables.at(idx);
		}

		const std::optional<ext::fetch::id> find_fetch(std::string_view name) const {
			for (ext::fetch::id idx = 0; idx < m_fetches.size(); ++idx) {
				const auto& fetch = m_fetches.at(idx);
				if (fetch.m_name == name) {
					return idx;
				}
			}

			return std::nullopt;
		}

		const ext::fetch* get_fetch(const ext::fetch::id idx) const {
			if (idx >= m_fetches.size()) {
				return nullptr;
			}

			return &m_fetches.at(idx);
		}

#ifdef MGMK_ENABLE_EXT_CMAKE
		const std::optional<ext::cmake::project::id> find_cmake(std::string_view name) const {
			for (ext::cmake::project::id idx = 0; idx < m_cmake_projects.size(); ++idx) {
				const auto& cmake_project = m_cmake_projects.at(idx);
				if (cmake_project.m_name == name) {
					return idx;
				}
			}

			return std::nullopt;
		}

		const ext::cmake::project* get_cmake(const ext::cmake::project::id idx) const {
			if (idx >= m_cmake_projects.size()) {
				return nullptr;
			}

			return &m_cmake_projects.at(idx);
		}
#endif // MGMK_ENABLE_EXT_CMAKE

		prep::result prepare(const build::request& req) const;

		dag::graph build(
			const build::request& req,
			const prep::result& prepared,
			dep::database& deps
		) const;

	private:
#ifdef MGMK_ENABLE_EXT_CMAKE
		inline void assert_known_provider_for(
			const std::optional<ext::provided_target_ref>& provider,
			std::string_view owner_name
		) const {
			if (!provider.has_value()) {
				return;
			}

			switch (provider->m_kind) {
				case ext::provider_kind::cmake:
					mgmkassert(
						find_cmake(provider->m_project).has_value(),
						"mgmake spec: target '" + std::string{owner_name} +
							"' uses unknown CMake project '" + provider->m_project + "'"
					);
					break;
			}
		}
#endif // MGMK_ENABLE_EXT_CMAKE

		inline constexpr void assert_known_libraries_for(
			const std::set<std::string>& libraries,
			std::string_view owner_name
		) const {
			for (const auto& library_name : libraries) {
				mgmkassert(
					find_library(library_name).has_value(),
					"mgmake spec: target '" + std::string{owner_name} +
						"' links unknown library '" + library_name + "'"
				);
			}
		}

		inline constexpr bool library_stack_contains(
			const std::vector<std::string_view>& stack,
			std::string_view name
		) const {
			for (const auto existing : stack) {
				if (existing == name) {
					return true;
				}
			}

			return false;
		}

		// The active stack catches cycles before lowering recursively walks library links.
		inline constexpr void assert_library_link_closure_is_acyclic(
			std::string_view library_name,
			const std::set<std::string>& linked_libraries,
			std::vector<std::string_view>& active_libraries
		) const {
			mgmkassert(
				not library_stack_contains(active_libraries, library_name),
				"mgmake spec: cyclic library dependency involving '" + std::string{library_name} + "'"
			);

			active_libraries.emplace_back(library_name);

			for (const auto& linked_name : linked_libraries) {
				mgmkassert(
					not library_stack_contains(active_libraries, linked_name),
					"mgmake spec: cyclic library dependency involving '" + linked_name + "'"
				);

				const auto linked_id = find_library(linked_name);
				/*
				mgmkassert(
					linked_id.has_value(),
					"mgmake spec: library '" + std::string{library_name} +
						"' links unknown library '" + linked_name + "'"
				);
				*/
				if (not linked_id.has_value()) {
					continue;
				}

				const auto& linked_library = m_libraries.at(linked_id.value());
				assert_library_link_closure_is_acyclic(
					linked_library.m_name,
					linked_library.m_linked_libraries,
					active_libraries
				);
			}

			active_libraries.pop_back();
		}

		inline constexpr void assert_library_link_closure_is_acyclic(
			const spec::library& lib
		) const {
			std::vector<std::string_view> active_libraries{};
			assert_library_link_closure_is_acyclic(
				lib.m_name,
				lib.m_linked_libraries,
				active_libraries
			);
		}
	};
}

#endif // MGMK_SPEC_PROJECT_HXX
