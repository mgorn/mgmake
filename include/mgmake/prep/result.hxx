#pragma once

#ifndef MGMK_PREP_RESULT_HXX
#define MGMK_PREP_RESULT_HXX

#include "fetched.hxx"
#include "../dag/graph.hxx"
#ifdef MGMK_ENABLE_EXT_CMAKE
#include "../ext/cmake/file_api.hxx"
#endif

#include <map>
#include <optional>
#include <string>
#include <string_view>

namespace mgmake::prep {
#ifdef MGMK_ENABLE_EXT_CMAKE
	using cmake_target = ext::cmake_file_api::target;
	using cmake_project = ext::cmake_file_api::project;
#endif

	struct result {
		dag::graph m_dag;
		std::map<std::string, prep::fetched> m_fetches;
#ifdef MGMK_ENABLE_EXT_CMAKE
		std::map<std::string, prep::cmake_project> m_cmake_projects;
#endif

		[[nodiscard]] const prep::fetched* find_fetch(std::string_view name) const {
			const auto found = m_fetches.find(std::string{name});
			return found == m_fetches.end() ? nullptr : &found->second;
		}

#ifdef MGMK_ENABLE_EXT_CMAKE
		[[nodiscard]] prep::cmake_project* find_cmake_project(std::string_view name) {
			const auto found = m_cmake_projects.find(std::string{name});
			return found == m_cmake_projects.end() ? nullptr : &found->second;
		}

		[[nodiscard]] const prep::cmake_project* find_cmake_project(std::string_view name) const {
			const auto found = m_cmake_projects.find(std::string{name});
			return found == m_cmake_projects.end() ? nullptr : &found->second;
		}
#endif
	};
}

#endif
