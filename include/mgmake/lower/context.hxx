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
#include <string_view>
#include <span>
#include <vector>

namespace mgmake::spec {
	struct project;
}

namespace mgmake::lower {
#ifdef MGMK_ENABLE_EXT_CMAKE
	struct cmake_target {
		dag::target::id m_dag_target{};
		dag::artifact::id m_ready_stamp{};
	};
#endif // MGMK_ENABLE_EXT_CMAKE

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
		void lower_executable(spec::executable::id id);
#ifdef MGMK_ENABLE_EXT_CMAKE
		lower::cmake_target lower_cmake_target(
			const ext::provider_ref& provider,
			std::span<const dag::artifact::id> outputs
		);
#endif // MGMK_ENABLE_EXT_CMAKE

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

#ifdef MGMK_ENABLE_EXT_CMAKE
		lower::target lower_provider_library(
			const spec::library& lib,
			lower::usage usage
		);

		void lower_provider_executable(
			const spec::executable& exe,
			lower::usage usage
		);
#endif // MGMK_ENABLE_EXT_CMAKE

		std::vector<std::optional<lower::target>> m_libraries;
		std::set<spec::library::id> m_active_libraries;
	};
}

#endif // MGMK_LOWER_CONTEXT_HXX
