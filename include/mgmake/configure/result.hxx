#pragma once

#ifndef MGMK_CONFIGURE_RESULT_HXX
#define MGMK_CONFIGURE_RESULT_HXX

#include "../dag/graph.hxx"
#ifdef MGMK_ENABLE_EXT_CMAKE
#include "cmake/project.hxx"
#endif // MGMK_ENABLE_EXT_CMAKE

#include <map>
#include <string>
#include <string_view>

namespace mgmake::configure {
	struct result {
		dag::graph m_graph;

#ifdef MGMK_ENABLE_EXT_CMAKE
		std::map<std::string, configure::cmake::project> m_cmake_projects;

		[[nodiscard]] const configure::cmake::project* find_cmake_project(
			std::string_view name
		) const {
			const auto found = m_cmake_projects.find(std::string{name});
			return found == m_cmake_projects.end() ? nullptr : &found->second;
		}
#endif // MGMK_ENABLE_EXT_CMAKE
	};
}

#endif // MGMK_CONFIGURE_RESULT_HXX
