#pragma once

#ifndef MGMK_PREP_CMAKE_FINALIZE_HXX
#define MGMK_PREP_CMAKE_FINALIZE_HXX

#include "project.hxx"
#include "../../configure/cmake/project.hxx"
#include "../../ext/cmake/file_api.hxx"

#include <expected>
#include <string>

namespace mgmake::prep::cmake {
	[[nodiscard]] inline std::expected<prep::cmake::project, std::string> finalize_project(
		const configure::cmake::project& configured
	) {
		prep::cmake::project result{};
		result.m_name = configured.m_name;
		result.m_source_dir = configured.m_source_dir;
		result.m_build_dir = configured.m_build_dir;
		result.m_install_dir = configured.m_install_dir;
		result.m_usage_root = configured.m_usage_root;

		if constexpr (!ext::has_json_backend) {
			return std::unexpected{
				"mgmake prep: CMake File API metadata for project '" + configured.m_name +
				"' requires a JSON backend. Define MGMK_JSON_BACKEND_HEADER or enable #urlinclude support."
			};
		}

		const auto loaded_targets = ext::cmake::file_api::load_reply_targets(
			result.m_codemodel,
			configured.m_build_dir
		);

		if (loaded_targets == 0) {
			return std::unexpected{
				"mgmake prep: CMake project '" + configured.m_name +
				"' loaded zero File API targets from '" +
				configured.m_build_dir.string() + "'"
			};
		}

		return result;
	}
}

#endif // MGMK_PREP_CMAKE_FINALIZE_HXX
