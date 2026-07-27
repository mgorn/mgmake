# MGMake Python Prototype

This is the modular version of the MGMake prototype build script.

## User-facing files

- `build.py` — tiny executable entry point.
- `project.py` — declarative GLM, SDL, ImGui, and `main` project definition.
- `main.cxx` — expected beside `build.py`; not included in this archive.

## Framework package

- `mgmake/core.py` — errors, build layout, process execution, and common helpers.
- `mgmake/model.py` — declarative sources, projects, and targets.
- `mgmake/tools.py` — tool roles, discovery, overrides, and cache.
- `mgmake/toolchains.py` — LLVM, Clang, Clang-CL, GCC, Apple, Android, iOS, and Emscripten profiles.
- `mgmake/traversal.py` — target DAG traversal and derived requirements.
- `mgmake/sources.py` — local and Git source materialization.
- `mgmake/usage.py` — transitive compile/link usage requirements and cache.
- `mgmake/cmake_file_api.py` — CMake File API parsing.
- `mgmake/adapters/cmake.py` — CMake configure/build/install/probe adapter.
- `mgmake/adapters/native.py` — native C++ compile/archive/link adapter.
- `mgmake/engine.py` — recursive target build orchestration.
- `mgmake/graph.py` — command graph construction and Graphviz DOT export.
- `mgmake/cli.py` — task dispatch and command-line options.

## Usage

```text
python build.py help
python build.py tools
python build.py fetch
python build.py configure
python build.py graph
python build.py build
```
