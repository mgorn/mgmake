#pragma once

#ifndef MGMAKE_CLI_DISPATCHER_HXX
#define MGMAKE_CLI_DISPATCHER_HXX

#include "options.hxx"

#include "../sys/exit_code.hxx"

// cli::dispatcher
// consumes the cli::options and executes the required action(s)

namespace mgmake::cli {
	template<typename list_t = meta::type_list<>>
	struct dispatcher {
		static inline constexpr std::expected<sys::exit_code, std::string> invoke(const cli::options& opts) {
			if (not opts.action().has_value()) {
				return std::unexpected("cli::dispatcher::invoke cannot invoke without an action!");
			}
			return list_t::type_switch([&]<typename action_t> -> std::expected<sys::exit_code, std::string> {
				static_assert(action_t::valid_handler, "action_t handler is invalid");
				return action_t::invoke(opts);
			}, opts.action().value());
		}

		using matches_type = std::bitset<list_t::size()>;
		static inline constexpr matches_type match(std::string_view arg) {
			return []<std::size_t... Is>(std::index_sequence<Is...>, std::string_view arg) {
				matches_type matches{};
				(matches.set(Is, list_t::template type_at<Is>::match(arg)), ...);
				return matches;
			}(std::make_index_sequence<list_t::size()>{}, arg);
		}
	};
}

#endif // MGMAKE_CLI_DISPATCHER_HXX