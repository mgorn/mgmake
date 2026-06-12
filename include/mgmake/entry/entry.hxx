#pragma once

#ifndef MGMAKE_ENTRY_ENTRY_HXX
#define MGMAKE_ENTRY_ENTRY_HXX

#include "../backend/execute.hxx"
#include "../build/clean.hxx"
#include "../build/request_from_options.hxx"
#include "../build/toolchain_registry.hxx"
#include "../cli/help.hxx"
#include "../cli/parse.hxx"
#include "../detail/project_factory.hxx"
#include "../spec/project.hxx"
#include "../sys/command_line.hxx"
#include "exit_code.hxx"

#include <print>
#include <type_traits>
#include <utility>

namespace mgmake {
	template <build::toolchain_registry_like Toolchains>
	[[nodiscard]] inline int entry(
		const sys::command_line& command_line,
		const Toolchains& toolchains
	) {
		auto parsed = cli::parse(command_line.user_args());

		if (!parsed) {
			std::println(stderr, "mgmake: error: {}", parsed.m_error);
			std::println(stderr, "try '{} help'", command_line.program_name());
			return detail::entry_exit_usage_error;
		}

		const cli::options& opts = parsed.m_value;

		if (opts.m_show_help || opts.m_action == cli::action_kind::help) {
			cli::print_help(command_line.program_name(), toolchains);
			return detail::entry_exit_success;
		}

		if (opts.m_show_version || opts.m_action == cli::action_kind::version) {
			std::println("mgmake");
			return detail::entry_exit_success;
		}

		auto req_result = build::request_from_options(opts, toolchains);

		if (!req_result) {
			std::println(stderr, "{}", req_result.error());
			return detail::entry_exit_usage_error;
		}

		auto req = std::move(*req_result);

		if (opts.m_action == cli::action_kind::clean) {
			const auto clean_result = build::clean(req);

			if (!clean_result) {
				std::println(stderr, "{}", clean_result.error());
				return detail::entry_exit_action_failure;
			}

			return detail::entry_exit_success;
		}

		std::println(
			stderr,
			"mgmake: error: action '{}' requires a project, but no project was provided",
			cli::action_name(opts.m_action)
		);

		return detail::entry_exit_usage_error;
	}

	[[nodiscard]] inline int entry(const sys::command_line& command_line) {
		return entry(command_line, build::default_toolchains);
	}

	template <
		typename ProjectFactory,
		build::toolchain_registry_like Toolchains
	>
	[[nodiscard]] inline int entry(
		const sys::command_line& command_line,
		ProjectFactory&& project_factory,
		const Toolchains& toolchains
	) {
		auto parsed = cli::parse(command_line.user_args());

		if (!parsed) {
			std::println(stderr, "mgmake: error: {}", parsed.m_error);
			std::println(stderr, "try '{} help'", command_line.program_name());
			return detail::entry_exit_usage_error;
		}

		const cli::options& opts = parsed.m_value;

		if (opts.m_show_help || opts.m_action == cli::action_kind::help) {
			cli::print_help(command_line.program_name(), toolchains);
			return detail::entry_exit_success;
		}

		if (opts.m_show_version || opts.m_action == cli::action_kind::version) {
			std::println("mgmake");
			return detail::entry_exit_success;
		}

		auto req_result = build::request_from_options(opts, toolchains);

		if (!req_result) {
			std::println(stderr, "{}", req_result.error());
			return detail::entry_exit_usage_error;
		}

		auto req = std::move(*req_result);

		if (opts.m_action == cli::action_kind::clean) {
			const auto clean_result = build::clean(req);

			if (!clean_result) {
				std::println(stderr, "{}", clean_result.error());
				return detail::entry_exit_action_failure;
			}

			return detail::entry_exit_success;
		}

		auto proj = detail::make_project(
			std::forward<ProjectFactory>(project_factory),
			opts
		);

		auto graph = proj.graph(req);

		const auto action_result = backend::execute_project_action(
			opts,
			req,
			graph
		);

		if (!action_result) {
			std::println(stderr, "{}", action_result.error());
			return detail::entry_exit_action_failure;
		}

		return detail::entry_exit_success;
	}

	template <typename ProjectFactory>
		requires(!build::toolchain_registry_like<std::remove_cvref_t<ProjectFactory>>)
	[[nodiscard]] inline int entry(
		const sys::command_line& command_line,
		ProjectFactory&& project_factory
	) {
		return entry(
			command_line,
			std::forward<ProjectFactory>(project_factory),
			build::default_toolchains
		);
	}
}

#endif
