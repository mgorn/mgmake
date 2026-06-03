#pragma once

#ifndef MGMAKE_CLI_PARSE_HXX
#define MGMAKE_CLI_PARSE_HXX

#include "options.hxx"
#include "util.hxx"

#include <format>
#include <vector>
#include <span>
#include <string>
#include <string_view>

namespace mgmake::cli {
	struct parse_result {
		bool m_ok = false;
		options m_value{};
		std::string m_error{};

		[[nodiscard]] operator bool() const {
			return m_ok;
		}
		[[nodiscard]] operator options() const {
			return m_value;
		}

		static parse_result success(options opts) {
			parse_result result;
			result.m_ok = true;
			result.m_value = std::move(opts);
			return result;
		}

		static parse_result failure(std::string message) {
			parse_result result;
			result.m_ok = false;
			result.m_error = std::move(message);
			return result;
		}
	};

	[[nodiscard]] inline constexpr parse_result parse(std::span<const std::string> args) {
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

			if (arg == "-h" || arg == "--help") {
				opts.m_action = action_kind::help;
				opts.m_show_help = true;
				continue;
			}

			if (arg == "--version") {
				opts.m_action = action_kind::version;
				opts.m_show_version = true;
				continue;
			}

			if (arg == "-v" || arg == "--verbose") {
				opts.m_verbose = true;
				continue;
			}

			if (arg == "--dry-run") {
				opts.m_dry_run = true;
				continue;
			}

			if (arg == "--backend") {
				std::string value;
				std::string error;

				if (!consume_value(args, i, arg, value, error)) {
					return parse_result::failure(std::move(error));
				}

				backend_kind backend{};

				if (!parse_backend(value, backend)) {
					return parse_result::failure(
						std::format("unknown backend '{}'", value)
					);
				}

				opts.m_backend = backend;
				continue;
			}

			if (arg.starts_with("--backend=")) {
				std::string_view value = arg.substr(std::string_view("--backend=").size());

				backend_kind backend{};

				if (!parse_backend(value, backend)) {
					return parse_result::failure(
						std::format("unknown backend '{}'", value)
					);
				}

				opts.m_backend = backend;
				continue;
			}

			if (arg == "--build-dir") {
				std::string value;
				std::string error;

				if (!consume_value(args, i, arg, value, error)) {
					return parse_result::failure(std::move(error));
				}

				opts.m_build_dir = std::move(value);
				continue;
			}

			if (arg.starts_with("--build-dir=")) {
				std::string_view value = arg.substr(std::string_view("--build-dir=").size());

				if (value.empty()) {
					return parse_result::failure("missing value after '--build-dir='");
				}

				opts.m_build_dir = std::string(value);
				continue;
			}

			if (arg == "--target") {
				std::string value;
				std::string error;

				if (!consume_value(args, i, arg, value, error)) {
					return parse_result::failure(std::move(error));
				}

				opts.m_targets.emplace_back(std::move(value));
				continue;
			}

			if (arg.starts_with("--target=")) {
				std::string_view value = arg.substr(std::string_view("--target=").size());

				if (value.empty()) {
					return parse_result::failure("missing value after '--target='");
				}

				opts.m_targets.emplace_back(value);
				continue;
			}

			if (arg == "-j" || arg == "--jobs") {
				std::string value;
				std::string error;

				if (!consume_value(args, i, arg, value, error)) {
					return parse_result::failure(std::move(error));
				}

				int jobs = 0;

				if (!parse_positive_int(value, jobs)) {
					return parse_result::failure(
						std::format("invalid job count '{}'", value)
					);
				}

				opts.m_jobs = jobs;
				continue;
			}

			if (arg.starts_with("--jobs=")) {
				std::string_view value = arg.substr(std::string_view("--jobs=").size());

				int jobs = 0;

				if (!parse_positive_int(value, jobs)) {
					return parse_result::failure(
						std::format("invalid job count '{}'", value)
					);
				}

				opts.m_jobs = jobs;
				continue;
			}

			if (arg.starts_with("-j") && arg.size() > 2) {
				std::string_view value = arg.substr(2);

				int jobs = 0;

				if (!parse_positive_int(value, jobs)) {
					return parse_result::failure(
						std::format("invalid job count '{}'", value)
					);
				}

				opts.m_jobs = jobs;
				continue;
			}

			if (is_option(arg)) {
				return parse_result::failure(
					std::format("unknown option '{}'", arg)
				);
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

	inline constexpr auto parse(const sys::command_line& cmd) {
		return parse(cmd.m_args);
	}
}

#endif