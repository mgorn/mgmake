#pragma once

#ifndef MGMAKE_TASK_BUILD_HXX
#define MGMAKE_TASK_BUILD_HXX

#include "../sys/exit_code.hxx"

#include <print>

namespace mgmake::task {
	struct build {
		using option_type = cli::option
			::name<"build">
			::description<"Build the project.">
			::set<"task", std::size_t{0}>
			::task<true>::flag<false>
			::build;
		
		template<typename config_t>
		static inline constexpr std::expected<sys::exit_code, std::string> handle(auto& cmd, auto& opts) {
			// TODO: This would be the entrypoint/root for build
			std::println("Build task");
			std::println("Build dir: {}", opts.template get<cli::build_dir_option>().string());
			return sys::exit_code::success;
		}
	};
}

#endif // MGMAKE_TASK_BUILD_HXX