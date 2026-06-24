#pragma once

#ifndef MGMAKE_BUILD_RUN_HXX
#define MGMAKE_BUILD_RUN_HXX

#include "request.hxx"
#include "../cli/options.hxx"
#include "../dag/graph.hxx"
#include "../spec/project.hxx"
#include "../sys/command_line.hxx"

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>

namespace mgmake::build {
	[[nodiscard]] inline std::expected<std::string, std::string> resolve_run_target_name(
		const cli::options& opts,
		const spec::project& project
	) {
		if (opts.m_targets.empty()) {
			return std::unexpected{
				"mgmake: run action requires exactly one executable target"
			};
		}

		if (opts.m_targets.size() != 1) {
			return std::unexpected{
				"mgmake: run action requires exactly one executable target, got " +
				std::to_string(opts.m_targets.size())
			};
		}

		const auto& name = opts.m_targets.front();

		if (!project.find_executable(name).has_value()) {
			if (project.find_library(name).has_value()) {
				return std::unexpected{
					"mgmake: target '" + name + "' is a library and cannot be run"
				};
			}

			return std::unexpected{
				"mgmake: unknown executable target '" + name + "'"
			};
		}

		return name;
	}

	[[nodiscard]] inline std::expected<std::filesystem::path, std::string> run_target_executable_path(
		const dag::graph& graph,
		std::string_view target_name
	) {
		const auto target_id = graph.find_target(target_name);

		if (!target_id.has_value()) {
			return std::unexpected{
				"mgmake: DAG does not contain target '" + std::string{target_name} + "'"
			};
		}

		const auto& target = graph.target(target_id.value());
		const std::string name{target_name};

		if (target.m_outputs.empty()) {
			return std::unexpected{
				"mgmake: target '" + name + "' has no output artifact to run"
			};
		}

		if (target.m_outputs.size() != 1) {
			return std::unexpected{
				"mgmake: target '" + name + "' has multiple output artifacts; run requires one executable output"
			};
		}

		const auto& artifact = graph.artifact(*target.m_outputs.begin());

		if (artifact.m_kind != dag::artifact::kind::generated) {
			return std::unexpected{
				"mgmake: target '" + name + "' output is not a generated executable artifact"
			};
		}

		return artifact.m_path;
	}

	[[nodiscard]] inline std::expected<int, std::string> invoke_run_target(
		const cli::options& opts,
		const std::filesystem::path& executable_path
	) {
		sys::command_line command{};

		if (!opts.m_exe_wrapper.empty()) {
			command.m_args.emplace_back(opts.m_exe_wrapper);
		} else if (!opts.m_emulator.empty()) {
			command.m_args.emplace_back(opts.m_emulator);
		}

		command.m_args.emplace_back(executable_path.string());

		for (const auto& arg : opts.m_run_args) {
			command.m_args.emplace_back(arg);
		}

		return command.invoke({
			.m_verbose = opts.m_verbose,
			.m_dry_run = opts.m_dry_run
		});
	}
}

#endif // MGMAKE_BUILD_RUN_HXX
