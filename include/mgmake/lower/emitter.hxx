#pragma once

#ifndef MGMK_LOWER_EMITTER_HXX
#define MGMK_LOWER_EMITTER_HXX

#include "../dag/action.hxx"
#include "../dag/artifact.hxx"
#include "../dag/graph.hxx"
#include "../dag/target.hxx"
#include "../sys/command_line.hxx"

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace mgmake::lower {
	struct emitter {
		dag::graph& m_graph;

		emitter(dag::graph& graph)
			: m_graph{graph} {}

		dag::artifact::id source(const std::filesystem::path& path) {
			return m_graph.create_artifact(dag::artifact::kind::source, path);
		}

		dag::artifact::id generated(const std::filesystem::path& path) {
			return m_graph.create_artifact(dag::artifact::kind::generated, path);
		}

		const std::filesystem::path& path(dag::artifact::id id) const {
			return m_graph.artifact(id).m_path;
		}

		dag::action::id action(
			std::string name,
			std::string description,
			std::vector<dag::artifact::id> inputs,
			std::vector<dag::artifact::id> outputs,
			sys::command_line command,
			std::filesystem::path working_directory
		) {
			return m_graph.create_action(
				std::move(name),
				std::move(description),
				std::move(inputs),
				std::move(outputs),
				false,
				std::move(command),
				std::move(working_directory)
			);
		}

		dag::action::id action(
			std::string name,
			std::string description,
			std::vector<dag::artifact::id> inputs,
			std::vector<dag::artifact::id> outputs,
			sys::command_line command
		) {
			return action(
				std::move(name),
				std::move(description),
				std::move(inputs),
				std::move(outputs),
				std::move(command),
				{}
			);
		}

		dag::target::id target(dag::target target) {
			return m_graph.create_target(std::move(target));
		}
	};
}

#endif
