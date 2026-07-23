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
 * Why are tasks separate from normal callback options?
 *
 * Option callbacks run while the command line is being parsed. A task instead selects
 * which handler the dispatcher invokes after parsing succeeds.
 *
 * Each task exposes a `static constexpr auto option` marked with `.task<true>()`.
 * `config_impl::full_options()` automatically prepends those task options to the
 * configured CLI options, and the parser stores the selected task for the dispatcher.
 *
 * To make a task:
 *		1) Create a struct for the task.
 *		2) Define its `static constexpr auto option` with the desired settings.
 *		3) Mark that option with `.task<true>()`.
 *		4) Define a handler:
 *```
 * template<auto config_v>
 * static inline constexpr std::expected<sys::exit_code, std::string> handle(const sys::shell& cmd, const auto& opts)
 *```
 *		5) Add the task type to the task list supplied to the mgmake config.
 *
 * Do not also add the task's option to the config's normal option list; it is collected
 * automatically from the configured task types.
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