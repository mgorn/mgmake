#pragma once

#ifndef MGMAKE_FIND_TOOLS_HXX
#define MGMAKE_FIND_TOOLS_HXX

#include <filesystem>

namespace mgmake::find {
	template<typename tools_t = meta::value_list<>, typename toolchains_t = meta::value_list<>>
	struct tools_impl {
		using tools_type = tools_t;
		using toolchains_type = toolchains_t;

		// Find a tool's full path
		template<auto tool_v>
		static std::filesystem::path find(const auto& opts) {

		}
	};
}

#endif // MGMAKE_FIND_TOOLS_HXX