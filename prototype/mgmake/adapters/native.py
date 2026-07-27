from __future__ import annotations

import shutil
from pathlib import Path
from typing import Sequence

from ..core import BuildError, BuildLayout, BuildOptions, ProcessRunner, sanitize_name, unique_preserving_order
from ..model import CxxExecutable, CxxStaticLibrary, HeaderOnlyLibrary
from ..sources import SourceManager
from ..tools import ArchiverStyle, DriverStyle, ToolRegistry, ToolRole, ToolchainSpec
from ..usage import BuildResult, IncludeDirectory, UsageRequirements, configuration_compile_arguments, include_arguments, print_usage, save_usage


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
                "/link",
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
