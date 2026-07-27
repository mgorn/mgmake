from __future__ import annotations

import ctypes
import json
import os
import shlex
from pathlib import Path

from .core import BuildError, unique_preserving_order
from .usage import IncludeDirectory, UsageRequirements


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
