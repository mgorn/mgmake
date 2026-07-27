#!/usr/bin/env python3
from __future__ import annotations

import argparse
import ctypes
import json
import os
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


class BuildError(RuntimeError):
    pass


# -----------------------------------------------------------------------------
# Declarative model
# -----------------------------------------------------------------------------


class SourceSpec(Protocol):
    def required_tools(self) -> set[ToolRole]: ...


@dataclass(frozen=True)
class GitFetch:
    url: str
    ref: str | None = None
    shallow: bool = True

    def required_tools(self) -> set[ToolRole]:
        return {ToolRole.GIT}


@dataclass(frozen=True)
class LocalSource:
    path: Path

    def required_tools(self) -> set[ToolRole]:
        return set()


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

    def find_target(self, name: str | None) -> Target:
        if name is None:
            return self.default_target
        for target in self.targets:
            if target.name == name:
                return target
        available = ", ".join(sorted(target.name for target in self.targets))
        raise BuildError(f"unknown target '{name}'; available targets: {available}")


# -----------------------------------------------------------------------------
# Tool discovery
# -----------------------------------------------------------------------------


class ToolRole(Enum):
    GIT = "Git"
    CMAKE = "CMake"
    C_COMPILER = "C compiler"
    CXX_COMPILER = "C++ compiler"
    ARCHIVER = "static-library archiver"
    NINJA = "Ninja"


@dataclass(frozen=True)
class ToolDefinition:
    environment_variable: str | None
    candidates: tuple[str, ...]


TOOL_DEFINITIONS: dict[ToolRole, ToolDefinition] = {
    ToolRole.GIT: ToolDefinition("GIT", ("git",)),
    ToolRole.CMAKE: ToolDefinition("CMAKE", ("cmake",)),
    ToolRole.C_COMPILER: ToolDefinition("CC", ("clang", "gcc")),
    ToolRole.CXX_COMPILER: ToolDefinition("CXX", ("clang++", "g++")),
    ToolRole.ARCHIVER: ToolDefinition("AR", ("llvm-ar", "ar")),
    ToolRole.NINJA: ToolDefinition("NINJA", ("ninja",)),
}


class ToolRegistry:
    def __init__(self) -> None:
        self._tools: dict[ToolRole, str] = {}

    def discover(self, required: set[ToolRole]) -> None:
        print("Discovering required tools...")
        for role in sorted(required, key=lambda value: value.value):
            self._tools[role] = self._discover_one(role, required=True)

        if ToolRole.CMAKE in required:
            self._check_cmake_version(self.require(ToolRole.CMAKE))

    def require(self, role: ToolRole) -> str:
        try:
            return self._tools[role]
        except KeyError as error:
            raise BuildError(f"tool role was not discovered: {role.value}") from error

    def optional(self, role: ToolRole) -> str | None:
        if role in self._tools:
            return self._tools[role]
        result = self._discover_one(role, required=False)
        if result is not None:
            self._tools[role] = result
        return result

    def _discover_one(self, role: ToolRole, *, required: bool) -> str | None:
        definition = TOOL_DEFINITIONS[role]
        candidates: list[str] = []

        if definition.environment_variable is not None:
            override = os.environ.get(definition.environment_variable)
            if override:
                candidates.append(override)
        candidates.extend(definition.candidates)

        for candidate in candidates:
            path = self._resolve_executable(candidate)
            if path is not None:
                print(f"[ok] {role.value}: {path}")
                return path

        if not required:
            return None

        rendered = ", ".join(repr(candidate) for candidate in candidates)
        raise BuildError(f"could not find {role.value}; tried: {rendered}")

    @staticmethod
    def _resolve_executable(candidate: str) -> str | None:
        candidate_path = Path(candidate)
        if candidate_path.parent != Path(".") and candidate_path.is_file():
            return str(candidate_path.absolute())

        found = shutil.which(candidate)
        if found is None:
            return None
        return str(Path(found).absolute())

    @staticmethod
    def _check_cmake_version(cmake: str) -> None:
        first_line = capture([cmake, "--version"]).splitlines()[0]
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
# Build products and usage requirements
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
            # Link arguments are intentionally not deduplicated token-by-token.
            # Repeated flags such as "-framework" and deliberate repeated static
            # libraries are order-sensitive.
            link_args=tuple(argument for value in values for argument in value.link_args),
        )


