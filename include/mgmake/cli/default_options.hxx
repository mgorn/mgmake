#pragma once

#ifndef MGMAKE_CLI_DEFAULT_OPTIONS_HXX
#define MGMAKE_CLI_DEFAULT_OPTIONS_HXX

#include "default_actions.hxx"
#include "option.hxx"
#include "options.hxx"

#include "../meta/type_list.hxx"

#include <print>

namespace mgmake::cli {
	// Actions in `default_actions.hxx`
	
	// Switches
	using verbose_option = option
		::name<"verbose">::short_name<'v'>
		::description<"Print commands before executing them.">
		::set<meta::member_access<&options::m_verbose>, true>
		::build;
	
	using dry_run_option = option
		::name<"dry-run">
		::description<"Print commands without executing them.">
		::set<meta::member_access<&options::m_dry_run>, true>
		::build;

	using build_dir_option = option
		::name<"build-dir">
		::description<"Set the build directory.">
		::assign<meta::member_access<&options::m_build_dir>>
		// ::assign_hint<"path"> - Derive based on type..?
		::build;

	// Get the list of action options
	using action_options = default_actions::template fold<[]<typename state_t, typename action_t>() consteval {
		return std::type_identity<typename state_t::template append<typename action_t::option_type>>{};
	}, meta::type_list<>>;

    // Type list of default options
    // this way you can add your own option to default_options
    // before passing the list to the entry for your own CLI
    // options
    using default_options = meta::type_list<
		verbose_option,
		dry_run_option,
		build_dir_option
    >::append_list<action_options>; // Append the action options
}

#endif // MGMAKE_CLI_DEFAULT_OPTIONS_HXX