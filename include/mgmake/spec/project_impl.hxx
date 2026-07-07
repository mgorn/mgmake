#pragma once

#ifndef MGMK_SPEC_PROJECT_IMPL_HXX
#define MGMK_SPEC_PROJECT_IMPL_HXX

#include "project.hxx"
#include "../acquire/plan.hxx"
#include "../configure/plan.hxx"
#include "../lower/project_impl.hxx"
#include "../prep/finalize.hxx"

// Project wrappers keep custom entrypoints simple while the free phase functions
// remain the canonical implementations.

namespace mgmake::spec {
	inline mgmake::acquire::result project::acquire(
		const build::request& req
	) const {
		return mgmake::acquire::plan(*this, req);
	}

	inline mgmake::configure::result project::configure(
		const build::request& req,
		const mgmake::acquire::result& acquired
	) const {
		return mgmake::configure::plan(*this, req, acquired);
	}

	inline std::expected<mgmake::prep::result, std::string> project::prepare(
		const build::request& req,
		const mgmake::acquire::result& acquired,
		const mgmake::configure::result& configured
	) const {
		return mgmake::prep::finalize(*this, req, acquired, configured);
	}

	inline dag::graph project::lower(
		const build::request& req,
		const mgmake::prep::result& prepared,
		dep::database& deps
	) const {
		return mgmake::lower::project(*this, req, prepared, deps);
	}
}

#endif // MGMK_SPEC_PROJECT_IMPL_HXX
