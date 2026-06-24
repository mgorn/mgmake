#pragma once

#ifndef MGMK_SPEC_PROJECT_IMPL_HXX
#define MGMK_SPEC_PROJECT_IMPL_HXX

#include "project.hxx"
#include "../lower/context_impl.hxx"
#include "../prep/context_impl.hxx"

#include <utility>

namespace mgmake::spec {
	inline prep::result project::prepare(const build::request& req) const {
		prep::result result{};
		prep::context ctx{result, req, *this};

		for (ext::fetch::id id = 0; id < m_fetches.size(); ++id) {
			ctx.fetch(id);
		}

#ifdef MGMK_ENABLE_EXT_CMAKE
		for (ext::cmake::id id = 0; id < m_cmake_projects.size(); ++id) {
			ctx.cmake(id);
		}
#endif // MGMK_ENABLE_EXT_CMAKE

		return result;
	}

	inline dag::graph project::build(
		const build::request& req,
		const prep::result& prepared
	) const {
		dag::graph result{};
		lower::context ctx{result, req, *this, prepared};

		for (spec::library::id id = 0; id < m_libraries.size(); ++id) {
			ctx.lower_library(id);
		}

		for (spec::executable::id id = 0; id < m_executables.size(); ++id) {
			ctx.lower_executable(id);
		}

		return result;
	}
}

#endif // MGMK_SPEC_PROJECT_IMPL_HXX
