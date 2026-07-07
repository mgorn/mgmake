#pragma once

#ifndef MGMK_PREP_CMAKE_PROJECT_COMPAT_HXX
#define MGMK_PREP_CMAKE_PROJECT_COMPAT_HXX

#include "cmake/project.hxx"

// Compatibility include for code that still includes mgmake/prep/cmake_project.hxx during the phase split.

namespace mgmake::prep {
	using cmake_project = prep::cmake::project;
}

#endif // MGMK_PREP_CMAKE_PROJECT_COMPAT_HXX
