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
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, Sequence, TypeVar


ROOT = Path(__file__).resolve().parent
BUILD_ROOT = ROOT / ".build"
FETCH_ROOT = BUILD_ROOT / "fetch"
CMAKE_BUILD_ROOT = BUILD_ROOT / "cmake"
EXT_ROOT = BUILD_ROOT / "ext"
OUT_ROOT = BUILD_ROOT / "out"
MANUAL_BUILD_ROOT = BUILD_ROOT / "manual"
PROBE_SOURCE_ROOT = BUILD_ROOT / "file-api-probe"
PROBE_BUILD_ROOT = CMAKE_BUILD_ROOT / "file-api-probe"

GLM_URL = "https://github.com/g-truc/glm.git"
SDL_URL = "https://github.com/libsdl-org/SDL.git"
IMGUI_URL = "https://github.com/ocornut/imgui.git"
SDL_REF = "release-3.4.x"
IMGUI_REF = "v1.92.9"

GLM_SOURCE = FETCH_ROOT / "glm-src"
SDL_SOURCE = FETCH_ROOT / "sdl-src"
IMGUI_SOURCE = FETCH_ROOT / "imgui-src"
GLM_BUILD = CMAKE_BUILD_ROOT / "glm"
SDL_BUILD = CMAKE_BUILD_ROOT / "sdl"
IMGUI_BUILD = MANUAL_BUILD_ROOT / "imgui"
GLM_INSTALL = EXT_ROOT / "glm"
SDL_INSTALL = EXT_ROOT / "sdl"
IMGUI_INSTALL = EXT_ROOT / "imgui"
IMGUI_INCLUDE = IMGUI_INSTALL / "include"
IMGUI_LIBRARY = IMGUI_INSTALL / "imgui.lib"

IMGUI_SOURCES = (
    "imgui.cpp",
    "imgui_demo.cpp",
    "imgui_draw.cpp",
    "imgui_tables.cpp",
    "imgui_widgets.cpp",
    "backends/imgui_impl_sdl3.cpp",
)

IMGUI_HEADERS = (
    "imconfig.h",
    "imgui.h",
    "imgui_internal.h",
    "imstb_rectpack.h",
    "imstb_textedit.h",
    "imstb_truetype.h",
    "backends/imgui_impl_sdl3.h",
)


class BuildError(RuntimeError):
    pass


@dataclass(frozen=True)
class Tools:
    git: str
    cmake: str
    clang: str
    clangxx: str
    archiver: str


@dataclass(frozen=True)
class IncludeDirectory:
    path: Path
    is_system: bool


@dataclass(frozen=True)
class TargetInfo:
    name: str
    includes: tuple[IncludeDirectory, ...]
    defines: tuple[str, ...]
    frameworks: tuple[IncludeDirectory, ...]
    artifacts: tuple[Path, ...]
    link_args: tuple[str, ...]
    reply_directory: Path


def display_command(args: Sequence[str]) -> str:
    if os.name == "nt":
        return subprocess.list2cmdline(list(args))
    return shlex.join(args)


def run(args: Sequence[str | Path], *, cwd: Path | None = None) -> None:
    command = [str(arg) for arg in args]
    print(f"\n$ {display_command(command)}", flush=True)
    subprocess.run(command, cwd=cwd, check=True)


