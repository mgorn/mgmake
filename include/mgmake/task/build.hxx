#pragma once

#ifndef MGMAKE_TASK_BUILD_HXX
#define MGMAKE_TASK_BUILD_HXX

#include "../cli/option.hxx"
#include "../sys/exit_code.hxx"

namespace mgmake::task {
	struct build {
		using option_type = cli::option
			::name<"build">
			::description<"Build the project.">
			::task<true>::flag<false>
			::build;
		
		template<typename config_t>
		static inline constexpr std::expected<sys::exit_code, std::string> handle(auto& cmd, auto& opts) {
			// TODO: This would be the entrypoint/root for build
			std::println("Build task");
			return sys::exit_code::success;
		}
	};
}

#endif // MGMAKE_TASK_BUILD_HXX