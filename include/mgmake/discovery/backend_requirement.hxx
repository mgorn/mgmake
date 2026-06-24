#pragma once

#ifndef MGMAKE_DISCOVERY_BACKEND_REQUIREMENT_HXX
#define MGMAKE_DISCOVERY_BACKEND_REQUIREMENT_HXX

#include "tool_role.hxx"

#include <string>

// Backend tool requirements describe tools needed by a backend itself, such as ninja.

namespace mgmake::discovery {
	struct backend_tool_requirement {
		tool_role m_role{};
		std::string m_logical_name{};
		std::string m_needed_because{};
	};
}

#endif // MGMAKE_DISCOVERY_BACKEND_REQUIREMENT_HXX
