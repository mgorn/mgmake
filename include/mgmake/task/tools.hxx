#pragma once

#ifndef MGMAKE_TASK_TOOLS_HXX
#define MGMAKE_TASK_TOOLS_HXX

#include "../cli/default_options.hxx"
#include "../sys/exit_code.hxx"

#include <print>

namespace mgmake::task {
	struct tools {
		static constexpr auto option = cli::option
			.name<"tools">()
			.description<"Discover and cache required tools for the project.">()
			.task<true>().flag<false>();
		
		template<auto config_v>
		static inline constexpr std::expected<sys::exit_code, std::string> handle(auto& cmd, const auto& opts) {
			std::println("Tools task");
			return sys::exit_code::success;
		}
	};
}

#endif // MGMAKE_TASK_TOOLS_HXX