def capture(args: Sequence[str | Path], *, cwd: Path | None = None) -> str:
    command = [str(arg) for arg in args]
    result = subprocess.run(
        command,
        cwd=cwd,
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    return result.stdout.strip()


def require_tool(name: str) -> str:
    path = shutil.which(name)
    if path is None:
        raise BuildError(f"required tool '{name}' was not found in PATH")
    executable = str(Path(path).absolute())
    print(f"[ok] {name}: {executable}")
    return executable


def require_any_tool(names: Sequence[str]) -> str:
    for name in names:
        path = shutil.which(name)
        if path is not None:
            executable = str(Path(path).absolute())
            print(f"[ok] {name}: {executable}")
            return executable
    joined = ", ".join(f"'{name}'" for name in names)
    raise BuildError(f"none of the required tools were found in PATH: {joined}")


def check_cmake_version(cmake: str) -> None:
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


def check_requirements() -> Tools:
    print("Checking requirements...")
    git = require_tool("git")
    cmake = require_tool("cmake")
    check_cmake_version(cmake)
    return Tools(
        git=git,
        cmake=cmake,
        clang=require_tool("clang"),
        clangxx=require_tool("clang++"),
        archiver=require_any_tool(("llvm-ar", "ar")),
    )


def ensure_clean_repository(git: str, directory: Path) -> None:
    status = capture([git, "-C", directory, "status", "--porcelain"])
    if status:
        raise BuildError(
            f"refusing to update modified dependency repository: {directory}\n{status}"
        )


def fetch_repository(
    git: str,
    *,
    url: str,
    destination: Path,
    ref: str | None = None,
) -> None:
    destination.parent.mkdir(parents=True, exist_ok=True)

    if not destination.exists():
        command: list[str | Path] = [git, "clone", "--depth", "1"]
        if ref is not None:
            command.extend(["--branch", ref, "--single-branch"])
        command.extend([url, destination])
        run(command)
        return

    if not (destination / ".git").is_dir():
        raise BuildError(f"dependency destination exists but is not a git repository: {destination}")

    ensure_clean_repository(git, destination)
    remote_ref = ref if ref is not None else "HEAD"
    run([git, "-C", destination, "fetch", "--depth", "1", "origin", remote_ref])
    run([git, "-C", destination, "checkout", "--detach", "FETCH_HEAD"])


def prepare_file_api_query(build_directory: Path) -> None:
    query_directory = build_directory / ".cmake" / "api" / "v1" / "query" / "client-mgmake"
    query_directory.mkdir(parents=True, exist_ok=True)
    (query_directory / "codemodel-v2").touch()


def generator_arguments(build_directory: Path) -> list[str]:
    if (build_directory / "CMakeCache.txt").exists():
        return []
    if os.environ.get("CMAKE_GENERATOR"):
        return []
    if shutil.which("ninja") is not None:
        return ["-G", "Ninja"]
    return []


def configure_cmake(
    tools: Tools,
    *,
    source_directory: Path,
    build_directory: Path,
    install_directory: Path,
    config: str,
    definitions: dict[str, str],
    languages: Iterable[str],
) -> None:
    prepare_file_api_query(build_directory)
    build_directory.mkdir(parents=True, exist_ok=True)
    install_directory.mkdir(parents=True, exist_ok=True)

    command: list[str | Path] = [
        tools.cmake,
        "-S",
        source_directory,
        "-B",
        build_directory,
        *generator_arguments(build_directory),
        f"-DCMAKE_BUILD_TYPE:STRING={config}",
        f"-DCMAKE_INSTALL_PREFIX:PATH={install_directory}",
    ]

    language_set = set(languages)
    if "C" in language_set:
        command.append(f"-DCMAKE_C_COMPILER:FILEPATH={tools.clang}")
    if "CXX" in language_set:
        command.append(f"-DCMAKE_CXX_COMPILER:FILEPATH={tools.clangxx}")

    for name, value in definitions.items():
        command.append(f"-D{name}={value}")

    run(command)


def build_target(tools: Tools, build_directory: Path, target: str, config: str, jobs: int | None) -> None:
    command: list[str | Path] = [
        tools.cmake,
        "--build",
        build_directory,
        "--config",
        config,
        "--target",
        target,
    ]
    if jobs is not None:
        command.extend(["--parallel", str(jobs)])
    else:
        command.append("--parallel")
    run(command)


def install_project(tools: Tools, build_directory: Path, install_directory: Path, config: str) -> None:
    run(
        [
            tools.cmake,
            "--install",
            build_directory,
            "--config",
            config,
            "--prefix",
            install_directory,
        ]
    )


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
    raise BuildError(f"CMake codemodel has no '{requested_config}' configuration; available: {names}")


def resolve_cmake_path(value: str, base: Path) -> Path:
    path = Path(value)
    if not path.is_absolute():
        path = base / path
    return path.resolve()


T = TypeVar("T")


def unique_preserving_order(values: Iterable[T]) -> tuple[T, ...]:
    result: list[T] = []
    seen: set[T] = set()
    for value in values:
        if value not in seen:
            seen.add(value)
            result.append(value)
    return tuple(result)


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


def read_target_info(build_directory: Path, target_name: str, config: str) -> TargetInfo:
    reply_directory = build_directory / ".cmake" / "api" / "v1" / "reply"
    index = load_json(newest_reply_index(reply_directory))
    codemodel = select_codemodel(index, reply_directory)
    configuration = select_configuration(codemodel, config)

    target_reference = next(
        (target for target in configuration.get("targets", []) if target.get("name") == target_name),
        None,
    )
    if target_reference is None:
        names = ", ".join(sorted(target.get("name", "<unnamed>") for target in configuration.get("targets", [])))
        raise BuildError(f"target '{target_name}' was not found in the CMake codemodel; available: {names}")

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

    artifacts = tuple(
        resolve_cmake_path(artifact["path"], codemodel_build_root)
        for artifact in target.get("artifacts", [])
    )

    link_args: list[str] = []
    link = target.get("link", {})
    for command_fragment in link.get("commandFragments", []):
        if command_fragment.get("role") in {"flags", "libraries", "libraryPath", "frameworkPath"}:
            link_args.extend(split_command_fragment(command_fragment["fragment"]))

    return TargetInfo(
        name=target_name,
        includes=unique_preserving_order(includes),
        defines=unique_preserving_order(defines),
        frameworks=unique_preserving_order(frameworks),
        artifacts=unique_preserving_order(artifacts),
        link_args=tuple(link_args),
        reply_directory=reply_directory,
    )


def print_target_info(info: TargetInfo) -> None:
    print(f"\nCMake File API target: {info.name}")

    print("  include directories:")
    if info.includes:
        for include in info.includes:
            suffix = " (system)" if include.is_system else ""
            print(f"    {include.path}{suffix}")
    else:
        print("    <none reported>")

    print("  target artifacts:")
    if info.artifacts:
        for artifact in info.artifacts:
            print(f"    {artifact}")
    else:
        print("    <none reported>")

    if info.link_args:
        print("  link items:")
        for argument in info.link_args:
            print(f"    {argument}")


def write_consumer_probe() -> None:
    PROBE_SOURCE_ROOT.mkdir(parents=True, exist_ok=True)
    (PROBE_SOURCE_ROOT / "probe.cxx").write_text("int main() { return 0; }\n", encoding="utf-8")
    (PROBE_SOURCE_ROOT / "CMakeLists.txt").write_text(
        """cmake_minimum_required(VERSION 3.16)
project(mgmake_file_api_probe LANGUAGES CXX)

find_package(glm CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED COMPONENTS SDL3-static)

add_executable(mgmake_glm_probe probe.cxx)
target_link_libraries(mgmake_glm_probe PRIVATE glm::glm)

add_executable(mgmake_sdl_probe probe.cxx)
target_link_libraries(mgmake_sdl_probe PRIVATE SDL3::SDL3)

add_executable(mgmake_all_probe probe.cxx)
target_link_libraries(mgmake_all_probe PRIVATE glm::glm SDL3::SDL3)
""",
        encoding="utf-8",
    )


def configure_consumer_probe(tools: Tools, config: str) -> None:
    write_consumer_probe()
    prepare_file_api_query(PROBE_BUILD_ROOT)
    PROBE_BUILD_ROOT.mkdir(parents=True, exist_ok=True)

    prefix_path = f"{GLM_INSTALL};{SDL_INSTALL}"
    command: list[str | Path] = [
        tools.cmake,
        "-S",
        PROBE_SOURCE_ROOT,
        "-B",
        PROBE_BUILD_ROOT,
        *generator_arguments(PROBE_BUILD_ROOT),
        f"-DCMAKE_BUILD_TYPE:STRING={config}",
        f"-DCMAKE_CXX_COMPILER:FILEPATH={tools.clangxx}",
        f"-DCMAKE_PREFIX_PATH:STRING={prefix_path}",
    ]
    run(command)


def include_arguments(info: TargetInfo) -> list[str]:
    arguments: list[str] = []
    for include in info.includes:
        arguments.extend(["-isystem" if include.is_system else "-I", str(include.path)])
    for framework in info.frameworks:
        arguments.extend(["-iframework" if framework.is_system else "-F", str(framework.path)])
    for define in info.defines:
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


def install_imgui_headers() -> None:
    IMGUI_INCLUDE.mkdir(parents=True, exist_ok=True)
    for relative_path in IMGUI_HEADERS:
        source = IMGUI_SOURCE / relative_path
        destination = IMGUI_INCLUDE / relative_path
        if not source.is_file():
            raise BuildError(f"ImGui header does not exist: {source}")
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, destination)

    license_source = IMGUI_SOURCE / "LICENSE.txt"
    if license_source.is_file():
        shutil.copy2(license_source, IMGUI_INSTALL / "LICENSE.txt")


