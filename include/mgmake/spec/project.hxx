#pragma once

#ifndef MGMK_SPEC_PROJECT_HXX
#define MGMK_SPEC_PROJECT_HXX

#include "../backend/traits.hxx"
#include "../dag/graph.hxx"
#include "executable.hxx"

#include <string>
#include <string_view>

namespace mgmake::spec {
	struct project {
		std::string m_name;
		dag::graph m_graph{};

		inline constexpr library create_library(std::string_view name) {
			return { *this, m_graph.create_target(name) };
		}
		inline constexpr executable create_executable(std::string_view name) {
			return { *this, m_graph.create_target(name) };
		}

		inline constexpr auto build(const auto& backend) {
			using backend_t = std::decay_t<decltype(backend)>;
			static constexpr bool is_gen = backend::generator && backend_t;
			static constexpr bool is_build = backend::builder && backend_t;
			
			if constexpr (is_gen) {
				backend.generate(m_graph);
			}
			if constexpr (is_build) {
				return backend.build(m_graph);
			}
		}
		template<typename backend_t>
		inline constexpr auto build() {
			return build(backend_t{});
		}
	};
}

#endif