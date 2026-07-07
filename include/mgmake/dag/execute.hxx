#pragma once

#ifndef MGMK_DAG_EXECUTE_HXX
#define MGMK_DAG_EXECUTE_HXX

#include "graph.hxx"
#include "../build/request.hxx"
#include "../cli/options.hxx"
#include "../detail/hashes.hxx"
#include "../discovery/tool_environment.hxx"
#include "../sys/command_line.hxx"

#include <cstdlib>
#include <expected>
#include <filesystem>
#include <print>
#include <string>
#include <string_view>

// Generic DAG executor for pre-backend phase graphs such as acquisition and configuration.

namespace mgmake::dag {
	[[nodiscard]] inline bool action_is_up_to_date(
		const dag::graph& graph,
		const dag::action& action,
		detail::hashes& hashes
	) {
		if (action.m_always_run || action.m_outputs.empty()) {
			return false;
		}

		bool dirty = false;

		for (const auto input : action.m_inputs) {
			if (graph.artifact(input).check(hashes)) {
				dirty = true;
			}
		}

		for (const auto output : action.m_outputs) {
			if (graph.artifact(output).check(hashes)) {
				dirty = true;
			}
		}

		return !dirty;
	}

	inline void update_action_hashes(
		const dag::graph& graph,
		const dag::action& action,
		detail::hashes& hashes
	) {
		for (const auto input : action.m_inputs) {
			graph.artifact(input).update(hashes);
		}

		for (const auto output : action.m_outputs) {
			graph.artifact(output).update(hashes);
		}
	}

	inline void create_output_directories(
		const dag::graph& graph,
		const dag::action& action
	) {
		for (const auto output_id : action.m_outputs) {
			const auto& output = graph.artifact(output_id);

			if (output.m_kind == dag::artifact::kind::phony) {
				continue;
			}

			const auto parent = output.m_path.parent_path();

			if (!parent.empty()) {
				std::filesystem::create_directories(parent);
			}
		}
	}

	[[nodiscard]] inline int invoke_env_command(
		const build::request& req,
		const sys::command_line& command,
		sys::command_run_options opts = {}
	) {
		if (req.tool_environment().empty()) {
			return command.invoke(opts);
		}

		auto command_text = discovery::wrap_command_for_environment(
			req.tool_environment(),
			command.full_command()
		);

		if (opts.m_verbose || opts.m_dry_run) {
			std::println("{}", command_text);
		}

		if (opts.m_dry_run) {
			return 0;
		}

		return std::system(command_text.c_str());
	}

	[[nodiscard]] inline std::expected<void, std::string> execute(
		std::string_view phase_name,
		const cli::options& opts,
		const build::request& req,
		const dag::graph& graph,
		detail::hashes& hashes
	) {
		// Pre-backend phase actions run directly in DAG insertion order because each phase
		// produces filesystem state required before the next phase can be planned/finalized.
		for (std::size_t i = 0; i < graph.m_actions.size(); ++i) {
			const auto& action = graph.action(i);

			if (action_is_up_to_date(graph, action, hashes)) {
				continue;
			}

			if (!opts.m_dry_run) {
				create_output_directories(graph, action);
			}

			const auto old_cwd = std::filesystem::current_path();

			if (!action.m_working_directory.empty() && !opts.m_dry_run) {
				std::filesystem::current_path(action.m_working_directory);
			}

			const int exit_code = invoke_env_command(
				req,
				action.m_command,
				{
					.m_verbose = opts.m_verbose || opts.m_dry_run,
					.m_dry_run = opts.m_dry_run
				}
			);

			if (!action.m_working_directory.empty() && !opts.m_dry_run) {
				std::filesystem::current_path(old_cwd);
			}

			if (exit_code != 0) {
				return std::unexpected{
					"mgmake " + std::string{phase_name} + ": action '" + action.m_name +
					"' failed with exit code " + std::to_string(exit_code)
				};
			}

			if (!opts.m_dry_run) {
				update_action_hashes(graph, action, hashes);
			}
		}

		return {};
	}
}

#endif // MGMK_DAG_EXECUTE_HXX
