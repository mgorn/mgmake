#pragma once

#ifndef MGMAKE_CLI_PARSER_HXX
#define MGMAKE_CLI_PARSER_HXX

#include "option_storage.hxx"

#include "../detail/index_bit.hxx"
#include "../meta/type_list.hxx"
#include "../meta/type_traits.hxx"
#include "../sys/shell.hxx"

#include <bitset>
#include <expected>
#include <optional>
#include <string>
#include <utility>

namespace mgmake::cli {
    template<typename option_storage_t = option_storage<>>
    struct parser {
		using option_storage_type = option_storage_t;
		using list_type = option_storage_type::list_type;

		// Task options (first arg, no - or --)
		using tasks_type = typename list_type::template filter<[]<typename opt_t> -> bool {
			return opt_t::task_value;
		}>;
		// Switch option (- or -- prefix)
		using switches_type = typename list_type::template filter<[]<typename opt_t> -> bool {
			return not opt_t::task_value;
		}>;

		template<typename dispatcher_t>
        static inline constexpr std::expected<option_storage_type, std::string> parse(const sys::shell& cmd) {
			// The resulting options
			option_storage_type opts{};

			auto args = cmd.user_args();

			// Match switches
			for (auto it = args.begin(); it != args.end(); ++it) {
				std::string_view arg = *it;

				auto is_long = arg.starts_with("--");
				auto is_short = arg.starts_with("-");
				auto is_switch = is_long or is_short;
				auto is_task = it == args.begin() and not is_switch; // First and isn't switch? -> Task
				// Invalid usage errors + hints
				if (not is_switch and not is_task) {
					std::string error_hint = "";

					// 1) See if it could have been a short switch
					if (error_hint.empty()) {
						auto matches = match<list_type>(std::format("-{}", arg));
						if (matches.any()) {
							error_hint = std::format("Did you mean '-{}'?", arg);
						}
					}

					// 2) See if it could have been a long switch
					if (error_hint.empty()) {
						auto matches = match<list_type>(std::format("--{}", arg));
						if (matches.any()) {
							error_hint = std::format("Did you mean '--{}'?", arg);
						}
					}

					// 3) See if the arg is meant to be used as an task
					if (error_hint.empty()) {
						auto matches = match<tasks_type>(arg);
						if (matches.any()) {
							auto corrected = std::format("{} {} ...", cmd.program_name(), arg);
							error_hint = std::format("'{}' is a task, did you mean '{}'?", arg, corrected);
						}
					}

					if (not error_hint.empty()) {
						return std::unexpected(std::format("Invalid argument: {} ({})", arg, error_hint));
					}
					return std::unexpected(std::format("Invalid argument: '{}'", arg));
				}
				mgmkassert(is_task or is_switch, "Values for switches should be skipped/parsed by the switch needing it");

				// Shrimply doesn't exit?
				auto matches = match<list_type>(arg);
				if (not matches.any()) {
					return std::unexpected(std::format("Unknown argument: '{}'", arg));
				}
				// If this happens, there's a conflict with option names (either long or short)
				mgmkassert(matches.count() == 1, "Matched arg to more than one option?");

				auto index = detail::index_bit(matches);
				auto result = list_type::type_switch([&]<typename opt_t> -> std::expected<bool, std::string> {
					// If the option expects a value
					if constexpr (opt_t::parse_value) {
						// What is the expected value type?
						// TODO: If value_type is a std::vector or other container,
						// we need to keep reading each arg, parse them, and store...
						using value_type = opt_t::storage_value_type;

						// Is it `--switch=value` or `--switch value`?
						if (const auto seperator = arg.find_first_of("="); seperator != std::string_view::npos) {
							std::string_view value_text = arg.substr(seperator+1);
							arg = arg.substr(0, seperator);

							// assign
							auto result = opt_t::handle_parse(opts, arg, value_text);
							if (not result) {
								return std::unexpected(std::format("opt_t::handle_assign failed: {}", result.error()));
							}
						} else {
							if constexpr (meta::is_vector_v<value_type>) {
								// Get the next args until it can't be parsed as a value
								for (auto next_it = std::next(it); next_it != args.end(); next_it++) {
									std::string_view value_text = *next_it;
									if (value_text.starts_with("-")) {
										break;
									}
									// assign
									auto result = opt_t::handle_parse(opts, arg, value_text);
									if (not result) {
										break;
									}
									it = next_it;
								}
							} else {
								// Get the next arg
								it = std::next(it);
								if (it == args.end()) {
									return std::unexpected(std::format("argument '{}' expects a value", arg));
								}

								std::string_view value_text = *it;
								// assign
								auto result = opt_t::handle_parse(opts, arg, value_text);
								if (not result) {
									return std::unexpected(std::format("opt_t::handle_assign failed: {}", result.error()));
								}
							}
						}

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

					// If the option is a task
					if constexpr (opt_t::task_value) {
						auto result = opt_t::template handle_task<dispatcher_t>(opts, arg);
						if (not result) {
							return std::unexpected(std::format("opt_t::handle_task failed: {}", result.error()));
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

		template<typename opts_t>
		using matches_type = std::bitset<opts_t::size()>;
		template<typename opts_t>
		static inline constexpr matches_type<opts_t> match(std::string_view arg) {
			return []<std::size_t... Is>(std::index_sequence<Is...>, std::string_view arg) {
				matches_type<opts_t> matches{};
				(matches.set(Is, opts_t::template type_at<Is>::match(arg)), ...);
				return matches;
			}(std::make_index_sequence<opts_t::size()>{}, arg);
		}
    };
}

#endif // MGMAKE_CLI_PARSER_HXX