#pragma once

#ifndef MGMK_PREP_CONTEXT_HXX
#define MGMK_PREP_CONTEXT_HXX

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

		std::vector<std::optional<prep::fetched>> m_fetches;
		std::set<std::string> m_active_fetches;
		std::map<std::string, prep::fetched> m_named_fetches;
	};
}

#endif
