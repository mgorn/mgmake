#pragma once

#ifndef MGMAKE_DEP_FILE_HXX
#define MGMAKE_DEP_FILE_HXX

#include "format.hxx"

#include <filesystem>

// Describes one compiler-emitted dependency side file and the output target it belongs to.

namespace mgmake::dep {
	struct file {
		format_flags m_format{};
		std::filesystem::path m_path;
		std::filesystem::path m_target;
	};
}

#endif // MGMAKE_DEP_FILE_HXX
