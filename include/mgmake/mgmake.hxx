#pragma once

#ifndef MGMAKE_MGMAKE_HXX
#define MGMAKE_MGMAKE_HXX

#include "backend/graphviz.hxx"
#include "backend/ninja.hxx"
#include "backend/traits.hxx"
#include "cli/action.hxx"
#include "cli/backend.hxx"
#include "cli/options.hxx"
#include "cli/parse.hxx"
#include "cli/util.hxx"
#include "dag/action.hxx"
#include "dag/artifact.hxx"
#include "dag/graph.hxx"
#include "dag/target.hxx"
#include "sys/command_line.hxx"

namespace mgmake {
	template<typename ProjectType>
	int entry(const sys::command_line& command_line) {
		auto parsed = cli::parse(command_line.user_args());

		if (!parsed) {
			std::println(stderr, "mgmake: error: {}", parsed.m_error);
			std::println(stderr, "try '{} help'", command_line.program_name());
			return 2;
		}

		const cli::options& opts = parsed.m_value;

		if (opts.show_help) {
			cli::print_help(command_line.program_name());
			return 0;
		}

		std::println("action: {}", cli::action_name(opts.m_action));
		std::println("backend: {}", cli::backend_name(opts.m_backend));
		std::println("build dir: {}", opts.m_build_dir);

		for (const auto& target : opts.m_targets) {
			std::println("target: {}", target);
		}

		return 0;
	}
}
namespace mgmk = mgmake;

#ifdef MGMK_PLATFORM_WINDOWS
#define MGMAKE_BUILD_ENTRY(ProjectType) \
int wmain(int argc, wchar_t** argv) { \
    auto args = ::mgmk::sys::args_from_wide(argc, argv); \
    return ::mgmk::entry<ProjectType>(args); \
}
#else
#define MGMAKE_BUILD_ENTRY(ProjectType) \
int main(int argc, char** argv) { \
    auto args = ::mgmk::sys::args_from_utf8(argc, argv); \
    return ::mgmk::entry<ProjectType>(args); \
}
#endif

// Short-hand
#define MGMK_BUILD_ENTRY MGMAKE_BUILD_ENTRY
#define MGMK_ENTRY MGMK_BUILD_ENTRY

#endif