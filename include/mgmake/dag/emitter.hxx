#pragma once

#ifndef MGMK_DAG_EMITTER_HXX
#define MGMK_DAG_EMITTER_HXX

#include "../dag/action.hxx"
#include "../dag/artifact.hxx"
#include "../dag/graph.hxx"
#include "../dag/target.hxx"
#include "../sys/command_line.hxx"

#include <filesystem>
#include <initializer_list>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace mgmake::dag {
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
			std::string_view name,
			std::string_view description,
			std::span<const dag::artifact::id> inputs,
			std::span<const dag::artifact::id> outputs,
			const sys::command_line& command,
			const std::filesystem::path& working_directory
		) {
			return m_graph.create_action(dag::action{
				std::string{name},
				std::string{description},
				std::vector<dag::artifact::id>{inputs.begin(), inputs.end()},
				std::vector<dag::artifact::id>{outputs.begin(), outputs.end()},
				false,
				command,
				working_directory
			});
		}

		dag::action::id action(
			std::string_view name,
			std::string_view description,
			std::span<const dag::artifact::id> inputs,
			std::span<const dag::artifact::id> outputs,
			const sys::command_line& command
		) {
			return action(name, description, inputs, outputs, command, {});
		}

		dag::action::id action(
			std::string_view name,
			std::string_view description,
			const std::vector<dag::artifact::id>& inputs,
			const std::vector<dag::artifact::id>& outputs,
			const sys::command_line& command,
			const std::filesystem::path& working_directory
		) {
			return action(
				name,
				description,
				std::span<const dag::artifact::id>{inputs.data(), inputs.size()},
				std::span<const dag::artifact::id>{outputs.data(), outputs.size()},
				command,
				working_directory
			);
		}

		dag::action::id action(
			std::string_view name,
			std::string_view description,
			const std::vector<dag::artifact::id>& inputs,
			const std::vector<dag::artifact::id>& outputs,
			const sys::command_line& command
		) {
			return action(name, description, inputs, outputs, command, {});
		}

		dag::action::id action(
			std::string_view name,
			std::string_view description,
			const std::vector<dag::artifact::id>& inputs,
			std::initializer_list<dag::artifact::id> outputs,
			const sys::command_line& command,
			const std::filesystem::path& working_directory
		) {
			return action(
				name,
				description,
				std::span<const dag::artifact::id>{inputs.data(), inputs.size()},
				std::span<const dag::artifact::id>{outputs.begin(), outputs.size()},
				command,
				working_directory
			);
		}

		dag::action::id action(
			std::string_view name,
			std::string_view description,
			const std::vector<dag::artifact::id>& inputs,
			std::initializer_list<dag::artifact::id> outputs,
			const sys::command_line& command
		) {
			return action(name, description, inputs, outputs, command, {});
		}

		dag::action::id action(
			std::string_view name,
			std::string_view description,
			std::initializer_list<dag::artifact::id> inputs,
			const std::vector<dag::artifact::id>& outputs,
			const sys::command_line& command,
			const std::filesystem::path& working_directory
		) {
			return action(
				name,
				description,
				std::span<const dag::artifact::id>{inputs.begin(), inputs.size()},
				std::span<const dag::artifact::id>{outputs.data(), outputs.size()},
				command,
				working_directory
			);
		}

		dag::action::id action(
			std::string_view name,
			std::string_view description,
			std::initializer_list<dag::artifact::id> inputs,
			const std::vector<dag::artifact::id>& outputs,
			const sys::command_line& command
		) {
			return action(name, description, inputs, outputs, command, {});
		}

		dag::action::id action(
			std::string_view name,
			std::string_view description,
			std::initializer_list<dag::artifact::id> inputs,
			std::initializer_list<dag::artifact::id> outputs,
			const sys::command_line& command,
			const std::filesystem::path& working_directory
		) {
			return action(
				name,
				description,
				std::span<const dag::artifact::id>{inputs.begin(), inputs.size()},
				std::span<const dag::artifact::id>{outputs.begin(), outputs.size()},
				command,
				working_directory
			);
		}

		dag::action::id action(
			std::string_view name,
			std::string_view description,
			std::initializer_list<dag::artifact::id> inputs,
			std::initializer_list<dag::artifact::id> outputs,
			const sys::command_line& command
		) {
			return action(name, description, inputs, outputs, command, {});
		}

		dag::target::id target(const dag::target& target) {
			return m_graph.create_target(target);
		}
	};
}

#endif
