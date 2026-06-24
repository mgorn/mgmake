#pragma once

#ifndef MGMAKE_CLI_OPTION_PARSER_HXX
#define MGMAKE_CLI_OPTION_PARSER_HXX

#include "option_builder.hxx"
#include "parse_result.hxx"
#include "util.hxx"

#include <format>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <utility>

// The option parser walks argv once, dispatching known options to option_builder descriptors.

namespace mgmake::cli::detail {
	template <typename Option>
	[[nodiscard]] inline std::string option_usage_string() {
		std::string result;

		if constexpr (Option::short_name_value != '\0') {
			result += "-";
			result += Option::short_name_value;
			result += ", ";
		}

		result += "--";
		result += Option::long_name_view();

		constexpr option_mode mode = actual_option_mode<
			Option::field_value,
			Option::mode_value
		>::value;

		if constexpr (mode == option_mode::value || mode == option_mode::append) {
			result += " <";

			if constexpr (!Option::value_name_value.empty()) {
				result += Option::value_name_view();
			} else {
				result += "value";
			}

			result += ">";
		}

		return result;
	}

	template <typename Option>
	[[nodiscard]] inline std::string explicit_choices_string() {
		std::string result;
		bool first = true;

		Option::choices_type::for_each([&](std::string_view choice) {
			if (!first) {
				result += ", ";
			}

			result += choice;
			first = false;
		});

		return result;
	}

	template <typename Parser>
	[[nodiscard]] inline std::string parser_choices_string() {
		if constexpr (has_choices_string<Parser>) {
			return Parser::choices_string();
		} else if constexpr (has_for_each_choice<Parser>) {
			std::string result;
			bool first = true;

			Parser::for_each_choice([&](std::string_view choice) {
				if (!first) {
					result += ", ";
				}

				result += choice;
				first = false;
			});

			return result;
		} else {
			return {};
		}
	}

	template <typename Option>
	[[nodiscard]] inline std::string option_choices_string() {
		if constexpr (!Option::choices_type::empty()) {
			return explicit_choices_string<Option>();
		} else {
			constexpr option_mode mode = actual_option_mode<
				Option::field_value,
				Option::mode_value
			>::value;

			if constexpr (mode == option_mode::value || mode == option_mode::append) {
				using field_type = member_value_t<Option::field_value>;
				using value_type = option_value_t<field_type>;

				return parser_choices_string<value_parser<value_type>>();
			} else {
				return {};
			}
		}
	}

	template <typename Option>
	inline void print_option_help_row() {
		std::string usage = option_usage_string<Option>();
		std::string desc = std::string{ Option::description_view() };
		std::string choices = option_choices_string<Option>();

		if (!choices.empty()) {
			if (!desc.empty()) {
				desc += " ";
			}

			desc += "Choices: ";
			desc += choices;
			desc += ".";
		}

		std::println("  {:<24} {}", usage, desc);
	}

	inline void print_commands_help() {
		for_each_action_help([](
			[[maybe_unused]] action_kind action,
			std::string_view name,
			std::string_view description
		) {
			std::println("  {:<10} {}", name, description);
		});
	}
}

namespace mgmake::cli {
	template <typename... Options>
	struct option_parser {
		static void print_options_help() {
			(detail::print_option_help_row<Options>(), ...);
		}

		static void print_help(std::string_view program_name) {
			if (program_name.empty()) {
				program_name = "mgmake";
			}

			std::println("usage:");
			std::println("  {} [command] [options] [targets...] [-- args...]", program_name);
			std::println("");
			std::println("commands:");
			detail::print_commands_help();
			std::println("");
			std::println("options:");
			print_options_help();
		}

		[[nodiscard]] static option_parse_result try_parse_option(
			options& opts,
			std::span<const std::string> args,
			std::size_t& index,
			std::string_view arg
		) {
			option_parse_result result = option_parse_result::no_match();
			bool matched = false;

			([&] {
				if (!matched) {
					result = Options::try_parse(opts, args, index, arg);
					matched = result.m_matched;
				}
			}(), ...);

			return result;
		}

		[[nodiscard]] static parse_result parse(std::span<const std::string> args) {
			options opts;
			bool saw_first_positional = false;

			for (std::size_t i = 0; i < args.size(); ++i) {
				std::string_view arg = args[i];

				if (arg == "--") {
					auto& out = opts.m_action == action_kind::run
						? opts.m_run_args
						: opts.m_passthrough_args;

					for (++i; i < args.size(); ++i) {
						out.emplace_back(args[i]);
					}

					break;
				}

				option_parse_result option_result = try_parse_option(opts, args, i, arg);

				if (option_result.m_matched) {
					if (!option_result.m_ok) {
						return parse_result::failure(std::move(option_result.m_error));
					}

					continue;
				}

				if (is_option(arg)) {
					return parse_result::failure(std::format("unknown option '{}'", arg));
				}

				if (!saw_first_positional) {
					saw_first_positional = true;

					action_kind parsed_action{};

					if (parse_action(arg, parsed_action)) {
						opts.m_action = parsed_action;

						if (parsed_action == action_kind::help) {
							opts.m_show_help = true;
						}

						if (parsed_action == action_kind::version) {
							opts.m_show_version = true;
						}

						continue;
					}
				}

				opts.m_targets.emplace_back(arg);
			}

			return parse_result::success(std::move(opts));
		}
	};
}

#endif // MGMAKE_CLI_OPTION_PARSER_HXX
