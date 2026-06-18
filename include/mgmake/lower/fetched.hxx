#pragma once

#ifndef MGMAKE_LOWER_FETCHED_HXX
#define MGMAKE_LOWER_FETCHED_HXX

#include "../dag/artifact.hxx"
#include "../dag/target.hxx"

#include <filesystem>

namespace mgmake::lower {
	struct fetched {
		dag::target::id m_target{};
		dag::artifact::id m_stamp{};
		std::filesystem::path m_source_dir;
	};
}

#endif
