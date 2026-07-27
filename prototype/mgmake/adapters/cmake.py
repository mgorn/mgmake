from __future__ import annotations

import os
import shutil
from pathlib import Path
from typing import Sequence

from ..cmake_file_api import prepare_file_api_query, read_cmake_target_usage
from ..core import BuildError, BuildLayout, BuildOptions, ProcessRunner, sanitize_name, unique_preserving_order
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
            self.ensure_compatible_cache(
                build_directory,
                project.languages,
                install_directory=install_directory,
            )
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
        self.ensure_compatible_cache(build_directory, ("CXX",))
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
        generator_arguments = self.generator_arguments(build_directory)
        command = cmake.with_arguments(
            "-S",
            source_directory,
            "-B",
            build_directory,
            *generator_arguments,
            f"-DCMAKE_BUILD_TYPE:STRING={self.options.config}",
            f"-DCMAKE_INSTALL_PREFIX:PATH={install_directory}",
            *self.toolchain.cmake_options,
        )

        languages = {language.upper() for language in project.languages}
        using_visual_studio_generator = any(
            argument.startswith("Visual Studio ") for argument in generator_arguments
        )
        if not using_visual_studio_generator:
            # CMake projects may enable additional languages after their initial
            # project() declaration. SDL, for example, starts as a C project but
            # conditionally enables CXX later. Always pin the C++ compiler used
            # by our consumer probes, and pin C whenever the project declares it,
            # so CMake cannot mix the selected toolchain with a compiler found
            # incidentally on PATH.
            command.append(
                f"-DCMAKE_CXX_COMPILER:FILEPATH={self.tools.require(ToolRole.CXX).as_path()}"
            )
            if "C" in languages:
                command.append(
                    f"-DCMAKE_C_COMPILER:FILEPATH={self.tools.require(ToolRole.CC).as_path()}"
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
        generator_arguments = self.generator_arguments(build_directory)
        command = self.tools.require(ToolRole.CMAKE).with_arguments(
            "-S",
            self.layout.probe_source(target.name),
            "-B",
            build_directory,
            *generator_arguments,
            f"-DCMAKE_BUILD_TYPE:STRING={self.options.config}",
        )
        if not any(argument.startswith("Visual Studio ") for argument in generator_arguments):
            command.append(
                f"-DCMAKE_CXX_COMPILER:FILEPATH={self.tools.require(ToolRole.CXX).as_path()}"
            )
        command.extend(
            (
                f"-DCMAKE_PREFIX_PATH:STRING={prefix_path}",
                *self.toolchain.cmake_options,
            )
        )
        return command

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
            if role == ToolRole.MSBUILD:
                generator = self.visual_studio_generator()
                architecture = self.visual_studio_cmake_architecture()
                arguments = ["-G", generator, "-A", architecture]
                if self.toolchain.cmake_generator_toolset:
                    arguments.extend(["-T", self.toolchain.cmake_generator_toolset])
                return arguments
        return []

    def visual_studio_generator(self) -> str:
        version = self.runner.environment_value("VisualStudioVersion") or ""
        major = version.split(".", 1)[0]
        generators = {
            "18": "Visual Studio 18 2026",
            "17": "Visual Studio 17 2022",
            "16": "Visual Studio 16 2019",
            "15": "Visual Studio 15 2017",
        }
        try:
            return generators[major]
        except KeyError as error:
            raise BuildError(
                f"unsupported or unknown Visual Studio version: {version or '<unset>'}"
            ) from error

    def visual_studio_cmake_architecture(self) -> str:
        architecture = (self.toolchain.target_architecture or "x64").casefold()
        values = {
            "x86": "Win32",
            "win32": "Win32",
            "x64": "x64",
            "amd64": "x64",
            "arm": "ARM",
            "arm64": "ARM64",
        }
        try:
            return values[architecture]
        except KeyError as error:
            raise BuildError(
                f"unsupported Visual Studio target architecture: {architecture}"
            ) from error

    def ensure_compatible_cache(
        self,
        build_directory: Path,
        languages: Sequence[str],
        *,
        install_directory: Path | None = None,
    ) -> None:
        cache_path = build_directory / "CMakeCache.txt"
        if not cache_path.is_file():
            return

        cache = self.read_cmake_cache(cache_path)
        generator = cache.get("CMAKE_GENERATOR", "")
        if generator.startswith("Visual Studio "):
            return

        expected: dict[str, str] = {
            "CMAKE_CXX_COMPILER": self.tools.require(ToolRole.CXX).as_path(),
        }
        if "C" in {language.upper() for language in languages}:
            expected["CMAKE_C_COMPILER"] = self.tools.require(ToolRole.CC).as_path()

        mismatches: list[str] = []
        for name, expected_path in expected.items():
            cached_path = cache.get(name)
            if cached_path is None:
                continue
            if self.normalized_path(cached_path) != self.normalized_path(expected_path):
                mismatches.append(f"{name}: {cached_path} -> {expected_path}")

        if not mismatches:
            return

        print(
            f"Discarding incompatible CMake cache: {build_directory}\n"
            + "\n".join(f"  {item}" for item in mismatches)
        )
        shutil.rmtree(build_directory)
        if install_directory is not None and install_directory.exists():
            shutil.rmtree(install_directory)

    @staticmethod
    def read_cmake_cache(path: Path) -> dict[str, str]:
        values: dict[str, str] = {}
        for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
            if not line or line.startswith(("#", "//")) or "=" not in line:
                continue
            declaration, value = line.split("=", 1)
            name = declaration.split(":", 1)[0]
            values[name] = value
        return values

    @staticmethod
    def normalized_path(value: str) -> str:
        return os.path.normcase(os.path.normpath(value.replace("/", os.sep)))

    @staticmethod
    def cmake_value(value: object) -> str:
        if isinstance(value, bool):
            return "ON" if value else "OFF"
        return str(value)
