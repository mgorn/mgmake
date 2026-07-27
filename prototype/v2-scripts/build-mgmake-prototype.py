#!/usr/bin/env python3
from __future__ import annotations

import argparse
import ctypes
import json
import os
import platform
import re
import shlex
import shutil
import subprocess
import sys
from dataclasses import dataclass, field
from enum import Enum
from pathlib import Path
from typing import Iterable, Mapping, Protocol, Sequence, TypeVar


ROOT = Path(__file__).resolve().parent
VERSION_TEXT = "MGMake prototype script 0.2"
TOOL_CACHE_SCHEMA = 1
USAGE_CACHE_SCHEMA = 1


class BuildError(RuntimeError):
    pass


# -----------------------------------------------------------------------------
# Tool model and toolchains
# -----------------------------------------------------------------------------


class ToolRole(Enum):
    CC = "cc"
    CXX = "cxx"
    OBJC = "objcc"
    OBJCXX = "objcxx"
    ASM = "asm"
    CUDA = "cuda"
    HIP = "hip"
    RC = "rc"
    MIDL = "midl"
    AR = "ar"
    RANLIB = "ranlib"
    LIB = "lib"
    LINKER = "linker"
    SHARED_LINKER = "shared-linker"
    DLLTOOL = "dlltool"
    MT = "mt"
    STRIP = "strip"
    OBJCOPY = "objcopy"
    OBJDUMP = "objdump"
    NM = "nm"
    READELF = "readelf"
    DEBUG_SYMBOL_TOOL = "debug-symbol-tool"
    LIPO = "lipo"
    INSTALL_NAME_TOOL = "install-name-tool"
    CODESIGN = "codesign"
    NINJA = "ninja"
    MAKE = "make"
    MSBUILD = "msbuild"
    XCODEBUILD = "xcodebuild"
    CMAKE = "cmake"
    PKG_CONFIG = "pkg-config"
    GIT = "git"
    CURL = "curl"
    WGET = "wget"
    UNZIP = "unzip"
    TAR = "tar"
    EXE_WRAPPER = "exe-wrapper"
    EMULATOR = "emulator"


TOOL_LABELS: dict[ToolRole, str] = {
    ToolRole.CC: "C Compiler",
    ToolRole.CXX: "C++ Compiler",
    ToolRole.OBJC: "Objective-C Compiler",
    ToolRole.OBJCXX: "Objective-C++ Compiler",
    ToolRole.ASM: "Assembler",
    ToolRole.CUDA: "CUDA Compiler",
    ToolRole.HIP: "HIP Compiler",
    ToolRole.RC: "Resource Compiler",
    ToolRole.MIDL: "MIDL Compiler",
    ToolRole.AR: "Archiver",
    ToolRole.RANLIB: "Ranlib",
    ToolRole.LIB: "Librarian",
    ToolRole.LINKER: "Linker",
    ToolRole.SHARED_LINKER: "Shared Linker",
    ToolRole.DLLTOOL: "DLL Tool",
    ToolRole.MT: "Manifest Tool",
    ToolRole.STRIP: "Strip",
    ToolRole.OBJCOPY: "Objcopy",
    ToolRole.OBJDUMP: "Objdump",
    ToolRole.NM: "NM",
    ToolRole.READELF: "Readelf",
    ToolRole.DEBUG_SYMBOL_TOOL: "Debug Symbol Tool",
    ToolRole.LIPO: "Lipo",
    ToolRole.INSTALL_NAME_TOOL: "Install Name Tool",
    ToolRole.CODESIGN: "Codesign",
    ToolRole.NINJA: "Ninja",
    ToolRole.MAKE: "Make",
    ToolRole.MSBUILD: "MSBuild",
    ToolRole.XCODEBUILD: "Xcodebuild",
    ToolRole.CMAKE: "CMake",
    ToolRole.PKG_CONFIG: "pkg-config",
    ToolRole.GIT: "Git",
    ToolRole.CURL: "Curl",
    ToolRole.WGET: "Wget",
    ToolRole.UNZIP: "Unzip",
    ToolRole.TAR: "Tar",
    ToolRole.EXE_WRAPPER: "Executable Wrapper",
    ToolRole.EMULATOR: "Emulator",
}


DEFAULT_TOOL_CANDIDATES: dict[ToolRole, tuple[str, ...]] = {
    ToolRole.CC: ("clang", "gcc", "cl"),
    ToolRole.CXX: ("clang++", "g++", "clang-cl", "cl"),
    ToolRole.OBJC: ("clang",),
    ToolRole.OBJCXX: ("clang++",),
    ToolRole.ASM: ("clang", "gcc", "as"),
    ToolRole.RC: ("llvm-rc", "rc"),
    ToolRole.MIDL: ("midl",),
    ToolRole.AR: ("llvm-ar", "ar"),
    ToolRole.RANLIB: ("llvm-ranlib", "ranlib"),
    ToolRole.LIB: ("llvm-lib", "lib"),
    ToolRole.LINKER: ("clang++", "g++", "clang-cl", "cl"),
    ToolRole.SHARED_LINKER: ("clang++", "g++", "clang-cl", "cl"),
    ToolRole.DLLTOOL: ("llvm-dlltool", "dlltool"),
    ToolRole.MT: ("llvm-mt", "mt"),
    ToolRole.STRIP: ("llvm-strip", "strip"),
    ToolRole.OBJCOPY: ("llvm-objcopy", "objcopy"),
    ToolRole.OBJDUMP: ("llvm-objdump", "objdump"),
    ToolRole.NM: ("llvm-nm", "nm"),
    ToolRole.READELF: ("llvm-readelf", "readelf"),
    ToolRole.DEBUG_SYMBOL_TOOL: ("dsymutil", "llvm-pdbutil"),
    ToolRole.LIPO: ("llvm-lipo", "lipo"),
    ToolRole.INSTALL_NAME_TOOL: ("install_name_tool",),
    ToolRole.CODESIGN: ("codesign",),
    ToolRole.NINJA: ("ninja", "ninja-build"),
    ToolRole.MAKE: ("make", "gmake", "mingw32-make"),
    ToolRole.MSBUILD: ("MSBuild.exe", "msbuild"),
    ToolRole.XCODEBUILD: ("xcodebuild",),
    ToolRole.CMAKE: ("cmake",),
    ToolRole.PKG_CONFIG: ("pkg-config",),
    ToolRole.GIT: ("git",),
    ToolRole.CURL: ("curl",),
    ToolRole.WGET: ("wget",),
    ToolRole.UNZIP: ("unzip",),
    ToolRole.TAR: ("tar",),
}


class DriverStyle(Enum):
    GNU = "gnu"
    MSVC = "msvc"


class ArchiverStyle(Enum):
    AR = "ar"
    LIB = "lib"


@dataclass(frozen=True)
class ToolchainSpec:
    name: str
    description: str
    candidates: Mapping[ToolRole, tuple[str, ...]]
    driver_style: DriverStyle = DriverStyle.GNU
    archiver_style: ArchiverStyle = ArchiverStyle.AR
    compile_options: tuple[str, ...] = ()
    link_options: tuple[str, ...] = ()
    cmake_options: tuple[str, ...] = ()
    preferred_generators: tuple[ToolRole, ...] = (ToolRole.NINJA, ToolRole.MAKE)

    def candidates_for(self, role: ToolRole) -> tuple[str, ...]:
        return self.candidates.get(role, ())


@dataclass(frozen=True)
class Tool:
    role: ToolRole
    command: tuple[str, ...]
    source: str

    def with_arguments(self, *arguments: str | Path) -> list[str]:
        return [*self.command, *(str(argument) for argument in arguments)]

    def as_path(self) -> str:
        if len(self.command) != 1:
            raise BuildError(
                f"{TOOL_LABELS[self.role]} cannot be used as a compiler path: "
                f"{display_command(self.command)}"
            )
        return self.command[0]


@dataclass(frozen=True)
class ToolRequirements:
    required: frozenset[ToolRole] = frozenset()
    optional: frozenset[ToolRole] = frozenset()

    @staticmethod
    def merge(*values: ToolRequirements) -> ToolRequirements:
        required = set(role for value in values for role in value.required)
        optional = set(role for value in values for role in value.optional)
        optional.difference_update(required)
        return ToolRequirements(frozenset(required), frozenset(optional))


# -----------------------------------------------------------------------------
# Declarative source, project, and target model
# -----------------------------------------------------------------------------


class SourceSpec(Protocol):
    def required_tools(self) -> ToolRequirements: ...


@dataclass(frozen=True)
class GitFetch:
    url: str
    ref: str | None = None
    shallow: bool = True

    def required_tools(self) -> ToolRequirements:
        return ToolRequirements(required=frozenset({ToolRole.GIT}))


@dataclass(frozen=True)
class LocalSource:
    path: Path

    def required_tools(self) -> ToolRequirements:
        return ToolRequirements()


class Target:
    name: str
    dependencies: tuple[Target, ...]


@dataclass(eq=False)
class CMakeProject:
    name: str
    source: SourceSpec
    languages: tuple[str, ...] = ("CXX",)
    options: Mapping[str, object] = field(default_factory=dict)

    def library(
        self,
        *,
        name: str,
        package: str,
        target: str,
        components: Sequence[str] = (),
        dependencies: Sequence[Target] = (),
    ) -> CMakeLibrary:
        return CMakeLibrary(
            name=name,
            project=self,
            package=package,
            imported_target=target,
            components=tuple(components),
            dependencies=tuple(dependencies),
        )


@dataclass(eq=False)
class CMakeLibrary(Target):
    name: str
    project: CMakeProject
    package: str
    imported_target: str
    components: tuple[str, ...] = ()
    dependencies: tuple[Target, ...] = ()


@dataclass(eq=False)
class CxxStaticLibrary(Target):
    name: str
    source: SourceSpec
    sources: tuple[str, ...]
    public_include_directories: tuple[str, ...] = ()
    private_include_directories: tuple[str, ...] = ()
    public_defines: tuple[str, ...] = ()
    private_defines: tuple[str, ...] = ()
    dependencies: tuple[Target, ...] = ()


@dataclass(eq=False)
class HeaderOnlyLibrary(Target):
    name: str
    source: SourceSpec
    public_include_directories: tuple[str, ...] = (".",)
    public_defines: tuple[str, ...] = ()
    dependencies: tuple[Target, ...] = ()


@dataclass(eq=False)
class CxxExecutable(Target):
    name: str
    source: SourceSpec
    sources: tuple[str, ...]
    include_directories: tuple[str, ...] = ()
    defines: tuple[str, ...] = ()
    dependencies: tuple[Target, ...] = ()


