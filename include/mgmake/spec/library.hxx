#pragma once

#ifndef MGMK_SPEC_LIBRARY_HXX
#define MGMK_SPEC_LIBRARY_HXX

#include "../dag/graph.hxx"
#include "../dag/target.hxx"
#include "target.hxx"

namespace mgmake::spec {
	struct library : public target {
		enum struct kind {
			kstatic,
			kdynamic,
			interface
		};
	};
}

#endif