#pragma once

#ifndef MGMK_SPEC_EXECUTABLE_HXX
#define MGMK_SPEC_EXECUTABLE_HXX

#include "../dag/target.hxx"
#include "target.hxx"

#include <string_view>

namespace mgmake::spec {
	struct executable : public target {};
}

#endif