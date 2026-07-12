#pragma once

#ifndef MGMAKE_CLI_PARSER_HXX
#define MGMAKE_CLI_PARSER_HXX

#include "options.hxx"

#include "../meta/type_list.hxx"
#include "../sys/shell.hxx"

#include <bitset>
#include <expected>
#include <optional>
#include <string>

namespace mgmake::cli {
    template<typename list_t = meta::type_list<>>
    struct parser {
		using list_type = list_t;

		// Action options (first arg, no - or --)
		using actions_type = typename list_type::template filter<[]<typename opt_t> -> bool {
			return opt_t::action_value;
		}>;
		// Switch option (- or -- prefix)
		using switches_type = typename list_type::template filter<[]<typename opt_t> -> bool {
			return not opt_t::action_value;
		}>;

        static inline constexpr std::expected<options, std::string> parse(const sys::shell& cmd) {
			auto args = cmd.user_args();
			auto has_action = args.size() > 0 and not args.at(0).starts_with("-");
			// Have action? -> match it
			if (has_action) {
				auto action = args.at(0);

				// Make the action parser
				using action_parser = parser<actions_type>;
				// Any matches on actions?
				auto matches = action_parser::match(action);
				if (matches.any()) {
					return std::unexpected{ "Matched actions!" };
				} else {
					return std::unexpected{ std::format("Unknown action: '{}'", action) };
				}

				// args should now only be the switches
				args = args.subspan(1);
			}

			// TODO: Figure out what to do about parsing values and assign switches
			// Iterate assign switches seperately?
			// Pass switch & value args together
			// Use the `assign_type` to know if the option is expecting a value
			// Use `set` to know the default? or if it is used without a value?
			// Use the `member_access::value_type` to know the expected type?
			// automatically handle value/assign hints from the value type?
			//   (E.g. a `std::filesystem::path` would say `--switch=path` instead of just `--switch=value`)
			//   (E.g. maybe also `std::string` would say `--switch=text`?)

			// Match switches
			for (std::string_view arg : args) {
				auto is_long = arg.starts_with("--");
				auto is_short = arg.starts_with("-");
				auto is_switch = is_long or is_short;
				if (not is_switch) {
					// TODO: This is probably a value
					// e.g. for `--build-dir .build`
					// arg is probably `.build`
					continue;
				}
				auto matches = match(arg);
				if (matches.any()) {
					return std::unexpected("Matched switch!");
				}
			}

            return std::unexpected("Parser not yet implemented");
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

#endif // MGMAKE_CLI_PARSER_HXX