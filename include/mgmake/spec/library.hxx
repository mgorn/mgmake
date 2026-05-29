#pragma once

#ifndef MGMK_SPEC_LIBRARY_HXX
#define MGMK_SPEC_LIBRARY_HXX

#include "../dag/target.hxx"

namespace mgmake::spec {
	struct library {
		enum struct kind {
			static,
			dynamic,
			interface
		};
		struct project& m_project;
		dag::target::id m_graph_target;
	};
}

#endif