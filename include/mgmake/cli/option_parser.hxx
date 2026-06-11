#pragma once

#ifndef MGMAKE_CLI_OPTION_PARSER_HXX
#define MGMAKE_CLI_OPTION_PARSER_HXX

#include "option_builder.hxx"
#include "parse_result.hxx"
#include "util.hxx"

#include <format>
#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace mgmake::cli {
	template <typename... Options>
	struct option_parser {
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
					for (++i; i < args.size(); ++i) {
						opts.m_passthrough_args.emplace_back(args[i]);
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

#endif