@dataclass(frozen=True)
class BuildProject:
    name: str
    targets: tuple[Target, ...]
    default_target: Target
    toolchains: tuple[ToolchainSpec, ...]
    default_toolchain: str

    def find_target(self, name: str) -> Target:
        for target in self.targets:
            if target.name == name:
                return target
        available = ", ".join(sorted(target.name for target in self.targets))
        raise BuildError(f"unknown target '{name}'; available targets: {available}")

    def find_toolchain(self, name: str | None) -> ToolchainSpec:
        selected = name or self.default_toolchain
        for toolchain in self.toolchains:
            if toolchain.name.casefold() == selected.casefold():
                return toolchain
        available = ", ".join(toolchain.name for toolchain in self.toolchains)
        raise BuildError(f"unknown toolchain '{selected}'; available toolchains: {available}")


# -----------------------------------------------------------------------------
# Paths, process execution, and common values
# -----------------------------------------------------------------------------


@dataclass(frozen=True)
class BuildLayout:
    project_root: Path
    build_root: Path

    @property
    def fetch_root(self) -> Path:
        return self.build_root / "fetch"

    @property
    def project_build_root(self) -> Path:
        return self.build_root / "projects"

    @property
    def install_root(self) -> Path:
        return self.build_root / "ext"

    @property
    def object_root(self) -> Path:
        return self.build_root / "objects"

    @property
    def artifact_root(self) -> Path:
        return self.build_root / "out"

    @property
    def probe_root(self) -> Path:
        return self.build_root / "probes"

    @property
    def state_root(self) -> Path:
        return self.build_root / "state"

    @property
    def tool_cache(self) -> Path:
        return self.state_root / "tools.json"

    @property
    def graph_file(self) -> Path:
        return self.build_root / "graph.dot"

    def fetched_source(self, owner_name: str) -> Path:
        return self.fetch_root / owner_name

    def cmake_build(self, project_name: str) -> Path:
        return self.project_build_root / "cmake" / project_name

    def install_prefix(self, project_name: str) -> Path:
        return self.install_root / project_name

    def object_directory(self, target_name: str) -> Path:
        return self.object_root / target_name

    def static_library(self, target_name: str, toolchain: ToolchainSpec) -> Path:
        cross_unix_archive = toolchain.name in {"Android", "Emscripten", "iOS"}
        if toolchain.archiver_style == ArchiverStyle.LIB or (os.name == "nt" and not cross_unix_archive):
            return self.artifact_root / target_name / f"{target_name}.lib"
        return self.artifact_root / target_name / f"lib{target_name}.a"

    def executable(self, target_name: str, toolchain: ToolchainSpec) -> Path:
        if toolchain.name == "Emscripten":
            suffix = ".html"
        elif toolchain.name in {"Android", "iOS"}:
            suffix = ""
        else:
            suffix = ".exe" if os.name == "nt" else ""
        return self.artifact_root / target_name / f"{target_name}{suffix}"

    def probe_source(self, target_name: str) -> Path:
        return self.probe_root / target_name / "source"

    def probe_build(self, target_name: str) -> Path:
        return self.probe_root / target_name / "build"

    def usage_cache(self, target_name: str) -> Path:
        return self.state_root / "usage" / f"{sanitize_name(target_name)}.json"


@dataclass(frozen=True)
class BuildOptions:
    config: str
    jobs: int | None
    cxx_standard: str
    verbose: bool
    short: bool
    dry_run: bool


