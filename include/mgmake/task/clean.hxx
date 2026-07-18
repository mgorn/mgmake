#pragma once

#ifndef MGMAKE_TASK_CLEAN_HXX
#define MGMAKE_TASK_CLEAN_HXX

#include "../cli/default_options.hxx"
#include "../sys/exit_code.hxx"

#include <print>

namespace mgmake::task {
	struct clean {
		using option_type = cli::option
			::name<"clean">
			::description<"Delete all build files.">
			::set<"task", std::size_t{2}>
			::task<true>::flag<false>
			::build;
		
		template<typename config_t>
		static inline constexpr std::expected<sys::exit_code, std::string> handle(auto& cmd, const auto& opts) {
			// TODO: This would be the entrypoint/root for clean
			std::println("Clean task");
			return sys::exit_code::success;
		}
	};
}

#endif // MGMAKE_TASK_CLEAN_HXX