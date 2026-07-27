from __future__ import annotations

from typing import Sequence

from .adapters import CMakeAdapter, NativeCxxAdapter
from .core import BuildError, BuildLayout, BuildOptions, ProcessRunner
from .model import CMakeLibrary, CxxExecutable, CxxStaticLibrary, HeaderOnlyLibrary, Target
from .sources import SourceManager
from .tools import ToolRegistry, ToolchainSpec
from .traversal import topological_targets
from .usage import BuildResult


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