def build_imgui(
    tools: Tools,
    sdl_info: TargetInfo,
    config: str,
    cxx_standard: str,
) -> Path:
    if IMGUI_BUILD.exists():
        shutil.rmtree(IMGUI_BUILD)
    if IMGUI_INSTALL.exists():
        shutil.rmtree(IMGUI_INSTALL)

    IMGUI_BUILD.mkdir(parents=True, exist_ok=True)
    IMGUI_INSTALL.mkdir(parents=True, exist_ok=True)

    common_arguments: list[str | Path] = [
        f"-std={cxx_standard}",
        *configuration_compile_arguments(config),
        "-I",
        IMGUI_SOURCE,
        "-I",
        IMGUI_SOURCE / "backends",
        *include_arguments(sdl_info),
    ]

    objects: list[Path] = []
    object_suffix = ".obj" if os.name == "nt" else ".o"
    for relative_path in IMGUI_SOURCES:
        source = IMGUI_SOURCE / relative_path
        if not source.is_file():
            raise BuildError(f"ImGui source does not exist: {source}")

        object_name = relative_path.replace("/", "_").replace("\\", "_")
        object_path = IMGUI_BUILD / f"{Path(object_name).stem}{object_suffix}"
        run(
            [
                tools.clangxx,
                "-c",
                source,
                *common_arguments,
                "-o",
                object_path,
            ]
        )
        objects.append(object_path)

    if IMGUI_LIBRARY.exists():
        IMGUI_LIBRARY.unlink()
    run([tools.archiver, "rcs", IMGUI_LIBRARY, *objects])
    install_imgui_headers()

    print("\nManually built ImGui:")
    print(f"  include directory: {IMGUI_INCLUDE}")
    print(f"  backend includes:  {IMGUI_INCLUDE / 'backends'}")
    print(f"  static library:    {IMGUI_LIBRARY}")
    return IMGUI_LIBRARY


