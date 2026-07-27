from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path

from .core import USAGE_CACHE_SCHEMA, BuildLayout, unique_preserving_order
from .model import Target
from .tools import DriverStyle


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
