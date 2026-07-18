#pragma once

#ifndef MGMAKE_TASK_DISPATCHER_HXX
#define MGMAKE_TASK_DISPATCHER_HXX

#include "../cli/default_options.hxx"
#include "task_traits.hxx"

#include "../sys/exit_code.hxx"
#include "../sys/shell.hxx"

// task::dispatcher
// consumes the cli::options and executes the required task(s)

namespace mgmake::task {
	// The mgmake config
    template<typename config_t>
	struct dispatcher {
		using config_type = config_t;
		using list_type = config_type::tasks_type;

		static inline constexpr std::expected<sys::exit_code, std::string> invoke(const sys::shell& cmd, const auto& opts) {
			if constexpr (not opts.template has<cli::task_option>()) {
				return std::unexpected("cli::dispatcher::invoke cannot invoke without a task!");
			} else {
				return list_type::type_switch([&]<typename task_t> -> std::expected<sys::exit_code, std::string> {
					using traits_type = task_traits<task_t>;
					static_assert(traits_type::template valid_handler<config_type>, "task is missing a handle function");

					return task_t::template handle<config_type>(cmd, opts);
				}, opts.template get<cli::task_option>());
			}
		}

		using matches_type = std::bitset<list_type::size()>;
		static inline constexpr matches_type match(std::string_view arg) {
			return []<std::size_t... Is>(std::index_sequence<Is...>, std::string_view arg) {
				matches_type matches{};
				(matches.set(Is, task_traits<typename list_type::template type_at<Is>>::match(arg)), ...);
				return matches;
			}(std::make_index_sequence<list_type::size()>{}, arg);
		}
	};
}

#endif // MGMAKE_TASK_DISPATCHER_HXX