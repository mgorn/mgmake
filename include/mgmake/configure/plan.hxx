#pragma once

#ifndef MGMK_CONFIGURE_PLAN_HXX
#define MGMK_CONFIGURE_PLAN_HXX

#include "context_impl.hxx"
#include "result.hxx"
#include "../acquire/result.hxx"
#include "../build/request.hxx"
#include "../spec/project.hxx"

namespace mgmake::configure {
	[[nodiscard]] inline configure::result plan(
		const spec::project& project,
		const build::request& req,
		const acquire::result& acquired
	) {
		configure::result result{};
		configure::context ctx{result, req, project, acquired};

#ifdef MGMK_ENABLE_EXT_CMAKE
		for (ext::cmake::project::id id = 0; id < project.m_cmake_projects.size(); ++id) {
			ctx.cmake(id);
		}
#endif // MGMK_ENABLE_EXT_CMAKE

		return result;
	}
}

#endif // MGMK_CONFIGURE_PLAN_HXX
