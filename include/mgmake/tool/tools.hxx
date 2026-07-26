#pragma once

#ifndef MGMAKE_TOOL_TOOLS_HXX
#define MGMAKE_TOOL_TOOLS_HXX

#include <filesystem>

namespace mgmake::tool {
	template<typename tools_t = meta::value_list<>, typename toolchains_t = meta::value_list<>>
	struct tools_impl {
		using tools_type = tools_t;
		using toolchains_type = toolchains_t;

		// Find a tool's full path
		template<auto tool_v>
		static std::filesystem::path find(const auto& opts) {
			// 1) Check CLI override
			// 2) Check environment override
			// 3) Check selected toolchain
			// 4) Check for default tool name
			// 5) Check other toolchains for possible tool
			// 6) Error, tool needed but couldn't be found
			
			// At each step:
			//  a) Is it a full path & valid?
			//  b) Is it a named executable anywhere on PATH?
			//  c) Platform specific stuff?
			//   i) Windows Registry?
			//    o) SDK tool with known regitry values?
			//   ii) xcrun mac sdk stuff?
			//   iii) SDK tool with known path?
			//  d) Within any common executable folders on PATH?
			//   i) bin?
			//   ii) lib?
			//   iii) others?
		}
	};
}

#endif // MGMAKE_TOOL_TOOLS_HXX