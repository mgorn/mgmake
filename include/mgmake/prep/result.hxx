#pragma once

#ifndef MGMK_PREP_RESULT_HXX
#define MGMK_PREP_RESULT_HXX

#ifdef MGMK_ENABLE_EXT_CMAKE
#include "cmake/project.hxx"
#endif // MGMK_ENABLE_EXT_CMAKE

#include <map>
#include <string>
#include <string_view>

namespace mgmake::prep {
	struct result {
#ifdef MGMK_ENABLE_EXT_CMAKE
		std::map<std::string, prep::cmake::project> m_cmake_projects;

		[[nodiscard]] prep::cmake::project* find_cmake_project(std::string_view name) {
			const auto found = m_cmake_projects.find(std::string{name});
			return found == m_cmake_projects.end() ? nullptr : &found->second;
		}

		[[nodiscard]] const prep::cmake::project* find_cmake_project(std::string_view name) const {
			const auto found = m_cmake_projects.find(std::string{name});
			return found == m_cmake_projects.end() ? nullptr : &found->second;
		}
#endif // MGMK_ENABLE_EXT_CMAKE
	};
}

#endif // MGMK_PREP_RESULT_HXX
