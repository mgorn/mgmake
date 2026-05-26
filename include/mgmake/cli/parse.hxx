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
		bool ok = false;
		options value{};
		std::string error{};

		[[nodiscard]] explicit operator bool() const {
			return ok;
		}

		static parse_result success(options opts) {
			parse_result result;
			result.ok = true;
			result.value = std::move(opts);
			return result;
		}

		static parse_result failure(std::string message) {
			parse_result result;
			result.ok = false;
			result.error = std::move(message);
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
					opts.passthrough_args.emplace_back(args[i]);
				}

				break;
			}

			if (arg == "-h" || arg == "--help") {
				opts.action = action_kind::help;
				opts.show_help = true;
				continue;
			}

			if (arg == "--version") {
				opts.action = action_kind::version;
				opts.show_version = true;
				continue;
			}

			if (arg == "-v" || arg == "--verbose") {
				opts.verbose = true;
				continue;
			}

			if (arg == "--dry-run") {
				opts.dry_run = true;
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

				opts.backend = backend;
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

				opts.backend = backend;
				continue;
			}

			if (arg == "--build-dir") {
				std::string value;
				std::string error;

				if (!consume_value(args, i, arg, value, error)) {
					return parse_result::failure(std::move(error));
				}

				opts.build_dir = std::move(value);
				continue;
			}

			if (arg.starts_with("--build-dir=")) {
				std::string_view value = arg.substr(std::string_view("--build-dir=").size());

				if (value.empty()) {
					return parse_result::failure("missing value after '--build-dir='");
				}

				opts.build_dir = std::string(value);
				continue;
			}

			if (arg == "--target") {
				std::string value;
				std::string error;

				if (!consume_value(args, i, arg, value, error)) {
					return parse_result::failure(std::move(error));
				}

				opts.targets.emplace_back(std::move(value));
				continue;
			}

			if (arg.starts_with("--target=")) {
				std::string_view value = arg.substr(std::string_view("--target=").size());

				if (value.empty()) {
					return parse_result::failure("missing value after '--target='");
				}

				opts.targets.emplace_back(value);
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

				opts.jobs = jobs;
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

				opts.jobs = jobs;
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

				opts.jobs = jobs;
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
					opts.action = parsed_action;

					if (parsed_action == action_kind::help) {
						opts.show_help = true;
					}

					if (parsed_action == action_kind::version) {
						opts.show_version = true;
					}

					continue;
				}
			}

			opts.targets.emplace_back(arg);
		}

		return parse_result::success(std::move(opts));
	}
}

#endif