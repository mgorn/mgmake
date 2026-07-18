#pragma once

#ifndef MGMAKE_CLI_DEFAULT_TASKS_HXX
#define MGMAKE_CLI_DEFAULT_TASKS_HXX

#include "clean.hxx"
#include "build.hxx"
#include "fetch.hxx"
#include "help.hxx"

#include "../meta/type_list.hxx"
#include "../sys/exit_code.hxx"

#include <sstream>

/*
 * Why are tasks seperate from normal options? (Why can't they just be callback options?)
 *
 * Option callbacks are invoked during parsing and are functions to initialize the `cli::options` structure.
 *
 * The flow:
 * main -> parse -> cli::options -> tasks
 *			|-> match
 *			|-> invoke callbacks
 *
 * but options have a `task` setting? What's with that?
 * You still need to provide the tasks as options to the CLI parser.
 * They simply assign the task value in the `cli::options` structure.
 * Later on, that value is consumed to invoke the respective task handler.
 *
 * To make a task:
 *		1) Create a struct for your task
 *		2) Create an option alias with your desired settings
 *		3) Use the `::task<true>` option in your option alias
 *		3) Create a `handle` function:
 *```
 * template<typename config_t>
 * static inline constexpr std::expected<sys::exit_code, std::string> handle(const sys::shell& cmd, const sli::options& opts)
 *```
 * 
 * NOTE: You only provide the CLI option when making the task. Do not pass them as options in your config seperately.
 */

namespace mgmake::task {
	// Type list of default tasks
	//
    // this way you can add your own tasks to 
    // default_tasks before passing the list 
    // to your mgmake config for your own CLI
	using default_tasks = meta::type_list<
		task::build, // The default task when none is specified
		task::help,
		task::clean,
		task::fetch
	>;
}

#endif // MGMAKE_CLI_DEFAULT_TASKS_HXX