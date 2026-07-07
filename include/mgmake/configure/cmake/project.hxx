#pragma once

#ifndef MGMK_CONFIGURE_CMAKE_PROJECT_HXX
#define MGMK_CONFIGURE_CMAKE_PROJECT_HXX

#include "../../ext/path_root.hxx"

#include <filesystem>
#include <string>

namespace mgmake::configure::cmake {
	struct project {
		// Configured CMake project state needed by prep and lower. This bundles
		// source/build/install dirs with the root policy used later to resolve
		// provider-relative paths.
		std::string m_name;
		std::filesystem::path m_source_dir;
		std::filesystem::path m_build_dir;
		std::filesystem::path m_install_dir;
		ext::path_root m_usage_root = ext::path_root::build;
	};
}

#endif // MGMK_CONFIGURE_CMAKE_PROJECT_HXX
