#pragma once

#ifndef MGMK_LOWER_CONTEXT_HXX
#define MGMK_LOWER_CONTEXT_HXX

#include "target.hxx"
#include "usage.hxx"
#include "../build/request.hxx"
#include "../dag/emitter.hxx"
#include "../dep/database.hxx"
#include "../prep/result.hxx"
#include "../spec/executable.hxx"
#include "../spec/library.hxx"

#include <filesystem>
#include <optional>
#include <set>
#include <span>
#include <string_view>
#include <vector>

// Lowering context converts validated project specs plus prep results into a DAG of artifacts, actions, and targets.

namespace mgmake::spec {
	struct project;
}

namespace mgmake::lower {
	struct context {
		const build::request& m_req;
		const spec::project& m_project;
		const prep::result& m_prep;
		dep::database& m_deps;
		dag::emitter m_emit;

		context(
			dag::graph& graph,
			const build::request& req,
			const spec::project& project,
			const prep::result& prep,
			dep::database& deps
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

		const lower::target& lower_library(spec::library::id id);
		// Lowering a library by name is reserved for external/system libraries
		const lower::target& lower_library(std::string_view lib);
		lower::target lower_system_library(std::string_view lib);
		void lower_executable(spec::executable::id id);
		lower::usage use_libraries(
			const std::set<std::string>& libraries,
			std::string_view owner_name
		);

		template<typename target_t>
		std::vector<dag::artifact::id> lower_objects(
			const target_t& target,
			const std::set<std::filesystem::path>& include_dirs,
			std::span<const dag::artifact::id> usage_inputs
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

		std::vector<std::optional<lower::target>> m_libraries;
		std::set<spec::library::id> m_active_libraries;
	};
}

#endif // MGMK_LOWER_CONTEXT_HXX
