#pragma once

#ifndef MGMAKE_CLI_PARSER_HXX
#define MGMAKE_CLI_PARSER_HXX

#include "options.hxx"
#include "value_parser.hxx"

#include "../detail/index_bit.hxx"
#include "../meta/type_list.hxx"
#include "../sys/shell.hxx"

#include <bitset>
#include <expected>
#include <optional>
#include <string>
#include <utility>

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
			// The resulting options
			options opts{};

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
			// Pass switch & value args together - DONE
			// Use the `assign_type` to know if the option is expecting a value - DONE
			// Use `set` to know the default? or if it is used without a value?
			// Use the `member_access::value_type` to know the expected type? - Done?
			// automatically handle value/assign hints from the value type?
			//   (E.g. a `std::filesystem::path` would say `--switch=path` instead of just `--switch=value`)
			//   (E.g. maybe also `std::string` would say `--switch=text`?)

			// Match switches
			for (auto it = args.begin(); it != args.end(); ++it) {
				std::string_view arg = *it;

				auto is_long = arg.starts_with("--");
				auto is_short = arg.starts_with("-");
				auto is_switch = is_long or is_short;
				if (not is_switch) {
					// must be a value for the last switch
					continue;
				}
				mgmkassert(is_switch, "Values for switches should be skipped/parsed by the switch needing it");

				auto matches = match(arg);
				if (not matches.any()) {
					return std::unexpected(std::format("Unknown argument: '{}'", arg));
				}
				mgmkassert(matches.count() == 1, "Matched arg to more than one option?");

				auto index = detail::index_bit(matches);
				auto result = list_type::type_switch([&]<typename opt_t> -> std::expected<bool, std::string> {
					// If the option expects a value
					if constexpr (opt_t::is_assign) {
						// What is the expected value type?
						using assign_type = opt_t::assign_type;
						using value_type = assign_type::value_type;

						// Is it `--switch=value` or `--switch value`?
						std::string_view value_text{};
						bool move_next = false; // If we need to move the iterator after consuming an arg
						if (const auto seperator = arg.find_first_of("="); seperator != std::string_view::npos) {
							value_text = arg.substr(seperator+1);
						} else {
							// Get the next arg
							auto next_it = std::next(it);
							if (next_it == args.end()) {
								return std::unexpected(std::format("argument '{}' expects a value", arg));
							}

							value_text = *next_it;
							move_next = true;
						}

						// parse it
						using vp = value_parser<value_type>;
						auto result = vp::parse(value_text);
						if (not result.has_value()) {
							return std::unexpected(std::format("Error parsing value for arg '{}': {}", arg, result.error()));
						}

						// assign
						assign_type::set(opts, result.value());

						// Move the iterator
						if (move_next)
							it = std::next(it);
						return true;
					} else {
						return std::unexpected{"Not implemented"};
					}
				}, index);
				if (not result.has_value()) {
					return std::unexpected(result.error());
				}
			}

            return opts;
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