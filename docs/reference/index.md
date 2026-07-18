# mgmake reference

This directory documents the significant source files in the current mgmake tree. The pages describe the code as it exists in the uploaded snapshot, including incomplete APIs, stale examples, and known source-level inconsistencies.

## Recommended reading order

1. [`include/mgmake/mgmake.hxx`](include/mgmake/mgmake.md) — modular public umbrella.
2. [`include/mgmake/config.hxx`](include/mgmake/config.md) — configuration assembly and finalization.
3. [`include/mgmake/spec/target.hxx`](include/mgmake/spec/target.md) and [`project.hxx`](include/mgmake/spec/project.md) — build-description DSL.
4. [`include/mgmake/cli/entry.hxx`](include/mgmake/cli/entry.md) — runtime entry flow.
5. [`include/mgmake/cli/parser.hxx`](include/mgmake/cli/parser.md), [`option.hxx`](include/mgmake/cli/option.md), and [`option_storage.hxx`](include/mgmake/cli/option-storage.md) — CLI model.
6. [`include/mgmake/task/dispatcher.hxx`](include/mgmake/task/dispatcher.md) — task selection and invocation.
7. The [`meta`](#metaprogramming) pages — compile-time infrastructure supporting the DSL.

## Distribution and project entry files

- [`build.cxx`](build-cxx.md)
- [`mgmake.hxx`](single-header.md)
- [`include/mgmake/mgmake.hxx`](include/mgmake/mgmake.md)
- [`include/mgmake/config.hxx`](include/mgmake/config.md)
- [`tools/amalgamate.py`](tools/amalgamate.md)

## Command-line interface

- [`cli/default_options.hxx`](include/mgmake/cli/default-options.md)
- [`cli/entry.hxx`](include/mgmake/cli/entry.md)
- [`cli/option.hxx`](include/mgmake/cli/option.md)
- [`cli/option_storage.hxx`](include/mgmake/cli/option-storage.md)
- [`cli/parser.hxx`](include/mgmake/cli/parser.md)
- [`cli/value_parser.hxx`](include/mgmake/cli/value-parser.md)

## Tasks

- [`task/build.hxx`](include/mgmake/task/build.md)
- [`task/default_tasks.hxx`](include/mgmake/task/default-tasks.md)
- [`task/dispatcher.hxx`](include/mgmake/task/dispatcher.md)
- [`task/help.hxx`](include/mgmake/task/help.md)
- [`task/task_traits.hxx`](include/mgmake/task/task-traits.md)

## Project specification

- [`spec/project.hxx`](include/mgmake/spec/project.md)
- [`spec/target.hxx`](include/mgmake/spec/target.md)
- [`spec/toolchain.hxx`](include/mgmake/spec/toolchain.md)

## Tool discovery

These headers are experimental and are not included by the public umbrella.

- [`discovery/default_tools.hxx`](include/mgmake/discovery/default-tools.md)
- [`discovery/tool.hxx`](include/mgmake/discovery/tool.md)
- [`discovery/tool_role.hxx`](include/mgmake/discovery/tool-role.md)

## System utilities

- [`sys/exit_code.hxx`](include/mgmake/sys/exit-code.md)
- [`sys/shell.hxx`](include/mgmake/sys/shell.md)

## Metaprogramming

- [`meta/member_access.hxx`](include/mgmake/meta/member-access.md)
- [`meta/member_traits.hxx`](include/mgmake/meta/member-traits.md)
- [`meta/static_dict.hxx`](include/mgmake/meta/static-dict.md)
- [`meta/static_string.hxx`](include/mgmake/meta/static-string.md)
- [`meta/type_builder.hxx`](include/mgmake/meta/type-builder.md)
- [`meta/type_list.hxx`](include/mgmake/meta/type-list.md)
- [`meta/type_map.hxx`](include/mgmake/meta/type-map.md)
- [`meta/type_or.hxx`](include/mgmake/meta/type-or.md)
- [`meta/type_pair.hxx`](include/mgmake/meta/type-pair.md)
- [`meta/type_value.hxx`](include/mgmake/meta/type-value.md)

## Internal detail utilities

- [`detail/assert.hxx`](include/mgmake/detail/assert.md)
- [`detail/index_bit.hxx`](include/mgmake/detail/index-bit.md)

## Examples

### Hello world

- [`examples/hello_world/build.cxx`](examples/hello-world/build-cxx.md)
- [`examples/hello_world/build.sh`](examples/hello-world/build-sh.md)
- [`examples/hello_world/main.cxx`](examples/hello-world/main-cxx.md)

### Using CMake

- [`examples/using_cmake/build.cxx`](examples/using-cmake/build-cxx.md)
- [`examples/using_cmake/main.cxx`](examples/using-cmake/main-cxx.md)

## Snapshot-wide implementation notes

The most important current inconsistencies are documented on their owning pages:

- `target_builder::build` finalizes with `project_impl` instead of `target_impl`.
- Built-in task options write indices opposite to the order in `default_tasks`.
- The discovery subsystem contains unfinished macro/qualification issues and is not exported.
- `project_impl`, `target_impl`, `tool_impl`, and `toolchain_impl` do not yet expose their stored fields.
- Both example directories require source updates before they compile.

The project `README.md` and `.gitignore` are not given standalone reference pages because they are repository metadata rather than implementation files. The current README's historical file-tree section does not match this snapshot; this reference index follows the actual archive instead.
