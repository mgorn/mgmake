# `examples/hello_world/build.sh`

**Source:** `examples/hello_world/build.sh`  
**Status:** Example bootstrap script.

This Bash script compiles the example build description into a native build-program executable and then runs it.

## Commands

```sh
clang++ -o build -I ../../include -std=c++2c build.cxx
./build
```

`-I ../../include` selects the modular headers. The generated executable is named `build`.

## Assumptions

- The script is run from `examples/hello_world`.
- `clang++` supports the C++ features used by mgmake.
- The build-description source matches the current API.

The last assumption is presently false because `examples/hello_world/build.cxx` uses obsolete identifiers. The script itself remains a useful model for bootstrapping once the example source is updated.

A production version should normally stop on compilation failure, for example by enabling `set -e`, so it does not execute an old `build` binary.
