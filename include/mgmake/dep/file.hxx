#pragma once

#ifndef MGMAKE_DEP_FILE_HXX
#define MGMAKE_DEP_FILE_HXX

#include "format.hxx"

#include <filesystem>

namespace mgmake::dep {
	struct file {
		format_flags m_format{};
		std::filesystem::path m_path;
		std::filesystem::path m_target;
	};
}

#endif // MGMAKE_DEP_FILE_HXX
