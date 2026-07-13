#pragma once

#ifndef MGMAKE_CLI_DEFAULT_ACTIONS_HXX
#define MGMAKE_CLI_DEFAULT_ACTIONS_HXX

#include "action.hxx"

namespace mgmake::cli {
	using help_action = action
		::option<option
			::name<"help">::short_name<'h'>
			::description<"Show help.">
			::action<true>
			::build>
		::handler<[](auto& opts) {
			// TODO: Generate help from parser
			std::println("Help menu");
		}>
		::build;
	
	using build_action = action
		::option<option
			::name<"build">
			::description<"Build the project.">
			::action<true>::flag<false>
			::build>
		::handler<[](auto& opts) {
			// TODO: This would be the entrypoint/root for build
			std::println("Build action");
		}>
		::build;
	
	using default_actions = meta::type_list<
		help_action,
		build_action
	>;
}

#endif // MGMAKE_CLI_DEFAULT_ACTIONS_HXX