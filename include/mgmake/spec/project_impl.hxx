#pragma once

#ifndef MGMK_SPEC_PROJECT_IMPL_HXX
#define MGMK_SPEC_PROJECT_IMPL_HXX

#include "project.hxx"
#include "../lower/context_impl.hxx"

namespace mgmake::spec {
	inline dag::graph project::graph(const build::request& req) const {
		dag::graph result{};
		lower::context ctx{result, req, *this};

		for (ext::fetch::id id = 0; id < m_fetches.size(); ++id) {
			ctx.lower_fetch(id);
		}

		for (spec::library::id id = 0; id < m_libraries.size(); ++id) {
			ctx.lower_library(id);
		}

		for (spec::executable::id id = 0; id < m_executables.size(); ++id) {
			ctx.lower_executable(id);
		}

		return result;
	}
}

#endif
