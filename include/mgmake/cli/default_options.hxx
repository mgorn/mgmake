#pragma once

#ifndef MGMAKE_CLI_DEFAULT_OPTIONS_HXX
#define MGMAKE_CLI_DEFAULT_OPTIONS_HXX

#include "option.hxx"

#include "../meta/value_list.hxx"

#include <print>
#include <string>
#include <vector>

namespace mgmake::cli {
	using default_storage = meta::static_dict<>
		::template emplace<"task", std::size_t>
		::template emplace<"verbose", bool>
		::template emplace<"short", bool>
		::template emplace<"dry_run", bool>
		::template emplace<"build_dir", std::filesystem::path>
		::template emplace<"targets", std::vector<std::string>>;

	static constexpr auto verbose_option = option
		.name<"verbose">().short_name<'v'>()
		.description<"Print commands before executing them.">()
		.set<"verbose", true>();
	
	static constexpr auto short_option = option
		.name<"short">().short_name<'s'>()
		.description<"Keep command output short.">()
		.set<"short", true>();
	
	static constexpr auto dry_run_option = option
		.name<"dry-run">().short_name<'d'>()
		.description<"Print commands without executing them.">()
		.set<"dry_run", true>();

	static constexpr auto build_dir_option = option
		.name<"build-dir">().short_name<'b'>()
		.description<"Set the build directory.">()
		.parse<"build_dir">();

	static constexpr auto targets_option = option
		.name<"targets">().alias<"target">().short_name<'t'>()
		.description<"Build a specific target. May be passed multiple times.">()
		.parse<"targets">();
	
    // Type list of default options
	//
    // this way you can add your own options to 
    // default_options before passing the list 
    // to your mgmake config for your own CLI
    using default_options = meta::value_list<
		verbose_option,
		short_option,
		dry_run_option,
		build_dir_option,
		targets_option
	>;
}

#endif // MGMAKE_CLI_DEFAULT_OPTIONS_HXX