#pragma once

#ifndef MGMK_LOWER_PROVIDER_BUILD_HXX
#define MGMK_LOWER_PROVIDER_BUILD_HXX

#include "../dag/artifact.hxx"
#include "../dag/target.hxx"

// Provider build results connect an external provider build action to a ready stamp in the lowered DAG.

namespace mgmake::lower {
	struct provider_build {
		dag::target::id m_dag_target{};
		dag::artifact::id m_ready_stamp{};
	};
}

#endif // MGMK_LOWER_PROVIDER_BUILD_HXX
