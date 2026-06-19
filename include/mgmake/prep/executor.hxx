#pragma once

#ifndef MGMK_PREP_EXECUTOR_HXX
#define MGMK_PREP_EXECUTOR_HXX

#include "result.hxx"
#include "../cli/options.hxx"
#include "../dag/graph.hxx"
#include "../sys/command_line.hxx"

#include <expected>
#include <filesystem>
#include <print>
#include <string>
#include <utility>


namespace mgmake::prep {
	[[nodiscard]] inline bool output_exists(
		const dag::graph& graph,
		dag::artifact::id id
	) {
		const auto& artifact = graph.artifact(id);

		if (artifact.m_kind == dag::artifact::kind::phony) {
			return true;
		}

		return std::filesystem::exists(artifact.m_path);
	}

	[[nodiscard]] inline bool action_is_up_to_date(
		const dag::graph& graph,
		const dag::action& action
	) {
		if (action.m_always_run || action.m_outputs.empty()) {
			return false;
		}

		for (const auto output : action.m_outputs) {
			if (!output_exists(graph, output)) {
				return false;
			}
		}

		return true;
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

	[[nodiscard]] inline std::expected<void, std::string> execute(
		const cli::options& opts,
		prep::result& result
	) {
		const auto& graph = result.m_dag;

		for (std::size_t i = 0; i < graph.m_actions.size(); ++i) {
			const auto& action = graph.action(i);

			if (action_is_up_to_date(graph, action)) {
				continue;
			}

			if (!opts.m_dry_run) {
				create_output_directories(graph, action);
			}

			const auto old_cwd = std::filesystem::current_path();

			if (!action.m_working_directory.empty() && !opts.m_dry_run) {
				std::filesystem::current_path(action.m_working_directory);
			}

			const int exit_code = action.m_command.invoke({
				.m_verbose = opts.m_verbose || opts.m_dry_run,
				.m_dry_run = opts.m_dry_run
			});

			if (!action.m_working_directory.empty() && !opts.m_dry_run) {
				std::filesystem::current_path(old_cwd);
			}

			if (exit_code != 0) {
				return std::unexpected{
					"mgmake prep: action '" + action.m_name +
					"' failed with exit code " + std::to_string(exit_code)
				};
			}
		}

		return {};
	}
}

#endif
