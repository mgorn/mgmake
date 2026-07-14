#pragma once

#ifndef MGMAKE_CLI_DEFAULT_ACTIONS_HXX
#define MGMAKE_CLI_DEFAULT_ACTIONS_HXX

#include "action.hxx"

#include "../meta/type_list.hxx"
#include "../sys/exit_code.hxx"

namespace mgmake::cli {
	using help_action = action
		::option<option
			::name<"help">::short_name<'h'>
			::description<"Show help.">
			::action<true>
			::callback<[](auto& opts) {
				// This overrides the action with the help action
				opts.m_action = 0;
			}>
			::build>
		::handler<[](auto& opts) -> std::expected<sys::exit_code, std::string> {
			// TODO: Generate help from parser
			std::println("Help menu");
			return sys::exit_code::success;
		}>
		::build;
	
	using build_action = action
		::option<option
			::name<"build">
			::description<"Build the project.">
			::action<true>::flag<false>
			::build>
		::handler<[](auto& opts) -> std::expected<sys::exit_code, std::string> {
			// TODO: This would be the entrypoint/root for build
			std::println("Build action");
			return sys::exit_code::success;
		}>
		::build;
	
	using default_actions = meta::type_list<
		help_action,
		build_action
	>;
}

#endif // MGMAKE_CLI_DEFAULT_ACTIONS_HXX