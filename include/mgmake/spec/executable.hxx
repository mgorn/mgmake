#pragma once

#ifndef MGMK_SPEC_EXECUTABLE_HXX
#define MGMK_SPEC_EXECUTABLE_HXX

#include "../dag/target.hxx"

namespace mgmake::spec {
	struct executable {
		struct project& m_project;
		dag::target::id m_graph_target;
	};
}

#endif