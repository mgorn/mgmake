#pragma once

#ifndef MGMAKE_TASK_CLEAN_HXX
#define MGMAKE_TASK_CLEAN_HXX

#include "../cli/default_options.hxx"
#include "../sys/exit_code.hxx"

#include <print>

namespace mgmake::task {
	struct clean {
		static constexpr auto option = cli::option
			.name<"clean">()
			.description<"Delete all build files.">()
			.task<true>().flag<false>();
		
		template<auto config_v>
		static inline constexpr std::expected<sys::exit_code, std::string> handle(auto& cmd, const auto& opts) {
			// TODO: This would be the entrypoint/root for clean
			std::println("Clean task");
			return sys::exit_code::success;
		}
	};
}

#endif // MGMAKE_TASK_CLEAN_HXX