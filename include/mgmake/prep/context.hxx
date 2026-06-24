#pragma once

#ifndef MGMK_PREP_CONTEXT_HXX
#define MGMK_PREP_CONTEXT_HXX

#include "fetched.hxx"
#include "result.hxx"
#include "../build/request.hxx"
#include "../dag/emitter.hxx"
#include "../ext/fetch.hxx"
#ifdef MGMK_ENABLE_EXT_CMAKE
#include "../ext/cmake.hxx"
#endif // MGMK_ENABLE_EXT_CMAKE

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

// Prep context builds a DAG for external fetch/configure work and memoizes fetched/provider project results by name.

namespace mgmake::spec {
	struct project;
}

namespace mgmake::prep {
	struct context {
		prep::result& m_result;
		const build::request& m_req;
		const spec::project& m_project;
		dag::emitter m_emit;

		context(
			prep::result& result,
			const build::request& req,
			const spec::project& project
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

		const build::toolchain& toolchain() const {
			return m_req.toolchain();
		}

		const prep::fetched& fetch(ext::fetch::id id);
		prep::fetched fetch_value(const ext::fetch& fetch);
#ifdef MGMK_ENABLE_EXT_CMAKE
		const prep::cmake_project& cmake(ext::cmake::id id);
#endif // MGMK_ENABLE_EXT_CMAKE

	private:
		prep::fetched git_fetch(
			const ext::fetch& fetch,
			const ext::git_fetch& git
		);

		prep::fetched archive_fetch(
			const ext::fetch& fetch,
			const ext::archive_fetch& archive
		);

		prep::fetched local_fetch(
			const ext::fetch& fetch,
			const ext::local_fetch& local
		);

#ifdef MGMK_ENABLE_EXT_CMAKE
		prep::cmake_project cmake_value(const ext::cmake& cmake_project);
#endif // MGMK_ENABLE_EXT_CMAKE

		std::vector<std::optional<prep::fetched>> m_fetches;
#ifdef MGMK_ENABLE_EXT_CMAKE
		std::vector<std::optional<prep::cmake_project>> m_cmake_projects;
#endif // MGMK_ENABLE_EXT_CMAKE
		std::set<std::string> m_active_fetches;
		std::map<std::string, prep::fetched> m_named_fetches;
	};
}

#endif // MGMK_PREP_CONTEXT_HXX
