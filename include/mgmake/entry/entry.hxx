#pragma once

#ifndef MGMAKE_ENTRY_ENTRY_HXX
#define MGMAKE_ENTRY_ENTRY_HXX

#include "../backend/execute.hxx"
#include "../prep/executor.hxx"
#include "../build/clean.hxx"
#include "../build/request_from_options.hxx"
#include "../build/run.hxx"
#include "../build/toolchain_registry.hxx"
#include "../cli/help.hxx"
#include "../cli/parse.hxx"
#include "../detail/graphviz.hxx"
#include "../detail/hashes.hxx"
#include "../detail/project_factory.hxx"
#include "../discovery/discovery.hxx"
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
			const auto clean_result = build::clean(req, {
				.m_verbose = opts.m_verbose,
				.m_dry_run = opts.m_dry_run
			});

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
			const auto clean_result = build::clean(req, {
				.m_verbose = opts.m_verbose,
				.m_dry_run = opts.m_dry_run
			});

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

		if (opts.m_action == cli::action_kind::tools) {
			const auto tools_result = discovery::print_tools(opts, req, proj);

			if (!tools_result) {
				std::println(stderr, "{}", tools_result.error());
				return detail::entry_exit_usage_error;
			}

			return detail::entry_exit_success;
		}

		auto resolved_req_result = discovery::resolve_request(opts, req, proj);

		if (!resolved_req_result) {
			std::println(stderr, "{}", resolved_req_result.error());
			return detail::entry_exit_usage_error;
		}

		auto resolved_req = std::move(*resolved_req_result);
		auto hashes = detail::hashes::load(resolved_req);
		auto prep_result = proj.prepare(resolved_req);

		if (opts.m_action == cli::action_kind::graph) {
			const std::string graph_kind = opts.m_targets.empty()
				? std::string{"build"}
				: opts.m_targets.front();

			const auto graph_dir = resolved_req.build_dir() / "graph";

			if (graph_kind == "discovery" || graph_kind == "prep") {
				detail::write_graphviz_dot_file(
					prep_result.m_dag,
					graph_dir / "discovery.dot"
				);
				return detail::entry_exit_success;
			}
		}

		auto prep_execute_result = prep::execute(
			opts,
			prep_result,
			hashes
		);

		if (!prep_execute_result) {
			std::println(stderr, "{}", prep_execute_result.error());
			return detail::entry_exit_action_failure;
		}

		if (opts.m_action == cli::action_kind::graph) {
			const std::string graph_kind = opts.m_targets.empty()
				? std::string{"build"}
				: opts.m_targets.front();

			const auto graph_dir = resolved_req.build_dir() / "graph";

			if (graph_kind == "all") {
				detail::write_graphviz_dot_file(
					prep_result.m_dag,
					graph_dir / "discovery.dot"
				);
			}

			if (graph_kind == "build" || graph_kind == "all") {
				auto build_graph = proj.build(resolved_req, prep_result);
				detail::write_graphviz_dot_file(
					build_graph,
					graph_dir / "build.dot"
				);

				return detail::entry_exit_success;
			}

			std::println(
				stderr,
				"mgmake: unknown graph kind '{}'; expected discovery, build, or all",
				graph_kind
			);
			return detail::entry_exit_usage_error;
		}

		auto graph = proj.build(resolved_req, prep_result);

		if (opts.m_action == cli::action_kind::run) {
			const auto run_target = build::resolve_run_target_name(opts, proj);

			if (!run_target) {
				std::println(stderr, "{}", run_target.error());
				return detail::entry_exit_usage_error;
			}

			auto run_req = resolved_req;
			run_req.m_targets.clear();
			run_req.m_targets.emplace_back(*run_target);

			const auto build_result = backend::build_selected_backend(
				opts,
				graph,
				run_req,
				hashes
			);

			if (!build_result) {
				std::println(stderr, "{}", build_result.error());
				return detail::entry_exit_action_failure;
			}

			if (!opts.m_dry_run) {
				hashes.store(resolved_req);
			}

			const auto executable_path = build::run_target_executable_path(
				graph,
				*run_target
			);

			if (!executable_path) {
				std::println(stderr, "{}", executable_path.error());
				return detail::entry_exit_action_failure;
			}

			const auto run_result = build::invoke_run_target(
				opts,
				*executable_path
			);

			if (!run_result) {
				std::println(stderr, "{}", run_result.error());
				return detail::entry_exit_action_failure;
			}

			return *run_result;
		}

		const auto action_result = backend::execute_project_action(
			opts,
			resolved_req,
			graph,
			hashes
		);

		if (!action_result) {
			std::println(stderr, "{}", action_result.error());
			return detail::entry_exit_action_failure;
		}

		if (!opts.m_dry_run) {
			hashes.store(resolved_req);
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

#endif // MGMAKE_ENTRY_ENTRY_HXX
