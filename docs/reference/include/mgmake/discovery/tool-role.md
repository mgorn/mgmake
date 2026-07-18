# `include/mgmake/discovery/tool_role.hxx`

**Source:** `include/mgmake/discovery/tool_role.hxx`  
**Status:** Experimental catalog of tool roles; not exported by the public umbrella.

This file replaces a flat role enum with compile-time role types carrying human-readable names, environment-variable overrides, and eventually CLI option metadata.

## Role schema

A finalized `tool_role_impl` consumes:

| Field | Meaning |
|---|---|
| `logical_value` | Short logical identifier intended for the override option. |
| `name_value` | Human-readable role name. |
| `env_value` | Environment variable used to override discovery. |

It also intends to define an `option_type` whose description is `Override the <name> tool`.

## Catalog

The file declares role aliases for compilers, linkers, binary utilities, build systems, package tools, download/archive tools, and execution wrappers. Examples include:

```cpp
cc_role
cxx_role
archiver_role
linker_role
ninja_role
cmake_role
git_role
emulator_role
```

`default_tool_roles` collects all aliases into a `meta::type_list`.

## Environment overrides

Roles with an explicit override use names such as:

- `MGMK_CC`
- `MGMK_CXX`
- `MGMK_AR`
- `MGMK_LINKER`
- `MGMK_NINJA`
- `MGMK_CMAKE`

Some roles currently have no environment variable.

## Current limitations

This header is under active construction:

- The aliases set `name` and often `env`, but do not set `logical`, so the generated logical option name would be empty.
- `tool_role_impl` refers to `option` without the `cli::` qualifier.
- `std::format` is used without a direct `<format>` include.
- The subsystem is not exposed by the umbrella header.

The catalog is valuable as the intended semantic model, but consumers should not rely on it compiling independently in its present form.
