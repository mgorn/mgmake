#pragma once

#ifndef MGMK_SPEC_PROJECT_HXX
#define MGMK_SPEC_PROJECT_HXX

#include "../backend/traits.hxx"
#include "../build/request.hxx"
#include "../dag/graph.hxx"
#include "executable.hxx"
#include "library.hxx"

#include <string>
#include <string_view>
#include <vector>

namespace mgmake::spec {
	struct project {
		std::string m_name;
		std::vector<spec::executable> m_executables;
		std::vector<spec::library> m_libraries;

		inline constexpr project& add_target(const spec::executable& exe) {
			m_executables.emplace_back(exe);
			return *this;
		}
		inline constexpr project& add_target(const spec::library& lib) {
			m_libraries.emplace_back(lib);
			return *this;
		}

		// Generate the graph from all project info
		inline dag::graph graph(const build::request& req) const {
			auto& tc = req.toolchain();

			dag::graph result{};

			for (const auto& exe : m_executables) {
				mgmkassert(not exe.m_name.empty(), "mgmake spec: executable target has no name");
				mgmkassert (not exe.m_sources.empty(), "mgmake spec: executable target '" + exe.m_name + "' has no sources");

				std::vector<dag::artifact::id> inputs{};
				inputs.reserve(exe.m_sources.size());

				for (const auto& source : exe.m_sources) {
					inputs.emplace_back(result.create_artifact(
						dag::artifact::kind::source,
						source
					));
				}

				std::filesystem::path output = req.build_dir() / exe.m_name;
		#if defined(MGMK_PLATFORM_WINDOWS)
				output += ".exe";
		#endif

				auto output_id = result.create_artifact(
					dag::artifact::kind::generated,
					output
				);
				
				sys::command_line command{};
				command.m_args.emplace_back(tc.cxx());

				for (const auto& source : exe.m_sources) {
					command.m_args.emplace_back(source.string());
				}

				command.m_args.emplace_back("-o");
				command.m_args.emplace_back(output.string());

				result.create_action(
					std::string{"Build executable "} + exe.m_name,
					std::string{"Builds executable target '"} + exe.m_name + "'.",
					std::move(inputs),
					std::vector<dag::artifact::id>{output_id},
					false,
					std::move(command),
					std::filesystem::path{}
				);

				result.create_target(
					exe.m_name,
					std::vector<dag::artifact::id>{output_id}
				);
			}

			mgmkassert(m_libraries.empty(), "mgmake spec: lowering libraries to dag::graph is not implemented yet");

			return result;
		}
	};
}

#endif