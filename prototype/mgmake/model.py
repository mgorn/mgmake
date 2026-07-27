from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
from typing import Mapping, Protocol, Sequence

from .core import BuildError
from .tools import ToolRequirements, ToolRole, ToolchainSpec


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
