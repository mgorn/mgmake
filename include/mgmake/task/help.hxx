#pragma once

#ifndef MGMAKE_TASK_HELP_HXX
#define MGMAKE_TASK_HELP_HXX

#include "task_traits.hxx"

#include "../cli/option.hxx"
#include "../sys/exit_code.hxx"

#include <sstream>
#include <string>

namespace mgmake::task {
	struct help {
		using option_type = cli::option
			::name<"help">::short_name<'h'>
			::description<"Show help.">
			::task<true>
			//::set<meta::member_access<&cli::options::m_task>, 0>
			::callback<[](auto& opts) {
				// Set index with callback bc cyclical include :(
				opts.m_task = 0;
			}>
			::build;
		
		template<typename config_t>
		static inline constexpr std::expected<sys::exit_code, std::string> handle(auto& cmd, auto& opts) {
			using config_type = config_t;

			std::println("Usage:");
			std::println("\t{} [task] [options]", cmd.program_name());
			using tasks_type = config_type::tasks_type;
			using options_type = config_type::options_type;
			
			std::println("\nTasks:");
			static constexpr auto task_help = []<typename task_t>(auto& cmd){
				using traits_type = task_traits<task_t>;
				std::println("\t{:<10} {}", traits_type::name(), traits_type::description());
			};

			[]<std::size_t... Is>(std::index_sequence<Is...>, auto& cmd) {
				(task_help.template operator()<typename tasks_type::template type_at<Is>>(cmd), ...);
			}(std::make_index_sequence<tasks_type::size()>{}, cmd);

			std::println("\nOptions:");
			static constexpr auto option_help = []<typename opt_t>{
				// Only print switches, tasks will be shown first
				if constexpr (opt_t::flag_value) {
					std::stringstream ss;
					if constexpr (opt_t::short_name_value != '\0') {
						std::print(ss, "-{}, ", opt_t::short_name_value);
					}
					std::print(ss, "--{}", opt_t::name_value.view());
					if constexpr (opt_t::is_assign) {
						using vp = cli::value_parser<typename opt_t::assign_type::value_type>;
						std::print(ss, "=<{}>", vp::help_hint);
					}
					std::println("\t{:<24} {}", ss.str(), opt_t::description_value.view());
				}
			};

			[]<std::size_t... Is>(std::index_sequence<Is...>) {
				(option_help.template operator()<typename options_type::template type_at<Is>>(), ...);
			}(std::make_index_sequence<options_type::size()>{});

			return sys::exit_code::success;
		}
	};
}

#endif // MGMAKE_TASK_HELP_HXX