#pragma once

#ifndef MGMAKE_TASK_FETCH_HXX
#define MGMAKE_TASK_FETCH_HXX

#include "../cli/default_options.hxx"
#include "../sys/exit_code.hxx"

#include <print>

namespace mgmake::task {
	struct fetch {
		using option_type = cli::option
			::name<"fetch">
			::description<"Fetch (download, find, etc) all project dependencies.">
			::set<"task", std::size_t{3}>
			::task<true>::flag<false>
			::build;
		
		template<typename config_t>
		static inline constexpr std::expected<sys::exit_code, std::string> handle(auto& cmd, const auto& opts) {
			// TODO: This would be the entrypoint/root for clean
			std::println("Fetch task");
			return sys::exit_code::success;
		}
	};
}

#endif // MGMAKE_TASK_FETCH_HXX