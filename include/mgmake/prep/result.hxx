#pragma once

#ifndef MGMK_PREP_RESULT_HXX
#define MGMK_PREP_RESULT_HXX

#include "fetched.hxx"
#include "../dag/graph.hxx"
#ifdef MGMK_ENABLE_EXT_CMAKE
#include "cmake_project.hxx"
#endif // MGMK_ENABLE_EXT_CMAKE

#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <string_view>

// Prep result keeps the preparation DAG and lookup tables consumed by the lower phase.

namespace mgmake::prep {
	struct result {
		dag::graph m_dag;
		std::map<std::string, prep::fetched> m_fetches;
#ifdef MGMK_ENABLE_EXT_CMAKE
		std::map<std::string, prep::cmake_project> m_cmake_projects;
#endif // MGMK_ENABLE_EXT_CMAKE

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

		std::size_t reload_cmake_file_api_replies() {
			std::size_t loaded_targets = 0;

			for (auto& [name, project] : m_cmake_projects) {
				loaded_targets += project.reload_file_api_reply();
			}

			return loaded_targets;
		}
#endif // MGMK_ENABLE_EXT_CMAKE
	};
}

#endif // MGMK_PREP_RESULT_HXX
