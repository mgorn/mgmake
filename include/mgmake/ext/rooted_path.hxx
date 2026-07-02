#pragma once

#ifndef MGMK_EXT_ROOTED_PATH_HXX
#define MGMK_EXT_ROOTED_PATH_HXX

#include "path_root.hxx"

#include <filesystem>

// Rooted paths keep provider-relative paths independent from the final prepared source/build/install directories.

namespace mgmake::ext {
	struct rooted_path {
		path_root m_root = path_root::usage;
		std::filesystem::path m_path;
	};
}

#endif // MGMK_EXT_ROOTED_PATH_HXX
