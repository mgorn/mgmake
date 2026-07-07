#pragma once

#ifndef MGMK_ACQUIRE_CONTEXT_HXX
#define MGMK_ACQUIRE_CONTEXT_HXX

#include "fetched.hxx"
#include "result.hxx"
#include "../build/request.hxx"
#include "../dag/emitter.hxx"
#include "../ext/fetch.hxx"

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

// Acquire context builds a DAG that makes source trees available and memoizes repeated fetch specs by name.

namespace mgmake::spec {
	struct project;
}

namespace mgmake::acquire {
	struct context {
		acquire::result& m_result;
		const build::request& m_req;
		const spec::project& m_project;
		dag::emitter m_emit;

		context(
			acquire::result& result,
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

		const acquire::fetched& fetch(ext::fetch::id id);
		acquire::fetched fetch_value(const ext::fetch& fetch);

	private:
		acquire::fetched git_fetch(
			const ext::fetch& fetch,
			const ext::git_fetch& git
		);

		acquire::fetched archive_fetch(
			const ext::fetch& fetch,
			const ext::archive_fetch& archive
		);

		acquire::fetched local_fetch(
			const ext::fetch& fetch,
			const ext::local_fetch& local
		);

		std::vector<std::optional<acquire::fetched>> m_fetches;
		std::set<std::string> m_active_fetches;
		std::map<std::string, acquire::fetched> m_named_fetches;
	};
}

#endif // MGMK_ACQUIRE_CONTEXT_HXX
