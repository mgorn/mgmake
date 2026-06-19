#pragma once

#ifndef MGMK_PREP_RESULT_HXX
#define MGMK_PREP_RESULT_HXX

#include "fetched.hxx"
#include "../dag/graph.hxx"

#include <map>
#include <optional>
#include <string>
#include <string_view>

namespace mgmake::prep {
	struct result {
		dag::graph m_dag;
		std::map<std::string, prep::fetched> m_fetches;

		[[nodiscard]] const prep::fetched* find_fetch(std::string_view name) const {
			const auto found = m_fetches.find(std::string{name});
			return found == m_fetches.end() ? nullptr : &found->second;
		}
	};
}

#endif
