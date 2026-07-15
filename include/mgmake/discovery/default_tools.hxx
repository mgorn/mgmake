#pragma once

#ifndef MGMAKE_DISCOVERY_DEFAULT_TOOLS_HXX
#define MGMAKE_DISCOVERY_DEFAULT_TOOLS_HXX

#include "tool_role.hxx"
#include "tool.hxx"

namespace mgmake::discovery {
	struct default_tools {
		using clang = tool::logical<"clang">;
		
		using clang_c = clang::role<cc_role>::build;
		using clang_cxx = clang::role<cxx_role>::build;
	};
}

#endif // MGMAKE_DISCOVERY_DEFAULT_TOOLS_HXX