#pragma once

#ifndef MGMK_ACQUIRE_FETCHED_HXX
#define MGMK_ACQUIRE_FETCHED_HXX

#include "../dag/artifact.hxx"
#include "../dag/target.hxx"

#include <filesystem>

namespace mgmake::acquire {
	struct fetched {
		// Graph-local acquisition target and stamp plus the resulting source directory.
		// This is more than a source-dir wrapper: the ids are useful for graph output
		// and for relating acquisition actions to their filesystem result.
		dag::target::id m_target{};
		dag::artifact::id m_stamp{};
		std::filesystem::path m_source_dir;
	};
}

#endif // MGMK_ACQUIRE_FETCHED_HXX
