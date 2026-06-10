#pragma once

#ifndef MGMK_SPEC_EXECUTABLE_HXX
#define MGMK_SPEC_EXECUTABLE_HXX

#include "../build/request.hxx"
#include "../dag/graph.hxx"
#include "../dag/target.hxx"
#include "target.hxx"

#include <set>
#include <string_view>
#include <vector>

namespace mgmake::spec {
	struct project;

	struct executable : public target<executable> {
		using id = std::vector<executable>::size_type;

		dag::target graph(
			dag::graph& result,
			const build::request& req,
			const spec::project& proj,
			const std::vector<dag::artifact::id>& link_inputs,
			std::set<dag::target::id> target_dependencies
		) const;
	};
}

#endif
