#pragma once

#ifndef MGMK_LOWER_PROJECT_IMPL_HXX
#define MGMK_LOWER_PROJECT_IMPL_HXX

#include "project.hxx"
#include "context_impl.hxx"
#include "../spec/project.hxx"

// Project lowering consumes the validated spec plus prepared metadata and emits the final build DAG.

namespace mgmake::lower {
	[[nodiscard]] inline dag::graph project(
		const spec::project& project,
		const build::request& req,
		const prep::result& prepared,
		dep::database& deps
	) {
		dag::graph result{};
		lower::context ctx{result, req, project, prepared, deps};

		for (spec::library::id id = 0; id < project.m_libraries.size(); ++id) {
			ctx.lower_library(id);
		}

		for (spec::executable::id id = 0; id < project.m_executables.size(); ++id) {
			ctx.lower_executable(id);
		}

		return result;
	}
}

#endif // MGMK_LOWER_PROJECT_IMPL_HXX
