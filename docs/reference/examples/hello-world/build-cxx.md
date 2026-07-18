# `examples/hello_world/build.cxx`

**Source:** `examples/hello_world/build.cxx`  
**Status:** Stale example using an earlier mgmake API.

This file shows the intended idea of declaring a hello-world target and project, but its identifiers no longer match the current lowercase builder API.

## Intended flow

The example attempts to:

1. Include `<mgmake/mgmake.hxx>`.
2. Declare a target named `hello_world`.
3. Add `main.cxx` as a source.
4. Declare a project named `hello_world`.
5. Select a C++ language standard.
6. Add the target.
7. generate the build program entry point.

## Obsolete identifiers

The current source uses:

```cpp
Target
Sources
Project
BUILD_ENTRY
```

The active API instead exposes lowercase aliases such as `spec::executable`, `spec::project`, source aliases directly on the target builder, and `MGMK_ENTRY`.

There is also no current project-level `standard` field.

A current-shape declaration would begin approximately as:

```cpp
using hello_target = mgmake::spec::executable
    ::name<"hello_world">
    ::source<"main.cxx">
    ::build;

using hello_project = mgmake::spec::project
    ::name<"hello_world">
    ::target<hello_target>
    ::build;

using configured = mgmake::config::project<hello_project>;
MGMK_ENTRY(configured);
```

Even after updating syntax, the built-in build task is still a stub and will not compile the target yet.
