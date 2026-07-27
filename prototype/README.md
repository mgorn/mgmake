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
- `mgmake/toolchains.py` — LLVM, Clang, Clang-CL, MSVC/Visual Studio, GCC, Apple, Android, iOS, and Emscripten profiles.
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

## MSVC / Visual Studio

On Windows, select the Microsoft toolchain with:

```text
python build.py tools --toolchain MSVC
python build.py build --toolchain MSVC
```

The prototype uses `vswhere.exe` to find Visual Studio or the standalone Build Tools,
then initializes `VsDevCmd.bat` before discovering `cl.exe`, `lib.exe`, Windows SDK
tools, CMake, Ninja, and MSBuild. Ninja is preferred; the matching Visual Studio
CMake generator is used as a fallback.

The target and host architectures default to the current machine and may be overridden
with `MGMK_MSVC_ARCH` and `MGMK_MSVC_HOST_ARCH` (`x86`, `x64`, `arm`, or `arm64`).

## MSVC bootstrap troubleshooting

The `MSVC` and `Clang-CL` toolchains initialize a Visual Studio developer
environment automatically. The bootstrap first tries `VsDevCmd.bat` and then
falls back to `VC/Auxiliary/Build/vcvarsall.bat`. Existing partial or stale
Visual Studio environment variables are discarded before initialization.

If initialization fails, the error includes the exact batch command, exit code,
stdout, and stderr from both attempts. Architecture selection can be overridden
with:

```powershell
$env:MGMK_MSVC_ARCH = "x64"
$env:MGMK_MSVC_HOST_ARCH = "x64"
python build.py tools --toolchain MSVC
```

Supported architecture spellings include `x86`, `x64`, `amd64`, `arm`, and
`arm64`.
