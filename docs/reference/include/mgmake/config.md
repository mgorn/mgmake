# `include/mgmake/config.hxx`

**Source:** `include/mgmake/config.hxx`  
**Status:** Public configuration builder and final configuration type.

`config.hxx` combines the project specification, toolchains, task list, and CLI options into the compile-time type consumed by the entry point and task dispatcher.

## `config_impl`

`config_impl<storage_t>` consumes four keys from its `meta::type_map`:

| Field | Default | Meaning |
|---|---|---|
| `project_type` / `project` | `void` | Finalized project type. |
| `toolchains_type` / `toolchains` | `void` | Configured toolchain collection. |
| `tasks_type` / `tasks` | `void` | Task `meta::type_list`. |
| `option_storage_type` / `option_storage` | `cli::option_storage<>` | Runtime storage generated from all options. |

The consumer-field macros expose both the raw `<field>_type` and the resolved `<field>` alias.

## `config_builder`

The builder accepts four fluent fields:

```cpp
config::project<project_type>
      ::toolchains<toolchain_type>
      ::tasks<task_list>
      ::options<option_list>
```

`config` itself starts with `task::default_tasks` and `cli::default_options`.

## Finalization

`config_builder::build` performs more work than a normal `meta::type_builder::build`:

1. Resolve missing task and option lists to their defaults.
2. Fold over all tasks and collect each task's `option_type`.
3. Prepend those task options to the ordinary options list.
4. Construct `cli::option_storage<full_options_list>`.
5. Store that type under the `option_storage` key.
6. Build `config_impl`.

This guarantees that positional task names are visible to the same parser that handles switches.

## Example

```cpp
using tasks = mgmake::task::default_tasks::append<my_task>;
using options = mgmake::cli::default_options::append<my_option>;

using configured = mgmake::config
    ::project<my_project>
    ::tasks<tasks>
    ::options<options>;

using final_config = configured::build;
```

Pass the builder (`configured`), not `final_config`, to `MGMK_ENTRY`; the entry point calls `::build`.

## Ordering constraint

The runtime task value is an index into `tasks_type`. Any option that writes a hard-coded task index must agree with the final task-list order. The current built-in task options do not agree with `default_tasks`; see the task reference pages for the present mismatch.
