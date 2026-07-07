#pragma once

#ifndef MGMK_CONFIGURE_CONTEXT_HXX
#define MGMK_CONFIGURE_CONTEXT_HXX

#include "result.hxx"
#include "../acquire/result.hxx"
#include "../build/request.hxx"
#include "../dag/emitter.hxx"
#ifdef MGMK_ENABLE_EXT_CMAKE
#include "../ext/cmake/project.hxx"
#endif // MGMK_ENABLE_EXT_CMAKE

#include <optional>
#include <vector>

namespace mgmake::spec {
	struct project;
}

namespace mgmake::configure {
	struct context {
		configure::result& m_result;
		const build::request& m_req;
		const spec::project& m_project;
		const acquire::result& m_acquired;
		dag::emitter m_emit;

		context(
			configure::result& result,
			const build::request& req,
			const spec::project& project,
			const acquire::result& acquired
		);

		dag::emitter& emit() {
			return m_emit;
		}

		const dag::emitter& emit() const {
			return m_emit;
		}

		const build::request& request() const {
			return m_req;
		}

#ifdef MGMK_ENABLE_EXT_CMAKE
		const configure::cmake::project& cmake(ext::cmake::project::id id);

	private:
		configure::cmake::project cmake_value(const ext::cmake::project& cmake_project);

		std::vector<std::optional<configure::cmake::project>> m_cmake_projects;
#endif // MGMK_ENABLE_EXT_CMAKE
	};
}

#endif // MGMK_CONFIGURE_CONTEXT_HXX
