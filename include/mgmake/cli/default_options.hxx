#pragma once

#ifndef MGMAKE_CLI_DEFAULT_OPTIONS_HXX
#define MGMAKE_CLI_DEFAULT_OPTIONS_HXX

#include "option.hxx"

#include "../meta/type_list.hxx"

#include <print>

namespace mgmake::cli {
	using task_option = option
		::name<"task">
		::description<"Decides which task should run.">
		::storage<"task", std::size_t>
		::build;

	using verbose_option = option
		::name<"verbose">::short_name<'v'>
		::description<"Print commands before executing them.">
		::set<"verbose", true>
		::build;
	
	using dry_run_option = option
		::name<"dry-run">
		::description<"Print commands without executing them.">
		::set<"dry_run", true>
		::build;

	using build_dir_option = option
		::name<"build-dir">
		::description<"Set the build directory.">
		::parse<"build_dir", std::filesystem::path>
		::build;
	
    // Type list of default options
	//
    // this way you can add your own options to 
    // default_options before passing the list 
    // to your mgmake config for your own CLI
    using default_options = meta::type_list<
		task_option,
		verbose_option,
		dry_run_option,
		build_dir_option
    >;
}

#endif // MGMAKE_CLI_DEFAULT_OPTIONS_HXX