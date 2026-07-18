# `include/mgmake/spec/target.hxx`

**Source:** `include/mgmake/spec/target.hxx`  
**Status:** Public target-specification builder; contains a current finalization bug.

This header defines target kinds, library kinds, fluent source/include/link accumulation, and convenient aliases for common target categories.

## Kinds

```cpp
enum struct target_type {
    none,
    executable,
    library
};

enum struct library_type {
    none,
    static_lib,
    shared_lib,
    interface
};
```

`type<value>` accepts either enum. Selecting a library kind automatically promotes an untyped target to `target_type::library`.

## Builder fields

- `name<"...">`
- `sources<"...", "...">` and `source<"...">`
- `include_dirs<"...", "...">` and `include_dir<"...">`
- `links<T...>` and `link<T>`
- `type<target_type::...>` or `type<library_type::...>`

Sources and include directories are stored as unique `meta::type_value<static_string>` types. Links are stored as unique types and are intended to represent target dependencies or system-library descriptors.

## Convenience aliases

```cpp
target
library
static_lib
shared_lib
interface_lib
executable
```

Example:

```cpp
using core = interface_lib
    ::name<"core">
    ::include_dir<"include">
    ::build;

using app = executable
    ::name<"app">
    ::source<"main.cxx">
    ::link<core>
    ::build;
```

## Validation status

Assertions that would reject changing an executable into a library, or assigning a library kind to a non-library target, are currently commented out. Repeated calls can therefore overwrite kind fields without diagnostics.

## Current finalization bug

`target_builder::build` currently instantiates `project_impl`:

```cpp
using build =
    typename builder_type::template build<project_impl>;
```

It should conceptually finalize as `target_impl`. As written, built targets are project-implementation specializations, and `target_impl` is unused. This should be corrected before downstream code relies on finalized target identity.

Like `project_impl`, `target_impl` is currently empty and would still need consumer fields after that correction.
