#pragma once

#ifndef MGMK_SPEC_EXECUTABLE_HXX
#define MGMK_SPEC_EXECUTABLE_HXX

#include "target.hxx"

#include <vector>

namespace mgmake::spec {
	struct executable : public target<executable> {
		using id = std::vector<executable>::size_type;
	};
}

#endif
