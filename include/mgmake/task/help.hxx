#pragma once

#ifndef MGMAKE_TASK_HELP_HXX
#define MGMAKE_TASK_HELP_HXX

#include "task_traits.hxx"

#include "../cli/option.hxx"
#include "../sys/exit_code.hxx"

#include <print>
#include <sstream>
#include <string>

namespace mgmake::task {
	struct help {
		static constexpr auto option = cli::option
			.name<"help">().short_name<'h'>()
			.description<"Show help.">()
			.task<true>();
		
		template<auto config_v>
		static inline constexpr std::expected<sys::exit_code, std::string> handle(auto& cmd, auto& opts) {
			std::println("Usage:");
			std::println("\t{} [task] [options]", cmd.program_name());
			using tasks_type = decltype(config_v.tasks());
			using options_type = decltype(config_v.option_storage())::list_type;
			
			std::println("\nTasks:");
			tasks_type::for_each([]<typename task_t> {
				using traits_type = task_traits<task_t>;
				std::println("\t{:<10} {}", traits_type::name(), traits_type::description());
			});

			std::println("\nOptions:");
			options_type::for_each([]<auto opt_v> {
				// Only print switches, tasks will be shown first
				if constexpr (opt_v.flag()) {
					std::stringstream ss;
					if constexpr (opt_v.short_name() != '\0') {
						std::print(ss, "-{}, ", opt_v.short_name());
					}
					std::print(ss, "--{}", opt_v.name().view());
					if constexpr (opt_v.parses()) {
						using vp = cli::value_parser<typename decltype(opt_v)::storage_value_type>;
						std::print(ss, "=<{}>", vp::help_hint);
					}
					std::println("\t{:<24} {}", ss.str(), opt_v.description().view());
				}
			});

			return sys::exit_code::success;
		}
	};
}

#endif // MGMAKE_TASK_HELP_HXX