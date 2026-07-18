# `build.cxx`

**Source:** `build.cxx`  
**Status:** Project build-script example; partially executable with the current API.

This file is mgmake's own build description. It demonstrates the intended fluent, type-level DSL for declaring targets and selecting a project in the top-level configuration.

## Declared targets

The script creates two target types:

- `testlib`, an interface library named `testlib` with `test` as an include directory.
- `builder`, an executable named `build` whose source is `build.cxx` and which links to `testlib`.

The targets are collected into a project named `mgmake`, and that project is installed into an mgmake configuration:

```cpp
using proj = project::name<"mgmake">
    ::target<builder>
    ::build;

using c = config::project<proj>;
MGMK_ENTRY(c);
```

`MGMK_ENTRY(c)` defines `main`, finalizes `c` through `config_builder::build`, parses the command line, and dispatches a task.

## DSL pattern

Every declaration follows the same builder pattern:

1. Start with a builder alias such as `library`, `executable`, `project`, or `config`.
2. Chain aliases that add fields to a compile-time `meta::type_map`.
3. End a target or project declaration with `::build`.
4. Pass the configuration builder itself to `MGMK_ENTRY`; the entry point finalizes it.

The large commented block shows an older runtime-style API idea. It is not part of the active implementation. The second commented block demonstrates the intended extension point for custom CLI options.

## Current behavior

The specification layer is still declarative. The built-in `build` task currently prints diagnostic text rather than lowering the project to compiler commands, so this script documents the desired shape of a self-hosting mgmake build more than a complete build pipeline.

The source includes the generated root `mgmake.hxx`, not the modular `<mgmake/mgmake.hxx>` umbrella.
