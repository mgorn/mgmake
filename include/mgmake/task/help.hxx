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
			.set<"task", std::size_t{1}>()
			.task<true>();
		
		template<auto config_v>
		static inline constexpr std::expected<sys::exit_code, std::string> handle(auto& cmd, auto& opts) {
			std::println("Usage:");
			std::println("\t{} [task] [options]", cmd.program_name());
			using tasks_type = decltype(config_v.tasks());
			using options_type = decltype(config_v.option_storage())::type::list_type;
			
			std::println("\nTasks:");
			static constexpr auto task_help = []<typename task_t>(auto& cmd){
				using traits_type = task_traits<task_t>;
				std::println("\t{:<10} {}", traits_type::name(), traits_type::description());
			};

			[]<std::size_t... Is>(std::index_sequence<Is...>, auto& cmd) {
				(task_help.template operator()<typename tasks_type::template type_at<Is>>(cmd), ...);
			}(std::make_index_sequence<tasks_type::size()>{}, cmd);

			std::println("\nOptions:");
			static constexpr auto option_help = []<auto opt_v>{
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
			};

			[]<std::size_t... Is>(std::index_sequence<Is...>) {
				(option_help.template operator()<options_type::template value_at<Is>>(), ...);
			}(std::make_index_sequence<options_type::size()>{});

			return sys::exit_code::success;
		}
	};
}

#endif // MGMAKE_TASK_HELP_HXX