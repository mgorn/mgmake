# `include/mgmake/mgmake.hxx`

**Source:** `include/mgmake/mgmake.hxx`  
**Status:** Public modular umbrella header.

This is the primary include for consumers using mgmake from an include directory rather than the generated single-header file.

## Exports

The header includes:

- `cli/entry.hxx` for `mgmake::cli::entry` and `MGMK_ENTRY`.
- `spec/project.hxx` for the project builder.
- `spec/target.hxx` for target builders and target-kind aliases.
- `config.hxx` for the top-level configuration builder.

It also defines the convenience namespace alias:

```cpp
namespace mgmk = mgmake;
```

A typical modular include is therefore:

```cpp
#include <mgmake/mgmake.hxx>

using namespace mgmk;
using namespace mgmk::spec;
```

## Deliberate omissions

The umbrella does not currently include the discovery headers or `spec/toolchain.hxx`. Those components are experimental and are not part of the default public surface or the generated root `mgmake.hxx`.

## Dependency role

This file is also the input root for `tools/amalgamate.py`. Adding a header here causes its project-local dependency graph to be incorporated into the generated single-header distribution.
