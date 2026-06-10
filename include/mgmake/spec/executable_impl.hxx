#pragma once

#ifndef MGMK_SPEC_EXECUTABLE_IMPL_HXX
#define MGMK_SPEC_EXECUTABLE_IMPL_HXX

#include "executable.hxx"
#include "project.hxx"

#include <set>
#include <string_view>
#include <vector>

namespace mgmake::spec {
	inline dag::target spec::executable::graph(
		dag::graph& result,
		const build::request& req,
		const spec::project& proj,
		const std::vector<dag::artifact::id>& link_inputs,
		std::set<dag::target::id> target_dependencies
	) const {
		const auto& tc = req.toolchain();

		mgmkassert(not m_name.empty(), "mgmake spec: executable target has no name");
		mgmkassert(not m_sources.empty(), "mgmake spec: executable target '" + m_name + "' has no sources");

		std::vector<dag::artifact::id> inputs{};
		inputs.reserve(m_sources.size() + link_inputs.size());

		for (const auto& source : m_sources) {
			inputs.emplace_back(result.create_artifact(
				dag::artifact::kind::source,
				source
			));
		}

		inputs.insert(inputs.end(), link_inputs.begin(), link_inputs.end());

		std::filesystem::path output = req.build_dir() / m_name;

#if defined(MGMK_PLATFORM_WINDOWS)
		output += ".exe";
#endif

		auto output_id = result.create_artifact(
			dag::artifact::kind::generated,
			output
		);

		std::set<std::filesystem::path> include_dirs = proj.collect_includes(*this);

		sys::command_line command{};
		command.m_args.emplace_back(tc.cxx());

		for (const auto& include_dir : include_dirs) {
			switch (tc.dialect()) {
				case build::toolchain::dialect::gcc:
					command.m_args.emplace_back(std::string{"-I"} + include_dir.string());
					break;

				case build::toolchain::dialect::msvc:
					command.m_args.emplace_back(std::string{"/I"} + include_dir.string());
					break;
			}
		}

		for (const auto& source : m_sources) {
			command.m_args.emplace_back(source.string());
		}

		for (auto link_input : link_inputs) {
			command.m_args.emplace_back(result.artifact(link_input).m_path.string());
		}

		for (const auto& flag : tc.link_flags()) {
			command.m_args.emplace_back(flag);
		}

		command.m_args.emplace_back("-o");
		command.m_args.emplace_back(output.string());

		result.create_action(
			std::string{"Build executable "} + m_name,
			std::string{"Builds executable target '"} + m_name + "'.",
			std::move(inputs),
			std::vector<dag::artifact::id>{ output_id },
			false,
			std::move(command),
			std::filesystem::path{}
		);

		return dag::target{
			m_name,
			{ output_id },
			std::move(target_dependencies)
		};
	}
}

#endif