def imgui_include_arguments() -> list[str | Path]:
    return [
        "-I",
        IMGUI_INCLUDE,
        "-I",
        IMGUI_INCLUDE / "backends",
    ]


def compile_main(
    tools: Tools,
    info: TargetInfo,
    imgui_library: Path,
    cxx_standard: str,
    config: str,
) -> Path:
    source = ROOT / "main.cxx"
    if not source.is_file():
        raise BuildError(f"source file does not exist: {source}")

    OUT_ROOT.mkdir(parents=True, exist_ok=True)
    output = OUT_ROOT / ("main.exe" if os.name == "nt" else "main")

    command: list[str | Path] = [
        tools.clangxx,
        str(source),
        f"-std={cxx_standard}",
        *configuration_compile_arguments(config),
        *include_arguments(info),
        *imgui_include_arguments(),
        "-o",
        str(output),
        imgui_library,
        *info.link_args,
    ]
    run(command, cwd=PROBE_BUILD_ROOT)
    return output


def remove_build_outputs() -> None:
    if BUILD_ROOT.exists():
        print(f"Removing {BUILD_ROOT}")
        shutil.rmtree(BUILD_ROOT)


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Fetch and build GLM, SDL3, and Dear ImGui, then compile main.cxx."
    )
    parser.add_argument("--config", default="Release", help="CMake configuration (default: Release)")
    parser.add_argument("--jobs", type=int, default=None, help="parallel build job count")
    parser.add_argument("--std", default="c++26", help="clang++ language standard (default: c++26)")
    parser.add_argument("--clean", action="store_true", help="remove .build before starting")
    return parser.parse_args()


