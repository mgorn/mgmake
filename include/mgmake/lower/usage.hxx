#pragma once

#ifndef MGMK_LOWER_USAGE_HXX
#define MGMK_LOWER_USAGE_HXX

#include "../dag/artifact.hxx"
#include "../dag/target.hxx"

#include <filesystem>
#include <set>
#include <vector>

namespace mgmake::lower {
	struct usage {
		std::set<std::filesystem::path> m_include_dirs;
		std::vector<dag::artifact::id> m_link_inputs;
		std::set<dag::target::id> m_dag_dependencies;
	};
}

#endif
