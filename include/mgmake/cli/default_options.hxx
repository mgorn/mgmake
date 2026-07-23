#pragma once

#ifndef MGMAKE_CLI_DEFAULT_OPTIONS_HXX
#define MGMAKE_CLI_DEFAULT_OPTIONS_HXX

#include "option.hxx"

#include "../meta/value_list.hxx"

#include <print>
#include <string>
#include <vector>

namespace mgmake::cli {
	static constexpr auto task_option = option
		.name<"task">()
		.description<"Decides which task should run.">()
		.storage<"task", std::size_t>();

	static constexpr auto verbose_option = option
		.name<"verbose">().short_name<'v'>()
		.description<"Print commands before executing them.">()
		.set<"verbose", true>();
	
	static constexpr auto dry_run_option = option
		.name<"dry-run">().short_name<'d'>()
		.description<"Print commands without executing them.">()
		.set<"dry_run", true>();

	static constexpr auto build_dir_option = option
		.name<"build-dir">().short_name<'b'>()
		.description<"Set the build directory.">()
		.parse<"build_dir", std::filesystem::path>();

	static constexpr auto targets_option = option
		.name<"targets">().alias<"target">().short_name<'t'>()
		.description<"Build a specific target. May be passed multiple times.">()
		.parse<"targets", std::vector<std::string>>();
	
    // Type list of default options
	//
    // this way you can add your own options to 
    // default_options before passing the list 
    // to your mgmake config for your own CLI
    static constexpr auto default_options = meta::value_list<
		task_option,
		verbose_option,
		dry_run_option,
		build_dir_option,
		targets_option
	>{};
}

#endif // MGMAKE_CLI_DEFAULT_OPTIONS_HXX