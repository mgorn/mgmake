#pragma once

#ifndef MGMAKE_TASK_FETCH_HXX
#define MGMAKE_TASK_FETCH_HXX

#include "../cli/default_options.hxx"
#include "../sys/exit_code.hxx"

#include <print>

namespace mgmake::task {
	struct fetch {
		static constexpr auto option = cli::option
			.name<"fetch">()
			.description<"Fetch (download, find, etc) all project dependencies.">()
			.set<"task", std::size_t{3}>()
			.task<true>().flag<false>();
		
		template<auto config_v>
		static inline constexpr std::expected<sys::exit_code, std::string> handle(auto& cmd, const auto& opts) {
			// TODO: This would be the entrypoint/root for fetch
			std::println("Fetch task");
			return sys::exit_code::success;
		}
	};
}

#endif // MGMAKE_TASK_FETCH_HXX