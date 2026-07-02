#pragma once

#ifndef MGMK_EXT_PROVIDED_TARGET_REF_HXX
#define MGMK_EXT_PROVIDED_TARGET_REF_HXX

#include "provider_kind.hxx"

#include <string>

// A provided target ref connects a mgmake target to one target from an external provider project.

namespace mgmake::ext {
	struct provided_target_ref {
		provider_kind m_kind = provider_kind::cmake;
		std::string m_project;
		std::string m_target;
	};
}

#endif // MGMK_EXT_PROVIDED_TARGET_REF_HXX
