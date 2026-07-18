# `include/mgmake/spec/project.hxx`

**Source:** `include/mgmake/spec/project.hxx`  
**Status:** Public project-specification builder; storage consumer is still incomplete.

This header defines the fluent project declaration used to give a name to a collection of unique target types.

## Builder

```cpp
using project = project_builder<>;
```

Fields:

- `name<"...">`
- `set_targets<type_list>` as the low-level replacement operation
- `targets<T...>` to append multiple unique target types
- `target<T>` to append one target
- `build` to finalize

`targets` resolves a missing target list to `meta::type_list<>`, then uses `append_types_unique`, so adding the same target type again is ignored.

## Example

```cpp
using app = mgmake::spec::executable
    ::name<"app">
    ::source<"main.cxx">
    ::build;

using project = mgmake::spec::project
    ::name<"example">
    ::target<app>
    ::build;
```

## Finalized type

`build` creates `project_impl<storage_t>`. `project_impl` is currently empty: it retains the builder storage only as a template argument and does not yet expose `name_value`, `targets_type`, or validation. Consumers that need those fields will require consumer declarations to be added.

## Relationship to config

The finalized project type is normally installed with:

```cpp
using configured = mgmake::config::project<project>;
```

The current built-in build task does not yet lower the project specification.
