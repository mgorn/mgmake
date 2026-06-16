#pragma once

#ifndef MGMAKE_DISCOVERY_TOOL_BINDING_HXX
#define MGMAKE_DISCOVERY_TOOL_BINDING_HXX

#include "tool_role.hxx"

#include <string>

namespace mgmake::discovery {
	struct tool_binding {
		tool_role m_role{};
		std::string m_name{};
	};
}

#endif
