#pragma once

#ifndef MGMAKE_DISCOVERY_DEFAULT_TOOLS_HXX
#define MGMAKE_DISCOVERY_DEFAULT_TOOLS_HXX

#include "tool_role.hxx"
#include "tool.hxx"

namespace mgmake::discovery {
	struct default_tools {
		static constexpr auto clang = tool.logical<"clang">();

		static constexpr auto clang_c = clang.role<cc_role>();
		static constexpr auto clang_cxx = clang.role<cxx_role>();
	};
}

#endif // MGMAKE_DISCOVERY_DEFAULT_TOOLS_HXX
