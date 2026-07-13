#pragma once

#ifndef MGMAKE_CLI_DEFAULT_OPTIONS_HXX
#define MGMAKE_CLI_DEFAULT_OPTIONS_HXX

#include "option.hxx"
#include "options.hxx"

#include "../meta/type_list.hxx"

#include <print>

namespace mgmake::cli {
	// Actions
    using help_option = option
        ::name<"help">::short_name<'h'>
        ::description<"Show help.">
		::action<true>
		::set<meta::member_access<&options::m_action>, action::kind::help>
		::build;
	
	using build_option = option
		::name<"build">
		::description<"Build the project.">
		::action<true>::flag<false>
		::set<meta::member_access<&options::m_action>, action::kind::build>
		::build;
	
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
		::assign_hint<"path">
		::build;

    // Type list of default options
    // this way you can add your own option to default_options
    // before passing the list to the entry for your own CLI
    // options
    using default_options = meta::type_list<
        help_option,
		build_option,
		verbose_option,
		dry_run_option,
		build_dir_option
    >;
}

#endif // MGMAKE_CLI_DEFAULT_OPTIONS_HXX