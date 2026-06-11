#pragma once

#ifndef MGMAKE_CLI_PARSE_HXX
#define MGMAKE_CLI_PARSE_HXX

#include "option_parser.hxx"
#include "../sys/command_line.hxx"

namespace mgmake::cli {
	inline void apply_help(options& opts) {
		opts.m_action = action_kind::help;
		opts.m_show_help = true;
	}

	inline void apply_version(options& opts) {
		opts.m_action = action_kind::version;
		opts.m_show_version = true;
	}

	using help_option =
		callback_option<"help", 'h', apply_help>
			::description<"Show help.">;

	using version_option =
		callback_option<"version", '\0', apply_version>
			::description<"Show version information.">;

	using verbose_option =
		flag_option<&options::m_verbose, "verbose", 'v'>
			::description<"Print commands before executing them.">;

	using dry_run_option =
		flag_option<&options::m_dry_run, "dry-run">
			::description<"Print commands without executing them.">;

	using backend_option =
		value_option<&options::m_backend, "backend">
			::value_name<"name">
			::description<"Select a build backend to use.">;

	using build_dir_option =
		value_option<&options::m_build_dir, "build-dir">
			::value_name<"path">
			::description<"Set the build directory.">;

	using jobs_option =
		value_option<&options::m_jobs, "jobs", 'j'>
			::value_name<"count">
			::description<"Set the maximum number of parallel jobs.">;

	using target_option =
		append_option<&options::m_targets, "target">
			::value_name<"name">
			::description<"Build a specific target. May be passed multiple times.">;

	using default_parser = option_parser<
		help_option,
		version_option,
		verbose_option,
		dry_run_option,
		backend_option,
		build_dir_option,
		jobs_option,
		target_option
	>;

	[[nodiscard]] inline parse_result parse(std::span<const std::string> args) {
		return default_parser::parse(args);
	}

	inline auto parse(const sys::command_line& cmd) {
		return parse(cmd.user_args());
	}

	inline void print_help(std::string_view program_name) {
		default_parser::print_help(program_name);
	}
}

#endif