class ProcessRunner:
    def __init__(self, options: BuildOptions) -> None:
        self.options = options

    def run(self, args: Sequence[str | Path], *, cwd: Path | None = None) -> None:
        command = [str(argument) for argument in args]
        if self.options.verbose or self.options.dry_run:
            self._print_command(command, cwd)
        if self.options.dry_run:
            return

        if self.options.short:
            result = subprocess.run(
                command,
                cwd=cwd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
            )
            if result.returncode != 0:
                if result.stdout:
                    print(result.stdout, end="", file=sys.stderr)
                if result.stderr:
                    print(result.stderr, end="", file=sys.stderr)
                raise subprocess.CalledProcessError(result.returncode, command)
            return

        subprocess.run(command, cwd=cwd, check=True)

    def capture(self, args: Sequence[str | Path], *, cwd: Path | None = None) -> str:
        command = [str(argument) for argument in args]
        if self.options.verbose or self.options.dry_run:
            self._print_command(command, cwd)
        if self.options.dry_run:
            return ""

        result = subprocess.run(
            command,
            cwd=cwd,
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        return result.stdout.strip()

    @staticmethod
    def _print_command(command: Sequence[str], cwd: Path | None) -> None:
        prefix = ""
        if cwd is not None:
            prefix = f"(cd {display_command((str(cwd),))}) "
        print(f"$ {prefix}{display_command(command)}", flush=True)


def display_command(args: Sequence[str]) -> str:
    if os.name == "nt":
        return subprocess.list2cmdline(list(args))
    return shlex.join(args)


T = TypeVar("T")


def unique_preserving_order(values: Iterable[T]) -> tuple[T, ...]:
    result: list[T] = []
    seen: set[T] = set()
    for value in values:
        if value not in seen:
            seen.add(value)
            result.append(value)
    return tuple(result)


def sanitize_name(value: str) -> str:
    sanitized = re.sub(r"[^A-Za-z0-9_.-]+", "_", value)
    return sanitized.strip("._-") or "target"


def resolve_build_root(value: str | None) -> Path:
    if value is None:
        return ROOT / ".build"
    path = Path(value).expanduser()
    if not path.is_absolute():
        path = ROOT / path
    return path.resolve()


# -----------------------------------------------------------------------------
# Tool discovery and cache
# -----------------------------------------------------------------------------


class ToolRegistry:
    def __init__(
        self,
        layout: BuildLayout,
        toolchain: ToolchainSpec,
        runner: ProcessRunner,
        overrides: Mapping[ToolRole, str],
    ) -> None:
        self.layout = layout
        self.toolchain = toolchain
        self.runner = runner
        self.overrides = dict(overrides)
        self._tools: dict[ToolRole, Tool] = {}

    def discover(self, requirements: ToolRequirements, *, allow_cache: bool = True) -> None:
        cached = self._load_cache() if allow_cache else {}
        all_roles = sorted(
            requirements.required | requirements.optional,
            key=lambda role: role.value,
        )

        for role in all_roles:
            required = role in requirements.required
            override = self.overrides.get(role)
            if override is not None:
                tool = self._resolve_tool(role, (override,), "command-line override", required=True)
            elif role in cached:
                tool = cached[role]
            else:
                toolchain_candidates = self.toolchain.candidates_for(role)
                candidates = (
                    toolchain_candidates
                    if toolchain_candidates
                    else DEFAULT_TOOL_CANDIDATES.get(role, ())
                )
                tool = self._resolve_tool(role, candidates, "discovered", required=required)

            if tool is not None:
                self._tools[role] = tool

        if ToolRole.CMAKE in self._tools and not self.runner.options.dry_run:
            self._check_cmake_version(self.require(ToolRole.CMAKE))

        if not self.runner.options.dry_run:
            self._save_cache()

    def require(self, role: ToolRole) -> Tool:
        try:
            return self._tools[role]
        except KeyError as error:
            raise BuildError(f"required tool was not discovered: {TOOL_LABELS[role]}") from error

    def optional(self, role: ToolRole) -> Tool | None:
        return self._tools.get(role)

    def print_tools(self) -> None:
        print(f"Toolchain: {self.toolchain.name}")
        if not self._tools:
            print("  <no tools required>")
            return
        for role in sorted(self._tools, key=lambda item: item.value):
            tool = self._tools[role]
            print(
                f"  {TOOL_LABELS[role]:24} "
                f"{display_command(tool.command)} [{tool.source}]"
            )

    def _resolve_tool(
        self,
        role: ToolRole,
        candidates: Sequence[str],
        source: str,
        *,
        required: bool,
    ) -> Tool | None:
        for candidate in candidates:
            command = self._resolve_candidate(candidate)
            if command is not None:
                return Tool(role=role, command=command, source=source)

        if self.runner.options.dry_run and candidates:
            return Tool(role=role, command=(candidates[0],), source="dry-run placeholder")
        if not required:
            return None

        rendered = ", ".join(repr(candidate) for candidate in candidates)
        raise BuildError(f"could not find {TOOL_LABELS[role]}; tried: {rendered}")

    def _resolve_candidate(self, candidate: str) -> tuple[str, ...] | None:
        expanded = os.path.expandvars(os.path.expanduser(candidate))

        if expanded.startswith("xcrun://"):
            _, remainder = expanded.split("xcrun://", 1)
            sdk, tool = remainder.split("/", 1)
            xcrun = shutil.which("xcrun")
            if xcrun is None:
                return None
            try:
                resolved = subprocess.run(
                    [xcrun, "--sdk", sdk, "--find", tool],
                    check=True,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    text=True,
                ).stdout.strip()
            except subprocess.CalledProcessError:
                return None
            return (resolved,)

        path = Path(expanded)
        has_directory = path.is_absolute() or path.parent != Path(".")
        if has_directory:
            if path.is_file():
                return (str(path.absolute()),)
            if os.name == "nt" and path.with_suffix(".exe").is_file():
                return (str(path.with_suffix(".exe").absolute()),)
            return None

        found = shutil.which(expanded)
        if found is None:
            return None
        # Preserve argv[0] as found. In particular, do not resolve clang++
        # symlinks to clang because driver behavior depends on its invoked name.
        return (str(Path(found).absolute()),)

    def _load_cache(self) -> dict[ToolRole, Tool]:
        path = self.layout.tool_cache
        if not path.is_file():
            return {}
        try:
            data = json.loads(path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError):
            return {}

        if data.get("schema") != TOOL_CACHE_SCHEMA:
            return {}
        if data.get("toolchain", "").casefold() != self.toolchain.name.casefold():
            return {}
        if data.get("host") != platform.platform():
            return {}

        result: dict[ToolRole, Tool] = {}
        for role_name, value in data.get("tools", {}).items():
            try:
                role = ToolRole(role_name)
                command = tuple(str(item) for item in value["command"])
            except (ValueError, KeyError, TypeError):
                continue
            if command and self._cached_command_is_valid(command):
                result[role] = Tool(role, command, "cache")
        return result

    @staticmethod
    def _cached_command_is_valid(command: tuple[str, ...]) -> bool:
        executable = command[0]
        path = Path(executable)
        if path.is_absolute() or path.parent != Path("."):
            return path.is_file()
        return shutil.which(executable) is not None

    def _save_cache(self) -> None:
        path = self.layout.tool_cache
        path.parent.mkdir(parents=True, exist_ok=True)
        data = {
            "schema": TOOL_CACHE_SCHEMA,
            "toolchain": self.toolchain.name,
            "host": platform.platform(),
            "tools": {
                role.value: {"command": list(tool.command)}
                for role, tool in sorted(self._tools.items(), key=lambda item: item[0].value)
            },
        }
        path.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")

    def _check_cmake_version(self, cmake: Tool) -> None:
        first_line = self.runner.capture(cmake.with_arguments("--version")).splitlines()[0]
        match = re.search(r"(\d+)\.(\d+)(?:\.(\d+))?", first_line)
        if match is None:
            raise BuildError(f"could not parse CMake version from: {first_line}")
        version = tuple(int(part or 0) for part in match.groups())
        minimum = (3, 16, 0)
        if version < minimum:
            raise BuildError(
                f"CMake {minimum[0]}.{minimum[1]} or newer is required; found {first_line}"
            )


# -----------------------------------------------------------------------------
# Usage requirements and caches
# -----------------------------------------------------------------------------


@dataclass(frozen=True)
class IncludeDirectory:
    path: Path
    is_system: bool = False


@dataclass(frozen=True)
class UsageRequirements:
    includes: tuple[IncludeDirectory, ...] = ()
    defines: tuple[str, ...] = ()
    frameworks: tuple[IncludeDirectory, ...] = ()
    link_args: tuple[str | Path, ...] = ()
    unresolved: tuple[str, ...] = ()

    @staticmethod
    def merge(*values: UsageRequirements) -> UsageRequirements:
        return UsageRequirements(
            includes=unique_preserving_order(
                include for value in values for include in value.includes
            ),
            defines=unique_preserving_order(
                define for value in values for define in value.defines
            ),
            frameworks=unique_preserving_order(
                framework for value in values for framework in value.frameworks
            ),
            # Link arguments are deliberately not deduplicated token-by-token.
            link_args=tuple(argument for value in values for argument in value.link_args),
            unresolved=unique_preserving_order(
                name for value in values for name in value.unresolved
            ),
        )


@dataclass(frozen=True)
class BuildResult:
    target: Target
    usage: UsageRequirements = UsageRequirements()
    artifacts: tuple[Path, ...] = ()
    package_prefixes: tuple[Path, ...] = ()


def save_usage(layout: BuildLayout, target_name: str, usage: UsageRequirements) -> None:
    path = layout.usage_cache(target_name)
    path.parent.mkdir(parents=True, exist_ok=True)
    data = {
        "schema": USAGE_CACHE_SCHEMA,
        "includes": [
            {"path": str(include.path), "system": include.is_system}
            for include in usage.includes
        ],
        "defines": list(usage.defines),
        "frameworks": [
            {"path": str(framework.path), "system": framework.is_system}
            for framework in usage.frameworks
        ],
        "link_args": [str(argument) for argument in usage.link_args],
    }
    path.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")


def load_usage(layout: BuildLayout, target_name: str) -> UsageRequirements | None:
    path = layout.usage_cache(target_name)
    if not path.is_file():
        return None
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
        if data.get("schema") != USAGE_CACHE_SCHEMA:
            return None
        return UsageRequirements(
            includes=tuple(
                IncludeDirectory(Path(item["path"]), bool(item.get("system", False)))
                for item in data.get("includes", [])
            ),
            defines=tuple(str(item) for item in data.get("defines", [])),
            frameworks=tuple(
                IncludeDirectory(Path(item["path"]), bool(item.get("system", False)))
                for item in data.get("frameworks", [])
            ),
            link_args=tuple(str(item) for item in data.get("link_args", [])),
        )
    except (OSError, json.JSONDecodeError, KeyError, TypeError):
        return None


def print_usage(name: str, usage: UsageRequirements) -> None:
    print(f"Usage requirements for '{name}':")
    if usage.includes:
        print("  include directories:")
        for include in usage.includes:
            suffix = " (system)" if include.is_system else ""
            print(f"    {include.path}{suffix}")
    if usage.defines:
        print("  definitions:")
        for define in usage.defines:
            print(f"    {define}")
    if usage.link_args:
        print("  link arguments:")
        for argument in usage.link_args:
            print(f"    {argument}")


def include_arguments(usage: UsageRequirements, style: DriverStyle) -> list[str]:
    arguments: list[str] = []
    if style == DriverStyle.MSVC:
        for include in usage.includes:
            arguments.append(f"/I{include.path}")
        for define in usage.defines:
            arguments.append(f"/D{define}")
        for name in usage.unresolved:
            arguments.append(f"<compile-usage:{name}>")
        return arguments

    for include in usage.includes:
        arguments.extend(["-isystem" if include.is_system else "-I", str(include.path)])
    for framework in usage.frameworks:
        arguments.extend(["-iframework" if framework.is_system else "-F", str(framework.path)])
    for define in usage.defines:
        arguments.append(f"-D{define}")
    for name in usage.unresolved:
        arguments.append(f"<compile-usage:{name}>")
    return arguments


def configuration_compile_arguments(config: str, style: DriverStyle) -> list[str]:
    normalized = config.casefold()
    if style == DriverStyle.MSVC:
        if normalized == "debug":
            return ["/Od", "/Zi"]
        if normalized == "relwithdebinfo":
            return ["/O2", "/Zi", "/DNDEBUG"]
        if normalized == "minsizerel":
            return ["/O1", "/DNDEBUG"]
        return ["/O2", "/DNDEBUG"]

    if normalized == "debug":
        return ["-O0", "-g"]
    if normalized == "relwithdebinfo":
        return ["-O2", "-g", "-DNDEBUG"]
    if normalized == "minsizerel":
        return ["-Os", "-DNDEBUG"]
    return ["-O3", "-DNDEBUG"]


# -----------------------------------------------------------------------------
# Target traversal and derived tool requirements
# -----------------------------------------------------------------------------


def topological_targets(targets: Sequence[Target]) -> tuple[Target, ...]:
    result: list[Target] = []
    visited: set[int] = set()
    visiting: set[int] = set()

    def visit(current: Target) -> None:
        key = id(current)
        if key in visited:
            return
        if key in visiting:
            raise BuildError(f"dependency cycle detected at target '{current.name}'")
        visiting.add(key)
        for dependency in current.dependencies:
            visit(dependency)
        visiting.remove(key)
        visited.add(key)
        result.append(current)

    for target in targets:
        visit(target)
    return tuple(result)


def source_for_target(target: Target) -> tuple[str, SourceSpec]:
    if isinstance(target, CMakeLibrary):
        return target.project.name, target.project.source
    if isinstance(target, (CxxStaticLibrary, HeaderOnlyLibrary, CxxExecutable)):
        return target.name, target.source
    raise BuildError(f"unsupported target type: {type(target).__name__}")


def required_tools_for(
    targets: Sequence[Target],
    task: str,
    toolchain: ToolchainSpec,
) -> ToolRequirements:
    requirements: list[ToolRequirements] = []
    ordered = topological_targets(targets)

    for target in ordered:
        _, source = source_for_target(target)
        requirements.append(source.required_tools())

        if task in {"configure", "build", "graph", "tools"}:
            if isinstance(target, CMakeLibrary):
                required = {ToolRole.CMAKE, ToolRole.CXX}
                languages = {language.upper() for language in target.project.languages}
                if "C" in languages:
                    required.add(ToolRole.CC)
                if "CXX" in languages:
                    required.add(ToolRole.CXX)
                requirements.append(
                    ToolRequirements(
                        required=frozenset(required),
                        optional=frozenset(toolchain.preferred_generators),
                    )
                )

        if task in {"build", "graph", "tools"}:
            if isinstance(target, CxxStaticLibrary):
                archiver = (
                    ToolRole.LIB
                    if toolchain.archiver_style == ArchiverStyle.LIB
                    else ToolRole.AR
                )
                requirements.append(
                    ToolRequirements(required=frozenset({ToolRole.CXX, archiver}))
                )
            elif isinstance(target, CxxExecutable):
                requirements.append(
                    ToolRequirements(required=frozenset({ToolRole.CXX, ToolRole.LINKER}))
                )

    return ToolRequirements.merge(*requirements)


def declared_package_prefixes(layout: BuildLayout, target: Target) -> tuple[Path, ...]:
    prefixes: list[Path] = []
    for current in topological_targets((target,)):
        if isinstance(current, CMakeLibrary):
            prefixes.append(layout.install_prefix(current.project.name))
    return unique_preserving_order(prefixes)


# -----------------------------------------------------------------------------
# Source adapter
# -----------------------------------------------------------------------------


class SourceManager:
    def __init__(
        self,
        layout: BuildLayout,
        tools: ToolRegistry,
        runner: ProcessRunner,
    ) -> None:
        self.layout = layout
        self.tools = tools
        self.runner = runner
        self._materialized: dict[tuple[int, str], Path] = {}

    def source_path(self, owner_name: str, source: SourceSpec) -> Path:
        if isinstance(source, LocalSource):
            return source.path.resolve()
        if isinstance(source, GitFetch):
            return self.layout.fetched_source(owner_name)
        raise BuildError(f"unsupported source specification: {type(source).__name__}")

    def materialize(self, owner_name: str, source: SourceSpec) -> Path:
        key = (id(source), owner_name)
        if key in self._materialized:
            return self._materialized[key]

        result = self.source_path(owner_name, source)
        if isinstance(source, LocalSource):
            if not result.is_dir() and not self.runner.options.dry_run:
                raise BuildError(f"local source directory does not exist: {result}")
        elif isinstance(source, GitFetch):
            self._fetch_git(source, result)

        self._materialized[key] = result
        return result

    def fetch_targets(self, targets: Sequence[Target]) -> None:
        seen: set[tuple[int, str]] = set()
        for target in topological_targets(targets):
            owner_name, source = source_for_target(target)
            key = (id(source), owner_name)
            if key in seen:
                continue
            seen.add(key)
            self.materialize(owner_name, source)

    def git_commands(self, source: GitFetch, destination: Path) -> tuple[list[str], ...]:
        git = self.tools.require(ToolRole.GIT)
        if not destination.exists():
            command = git.with_arguments("clone")
            if source.shallow:
                command.extend(["--depth", "1"])
            if source.ref is not None:
                command.extend(["--branch", source.ref, "--single-branch"])
            command.extend([source.url, str(destination)])
            return (command,)

        remote_ref = source.ref if source.ref is not None else "HEAD"
        fetch = git.with_arguments("-C", destination, "fetch")
        if source.shallow:
            fetch.extend(["--depth", "1"])
        fetch.extend(["origin", remote_ref])
        checkout = git.with_arguments("-C", destination, "checkout", "--detach", "FETCH_HEAD")
        return (fetch, checkout)

    def _fetch_git(self, source: GitFetch, destination: Path) -> None:
        destination.parent.mkdir(parents=True, exist_ok=True)

        if destination.exists() and not (destination / ".git").is_dir():
            raise BuildError(
                f"fetch destination exists but is not a Git repository: {destination}"
            )

        if destination.exists() and not self.runner.options.dry_run:
            git = self.tools.require(ToolRole.GIT)
            status = self.runner.capture(
                git.with_arguments("-C", destination, "status", "--porcelain")
            )
            if status:
                raise BuildError(
                    f"refusing to update modified dependency repository: {destination}\n{status}"
                )

        print(f"Fetching source '{destination.name}'...")
        for command in self.git_commands(source, destination):
            self.runner.run(command)


# -----------------------------------------------------------------------------
# CMake File API helpers
# -----------------------------------------------------------------------------


def prepare_file_api_query(build_directory: Path) -> None:
    query_directory = (
        build_directory
        / ".cmake"
        / "api"
        / "v1"
        / "query"
        / "client-mgmake-prototype"
    )
    query_directory.mkdir(parents=True, exist_ok=True)
    (query_directory / "codemodel-v2").touch()


def load_json(path: Path) -> dict:
    with path.open("r", encoding="utf-8") as file:
        value = json.load(file)
    if not isinstance(value, dict):
        raise BuildError(f"expected a JSON object in {path}")
    return value


def newest_reply_index(reply_directory: Path) -> Path:
    indexes = sorted(reply_directory.glob("index-*.json"))
    if not indexes:
        raise BuildError(f"CMake File API produced no reply index in {reply_directory}")
    return indexes[-1]


def select_codemodel(index: dict, reply_directory: Path) -> dict:
    references = [
        item
        for item in index.get("objects", [])
        if item.get("kind") == "codemodel"
        and item.get("version", {}).get("major") == 2
    ]
    if not references:
        raise BuildError("CMake File API reply does not contain codemodel version 2")
    reference = max(references, key=lambda item: item.get("version", {}).get("minor", -1))
    return load_json(reply_directory / reference["jsonFile"])


def select_configuration(codemodel: dict, requested_config: str) -> dict:
    configurations = codemodel.get("configurations", [])
    if not configurations:
        raise BuildError("CMake codemodel contains no configurations")
    for configuration in configurations:
        if configuration.get("name", "").casefold() == requested_config.casefold():
            return configuration
    if len(configurations) == 1:
        return configurations[0]
    names = ", ".join(
        configuration.get("name", "<unnamed>") for configuration in configurations
    )
    raise BuildError(
        f"CMake codemodel has no '{requested_config}' configuration; available: {names}"
    )


def resolve_cmake_path(value: str, base: Path) -> Path:
    path = Path(value)
    if not path.is_absolute():
        path = base / path
    return path.resolve()


def split_windows_command_line(fragment: str) -> list[str]:
    argc = ctypes.c_int()
    command_line_to_argv = ctypes.windll.shell32.CommandLineToArgvW
    command_line_to_argv.argtypes = [ctypes.c_wchar_p, ctypes.POINTER(ctypes.c_int)]
    command_line_to_argv.restype = ctypes.POINTER(ctypes.c_wchar_p)
    argv = command_line_to_argv(fragment, ctypes.byref(argc))
    if not argv:
        raise ctypes.WinError()
    try:
        return [argv[index] for index in range(argc.value)]
    finally:
        local_free = ctypes.windll.kernel32.LocalFree
        local_free.argtypes = [ctypes.c_void_p]
        local_free.restype = ctypes.c_void_p
        local_free(ctypes.cast(argv, ctypes.c_void_p))


def split_command_fragment(fragment: str) -> list[str]:
    if os.name == "nt":
        return split_windows_command_line(fragment)
    return shlex.split(fragment, posix=True)


def read_cmake_target_usage(
    build_directory: Path,
    target_name: str,
    config: str,
) -> UsageRequirements:
    reply_directory = build_directory / ".cmake" / "api" / "v1" / "reply"
    index = load_json(newest_reply_index(reply_directory))
    codemodel = select_codemodel(index, reply_directory)
    configuration = select_configuration(codemodel, config)

    target_reference = next(
        (
            target
            for target in configuration.get("targets", [])
            if target.get("name") == target_name
        ),
        None,
    )
    if target_reference is None:
        names = ", ".join(
            sorted(
                target.get("name", "<unnamed>")
                for target in configuration.get("targets", [])
            )
        )
        raise BuildError(
            f"target '{target_name}' was not found in the CMake codemodel; available: {names}"
        )

    target = load_json(reply_directory / target_reference["jsonFile"])
    codemodel_build_root = Path(codemodel["paths"]["build"])
    if not codemodel_build_root.is_absolute():
        codemodel_build_root = build_directory / codemodel_build_root
    codemodel_build_root = codemodel_build_root.resolve()

    includes: list[IncludeDirectory] = []
    defines: list[str] = []
    frameworks: list[IncludeDirectory] = []
    for group in target.get("compileGroups", []):
        for include in group.get("includes", []):
            includes.append(
                IncludeDirectory(
                    resolve_cmake_path(include["path"], codemodel_build_root),
                    bool(include.get("isSystem", False)),
                )
            )
        for define in group.get("defines", []):
            defines.append(define["define"])
        for framework in group.get("frameworks", []):
            frameworks.append(
                IncludeDirectory(
                    resolve_cmake_path(framework["path"], codemodel_build_root),
                    bool(framework.get("isSystem", False)),
                )
            )

    link_args: list[str | Path] = []
    for command_fragment in target.get("link", {}).get("commandFragments", []):
        if command_fragment.get("role") in {
            "flags",
            "libraries",
            "libraryPath",
            "frameworkPath",
        }:
            link_args.extend(split_command_fragment(command_fragment["fragment"]))

    return UsageRequirements(
        includes=unique_preserving_order(includes),
        defines=unique_preserving_order(defines),
        frameworks=unique_preserving_order(frameworks),
        link_args=tuple(link_args),
    )


# -----------------------------------------------------------------------------
# CMake adapter
# -----------------------------------------------------------------------------


class CMakeAdapter:
    def __init__(
        self,
        layout: BuildLayout,
        tools: ToolRegistry,
        sources: SourceManager,
        runner: ProcessRunner,
        options: BuildOptions,
        toolchain: ToolchainSpec,
    ) -> None:
        self.layout = layout
        self.tools = tools
        self.sources = sources
        self.runner = runner
        self.options = options
        self.toolchain = toolchain
        self._configured_projects: set[int] = set()
        self._built_projects: set[int] = set()

    def configure_target(self, target: CMakeLibrary) -> None:
        dependency_prefixes = unique_preserving_order(
            prefix
            for dependency in target.dependencies
            for prefix in declared_package_prefixes(self.layout, dependency)
        )
        self.configure_project(target.project, dependency_prefixes)

    def configure_project(
        self,
        project: CMakeProject,
        dependency_prefixes: Sequence[Path],
    ) -> None:
        project_key = id(project)
        if project_key in self._configured_projects:
            return

        source_directory = self.sources.materialize(project.name, project.source)
        build_directory = self.layout.cmake_build(project.name)
        install_directory = self.layout.install_prefix(project.name)
        if not self.runner.options.dry_run:
            build_directory.mkdir(parents=True, exist_ok=True)
            install_directory.mkdir(parents=True, exist_ok=True)

        print(f"Configuring CMake project '{project.name}'...")
        self.runner.run(
            self.configure_command(
                project,
                source_directory,
                build_directory,
                install_directory,
                dependency_prefixes,
            )
        )
        self._configured_projects.add(project_key)

    def build(
        self,
        target: CMakeLibrary,
        dependency_results: Sequence[BuildResult],
    ) -> BuildResult:
        dependency_usage = UsageRequirements.merge(
            *(result.usage for result in dependency_results)
        )
        dependency_prefixes = unique_preserving_order(
            prefix for result in dependency_results for prefix in result.package_prefixes
        )

        self.configure_project(target.project, dependency_prefixes)
        self.build_project(target.project)
        install_prefix = self.layout.install_prefix(target.project.name)

        if self.runner.options.dry_run:
            print(f"Resolving usage requirements for '{target.name}'...")
            self.runner.run(
                self.probe_command(
                    target,
                    (install_prefix, *dependency_prefixes),
                )
            )
            own_usage = load_usage(self.layout, target.name) or UsageRequirements(
                link_args=(f"<link-usage:{target.name}>",),
                unresolved=(target.name,),
            )
        else:
            own_usage = self.probe_installed_target(
                target,
                (install_prefix, *dependency_prefixes),
            )
            save_usage(self.layout, target.name, own_usage)
            if not self.options.short:
                print_usage(target.name, own_usage)

        return BuildResult(
            target=target,
            usage=UsageRequirements.merge(own_usage, dependency_usage),
            package_prefixes=unique_preserving_order((install_prefix, *dependency_prefixes)),
        )

    def build_project(self, project: CMakeProject) -> None:
        project_key = id(project)
        if project_key in self._built_projects:
            return
        print(f"Building and installing CMake project '{project.name}'...")
        self.runner.run(self.build_command(project))
        self._built_projects.add(project_key)

    def probe_installed_target(
        self,
        target: CMakeLibrary,
        prefixes: Sequence[Path],
    ) -> UsageRequirements:
        source_directory = self.layout.probe_source(target.name)
        build_directory = self.layout.probe_build(target.name)
        source_directory.mkdir(parents=True, exist_ok=True)
        build_directory.mkdir(parents=True, exist_ok=True)

        (source_directory / "probe.cxx").write_text(
            "int main() { return 0; }\n",
            encoding="utf-8",
        )
        components = ""
        if target.components:
            components = " COMPONENTS " + " ".join(target.components)
        (source_directory / "CMakeLists.txt").write_text(
            "\n".join(
                (
                    "cmake_minimum_required(VERSION 3.16)",
                    f"project(probe_{sanitize_name(target.name)} LANGUAGES CXX)",
                    f"find_package({target.package} CONFIG REQUIRED{components})",
                    "add_executable(probe probe.cxx)",
                    f"target_link_libraries(probe PRIVATE {target.imported_target})",
                    "",
                )
            ),
            encoding="utf-8",
        )
        prepare_file_api_query(build_directory)

        print(f"Resolving usage requirements for '{target.name}'...")
        self.runner.run(self.probe_command(target, prefixes))
        return read_cmake_target_usage(build_directory, "probe", self.options.config)

    def configure_command(
        self,
        project: CMakeProject,
        source_directory: Path,
        build_directory: Path,
        install_directory: Path,
        dependency_prefixes: Sequence[Path],
    ) -> list[str]:
        cmake = self.tools.require(ToolRole.CMAKE)
        command = cmake.with_arguments(
            "-S",
            source_directory,
            "-B",
            build_directory,
            *self.generator_arguments(build_directory),
            f"-DCMAKE_BUILD_TYPE:STRING={self.options.config}",
            f"-DCMAKE_INSTALL_PREFIX:PATH={install_directory}",
            *self.toolchain.cmake_options,
        )

        languages = {language.upper() for language in project.languages}
        if "C" in languages:
            command.append(
                f"-DCMAKE_C_COMPILER:FILEPATH={self.tools.require(ToolRole.CC).as_path()}"
            )
        if "CXX" in languages:
            command.append(
                f"-DCMAKE_CXX_COMPILER:FILEPATH={self.tools.require(ToolRole.CXX).as_path()}"
            )
        if dependency_prefixes:
            prefix_path = ";".join(str(path) for path in dependency_prefixes)
            command.append(f"-DCMAKE_PREFIX_PATH:STRING={prefix_path}")
        for name, value in project.options.items():
            command.append(f"-D{name}={self.cmake_value(value)}")
        return command

    def build_command(self, project: CMakeProject) -> list[str]:
        command = self.tools.require(ToolRole.CMAKE).with_arguments(
            "--build",
            self.layout.cmake_build(project.name),
            "--config",
            self.options.config,
            "--target",
            "install",
        )
        if self.options.jobs is None:
            command.append("--parallel")
        else:
            command.extend(["--parallel", str(self.options.jobs)])
        return command

    def probe_command(self, target: CMakeLibrary, prefixes: Sequence[Path]) -> list[str]:
        build_directory = self.layout.probe_build(target.name)
        prefix_path = ";".join(str(path) for path in unique_preserving_order(prefixes))
        return self.tools.require(ToolRole.CMAKE).with_arguments(
            "-S",
            self.layout.probe_source(target.name),
            "-B",
            build_directory,
            *self.generator_arguments(build_directory),
            f"-DCMAKE_BUILD_TYPE:STRING={self.options.config}",
            f"-DCMAKE_CXX_COMPILER:FILEPATH={self.tools.require(ToolRole.CXX).as_path()}",
            f"-DCMAKE_PREFIX_PATH:STRING={prefix_path}",
            *self.toolchain.cmake_options,
        )

    def generator_arguments(self, build_directory: Path) -> list[str]:
        if (build_directory / "CMakeCache.txt").exists():
            return []
        if os.environ.get("CMAKE_GENERATOR"):
            return []

        for role in self.toolchain.preferred_generators:
            tool = self.tools.optional(role)
            if tool is None:
                continue
            if role == ToolRole.NINJA:
                return ["-G", "Ninja", f"-DCMAKE_MAKE_PROGRAM:FILEPATH={tool.as_path()}"]
            if role == ToolRole.MAKE:
                return [
                    "-G",
                    "Unix Makefiles",
                    f"-DCMAKE_MAKE_PROGRAM:FILEPATH={tool.as_path()}",
                ]
            if role == ToolRole.XCODEBUILD:
                return ["-G", "Xcode"]
        return []

    @staticmethod
    def cmake_value(value: object) -> str:
        if isinstance(value, bool):
            return "ON" if value else "OFF"
        return str(value)


# -----------------------------------------------------------------------------
# Native C/C++ adapter
# -----------------------------------------------------------------------------


class NativeCxxAdapter:
    def __init__(
        self,
        layout: BuildLayout,
        tools: ToolRegistry,
        sources: SourceManager,
        runner: ProcessRunner,
        options: BuildOptions,
        toolchain: ToolchainSpec,
    ) -> None:
        self.layout = layout
        self.tools = tools
        self.sources = sources
        self.runner = runner
        self.options = options
        self.toolchain = toolchain

    def build_static_library(
        self,
        target: CxxStaticLibrary,
        dependency_results: Sequence[BuildResult],
    ) -> BuildResult:
        source_directory = self.sources.materialize(target.name, target.source)
        dependency_usage = UsageRequirements.merge(
            *(result.usage for result in dependency_results)
        )
        public_includes = tuple(
            IncludeDirectory((source_directory / path).resolve())
            for path in target.public_include_directories
        )
        private_includes = tuple(
            IncludeDirectory((source_directory / path).resolve())
            for path in target.private_include_directories
        )
        own_compile_usage = UsageRequirements(
            includes=unique_preserving_order((*public_includes, *private_includes)),
            defines=unique_preserving_order(
                (*target.public_defines, *target.private_defines)
            ),
        )
        compile_usage = UsageRequirements.merge(own_compile_usage, dependency_usage)

        object_directory = self.layout.object_directory(target.name)
        if object_directory.exists() and not self.runner.options.dry_run:
            shutil.rmtree(object_directory)
        if not self.runner.options.dry_run:
            object_directory.mkdir(parents=True, exist_ok=True)

        print(f"Compiling static library '{target.name}'...")
        objects: list[Path] = []
        object_suffix = ".obj" if self.toolchain.driver_style == DriverStyle.MSVC else ".o"
        for relative_source in target.sources:
            source = source_directory / relative_source
            if not source.is_file() and not self.runner.options.dry_run:
                raise BuildError(f"source file does not exist: {source}")
            object_path = object_directory / f"{sanitize_name(relative_source)}{object_suffix}"
            self.runner.run(self.compile_command(source, object_path, compile_usage))
            objects.append(object_path)

        library = self.layout.static_library(target.name, self.toolchain)
        if not self.runner.options.dry_run:
            library.parent.mkdir(parents=True, exist_ok=True)
            if library.exists():
                library.unlink()

        print(f"Archiving static library '{target.name}'...")
        self.runner.run(self.archive_command(library, objects))

        public_usage = UsageRequirements(
            includes=public_includes,
            defines=target.public_defines,
            link_args=(library,),
        )
        usage = UsageRequirements.merge(public_usage, dependency_usage)
        prefixes = unique_preserving_order(
            prefix for result in dependency_results for prefix in result.package_prefixes
        )
        if not self.runner.options.dry_run:
            save_usage(self.layout, target.name, usage)
        if not self.options.short:
            print_usage(target.name, usage)
        return BuildResult(target, usage, (library,), prefixes)

    def build_header_only(
        self,
        target: HeaderOnlyLibrary,
        dependency_results: Sequence[BuildResult],
    ) -> BuildResult:
        source_directory = self.sources.materialize(target.name, target.source)
        dependency_usage = UsageRequirements.merge(
            *(result.usage for result in dependency_results)
        )
        own_usage = UsageRequirements(
            includes=tuple(
                IncludeDirectory((source_directory / path).resolve())
                for path in target.public_include_directories
            ),
            defines=target.public_defines,
        )
        usage = UsageRequirements.merge(own_usage, dependency_usage)
        prefixes = unique_preserving_order(
            prefix for result in dependency_results for prefix in result.package_prefixes
        )
        if not self.runner.options.dry_run:
            save_usage(self.layout, target.name, usage)
        return BuildResult(target, usage, package_prefixes=prefixes)

    def build_executable(
        self,
        target: CxxExecutable,
        dependency_results: Sequence[BuildResult],
    ) -> BuildResult:
        source_directory = self.sources.materialize(target.name, target.source)
        dependency_usage = UsageRequirements.merge(
            *(result.usage for result in dependency_results)
        )
        own_usage = UsageRequirements(
            includes=tuple(
                IncludeDirectory((source_directory / path).resolve())
                for path in target.include_directories
            ),
            defines=target.defines,
        )
        compile_usage = UsageRequirements.merge(own_usage, dependency_usage)

        sources: list[Path] = []
        for relative_source in target.sources:
            source = source_directory / relative_source
            if not source.is_file() and not self.runner.options.dry_run:
                raise BuildError(f"source file does not exist: {source}")
            sources.append(source)

        output = self.layout.executable(target.name, self.toolchain)
        if not self.runner.options.dry_run:
            output.parent.mkdir(parents=True, exist_ok=True)

        print(f"Compiling and linking executable '{target.name}'...")
        self.runner.run(self.executable_command(sources, output, compile_usage, dependency_usage))
        prefixes = unique_preserving_order(
            prefix for result in dependency_results for prefix in result.package_prefixes
        )
        return BuildResult(target, artifacts=(output,), package_prefixes=prefixes)

    def compile_command(
        self,
        source: Path,
        object_path: Path,
        usage: UsageRequirements,
    ) -> list[str]:
        cxx = self.tools.require(ToolRole.CXX)
        if self.toolchain.driver_style == DriverStyle.MSVC:
            return cxx.with_arguments(
                "/nologo",
                "/c",
                source,
                "/std:c++latest",
                *configuration_compile_arguments(self.options.config, self.toolchain.driver_style),
                *self.toolchain.compile_options,
                *include_arguments(usage, self.toolchain.driver_style),
                f"/Fo{object_path}",
            )
        return cxx.with_arguments(
            "-c",
            source,
            f"-std={self.options.cxx_standard}",
            *configuration_compile_arguments(self.options.config, self.toolchain.driver_style),
            *self.toolchain.compile_options,
            *include_arguments(usage, self.toolchain.driver_style),
            "-o",
            object_path,
        )

    def archive_command(self, library: Path, objects: Sequence[Path]) -> list[str]:
        if self.toolchain.archiver_style == ArchiverStyle.LIB:
            return self.tools.require(ToolRole.LIB).with_arguments(
                "/NOLOGO",
                f"/OUT:{library}",
                *objects,
            )
        return self.tools.require(ToolRole.AR).with_arguments("rcs", library, *objects)

    def executable_command(
        self,
        sources: Sequence[Path],
        output: Path,
        compile_usage: UsageRequirements,
        dependency_usage: UsageRequirements,
    ) -> list[str]:
        linker = self.tools.require(ToolRole.LINKER)
        if self.toolchain.driver_style == DriverStyle.MSVC:
            return linker.with_arguments(
                "/nologo",
                *sources,
                "/std:c++latest",
                *configuration_compile_arguments(self.options.config, self.toolchain.driver_style),
                *self.toolchain.compile_options,
                *include_arguments(compile_usage, self.toolchain.driver_style),
                f"/Fe:{output}",
                *dependency_usage.link_args,
                *self.toolchain.link_options,
            )
        return linker.with_arguments(
            *sources,
            f"-std={self.options.cxx_standard}",
            *configuration_compile_arguments(self.options.config, self.toolchain.driver_style),
            *self.toolchain.compile_options,
            *include_arguments(compile_usage, self.toolchain.driver_style),
            "-o",
            output,
            *dependency_usage.link_args,
            *self.toolchain.link_options,
        )


# -----------------------------------------------------------------------------
# Build engine
# -----------------------------------------------------------------------------


class BuildEngine:
    def __init__(
        self,
        layout: BuildLayout,
        tools: ToolRegistry,
        runner: ProcessRunner,
        options: BuildOptions,
        toolchain: ToolchainSpec,
    ) -> None:
        self.layout = layout
        self.tools = tools
        self.runner = runner
        self.options = options
        self.toolchain = toolchain
        self.sources = SourceManager(layout, tools, runner)
        self.cmake = CMakeAdapter(
            layout,
            tools,
            self.sources,
            runner,
            options,
            toolchain,
        )
        self.native = NativeCxxAdapter(
            layout,
            tools,
            self.sources,
            runner,
            options,
            toolchain,
        )
        self._results: dict[int, BuildResult] = {}
        self._visiting: set[int] = set()

    def fetch(self, targets: Sequence[Target]) -> None:
        self.sources.fetch_targets(targets)

    def configure(self, targets: Sequence[Target]) -> None:
        self.fetch(targets)
        for target in topological_targets(targets):
            if isinstance(target, CMakeLibrary):
                self.cmake.configure_target(target)

    def build_many(self, targets: Sequence[Target]) -> tuple[BuildResult, ...]:
        return tuple(self.build(target) for target in targets)

    def build(self, target: Target) -> BuildResult:
        key = id(target)
        if key in self._results:
            return self._results[key]
        if key in self._visiting:
            raise BuildError(f"dependency cycle detected at target '{target.name}'")

        self._visiting.add(key)
        dependency_results = [self.build(dependency) for dependency in target.dependencies]

        if isinstance(target, CMakeLibrary):
            result = self.cmake.build(target, dependency_results)
        elif isinstance(target, CxxStaticLibrary):
            result = self.native.build_static_library(target, dependency_results)
        elif isinstance(target, HeaderOnlyLibrary):
            result = self.native.build_header_only(target, dependency_results)
        elif isinstance(target, CxxExecutable):
            result = self.native.build_executable(target, dependency_results)
        else:
            raise BuildError(f"unsupported target type: {type(target).__name__}")

        self._visiting.remove(key)
        self._results[key] = result
        return result


# -----------------------------------------------------------------------------
# Command graph
# -----------------------------------------------------------------------------


@dataclass(frozen=True)
class ActionNode:
    identifier: str
    label: str
    command: str | None
    dependencies: tuple[str, ...] = ()


class CommandGraphBuilder:
    def __init__(
        self,
        layout: BuildLayout,
        tools: ToolRegistry,
        options: BuildOptions,
        toolchain: ToolchainSpec,
    ) -> None:
        self.layout = layout
        self.tools = tools
        self.options = options
        self.toolchain = toolchain
        self.runner = ProcessRunner(options)
        self.sources = SourceManager(layout, tools, self.runner)
        self.cmake = CMakeAdapter(
            layout,
            tools,
            self.sources,
            self.runner,
            options,
            toolchain,
        )
        self.native = NativeCxxAdapter(
            layout,
            tools,
            self.sources,
            self.runner,
            options,
            toolchain,
        )
        self.nodes: dict[str, ActionNode] = {}
        self.ready_nodes: dict[int, str] = {}
        self.source_nodes: dict[tuple[int, str], str] = {}
        self.project_nodes: dict[int, str] = {}

    def build(self, targets: Sequence[Target]) -> tuple[ActionNode, ...]:
        discovery_id = "tools"
        self.nodes[discovery_id] = ActionNode(
            discovery_id,
            f"Discover and cache tools ({self.toolchain.name})",
            None,
        )
        for target in targets:
            self._add_target(target, discovery_id)
        return tuple(self.nodes.values())

    def _add_source(self, owner_name: str, source: SourceSpec, tools_node: str) -> str:
        key = (id(source), owner_name)
        if key in self.source_nodes:
            return self.source_nodes[key]
        identifier = f"fetch:{sanitize_name(owner_name)}"
        if isinstance(source, LocalSource):
            node = ActionNode(identifier, f"Use local source: {source.path.resolve()}", None, (tools_node,))
        elif isinstance(source, GitFetch):
            destination = self.layout.fetched_source(owner_name)
            commands = self.sources.git_commands(source, destination)
            command = "\n".join(display_command(item) for item in commands)
            node = ActionNode(identifier, f"Fetch {owner_name}", command, (tools_node,))
        else:
            raise BuildError(f"unsupported source specification: {type(source).__name__}")
        self.nodes[identifier] = node
        self.source_nodes[key] = identifier
        return identifier

    def _add_target(self, target: Target, tools_node: str) -> str:
        key = id(target)
        if key in self.ready_nodes:
            return self.ready_nodes[key]
        dependency_nodes = tuple(self._add_target(dep, tools_node) for dep in target.dependencies)
        owner_name, source = source_for_target(target)
        source_node = self._add_source(owner_name, source, tools_node)

        if isinstance(target, CMakeLibrary):
            ready = self._add_cmake_target(target, source_node, dependency_nodes)
        elif isinstance(target, CxxStaticLibrary):
            ready = self._add_static_library(target, source_node, dependency_nodes)
        elif isinstance(target, HeaderOnlyLibrary):
            identifier = f"ready:{sanitize_name(target.name)}"
            self.nodes[identifier] = ActionNode(
                identifier,
                f"Header-only target {target.name} ready",
                None,
                (source_node, *dependency_nodes),
            )
            ready = identifier
        elif isinstance(target, CxxExecutable):
            ready = self._add_executable(target, source_node, dependency_nodes)
        else:
            raise BuildError(f"unsupported target type: {type(target).__name__}")

        self.ready_nodes[key] = ready
        return ready

    def _add_cmake_target(
        self,
        target: CMakeLibrary,
        source_node: str,
        dependency_nodes: tuple[str, ...],
    ) -> str:
        project_key = id(target.project)
        if project_key not in self.project_nodes:
            source_directory = self.sources.source_path(target.project.name, target.project.source)
            dependency_prefixes = unique_preserving_order(
                prefix
                for dependency in target.dependencies
                for prefix in declared_package_prefixes(self.layout, dependency)
            )
            configure_id = f"configure:{sanitize_name(target.project.name)}"
            configure_command = self.cmake.configure_command(
                target.project,
                source_directory,
                self.layout.cmake_build(target.project.name),
                self.layout.install_prefix(target.project.name),
                dependency_prefixes,
            )
            self.nodes[configure_id] = ActionNode(
                configure_id,
                f"Configure CMake project {target.project.name}",
                display_command(configure_command),
                (source_node, *dependency_nodes),
            )
            build_id = f"cmake-build:{sanitize_name(target.project.name)}"
            self.nodes[build_id] = ActionNode(
                build_id,
                f"Build/install CMake project {target.project.name}",
                display_command(self.cmake.build_command(target.project)),
                (configure_id,),
            )
            self.project_nodes[project_key] = build_id

        build_id = self.project_nodes[project_key]
        probe_id = f"probe:{sanitize_name(target.name)}"
        prefixes = (
            self.layout.install_prefix(target.project.name),
            *unique_preserving_order(
                prefix
                for dependency in target.dependencies
                for prefix in declared_package_prefixes(self.layout, dependency)
            ),
        )
        self.nodes[probe_id] = ActionNode(
            probe_id,
            f"Resolve CMake usage requirements for {target.imported_target}",
            display_command(self.cmake.probe_command(target, prefixes)),
            (build_id, *dependency_nodes),
        )
        return probe_id

    def _usage_for_dependencies(self, dependencies: Sequence[Target]) -> UsageRequirements:
        values: list[UsageRequirements] = []
        for dependency in dependencies:
            cached = load_usage(self.layout, dependency.name)
            if cached is not None:
                values.append(cached)
                continue
            if isinstance(dependency, CMakeLibrary):
                values.append(
                    UsageRequirements(
                        link_args=(f"<link-usage:{dependency.name}>",),
                        unresolved=(dependency.name,),
                    )
                )
            elif isinstance(dependency, CxxStaticLibrary):
                source_directory = self.sources.source_path(dependency.name, dependency.source)
                own = UsageRequirements(
                    includes=tuple(
                        IncludeDirectory((source_directory / path).resolve())
                        for path in dependency.public_include_directories
                    ),
                    defines=dependency.public_defines,
                    link_args=(
                        self.layout.static_library(
                            dependency.name,
                            self.toolchain,
                        ),
                    ),
                )
                values.append(UsageRequirements.merge(own, self._usage_for_dependencies(dependency.dependencies)))
            elif isinstance(dependency, HeaderOnlyLibrary):
                source_directory = self.sources.source_path(dependency.name, dependency.source)
                own = UsageRequirements(
                    includes=tuple(
                        IncludeDirectory((source_directory / path).resolve())
                        for path in dependency.public_include_directories
                    ),
                    defines=dependency.public_defines,
                )
                values.append(UsageRequirements.merge(own, self._usage_for_dependencies(dependency.dependencies)))
        return UsageRequirements.merge(*values)

    def _add_static_library(
        self,
        target: CxxStaticLibrary,
        source_node: str,
        dependency_nodes: tuple[str, ...],
    ) -> str:
        source_directory = self.sources.source_path(target.name, target.source)
        dependency_usage = self._usage_for_dependencies(target.dependencies)
        own_usage = UsageRequirements(
            includes=tuple(
                IncludeDirectory((source_directory / path).resolve())
                for path in (*target.public_include_directories, *target.private_include_directories)
            ),
            defines=(*target.public_defines, *target.private_defines),
        )
        compile_usage = UsageRequirements.merge(own_usage, dependency_usage)
        compile_nodes: list[str] = []
        suffix = ".obj" if self.toolchain.driver_style == DriverStyle.MSVC else ".o"
        for relative_source in target.sources:
            identifier = f"compile:{sanitize_name(target.name)}:{sanitize_name(relative_source)}"
            object_path = self.layout.object_directory(target.name) / f"{sanitize_name(relative_source)}{suffix}"
            command = self.native.compile_command(
                source_directory / relative_source,
                object_path,
                compile_usage,
            )
            self.nodes[identifier] = ActionNode(
                identifier,
                f"Compile {target.name}: {relative_source}",
                display_command(command),
                (source_node, *dependency_nodes),
            )
            compile_nodes.append(identifier)

        archive_id = f"archive:{sanitize_name(target.name)}"
        library = self.layout.static_library(target.name, self.toolchain)
        object_paths = [
            self.layout.object_directory(target.name) / f"{sanitize_name(source)}{suffix}"
            for source in target.sources
        ]
        self.nodes[archive_id] = ActionNode(
            archive_id,
            f"Archive static library {target.name}",
            display_command(self.native.archive_command(library, object_paths)),
            tuple(compile_nodes),
        )
        return archive_id

    def _add_executable(
        self,
        target: CxxExecutable,
        source_node: str,
        dependency_nodes: tuple[str, ...],
    ) -> str:
        source_directory = self.sources.source_path(target.name, target.source)
        dependency_usage = self._usage_for_dependencies(target.dependencies)
        own_usage = UsageRequirements(
            includes=tuple(
                IncludeDirectory((source_directory / path).resolve())
                for path in target.include_directories
            ),
            defines=target.defines,
        )
        compile_usage = UsageRequirements.merge(own_usage, dependency_usage)
        sources = [source_directory / item for item in target.sources]
        output = self.layout.executable(target.name, self.toolchain)
        identifier = f"link:{sanitize_name(target.name)}"
        command = self.native.executable_command(
            sources,
            output,
            compile_usage,
            dependency_usage,
        )
        cached_missing = list(dependency_usage.unresolved)
        label = f"Compile/link executable {target.name}"
        if cached_missing:
            label += "\n(usage flags finalized after CMake probes: " + ", ".join(cached_missing) + ")"
        self.nodes[identifier] = ActionNode(
            identifier,
            label,
            display_command(command),
            (source_node, *dependency_nodes),
        )
        return identifier


def write_dot_graph(path: Path, nodes: Sequence[ActionNode]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    lines = [
        "digraph mgmake_prototype {",
        "  rankdir=LR;",
        '  graph [fontname="Consolas"];',
        '  node [shape=box, fontname="Consolas"];',
        '  edge [fontname="Consolas"];',
    ]
    for node in nodes:
        label = node.label
        if node.command:
            label += "\n\n" + node.command
        lines.append(f'  "{dot_escape(node.identifier)}" [label="{dot_escape(label)}"];')
    for node in nodes:
        for dependency in node.dependencies:
            lines.append(
                f'  "{dot_escape(dependency)}" -> "{dot_escape(node.identifier)}";'
            )
    lines.append("}")
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def dot_escape(value: str) -> str:
    return (
        value.replace("\\", "\\\\")
        .replace('"', '\\"')
        .replace("\n", "\\n")
    )


# -----------------------------------------------------------------------------
# Toolchain declarations
# -----------------------------------------------------------------------------


def android_toolchain() -> ToolchainSpec:
    ndk = os.environ.get("ANDROID_NDK_HOME") or os.environ.get("ANDROID_NDK_ROOT")
    abi = os.environ.get("ANDROID_ABI", "arm64-v8a")
    api = os.environ.get("ANDROID_API", "21")
    host_system = {
        "win32": "windows-x86_64",
        "darwin": "darwin-x86_64",
        "linux": "linux-x86_64",
    }.get(sys.platform, "linux-x86_64")
    triples = {
        "arm64-v8a": "aarch64-linux-android",
        "armeabi-v7a": "armv7a-linux-androideabi",
        "x86": "i686-linux-android",
        "x86_64": "x86_64-linux-android",
    }
    triple = triples.get(abi, "aarch64-linux-android")
    candidates: dict[ToolRole, tuple[str, ...]] = {}
    cmake_options: list[str] = []
    if ndk:
        ndk_path = Path(ndk)
        bin_dir = ndk_path / "toolchains" / "llvm" / "prebuilt" / host_system / "bin"
        suffix = ".cmd" if os.name == "nt" else ""
        candidates = {
            ToolRole.CC: (str(bin_dir / f"{triple}{api}-clang{suffix}"),),
            ToolRole.CXX: (str(bin_dir / f"{triple}{api}-clang++{suffix}"),),
            ToolRole.LINKER: (str(bin_dir / f"{triple}{api}-clang++{suffix}"),),
            ToolRole.AR: (str(bin_dir / ("llvm-ar.exe" if os.name == "nt" else "llvm-ar")),),
            ToolRole.RANLIB: (str(bin_dir / ("llvm-ranlib.exe" if os.name == "nt" else "llvm-ranlib")),),
            ToolRole.CMAKE: ("cmake",),
            ToolRole.NINJA: ("ninja",),
            ToolRole.GIT: ("git",),
        }
        cmake_options = [
            f"-DCMAKE_TOOLCHAIN_FILE:FILEPATH={ndk_path / 'build' / 'cmake' / 'android.toolchain.cmake'}",
            f"-DANDROID_ABI:STRING={abi}",
            f"-DANDROID_PLATFORM:STRING=android-{api}",
        ]
    else:
        candidates = {
            ToolRole.CC: (f"{triple}{api}-clang",),
            ToolRole.CXX: (f"{triple}{api}-clang++",),
            ToolRole.LINKER: (f"{triple}{api}-clang++",),
            ToolRole.AR: ("llvm-ar",),
            ToolRole.RANLIB: ("llvm-ranlib",),
            ToolRole.CMAKE: ("cmake",),
            ToolRole.NINJA: ("ninja",),
            ToolRole.GIT: ("git",),
        }

    return ToolchainSpec(
        name="Android",
        description="Android NDK Clang toolchain (ANDROID_NDK_HOME, ANDROID_ABI, ANDROID_API)",
        candidates=candidates,
        cmake_options=tuple(cmake_options),
        preferred_generators=(ToolRole.NINJA,),
    )


def ios_toolchain() -> ToolchainSpec:
    sdk = os.environ.get("IOS_SDK", "iphoneos")
    arch = os.environ.get("IOS_ARCH", "arm64")
    deployment = os.environ.get("IOS_DEPLOYMENT_TARGET", "13.0")
    return ToolchainSpec(
        name="iOS",
        description="Apple iOS toolchain selected through xcrun",
        candidates={
            ToolRole.CC: (f"xcrun://{sdk}/clang",),
            ToolRole.CXX: (f"xcrun://{sdk}/clang++",),
            ToolRole.LINKER: (f"xcrun://{sdk}/clang++",),
            ToolRole.AR: (f"xcrun://{sdk}/ar",),
            ToolRole.RANLIB: (f"xcrun://{sdk}/ranlib",),
            ToolRole.CMAKE: ("cmake",),
            ToolRole.XCODEBUILD: ("xcodebuild",),
            ToolRole.NINJA: ("ninja",),
            ToolRole.GIT: ("git",),
        },
        compile_options=("-arch", arch, f"-mios-version-min={deployment}"),
        link_options=("-arch", arch, f"-mios-version-min={deployment}"),
        cmake_options=(
            "-DCMAKE_SYSTEM_NAME:STRING=iOS",
            f"-DCMAKE_OSX_SYSROOT:STRING={sdk}",
            f"-DCMAKE_OSX_ARCHITECTURES:STRING={arch}",
            f"-DCMAKE_OSX_DEPLOYMENT_TARGET:STRING={deployment}",
        ),
        preferred_generators=(ToolRole.XCODEBUILD, ToolRole.NINJA),
    )


def define_toolchains() -> tuple[ToolchainSpec, ...]:
    llvm = ToolchainSpec(
        name="LLVM",
        description="LLVM tools with the Clang C and C++ drivers",
        candidates={
            ToolRole.CC: ("clang",),
            ToolRole.CXX: ("clang++",),
            ToolRole.LINKER: ("clang++",),
            ToolRole.SHARED_LINKER: ("clang++",),
            ToolRole.AR: ("llvm-ar",),
            ToolRole.RANLIB: ("llvm-ranlib",),
            ToolRole.LIB: ("llvm-lib",),
            ToolRole.RC: ("llvm-rc",),
            ToolRole.CMAKE: ("cmake",),
            ToolRole.NINJA: ("ninja",),
            ToolRole.GIT: ("git",),
        },
    )
    clang = ToolchainSpec(
        name="Clang",
        description="Clang drivers with host archive and build tools",
        candidates={
            ToolRole.CC: ("clang",),
            ToolRole.CXX: ("clang++",),
            ToolRole.LINKER: ("clang++",),
            ToolRole.AR: ("ar", "llvm-ar"),
            ToolRole.RANLIB: ("ranlib", "llvm-ranlib"),
            ToolRole.CMAKE: ("cmake",),
            ToolRole.NINJA: ("ninja",),
            ToolRole.MAKE: ("make",),
            ToolRole.GIT: ("git",),
        },
    )
    clang_cl = ToolchainSpec(
        name="Clang-CL",
        description="clang-cl, llvm-lib, and a Windows-compatible linker",
        candidates={
            ToolRole.CC: ("clang-cl",),
            ToolRole.CXX: ("clang-cl",),
            ToolRole.LINKER: ("clang-cl",),
            ToolRole.SHARED_LINKER: ("clang-cl",),
            ToolRole.LIB: ("llvm-lib", "lib"),
            ToolRole.RC: ("llvm-rc", "rc"),
            ToolRole.CMAKE: ("cmake",),
            ToolRole.NINJA: ("ninja",),
            ToolRole.MSBUILD: ("MSBuild.exe", "msbuild"),
            ToolRole.GIT: ("git",),
        },
        driver_style=DriverStyle.MSVC,
        archiver_style=ArchiverStyle.LIB,
        preferred_generators=(ToolRole.NINJA, ToolRole.MSBUILD),
    )
    gcc = ToolchainSpec(
        name="GCC",
        description="GNU Compiler Collection toolchain",
        candidates={
            ToolRole.CC: ("gcc",),
            ToolRole.CXX: ("g++",),
            ToolRole.LINKER: ("g++",),
            ToolRole.AR: ("ar",),
            ToolRole.RANLIB: ("ranlib",),
            ToolRole.CMAKE: ("cmake",),
            ToolRole.NINJA: ("ninja",),
            ToolRole.MAKE: ("make",),
            ToolRole.GIT: ("git",),
        },
    )
    apple = ToolchainSpec(
        name="AppleClang",
        description="Apple Clang for macOS",
        candidates={
            ToolRole.CC: ("xcrun://macosx/clang",),
            ToolRole.CXX: ("xcrun://macosx/clang++",),
            ToolRole.LINKER: ("xcrun://macosx/clang++",),
            ToolRole.AR: ("xcrun://macosx/ar",),
            ToolRole.RANLIB: ("xcrun://macosx/ranlib",),
            ToolRole.CMAKE: ("cmake",),
            ToolRole.NINJA: ("ninja",),
            ToolRole.XCODEBUILD: ("xcodebuild",),
            ToolRole.GIT: ("git",),
        },
        preferred_generators=(ToolRole.NINJA, ToolRole.XCODEBUILD),
    )
    emscripten = ToolchainSpec(
        name="Emscripten",
        description="Emscripten WebAssembly toolchain",
        candidates={
            ToolRole.CC: ("emcc",),
            ToolRole.CXX: ("em++",),
            ToolRole.LINKER: ("em++",),
            ToolRole.AR: ("emar",),
            ToolRole.RANLIB: ("emranlib",),
            ToolRole.CMAKE: ("cmake",),
            ToolRole.NINJA: ("ninja",),
            ToolRole.GIT: ("git",),
            ToolRole.EMULATOR: ("node",),
        },
        cmake_options=("-DCMAKE_SYSTEM_NAME:STRING=Emscripten",),
        preferred_generators=(ToolRole.NINJA,),
    )
    return (llvm, clang, clang_cl, gcc, apple, android_toolchain(), ios_toolchain(), emscripten)


# -----------------------------------------------------------------------------
# Project declaration
# -----------------------------------------------------------------------------


def define_project() -> BuildProject:
    glm_project = CMakeProject(
        name="glm",
        source=GitFetch(url="https://github.com/g-truc/glm.git"),
        languages=("CXX",),
        options={
            "GLM_BUILD_LIBRARY:BOOL": True,
            "GLM_BUILD_TESTS:BOOL": False,
            "GLM_BUILD_INSTALL:BOOL": True,
            "BUILD_SHARED_LIBS:BOOL": False,
        },
    )
    glm = glm_project.library(
        name="glm",
        package="glm",
        target="glm::glm",
    )

    sdl_project = CMakeProject(
        name="sdl",
        source=GitFetch(
            url="https://github.com/libsdl-org/SDL.git",
            ref="release-3.4.x",
        ),
        languages=("C",),
        options={
            "SDL_EXAMPLES:BOOL": False,
            "SDL_WERROR:BOOL": False,
            "SDL_SHARED_DEFAULT:BOOL": False,
            "BUILD_SHARED_LIBS:BOOL": False,
            "SDL_SHARED:BOOL": False,
            "SDL_STATIC:BOOL": True,
            "SDL_INSTALL:BOOL": True,
            "SDL_TEST_LIBRARY:BOOL": False,
            "SDL_TESTS:BOOL": False,
            "SDL_INSTALL_TESTS:BOOL": False,
        },
    )
    sdl = sdl_project.library(
        name="sdl",
        package="SDL3",
        target="SDL3::SDL3",
        components=("SDL3-static",),
    )

    imgui = CxxStaticLibrary(
        name="imgui",
        source=GitFetch(
            url="https://github.com/ocornut/imgui.git",
            ref="v1.92.9",
        ),
        sources=(
            "imgui.cpp",
            "imgui_demo.cpp",
            "imgui_draw.cpp",
            "imgui_tables.cpp",
            "imgui_widgets.cpp",
            "backends/imgui_impl_sdl3.cpp",
            "backends/imgui_impl_sdlrenderer3.cpp",
        ),
        public_include_directories=(".", "backends"),
        dependencies=(sdl,),
    )

    main = CxxExecutable(
        name="main",
        source=LocalSource(ROOT),
        sources=("main.cxx",),
        dependencies=(glm, imgui),
    )

    return BuildProject(
        name="glm-sdl-imgui-example",
        targets=(glm, sdl, imgui, main),
        default_target=main,
        toolchains=define_toolchains(),
        default_toolchain="LLVM",
    )


# -----------------------------------------------------------------------------
# CLI and tasks
# -----------------------------------------------------------------------------


TASKS: tuple[tuple[str, str], ...] = (
    ("build", "Build the project."),
    ("help", "Show help."),
    ("clean", "Delete all build files."),
    ("tools", "Discover and cache required tools for the project."),
    ("fetch", "Fetch all project dependencies."),
    ("configure", "Configure build-system projects."),
    ("graph", "Export the complete build command graph as Graphviz DOT."),
    ("version", "Display the version of MGMake."),
)


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(add_help=False, allow_abbrev=False)
    parser.add_argument("task", nargs="?", default="help")
    parser.add_argument("-h", "--help", action="store_true")
    parser.add_argument("--version", action="store_true")
    parser.add_argument("-v", "--verbose", action="store_true")
    parser.add_argument("-s", "--short", action="store_true")
    parser.add_argument("-d", "--dry-run", action="store_true")
    parser.add_argument("-b", "--build-dir")
    parser.add_argument("-t", "--targets", nargs="+", action="append", default=[])
    parser.add_argument("--toolchain")
    parser.add_argument("--config", default="Release")
    parser.add_argument("--jobs", type=int, default=None)
    parser.add_argument("--std", default="c++26")

    for role in ToolRole:
        parser.add_argument(f"--{role.value}", dest=f"tool_{role.value.replace('-', '_')}")

    return parser.parse_args()


def selected_targets(project: BuildProject, arguments: argparse.Namespace) -> tuple[Target, ...]:
    names = [name for group in arguments.targets for name in group]
    if not names:
        return (project.default_target,)
    return unique_preserving_order(project.find_target(name) for name in names)


def tool_overrides(arguments: argparse.Namespace) -> dict[ToolRole, str]:
    result: dict[ToolRole, str] = {}
    for role in ToolRole:
        value = getattr(arguments, f"tool_{role.value.replace('-', '_')}")
        if value:
            result[role] = value
    return result


def print_help(project: BuildProject) -> None:
    program = str(Path(sys.argv[0]).resolve())
    print("Usage:")
    print(f"        {program} [task] [options]")
    print("\nTasks:")
    width = max(len(name) for name, _ in TASKS)
    for name, description in TASKS:
        print(f"        {name:<{width}}  {description}")

    options: list[tuple[str, str]] = [
        ("-h, --help", "Show help."),
        ("--version", "Display the version of MGMake."),
        ("-v, --verbose", "Print commands before executing them."),
        ("-s, --short", "Keep command output short."),
        ("-d, --dry-run", "Print commands without executing them."),
        ("-b, --build-dir=<path>", "Set the build directory."),
        ("-t, --targets=<strings...>", "Build specific targets. May be passed multiple times."),
        ("--toolchain=<toolchain>", "Select a declared toolchain."),
        ("--config=<name>", "Set the build configuration (default: Release)."),
        ("--jobs=<count>", "Set the parallel CMake build job count."),
        ("--std=<standard>", "Set the C++ language standard (default: c++26)."),
    ]
    options.extend(
        (f"--{role.value}=<path>", f"Override the {TOOL_LABELS[role]} tool.")
        for role in ToolRole
    )
    print("\nOptions:")
    option_width = max(len(name) for name, _ in options)
    for name, description in options:
        print(f"        {name:<{option_width}}  {description}")

    print("\nTargets:")
    for target in project.targets:
        default = " (default)" if target is project.default_target else ""
        print(f"        {target.name:<20} {type(target).__name__}{default}")

    print("\nToolchains:")
    toolchain_width = max(len(toolchain.name) for toolchain in project.toolchains)
    for toolchain in project.toolchains:
        default = " (default)" if toolchain.name == project.default_toolchain else ""
        print(
            f"        {toolchain.name:<{toolchain_width}}  "
            f"{toolchain.description}{default}"
        )


def run_task(
    project: BuildProject,
    arguments: argparse.Namespace,
) -> int:
    task = arguments.task.casefold()
    valid_tasks = {name for name, _ in TASKS}
    if arguments.help:
        task = "help"
    if arguments.version:
        task = "version"
    if task not in valid_tasks:
        raise BuildError(
            f"unknown task '{arguments.task}'; available tasks: "
            + ", ".join(name for name, _ in TASKS)
        )

    if task == "help":
        print_help(project)
        return 0
    if task == "version":
        print(VERSION_TEXT)
        return 0

    layout = BuildLayout(ROOT, resolve_build_root(arguments.build_dir))
    if task == "clean":
        if layout.build_root.exists():
            print(f"Removing {layout.build_root}")
            if not arguments.dry_run:
                shutil.rmtree(layout.build_root)
        else:
            print(f"Build directory does not exist: {layout.build_root}")
        return 0

    targets = selected_targets(project, arguments)
    toolchain = project.find_toolchain(arguments.toolchain)
    options = BuildOptions(
        config=arguments.config,
        jobs=arguments.jobs,
        cxx_standard=arguments.std,
        verbose=arguments.verbose,
        short=arguments.short,
        dry_run=arguments.dry_run,
    )
    runner = ProcessRunner(options)
    registry = ToolRegistry(
        layout,
        toolchain,
        runner,
        tool_overrides(arguments),
    )
    requirements = required_tools_for(targets, task, toolchain)
    registry.discover(requirements)

    if task == "tools":
        registry.print_tools()
        return 0

    engine = BuildEngine(layout, registry, runner, options, toolchain)
    if task == "fetch":
        engine.fetch(targets)
        print("Fetch complete.")
        return 0
    if task == "configure":
        engine.configure(targets)
        print("Configure complete.")
        return 0
    if task == "graph":
        nodes = CommandGraphBuilder(layout, registry, options, toolchain).build(targets)
        write_dot_graph(layout.graph_file, nodes)
        print(f"Wrote Graphviz build graph: {layout.graph_file}")
        return 0
    if task == "build":
        results = engine.build_many(targets)
        artifacts = [artifact for result in results for artifact in result.artifacts]
        if artifacts:
            print("Dry run complete; planned artifacts:" if arguments.dry_run else "Build complete:")
            for artifact in artifacts:
                print(f"  {artifact}")
        else:
            print("Dry run complete." if arguments.dry_run else "Selected targets are ready.")
        return 0

    raise BuildError(f"unhandled task: {task}")


def main() -> int:
    try:
        arguments = parse_arguments()
        return run_task(define_project(), arguments)
    except (BuildError, subprocess.CalledProcessError, OSError, json.JSONDecodeError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
