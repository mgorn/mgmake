#pragma once

#ifndef MGMK_LOWER_CONTEXT_HXX
#define MGMK_LOWER_CONTEXT_HXX

#include "emitter.hxx"
#include "fetched.hxx"
#include "target.hxx"
#include "usage.hxx"
#include "../build/request.hxx"
#include "../ext/fetch.hxx"
#include "../spec/executable.hxx"
#include "../spec/library.hxx"

#include <filesystem>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace mgmake::spec {
	struct project;
}

namespace mgmake::lower {
	struct context {
		const build::request& m_req;
		const spec::project& m_project;
		lower::emitter m_emit;

		context(
			dag::graph& graph,
			const build::request& req,
			const spec::project& project
		);

		lower::emitter& emit() {
			return m_emit;
		}

		const lower::emitter& emit() const {
			return m_emit;
		}

		const build::request& request() const {
			return m_req;
		}

		const build::toolchain& toolchain() const {
			return m_req.toolchain();
		}

		const lower::target& lower_library(spec::library::id id);
		void lower_executable(spec::executable::id id);
		const lower::fetched& lower_fetch(ext::fetch::id id);
		lower::fetched lower_fetch_value(const ext::fetch& fetch);

		lower::usage use_libraries(
			const std::set<std::string>& libraries,
			std::string_view owner_name
		);

		template<typename target_t>
		std::vector<dag::artifact::id> lower_objects(
			const target_t& target,
			const std::set<std::filesystem::path>& include_dirs
		);

	private:
		lower::target lower_interface_library(
			const spec::library& lib,
			lower::usage usage
		);

		lower::target lower_static_library(
			const spec::library& lib,
			lower::usage usage
		);

		lower::target lower_shared_library(
			const spec::library& lib,
			lower::usage usage
		);

		lower::fetched lower_git_fetch(
			const ext::fetch& fetch,
			const ext::git_fetch& git
		);

		lower::fetched lower_archive_fetch(
			const ext::fetch& fetch,
			const ext::archive_fetch& archive
		);

		lower::fetched lower_local_fetch(
			const ext::fetch& fetch,
			const ext::local_fetch& local
		);

		std::vector<std::optional<lower::target>> m_libraries;
		std::set<spec::library::id> m_active_libraries;
		std::vector<std::optional<lower::fetched>> m_fetches;
		std::set<std::string> m_active_fetches;
		std::map<std::string, lower::fetched> m_named_fetches;
	};
}

#endif
