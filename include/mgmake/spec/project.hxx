#pragma once

#ifndef MGMK_SPEC_PROJECT_HXX
#define MGMK_SPEC_PROJECT_HXX

#include "../build/request.hxx"
#include "../dag/graph.hxx"
#include "../detail/assert.hxx"
#include "../lower/context.hxx"
#include "executable.hxx"
#include "library.hxx"

#include <optional>
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

		// Generate the graph from all project info
		inline dag::graph graph(const build::request& req) const {
			dag::graph result{};
			lower::context ctx{result, req, *this};

			for (spec::library::id id = 0; id < m_libraries.size(); ++id) {
				ctx.lower_library(id);
			}

			for (spec::executable::id id = 0; id < m_executables.size(); ++id) {
				ctx.lower_executable(id);
			}

			return result;
		}
	};
}

#include "../lower/context_impl.hxx"

#endif
