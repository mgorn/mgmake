#pragma once

#ifndef MGMK_LOWER_PROJECT_HXX
#define MGMK_LOWER_PROJECT_HXX

#include "../build/request.hxx"
#include "../dag/graph.hxx"
#include "../dep/database.hxx"
#include "../prep/result.hxx"

namespace mgmake::spec {
	struct project;
}

namespace mgmake::lower {
	[[nodiscard]] dag::graph project(
		const spec::project& project,
		const build::request& req,
		const prep::result& prepared,
		dep::database& deps
	);
}

#endif // MGMK_LOWER_PROJECT_HXX
