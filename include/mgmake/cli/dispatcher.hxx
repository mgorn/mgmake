#pragma once

#ifndef MGMAKE_CLI_DISPATCHER_HXX
#define MGMAKE_CLI_DISPATCHER_HXX

#include "options.hxx"

#include "../sys/exit_code.hxx"
#include "../sys/shell.hxx"

// cli::dispatcher
// consumes the cli::options and executes the required action(s)

namespace mgmake::cli {
	// The mgmake config
    template<typename config_t>
	struct dispatcher {
		using config_type = config_t;
		using list_type = config_type::actions_type;

		static inline constexpr std::expected<sys::exit_code, std::string> invoke(const sys::shell& cmd, const cli::options& opts) {
			if (not opts.action().has_value()) {
				return std::unexpected("cli::dispatcher::invoke cannot invoke without an action!");
			}
			return list_type::type_switch([&]<typename action_t> -> std::expected<sys::exit_code, std::string> {
				static_assert(action_t::valid_handler, "action_t handler is invalid");
				return action_t::template invoke<config_type>(cmd, opts);
			}, opts.action().value());
		}

		using matches_type = std::bitset<list_type::size()>;
		static inline constexpr matches_type match(std::string_view arg) {
			return []<std::size_t... Is>(std::index_sequence<Is...>, std::string_view arg) {
				matches_type matches{};
				(matches.set(Is, list_type::template type_at<Is>::match(arg)), ...);
				return matches;
			}(std::make_index_sequence<list_type::size()>{}, arg);
		}
	};
}

#endif // MGMAKE_CLI_DISPATCHER_HXX