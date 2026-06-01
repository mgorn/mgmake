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
		std::vector<spec::executable> m_executables;
		std::vector<spec::library> m_libraries;

		inline constexpr project& add_target(const spec::executable& exe) {
			m_executables.emplace_back(exe);
			return *this;
		}
		inline constexpr project& add_target(const spec::library& lib) {
			m_libraries.emplace_back(lib);
			return *this;
		}

		// Generate the graph from all project info
		inline constexpr dag::graph graph() const {
			return {};
		}
	};
}

#endif