#pragma once

#ifndef MGMAKE_MGMAKE_HXX
#define MGMAKE_MGMAKE_HXX

#include "backend/graphviz.hxx"
#include "backend/ninja.hxx"
#include "backend/traits.hxx"
#include "build/request.hxx"
#include "build/toolchain.hxx"
#include "cli/action.hxx"
#include "cli/backend.hxx"
#include "cli/options.hxx"
#include "cli/parse.hxx"
#include "cli/util.hxx"
#include "dag/action.hxx"
#include "dag/artifact.hxx"
#include "dag/graph.hxx"
#include "dag/target.hxx"
#include "detail/convert.hxx"
#include "detail/static_string.hxx"
#include "spec/executable.hxx"
#include "spec/executable_impl.hxx"
#include "spec/library.hxx"
#include "spec/library_impl.hxx"
#include "spec/project.hxx"
#include "sys/command_line.hxx"
#include "sys/platform.hxx"
#include "sys/util.hxx"

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

		if (opts.m_show_help) {
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

#if defined(MGMK_PLATFORM_WINDOWS) and defined(WIN32_LEAN_AND_MEAN)
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