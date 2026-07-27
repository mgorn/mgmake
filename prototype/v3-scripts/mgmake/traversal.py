from __future__ import annotations

from pathlib import Path
from typing import Sequence

from .core import BuildError, BuildLayout, unique_preserving_order
from .model import CMakeLibrary, CxxExecutable, CxxStaticLibrary, HeaderOnlyLibrary, SourceSpec, Target
from .tools import ArchiverStyle, ToolRequirements, ToolRole, ToolchainSpec


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
