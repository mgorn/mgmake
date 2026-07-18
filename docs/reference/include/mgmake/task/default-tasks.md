# `include/mgmake/task/default_tasks.hxx`

**Source:** `include/mgmake/task/default_tasks.hxx`  
**Status:** Public default task list and design notes.

This header defines the task set used by the default configuration and explains why tasks are represented separately from ordinary option callbacks.

## Task model

An option callback runs during parsing and initializes option storage. A task handler runs after parsing and receives the complete command and option state.

Each task type therefore provides:

1. `option_type`, used by the CLI parser to recognize the task.
2. `handle<config_t>(cmd, opts)`, used by the dispatcher.

`config_builder::build` automatically extracts every task's `option_type`, so users add task types to the configured task list and should not separately add their options.

## Default list

```cpp
using default_tasks = meta::type_list<
    task::build,
    task::help
>;
```

Index zero is intentionally the default because the `"task"` storage value is a default-constructed `std::size_t`. With this ordering, no explicit task selects `build`.

## Adding a task

```cpp
using tasks = task::default_tasks::append<my_task>;
using configured = config::tasks<tasks>;
```

The handler should return `std::expected<sys::exit_code, std::string>`.

## Current consistency issue

The built-in options hard-code `build` as index `1` and `help` as index `0`, the reverse of this list. Positional task execution is therefore swapped in the current source. The general `option_impl::handle_task` path can derive indices, but built-in task options use callbacks and bypass it.
