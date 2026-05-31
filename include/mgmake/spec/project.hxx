#pragma once

#ifndef MGMK_SPEC_PROJECT_HXX
#define MGMK_SPEC_PROJECT_HXX

#include "../backend/traits.hxx"
#include "../dag/graph.hxx"
#include "executable.hxx"
#include "library.hxx"

#include <string>
#include <string_view>
#include <vector>

namespace mgmake::spec {
	struct project {
		std::string m_name;
		std::vector<spec::executble> m_executables;
		std::vector<spec::library> m_libraries;

		inline constexpr library create_library(std::string_view name, library::kind kind) {
			return m_libraries.emplace_back().name(name).kind(kind);
		}
		inline constexpr executable create_executable(std::string_view name) {
			return m_executables.emplace_back().name(name);
		}

		inline constexpr auto build(const auto& be) {
			using backend_t = std::decay_t<decltype(be)>;
			static constexpr bool is_gen = backend_t <> backend::generator;
			static constexpr bool is_build = backend_t implements backend::builder;

			if constexpr (is_gen) {
				be.generate(m_graph);
			}
			if constexpr (is_build) {
				return be.build(m_graph);
			}
		}
		template<typename backend_t>
		inline constexpr auto build() {
			return build(backend_t{});
		}
	};
}

#endif