@dataclass(frozen=True)
class BuildResult:
    target: Target
    usage: UsageRequirements = UsageRequirements()
    artifacts: tuple[Path, ...] = ()
    package_prefixes: tuple[Path, ...] = ()


# -----------------------------------------------------------------------------
# Paths and process helpers
# -----------------------------------------------------------------------------


@dataclass(frozen=True)
class BuildLayout:
    root: Path

    @property
    def build_root(self) -> Path:
        return self.root / ".build"

    @property
    def fetch_root(self) -> Path:
        return self.build_root / "fetch"

    @property
    def project_root(self) -> Path:
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

    def fetched_source(self, owner_name: str) -> Path:
        return self.fetch_root / owner_name

    def cmake_build(self, project_name: str) -> Path:
        return self.project_root / "cmake" / project_name

    def install_prefix(self, project_name: str) -> Path:
        return self.install_root / project_name

    def object_directory(self, target_name: str) -> Path:
        return self.object_root / target_name

    def static_library(self, target_name: str) -> Path:
        if os.name == "nt":
            return self.artifact_root / target_name / f"{target_name}.lib"
        return self.artifact_root / target_name / f"lib{target_name}.a"

    def executable(self, target_name: str) -> Path:
        suffix = ".exe" if os.name == "nt" else ""
        return self.artifact_root / target_name / f"{target_name}{suffix}"

    def probe_source(self, target_name: str) -> Path:
        return self.probe_root / target_name / "source"

    def probe_build(self, target_name: str) -> Path:
        return self.probe_root / target_name / "build"


@dataclass(frozen=True)
class BuildOptions:
    config: str
    jobs: int | None
    cxx_standard: str


def display_command(args: Sequence[str]) -> str:
    if os.name == "nt":
        return subprocess.list2cmdline(list(args))
    return shlex.join(args)


def run(args: Sequence[str | Path], *, cwd: Path | None = None) -> None:
    command = [str(argument) for argument in args]
    print(f"\n$ {display_command(command)}", flush=True)
    subprocess.run(command, cwd=cwd, check=True)


