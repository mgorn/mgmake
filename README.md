# mgmake

**mgmake** is an experimental C++ build system that uses C++ itself as the project-description language.

Join the [GornCord](https://discord.gg/ECDfZGTzVQ)!

Instead of introducing another configuration language, mgmake lets a build script describe projects, targets, toolchains, command-line options, and tasks through a compile-time C++ API. The build script is compiled into a small project-specific command-line program.

> [!WARNING]
> mgmake is in early development. The specification API, command-line system, tool discovery, and build execution pipeline are still evolving, and the built-in `build` task is currently only a scaffold. Expect breaking API changes.

## Goals

- Use modern C++ as the build-description language.
- Keep project specifications strongly typed and largely compile-time.
- Support both modular headers and a generated single-header distribution.
- Make command-line options and tasks extensible from a project's build script.
- Separate project intent from toolchain discovery and backend execution.
- Produce useful compile-time diagnostics for invalid build specifications.

## Requirements

The current sources use recent standard-library facilities including `std::expected`, `std::format`, `std::print`, `std::ranges::to`, and `std::to_underlying`.

A recent C++ compiler and standard library are therefore required. Development currently uses Clang with the C++2c language mode:

```sh
clang++ -std=c++2c
```

Python 3 is only required when regenerating the amalgamated `mgmake.hxx` header.

## Using mgmake

mgmake can be included in either of two forms:

```cpp
// Modular headers, with include/ on the compiler's include path.
#include <mgmake/mgmake.hxx>
```

```cpp
// Generated single-header distribution copied beside the build script.
#include "mgmake.hxx"
```

The public convenience header also defines `mgmk` as an alias for the `mgmake` namespace.

### Minimal build program

A build program owns the executable entry point through `MGMK_ENTRY`:

```cpp
#include "mgmake.hxx"

MGMK_ENTRY();
```

Compile it like a normal C++ program:

```sh
clang++ -std=c++2c build.cxx -o build
```

The resulting executable parses mgmake's command-line options and dispatches a configured task.

### Project specification

The current specification API uses compile-time builders:

```cpp
#include "mgmake.hxx"

using namespace mgmk;
using namespace mgmk::spec;

using support = interface_lib
    ::name<"support">
    ::include_dirs<"include">
    ::build;

using app = executable
    ::name<"app">
    ::sources<"src/main.cxx">
    ::link<support>
    ::build;

using app_project = project
    ::name<"example">
    ::targets<support, app>
    ::build;

using app_config = config
    ::project<app_project>;

MGMK_ENTRY(app_config);
```

This example shows the intended shape of the public DSL. Project lowering, tool selection, command generation, and actual target compilation are still being implemented.

## Built-in command-line model

A generated build program is composed from compile-time option and task lists.

The current default configuration includes:

- `build` and `help` tasks.
- `-v` / `--verbose` to print commands before execution.
- `--dry-run` to print commands without executing them.
- `--build-dir <path>` or `--build-dir=<path>` to select the build directory.

Projects can define additional options and tasks and attach them to a custom `mgmake::config`.

## Repository layout

```text
.
├── build.cxx                  Development build specification
├── mgmake.hxx                 Generated single-header distribution
├── include/mgmake/
│   ├── cli/                   Compile-time CLI options and runtime parsing
│   ├── detail/                Internal assertions and small utilities
│   ├── discovery/             Tool roles and tool-discovery models
│   ├── meta/                  Compile-time strings, lists, maps, and builders
│   ├── spec/                  Project, target, and toolchain specifications
│   ├── sys/                   Process-facing and platform-facing utilities
│   ├── task/                  Task definitions and dispatch
│   ├── config.hxx             Top-level mgmake configuration builder
│   └── mgmake.hxx             Modular public convenience header
├── examples/                  Example build scripts and source files
├── docs/reference/            File-by-file implementation reference
└── tools/amalgamate.py        Single-header generation tool
```

## Regenerating the single header

The root `mgmake.hxx` file is generated from `include/mgmake/mgmake.hxx`. Do not edit it directly.

```sh
python3 tools/amalgamate.py
```

The amalgamation tool recursively expands project-local includes while preserving standard-library includes.

## Documentation

The implementation reference starts at [`docs/reference/index.md`](docs/reference/index.md). It contains a page for each significant source, example, and tooling file in the project.

Because the project is still changing rapidly, the reference documentation describes both the intended role of each component and any important incomplete or experimental behavior in the current snapshot.

## Contributing

Bug reports, design feedback, and implementation contributions are welcome through the [GitHub issue tracker](https://github.com/mgorn/mgmake/issues).

Discussion is also available in the [GornCord Discord server](https://discord.gg/ECDfZGTzVQ).