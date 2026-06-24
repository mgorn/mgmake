#pragma once

#ifndef MGMK_LOWER_TARGET_HXX
#define MGMK_LOWER_TARGET_HXX

#include "../dag/artifact.hxx"
#include "../dag/target.hxx"

#include <filesystem>
#include <optional>
#include <set>
#include <vector>

// A lowered target is the internal result of lowering a spec target into linkable artifacts and DAG metadata.

namespace mgmake::lower {
	struct target {
		std::optional<dag::target::id> m_dag_target;
		std::vector<dag::artifact::id> m_linkable_artifacts;
		// Include dirs inherited by consumers of this lowered target.
		std::set<std::filesystem::path> m_include_dirs;
		// Artifacts that must exist before consumers use this target's usage data.
		std::vector<dag::artifact::id> m_usage_inputs;
	};
}

#endif // MGMK_LOWER_TARGET_HXX
