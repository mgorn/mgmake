#pragma once

#ifndef MGMK_PREP_FINALIZE_HXX
#define MGMK_PREP_FINALIZE_HXX

#include "result.hxx"
#include "../acquire/result.hxx"
#include "../build/request.hxx"
#include "../configure/result.hxx"
#ifdef MGMK_ENABLE_EXT_CMAKE
#include "cmake/finalize.hxx"
#endif // MGMK_ENABLE_EXT_CMAKE
#include "../spec/project.hxx"

#include <expected>
#include <string>
#include <utility>
#include <utility>

namespace mgmake::prep {
	[[nodiscard]] inline std::expected<prep::result, std::string> finalize(
		const spec::project& project,
		const build::request& req,
		const acquire::result& acquired,
		const configure::result& configured
	) {
		(void)project;
		(void)req;
		(void)acquired;

		prep::result result{};

#ifdef MGMK_ENABLE_EXT_CMAKE
		for (const auto& [name, configured_cmake] : configured.m_cmake_projects) {
			auto prepared = prep::cmake::finalize_project(configured_cmake);

			if (!prepared) {
				return std::unexpected{prepared.error()};
			}

			result.m_cmake_projects.insert_or_assign(name, std::move(*prepared));
		}
#endif // MGMK_ENABLE_EXT_CMAKE

		return result;
	}
}

#endif // MGMK_PREP_FINALIZE_HXX
