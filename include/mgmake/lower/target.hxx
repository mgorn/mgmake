#pragma once

#ifndef MGMK_LOWER_TARGET_HXX
#define MGMK_LOWER_TARGET_HXX

#include "../dag/artifact.hxx"
#include "../dag/target.hxx"

#include <filesystem>
#include <optional>
#include <set>
#include <vector>

namespace mgmake::lower {
	struct target {
		std::optional<dag::target::id> m_dag_target;
		std::vector<dag::artifact::id> m_linkable_artifacts;
		std::set<std::filesystem::path> m_include_dirs;
	};
}

#endif
