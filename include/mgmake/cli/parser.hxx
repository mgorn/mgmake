#pragma once

#ifndef MGMAKE_CLI_PARSER_HXX
#define MGMAKE_CLI_PARSER_HXX

#include "options.hxx"

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

			// Match switches
			for (auto it = args.begin(); it != args.end(); ++it) {
				std::string_view arg = *it;

				auto is_long = arg.starts_with("--");
				auto is_short = arg.starts_with("-");
				auto is_switch = is_long or is_short;
				auto is_action = it == args.begin() and not is_switch; // First and isn't switch? -> Action
				// Invalid usage errors + hints
				if (not is_switch and not is_action) {
					std::string error_hint = "";

					// 1) See if it could have been a short switch
					if (error_hint.empty()) {
						auto matches = match(std::format("-{}", arg));
						if (matches.any()) {
							error_hint = std::format("Did you mean '-{}'?", arg);
						}
					}

					// 2) See if it could have been a long switch
					if (error_hint.empty()) {
						auto matches = match(std::format("--{}", arg));
						if (matches.any()) {
							error_hint = std::format("Did you mean '--{}'?", arg);
						}
					}

					// 3) See if the arg is meant to be used as an action
					if (error_hint.empty()) {
						using action_parser = parser<actions_type>;
						auto matches = action_parser::match(arg);
						if (matches.any()) {
							auto corrected = std::format("{} {} ...", cmd.program_name(), arg);
							error_hint = std::format("'{}' is an action, did you mean '{}'?", arg, corrected);
						}
					}

					if (not error_hint.empty()) {
						return std::unexpected(std::format("Invalid argument: {} ({})", arg, error_hint));
					}
					return std::unexpected(std::format("Invalid argument: {}", arg));
				}
				mgmkassert(is_action or is_switch, "Values for switches should be skipped/parsed by the switch needing it");

				// Shrimply doesn't exit?
				auto matches = match(arg);
				if (not matches.any()) {
					return std::unexpected(std::format("Unknown argument: '{}'", arg));
				}
				// If this happens, there's a conflict with option names (either long or short)
				mgmkassert(matches.count() == 1, "Matched arg to more than one option?");

				auto index = detail::index_bit(matches);
				auto result = list_type::type_switch([&]<typename opt_t> -> std::expected<bool, std::string> {
					// If the option expects a value
					if constexpr (opt_t::is_assign) {
						// What is the expected value type?
						using assign_type = opt_t::assign_type;
						// TODO: If value_type is a std::vector or other container,
						// we need to keep reading each arg, parse them, and store...
						using value_type = assign_type::value_type;

						// Is it `--switch=value` or `--switch value`?
						std::string_view value_text{};
						bool move_next = false; // If we need to move the iterator after consuming an arg
						if (const auto seperator = arg.find_first_of("="); seperator != std::string_view::npos) {
							value_text = arg.substr(seperator+1);
							arg = arg.substr(0, seperator);
						} else {
							// Get the next arg
							auto next_it = std::next(it);
							if (next_it == args.end()) {
								return std::unexpected(std::format("argument '{}' expects a value", arg));
							}

							value_text = *next_it;
							move_next = true;
						}

						// assign
						auto result = opt_t::handle_assign(opts, arg, value_text);
						if (not result) {
							return std::unexpected(std::format("opt_t::handle_assign failed: {}", result.error()));
						}

						// Move the iterator
						if (move_next)
							it = std::next(it);
						return true;
					}
					
					// If the option invokes a callback
					if constexpr (opt_t::is_callback) {
						auto result = opt_t::handle_callback(opts, arg);
						if (not result) {
							return std::unexpected(std::format("opt_t::handle_callback failed: {}", result.error()));
						}

						return true;
					}

					// If the option is an action
					if constexpr (opt_t::action_value) {
						auto result = opt_t::handle_action(opts, arg);
						if (not result) {
							return std::unexpected(std::format("opt_t::handle_action failed: {}", result.error()));
						}

						return true;
					}

					return std::unexpected("cli::parser::parse: Not implemented");
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