def capture(args: Sequence[str | Path], *, cwd: Path | None = None) -> str:
    command = [str(argument) for argument in args]
    result = subprocess.run(
        command,
        cwd=cwd,
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    return result.stdout.strip()


T = TypeVar("T")


def unique_preserving_order(values: Iterable[T]) -> tuple[T, ...]:
    result: list[T] = []
    seen: set[T] = set()
    for value in values:
        if value not in seen:
            seen.add(value)
            result.append(value)
    return tuple(result)


def include_arguments(usage: UsageRequirements) -> list[str]:
    arguments: list[str] = []
    for include in usage.includes:
        arguments.extend(["-isystem" if include.is_system else "-I", str(include.path)])
    for framework in usage.frameworks:
        arguments.extend(["-iframework" if framework.is_system else "-F", str(framework.path)])
    for define in usage.defines:
        arguments.append(f"-D{define}")
    return arguments


def configuration_compile_arguments(config: str) -> list[str]:
    normalized = config.casefold()
    if normalized == "debug":
        return ["-O0", "-g"]
    if normalized == "relwithdebinfo":
        return ["-O2", "-g", "-DNDEBUG"]
    if normalized == "minsizerel":
        return ["-Os", "-DNDEBUG"]
    return ["-O3", "-DNDEBUG"]


# -----------------------------------------------------------------------------
# Fetch adapters
# -----------------------------------------------------------------------------


class SourceManager:
    def __init__(self, layout: BuildLayout, tools: ToolRegistry) -> None:
        self.layout = layout
        self.tools = tools
        self._materialized: dict[tuple[int, str], Path] = {}

    def materialize(self, owner_name: str, source: SourceSpec) -> Path:
        key = (id(source), owner_name)
        if key in self._materialized:
            return self._materialized[key]

        if isinstance(source, LocalSource):
            result = source.path.resolve()
            if not result.is_dir():
                raise BuildError(f"local source directory does not exist: {result}")
        elif isinstance(source, GitFetch):
            result = self.layout.fetched_source(owner_name)
            self._fetch_git(source, result)
        else:
            raise BuildError(f"unsupported source specification: {type(source).__name__}")

        self._materialized[key] = result
        return result

    def _fetch_git(self, source: GitFetch, destination: Path) -> None:
        git = self.tools.require(ToolRole.GIT)
        destination.parent.mkdir(parents=True, exist_ok=True)

        if not destination.exists():
            command: list[str | Path] = [git, "clone"]
            if source.shallow:
                command.extend(["--depth", "1"])
            if source.ref is not None:
                command.extend(["--branch", source.ref, "--single-branch"])
            command.extend([source.url, destination])
            run(command)
            return

        if not (destination / ".git").is_dir():
            raise BuildError(
                f"fetch destination exists but is not a Git repository: {destination}"
            )

        status = capture([git, "-C", destination, "status", "--porcelain"])
        if status:
            raise BuildError(
                f"refusing to update modified dependency repository: {destination}\n{status}"
            )

        remote_ref = source.ref if source.ref is not None else "HEAD"
        command = [git, "-C", destination, "fetch"]
        if source.shallow:
            command.extend(["--depth", "1"])
        command.extend(["origin", remote_ref])
        run(command)
        run([git, "-C", destination, "checkout", "--detach", "FETCH_HEAD"])


# -----------------------------------------------------------------------------
# CMake adapter and File API
# -----------------------------------------------------------------------------


def prepare_file_api_query(build_directory: Path) -> None:
    query_directory = build_directory / ".cmake" / "api" / "v1" / "query" / "client-declarative-build"
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
        if item.get("kind") == "codemodel" and item.get("version", {}).get("major") == 2
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

    names = ", ".join(configuration.get("name", "<unnamed>") for configuration in configurations)
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
        (target for target in configuration.get("targets", []) if target.get("name") == target_name),
        None,
    )
    if target_reference is None:
        names = ", ".join(
            sorted(target.get("name", "<unnamed>") for target in configuration.get("targets", []))
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


class CMakeAdapter:
    def __init__(
        self,
        layout: BuildLayout,
        tools: ToolRegistry,
        sources: SourceManager,
        options: BuildOptions,
    ) -> None:
        self.layout = layout
        self.tools = tools
        self.sources = sources
        self.options = options
        self._built_projects: set[int] = set()

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

        self._build_project(target.project, dependency_prefixes)
        install_prefix = self.layout.install_prefix(target.project.name)
        own_usage = self._probe_installed_target(target, (install_prefix, *dependency_prefixes))

        return BuildResult(
            target=target,
            usage=UsageRequirements.merge(own_usage, dependency_usage),
            package_prefixes=unique_preserving_order((install_prefix, *dependency_prefixes)),
        )

    def _build_project(
        self,
        project: CMakeProject,
        dependency_prefixes: Sequence[Path],
    ) -> None:
        project_key = id(project)
        if project_key in self._built_projects:
            return

        source_directory = self.sources.materialize(project.name, project.source)
        build_directory = self.layout.cmake_build(project.name)
        install_directory = self.layout.install_prefix(project.name)
        build_directory.mkdir(parents=True, exist_ok=True)
        install_directory.mkdir(parents=True, exist_ok=True)

        print(f"\nConfiguring CMake project '{project.name}'...")
        command: list[str | Path] = [
            self.tools.require(ToolRole.CMAKE),
            "-S",
            source_directory,
            "-B",
            build_directory,
            *self._generator_arguments(build_directory),
            f"-DCMAKE_BUILD_TYPE:STRING={self.options.config}",
            f"-DCMAKE_INSTALL_PREFIX:PATH={install_directory}",
        ]

        languages = {language.upper() for language in project.languages}
        if "C" in languages:
            command.append(
                f"-DCMAKE_C_COMPILER:FILEPATH={self.tools.require(ToolRole.C_COMPILER)}"
            )
        if "CXX" in languages:
            command.append(
                f"-DCMAKE_CXX_COMPILER:FILEPATH={self.tools.require(ToolRole.CXX_COMPILER)}"
            )

        if dependency_prefixes:
            prefix_path = ";".join(str(path) for path in dependency_prefixes)
            command.append(f"-DCMAKE_PREFIX_PATH:STRING={prefix_path}")

        for name, value in project.options.items():
            command.append(f"-D{name}={self._cmake_value(value)}")

        run(command)

        print(f"\nBuilding and installing CMake project '{project.name}'...")
        build_command: list[str | Path] = [
            self.tools.require(ToolRole.CMAKE),
            "--build",
            build_directory,
            "--config",
            self.options.config,
            "--target",
            "install",
        ]
        if self.options.jobs is None:
            build_command.append("--parallel")
        else:
            build_command.extend(["--parallel", str(self.options.jobs)])
        run(build_command)

        self._built_projects.add(project_key)

    def _probe_installed_target(
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
        prefix_path = ";".join(str(path) for path in unique_preserving_order(prefixes))

        print(f"\nResolving usage requirements for '{target.name}'...")
        run(
            [
                self.tools.require(ToolRole.CMAKE),
                "-S",
                source_directory,
                "-B",
                build_directory,
                *self._generator_arguments(build_directory),
                f"-DCMAKE_BUILD_TYPE:STRING={self.options.config}",
                f"-DCMAKE_CXX_COMPILER:FILEPATH={self.tools.require(ToolRole.CXX_COMPILER)}",
                f"-DCMAKE_PREFIX_PATH:STRING={prefix_path}",
            ]
        )

        usage = read_cmake_target_usage(build_directory, "probe", self.options.config)
        print_usage(target.name, usage)
        return usage

    def _generator_arguments(self, build_directory: Path) -> list[str]:
        if (build_directory / "CMakeCache.txt").exists():
            return []
        if os.environ.get("CMAKE_GENERATOR"):
            return []

        ninja = self.tools.optional(ToolRole.NINJA)
        if ninja is None:
            return []
        return ["-G", "Ninja", f"-DCMAKE_MAKE_PROGRAM:FILEPATH={ninja}"]

    @staticmethod
    def _cmake_value(value: object) -> str:
        if isinstance(value, bool):
            return "ON" if value else "OFF"
        return str(value)


# -----------------------------------------------------------------------------
# Native C++ adapter
# -----------------------------------------------------------------------------


class NativeCxxAdapter:
    def __init__(
        self,
        layout: BuildLayout,
        tools: ToolRegistry,
        sources: SourceManager,
        options: BuildOptions,
    ) -> None:
        self.layout = layout
        self.tools = tools
        self.sources = sources
        self.options = options

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
            defines=unique_preserving_order((*target.public_defines, *target.private_defines)),
        )
        compile_usage = UsageRequirements.merge(own_compile_usage, dependency_usage)

        object_directory = self.layout.object_directory(target.name)
        if object_directory.exists():
            shutil.rmtree(object_directory)
        object_directory.mkdir(parents=True, exist_ok=True)

        print(f"\nCompiling static library '{target.name}'...")
        objects: list[Path] = []
        object_suffix = ".obj" if os.name == "nt" else ".o"

        for relative_source in target.sources:
            source = source_directory / relative_source
            if not source.is_file():
                raise BuildError(f"source file does not exist: {source}")

            object_name = sanitize_name(relative_source)
            object_path = object_directory / f"{object_name}{object_suffix}"
            run(
                [
                    self.tools.require(ToolRole.CXX_COMPILER),
                    "-c",
                    source,
                    f"-std={self.options.cxx_standard}",
                    *configuration_compile_arguments(self.options.config),
                    *include_arguments(compile_usage),
                    "-o",
                    object_path,
                ]
            )
            objects.append(object_path)

        library = self.layout.static_library(target.name)
        library.parent.mkdir(parents=True, exist_ok=True)
        if library.exists():
            library.unlink()

        print(f"\nArchiving static library '{target.name}'...")
        run([self.tools.require(ToolRole.ARCHIVER), "rcs", library, *objects])

        public_usage = UsageRequirements(
            includes=public_includes,
            defines=target.public_defines,
            link_args=(library,),
        )
        usage = UsageRequirements.merge(public_usage, dependency_usage)
        prefixes = unique_preserving_order(
            prefix for result in dependency_results for prefix in result.package_prefixes
        )

        print_usage(target.name, usage)
        return BuildResult(
            target=target,
            usage=usage,
            artifacts=(library,),
            package_prefixes=prefixes,
        )

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
        prefixes = unique_preserving_order(
            prefix for result in dependency_results for prefix in result.package_prefixes
        )
        return BuildResult(
            target=target,
            usage=UsageRequirements.merge(own_usage, dependency_usage),
            package_prefixes=prefixes,
        )

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
            if not source.is_file():
                raise BuildError(f"source file does not exist: {source}")
            sources.append(source)

        output = self.layout.executable(target.name)
        output.parent.mkdir(parents=True, exist_ok=True)

        print(f"\nCompiling and linking executable '{target.name}'...")
        run(
            [
                self.tools.require(ToolRole.CXX_COMPILER),
                *sources,
                f"-std={self.options.cxx_standard}",
                *configuration_compile_arguments(self.options.config),
                *include_arguments(compile_usage),
                "-o",
                output,
                *dependency_usage.link_args,
            ]
        )

        prefixes = unique_preserving_order(
            prefix for result in dependency_results for prefix in result.package_prefixes
        )
        return BuildResult(
            target=target,
            artifacts=(output,),
            package_prefixes=prefixes,
        )


# -----------------------------------------------------------------------------
# Graph planning and execution
# -----------------------------------------------------------------------------


class BuildEngine:
    def __init__(
        self,
        layout: BuildLayout,
        tools: ToolRegistry,
        options: BuildOptions,
    ) -> None:
        self.layout = layout
        self.tools = tools
        self.options = options
        self.sources = SourceManager(layout, tools)
        self.cmake = CMakeAdapter(layout, tools, self.sources, options)
        self.native = NativeCxxAdapter(layout, tools, self.sources, options)
        self._results: dict[int, BuildResult] = {}
        self._visiting: set[int] = set()

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


def required_tools_for(target: Target) -> set[ToolRole]:
    result: set[ToolRole] = set()
    visited: set[int] = set()

    def visit(current: Target) -> None:
        key = id(current)
        if key in visited:
            return
        visited.add(key)

        for dependency in current.dependencies:
            visit(dependency)

        if isinstance(current, CMakeLibrary):
            result.update(current.project.source.required_tools())
            result.add(ToolRole.CMAKE)
            result.add(ToolRole.CXX_COMPILER)  # Used by the consumer probe.
            languages = {language.upper() for language in current.project.languages}
            if "C" in languages:
                result.add(ToolRole.C_COMPILER)
            if "CXX" in languages:
                result.add(ToolRole.CXX_COMPILER)
        elif isinstance(current, CxxStaticLibrary):
            result.update(current.source.required_tools())
            result.update({ToolRole.CXX_COMPILER, ToolRole.ARCHIVER})
        elif isinstance(current, HeaderOnlyLibrary):
            result.update(current.source.required_tools())
        elif isinstance(current, CxxExecutable):
            result.update(current.source.required_tools())
            result.add(ToolRole.CXX_COMPILER)
        else:
            raise BuildError(f"unsupported target type: {type(current).__name__}")

    visit(target)
    return result


def topological_targets(target: Target) -> tuple[Target, ...]:
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

    visit(target)
    return tuple(result)


def print_plan(target: Target) -> None:
    print(f"Build target: {target.name}")
    print("\nTarget order:")
    for index, planned_target in enumerate(topological_targets(target), start=1):
        dependencies = ", ".join(dependency.name for dependency in planned_target.dependencies)
        if not dependencies:
            dependencies = "<none>"
        print(
            f"  {index}. {planned_target.name} "
            f"[{type(planned_target).__name__}] <- {dependencies}"
        )

    print("\nRequired tool roles:")
    for role in sorted(required_tools_for(target), key=lambda value: value.value):
        print(f"  {role.value}")


def print_usage(name: str, usage: UsageRequirements) -> None:
    print(f"\nUsage requirements for '{name}':")
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


def sanitize_name(value: str) -> str:
    sanitized = re.sub(r"[^A-Za-z0-9_.-]+", "_", value)
    return sanitized.strip("._-") or "target"


# -----------------------------------------------------------------------------
# Project declaration
# -----------------------------------------------------------------------------


def define_project() -> BuildProject:
    glm_project = CMakeProject(
        name="glm",
        source=GitFetch(
            url="https://github.com/g-truc/glm.git",
        ),
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
        public_include_directories=(
            ".",
            "backends",
        ),
        dependencies=(sdl,),
    )

    main = CxxExecutable(
        name="main",
        source=LocalSource(ROOT),
        sources=("main.cxx",),
        # SDL is intentionally absent here. ImGui depends on SDL, so SDL's
        # include and link requirements reach main transitively.
        dependencies=(),
    )

    return BuildProject(
        name="glm-sdl-imgui-example",
        targets=(glm, sdl, imgui, main),
        default_target=main,
    )


# -----------------------------------------------------------------------------
# CLI
# -----------------------------------------------------------------------------


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Declaratively fetch and build the GLM, SDL3, ImGui example.",
    )
    parser.add_argument("target", nargs="?", help="target to build (default: main)")
    parser.add_argument("--config", default="Release", help="build configuration (default: Release)")
    parser.add_argument("--jobs", type=int, default=None, help="parallel CMake build job count")
    parser.add_argument("--std", default="c++26", help="C++ language standard (default: c++26)")
    parser.add_argument("--clean", action="store_true", help="remove .build before building")
    parser.add_argument("--plan", action="store_true", help="print the derived graph and exit")
    return parser.parse_args()


def main() -> int:
    arguments = parse_arguments()

    try:
        project = define_project()
        selected_target = project.find_target(arguments.target)

        if arguments.plan:
            print_plan(selected_target)
            return 0

        # Tool discovery is the first build operation and is derived solely
        # from the selected target's reachable dependency graph.
        tools = ToolRegistry()
        tools.discover(required_tools_for(selected_target))

        layout = BuildLayout(ROOT)
        if arguments.clean and layout.build_root.exists():
            print(f"\nRemoving {layout.build_root}")
            shutil.rmtree(layout.build_root)

        options = BuildOptions(
            config=arguments.config,
            jobs=arguments.jobs,
            cxx_standard=arguments.std,
        )
        result = BuildEngine(layout, tools, options).build(selected_target)

        if result.artifacts:
            print("\nBuild complete:")
            for artifact in result.artifacts:
                print(f"  {artifact}")
        else:
            print(f"\nTarget '{selected_target.name}' is ready.")
        return 0
    except (BuildError, subprocess.CalledProcessError, OSError, json.JSONDecodeError) as error:
        print(f"\nerror: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
