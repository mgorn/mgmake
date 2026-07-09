#pragma once

#ifndef MGMAKE_ENTRY_ENTRY_HXX
#define MGMAKE_ENTRY_ENTRY_HXX

#include "../backend/execute.hxx"
#include "../dag/execute.hxx"
#include "../build/clean.hxx"
#include "../build/request_from_options.hxx"
#include "../build/run.hxx"
#include "../build/toolchain_registry.hxx"
#include "../dep/database.hxx"
#include "../cli/help.hxx"
#include "../cli/parse.hxx"
#include "../detail/graphviz.hxx"
#include "../detail/hashes.hxx"
#include "../detail/project_factory.hxx"
#include "../discovery/discovery.hxx"
#include "../spec/project.hxx"
#include "../sys/command_line.hxx"
#include "exit_code.hxx"

#include <print>
#include <type_traits>
#include <utility>

// The entry point owns program flow: parse CLI, build a request, resolve tools, build phase graphs, prepare metadata, lower the DAG, then dispatch the action.

namespace mgmake::entry {
	template<auto factory_fn = nullptr, auto toolchains_v = nullptr>
	inline int entry(const sys::command_line& cmd) {
		auto parsed = cli::parse(cmd);

		if (not parsed) {
			std::println()
		}
	}
}

#endif // MGMAKE_ENTRY_ENTRY_HXX
