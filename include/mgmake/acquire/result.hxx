#pragma once

#ifndef MGMK_ACQUIRE_RESULT_HXX
#define MGMK_ACQUIRE_RESULT_HXX

#include "fetched.hxx"
#include "../dag/graph.hxx"

#include <filesystem>
#include <map>
#include <string>
#include <string_view>

namespace mgmake::acquire {
	struct result {
		dag::graph m_graph;

		// Maps source names to acquisition results. Example: "sdl-src" points at
		// the source tree made available by the acquisition graph.
		std::map<std::string, acquire::fetched> m_fetches;

		[[nodiscard]] const acquire::fetched* find_fetch(std::string_view name) const {
			const auto found = m_fetches.find(std::string{name});
			return found == m_fetches.end() ? nullptr : &found->second;
		}

		[[nodiscard]] const std::filesystem::path* find_source_dir(std::string_view name) const {
			const auto* fetched = find_fetch(name);
			return fetched == nullptr ? nullptr : &fetched->m_source_dir;
		}
	};
}

#endif // MGMK_ACQUIRE_RESULT_HXX
