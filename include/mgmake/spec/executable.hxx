#pragma once

#ifndef MGMK_SPEC_EXECUTABLE_HXX
#define MGMK_SPEC_EXECUTABLE_HXX

#include "../dag/target.hxx"
#include "target.hxx"

#include <string_view>
#include <vector>

namespace mgmake::spec {
	struct executable : public target<executable> {
		using id = std::vector<executable>::size_type;
	};
}

#endif