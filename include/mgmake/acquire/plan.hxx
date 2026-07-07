#pragma once

#ifndef MGMK_ACQUIRE_PLAN_HXX
#define MGMK_ACQUIRE_PLAN_HXX

#include "context_impl.hxx"
#include "result.hxx"
#include "../build/request.hxx"
#include "../spec/project.hxx"

namespace mgmake::acquire {
	[[nodiscard]] inline acquire::result plan(
		const spec::project& project,
		const build::request& req
	) {
		acquire::result result{};
		acquire::context ctx{result, req, project};

		for (ext::fetch::id id = 0; id < project.m_fetches.size(); ++id) {
			ctx.fetch(id);
		}

#ifdef MGMK_ENABLE_EXT_CMAKE
		for (const auto& cmake_project : project.m_cmake_projects) {
			if (cmake_project.m_source.has_value()) {
				ctx.fetch_value(cmake_project.m_source.value());
			}
		}
#endif // MGMK_ENABLE_EXT_CMAKE

		return result;
	}
}

#endif // MGMK_ACQUIRE_PLAN_HXX