def main() -> int:
    arguments = parse_arguments()

    try:
        if arguments.clean:
            remove_build_outputs()

        tools = check_requirements()

        print("\nFetching dependencies...")
        fetch_repository(tools.git, url=GLM_URL, destination=GLM_SOURCE)
        fetch_repository(tools.git, url=SDL_URL, destination=SDL_SOURCE, ref=SDL_REF)
        fetch_repository(tools.git, url=IMGUI_URL, destination=IMGUI_SOURCE, ref=IMGUI_REF)

        print("\nConfiguring GLM...")
        configure_cmake(
            tools,
            source_directory=GLM_SOURCE,
            build_directory=GLM_BUILD,
            install_directory=GLM_INSTALL,
            config=arguments.config,
            languages={"CXX"},
            definitions={
                "GLM_BUILD_LIBRARY:BOOL": "ON",
                "GLM_BUILD_TESTS:BOOL": "OFF",
                "GLM_BUILD_INSTALL:BOOL": "ON",
                "BUILD_SHARED_LIBS:BOOL": "OFF",
            },
        )

        print("\nConfiguring SDL...")
        configure_cmake(
            tools,
            source_directory=SDL_SOURCE,
            build_directory=SDL_BUILD,
            install_directory=SDL_INSTALL,
            config=arguments.config,
            languages={"C"},
            definitions={
                "SDL_EXAMPLES:BOOL": "OFF",
                "SDL_WERROR:BOOL": "OFF",
                "SDL_SHARED_DEFAULT:BOOL": "OFF",
                "BUILD_SHARED_LIBS:BOOL": "OFF",
                "SDL_SHARED:BOOL": "OFF",
                "SDL_STATIC:BOOL": "ON",
                "SDL_INSTALL:BOOL": "ON",
                "SDL_TEST_LIBRARY:BOOL": "OFF",
                "SDL_TESTS:BOOL": "OFF",
                "SDL_INSTALL_TESTS:BOOL": "OFF",
            },
        )

        print("\nBuilding and installing GLM...")
        build_target(tools, GLM_BUILD, "glm", arguments.config, arguments.jobs)
        install_project(tools, GLM_BUILD, GLM_INSTALL, arguments.config)

        print("\nBuilding and installing SDL...")
        build_target(tools, SDL_BUILD, "SDL3-static", arguments.config, arguments.jobs)
        install_project(tools, SDL_BUILD, SDL_INSTALL, arguments.config)

        glm_target = read_target_info(GLM_BUILD, "glm", arguments.config)
        sdl_target = read_target_info(SDL_BUILD, "SDL3-static", arguments.config)
        print_target_info(glm_target)
        print_target_info(sdl_target)

        print("\nConfiguring installed-package consumer probe...")
        configure_consumer_probe(tools, arguments.config)

        glm_consumer = read_target_info(PROBE_BUILD_ROOT, "mgmake_glm_probe", arguments.config)
        sdl_consumer = read_target_info(PROBE_BUILD_ROOT, "mgmake_sdl_probe", arguments.config)
        all_consumer = read_target_info(PROBE_BUILD_ROOT, "mgmake_all_probe", arguments.config)

        print("\nInstalled-package usage requirements:")
        print_target_info(glm_consumer)
        print_target_info(sdl_consumer)

        print("\nBuilding and installing ImGui manually...")
        imgui_library = build_imgui(
            tools,
            sdl_consumer,
            arguments.config,
            arguments.std,
        )

        print("\nCompiling main.cxx with clang++...")
        output = compile_main(
            tools,
            all_consumer,
            imgui_library,
            arguments.std,
            arguments.config,
        )
        print(f"\nBuild complete: {output}")
        return 0
    except (BuildError, subprocess.CalledProcessError, OSError, json.JSONDecodeError) as error:
        print(f"\nerror: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
