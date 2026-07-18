# `include/mgmake/task/help.hxx`

**Source:** `include/mgmake/task/help.hxx`  
**Status:** Built-in task that renders generated CLI help.

The help task introspects the configured task and option type lists and prints names, descriptions, short aliases, and value hints.

## Option metadata

The option accepts all of:

```text
help
--help
-h
```

It is declared as a task and remains a flag because `flag` is not disabled. Its callback writes task index `0`.

## Output

The handler prints:

1. `Usage: <program> [task] [options]`
2. A `Tasks:` section generated from `config_t::tasks_type`
3. An `Options:` section generated from `config_t::option_storage::list_type`

Task rows use `task_traits` to obtain the task option's name and description.

Option rows omit non-flag entries, so positional-only task options and storage-only pseudo-options are not repeated. A short name is printed when nonzero. Value-taking options append `=<help_hint>`, where the hint comes from `cli::value_parser<storage_value_type>`.

## Compile-time iteration

Both sections expand an `std::index_sequence` and invoke templated lambdas for each list element. Formatting itself occurs at runtime with `std::print` and `std::println`.

## Current index mismatch

The default task list places `build` at index `0` and `help` at index `1`, but this option writes `0`. In the current parser, that causes an explicit `help` request to dispatch the build task. The metadata and rendering logic remain useful once task indices are made consistent.
