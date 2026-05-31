#pragma once

#ifndef MGMK_SPEC_TARGET_HXX
#define MGMK_SPEC_TARGET_HXX

#include "../dag/artifact.hxx"
#include "../dag/target.hxx"

#include <string_view>

namespace mgmake::spec {
	struct target {
		dag::graph& m_graph;
		dag::target::id m_graph_target;
		std::string m_name = "out";
		std::optional<dag::artifact::id> m_out_artifact;

		inline constexpr auto name(std::string_view file) {
			m_out_artifact = m_graph.create_artifact(dag::artifact::kind::generated, file);
			m_name = file;
		}
		inline constexpr auto add_source(std::string_view file) {
			if (not m_out_artifact.has_value()) {
				name("out");
			}
			auto artifact = m_graph.create_artifact(dag::artifact::kind::source, file);
			m_graph.create_action("Compile", std::format("Compiles the source file '{}'", file),
				std::vector<dag::artifact::id>{ artifact }, std::vector<dag::artifact::id>{ m_out_artifact.value() },
				true, sys::command_line{std::vector<std::string>{ std::string{"clang-mg++"}, std::string{file}, std::string{"-o"}, std::string{m_name} }});
		}
	};
}

#endif