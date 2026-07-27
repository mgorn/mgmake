from __future__ import annotations

import os
from pathlib import Path
from typing import Sequence

from ..cmake_file_api import prepare_file_api_query, read_cmake_target_usage
from ..core import BuildLayout, BuildOptions, ProcessRunner, sanitize_name, unique_preserving_order
from ..model import CMakeLibrary, CMakeProject
from ..sources import SourceManager
from ..tools import ToolRegistry, ToolRole, ToolchainSpec
from ..traversal import declared_package_prefixes
from ..usage import BuildResult, UsageRequirements, load_usage, print_usage, save_usage


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
