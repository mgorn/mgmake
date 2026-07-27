from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Sequence

from .adapters import CMakeAdapter, NativeCxxAdapter
from .core import BuildError, BuildLayout, BuildOptions, ProcessRunner, display_command, sanitize_name, unique_preserving_order
from .model import CMakeLibrary, CxxExecutable, CxxStaticLibrary, GitFetch, HeaderOnlyLibrary, LocalSource, SourceSpec, Target
from .sources import SourceManager
from .tools import DriverStyle, ToolRegistry, ToolchainSpec
from .traversal import declared_package_prefixes, source_for_target
from .usage import IncludeDirectory, UsageRequirements, load_usage


@dataclass(frozen=True)
class ActionNode:
    identifier: str
    label: str
    command: str | None
    dependencies: tuple[str, ...] = ()


class CommandGraphBuilder:
    def __init__(
        self,
        layout: BuildLayout,
        tools: ToolRegistry,
        options: BuildOptions,
        toolchain: ToolchainSpec,
    ) -> None:
        self.layout = layout
        self.tools = tools
        self.options = options
        self.toolchain = toolchain
        self.runner = ProcessRunner(options)
        self.sources = SourceManager(layout, tools, self.runner)
        self.cmake = CMakeAdapter(
            layout,
            tools,
            self.sources,
            self.runner,
            options,
            toolchain,
        )
        self.native = NativeCxxAdapter(
            layout,
            tools,
            self.sources,
            self.runner,
            options,
            toolchain,
        )
        self.nodes: dict[str, ActionNode] = {}
        self.ready_nodes: dict[int, str] = {}
        self.source_nodes: dict[tuple[int, str], str] = {}
        self.project_nodes: dict[int, str] = {}

    def build(self, targets: Sequence[Target]) -> tuple[ActionNode, ...]:
        discovery_id = "tools"
        self.nodes[discovery_id] = ActionNode(
            discovery_id,
            f"Discover and cache tools ({self.toolchain.name})",
            None,
        )
        for target in targets:
            self._add_target(target, discovery_id)
        return tuple(self.nodes.values())

    def _add_source(self, owner_name: str, source: SourceSpec, tools_node: str) -> str:
        key = (id(source), owner_name)
        if key in self.source_nodes:
            return self.source_nodes[key]
        identifier = f"fetch:{sanitize_name(owner_name)}"
        if isinstance(source, LocalSource):
            node = ActionNode(identifier, f"Use local source: {source.path.resolve()}", None, (tools_node,))
        elif isinstance(source, GitFetch):
            destination = self.layout.fetched_source(owner_name)
            commands = self.sources.git_commands(source, destination)
            command = "\n".join(display_command(item) for item in commands)
            node = ActionNode(identifier, f"Fetch {owner_name}", command, (tools_node,))
        else:
            raise BuildError(f"unsupported source specification: {type(source).__name__}")
        self.nodes[identifier] = node
        self.source_nodes[key] = identifier
        return identifier

    def _add_target(self, target: Target, tools_node: str) -> str:
        key = id(target)
        if key in self.ready_nodes:
            return self.ready_nodes[key]
        dependency_nodes = tuple(self._add_target(dep, tools_node) for dep in target.dependencies)
        owner_name, source = source_for_target(target)
        source_node = self._add_source(owner_name, source, tools_node)

        if isinstance(target, CMakeLibrary):
            ready = self._add_cmake_target(target, source_node, dependency_nodes)
        elif isinstance(target, CxxStaticLibrary):
            ready = self._add_static_library(target, source_node, dependency_nodes)
        elif isinstance(target, HeaderOnlyLibrary):
            identifier = f"ready:{sanitize_name(target.name)}"
            self.nodes[identifier] = ActionNode(
                identifier,
                f"Header-only target {target.name} ready",
                None,
                (source_node, *dependency_nodes),
            )
            ready = identifier
        elif isinstance(target, CxxExecutable):
            ready = self._add_executable(target, source_node, dependency_nodes)
        else:
            raise BuildError(f"unsupported target type: {type(target).__name__}")

        self.ready_nodes[key] = ready
        return ready

    def _add_cmake_target(
        self,
        target: CMakeLibrary,
        source_node: str,
        dependency_nodes: tuple[str, ...],
    ) -> str:
        project_key = id(target.project)
        if project_key not in self.project_nodes:
            source_directory = self.sources.source_path(target.project.name, target.project.source)
            dependency_prefixes = unique_preserving_order(
                prefix
                for dependency in target.dependencies
                for prefix in declared_package_prefixes(self.layout, dependency)
            )
            configure_id = f"configure:{sanitize_name(target.project.name)}"
            configure_command = self.cmake.configure_command(
                target.project,
                source_directory,
                self.layout.cmake_build(target.project.name),
                self.layout.install_prefix(target.project.name),
                dependency_prefixes,
            )
            self.nodes[configure_id] = ActionNode(
                configure_id,
                f"Configure CMake project {target.project.name}",
                display_command(configure_command),
                (source_node, *dependency_nodes),
            )
            build_id = f"cmake-build:{sanitize_name(target.project.name)}"
            self.nodes[build_id] = ActionNode(
                build_id,
                f"Build/install CMake project {target.project.name}",
                display_command(self.cmake.build_command(target.project)),
                (configure_id,),
            )
            self.project_nodes[project_key] = build_id

        build_id = self.project_nodes[project_key]
        probe_id = f"probe:{sanitize_name(target.name)}"
        prefixes = (
            self.layout.install_prefix(target.project.name),
            *unique_preserving_order(
                prefix
                for dependency in target.dependencies
                for prefix in declared_package_prefixes(self.layout, dependency)
            ),
        )
        self.nodes[probe_id] = ActionNode(
            probe_id,
            f"Resolve CMake usage requirements for {target.imported_target}",
            display_command(self.cmake.probe_command(target, prefixes)),
            (build_id, *dependency_nodes),
        )
        return probe_id

    def _usage_for_dependencies(self, dependencies: Sequence[Target]) -> UsageRequirements:
        values: list[UsageRequirements] = []
        for dependency in dependencies:
            cached = load_usage(self.layout, dependency.name)
            if cached is not None:
                values.append(cached)
                continue
            if isinstance(dependency, CMakeLibrary):
                values.append(
                    UsageRequirements(
                        link_args=(f"<link-usage:{dependency.name}>",),
                        unresolved=(dependency.name,),
                    )
                )
            elif isinstance(dependency, CxxStaticLibrary):
                source_directory = self.sources.source_path(dependency.name, dependency.source)
                own = UsageRequirements(
                    includes=tuple(
                        IncludeDirectory((source_directory / path).resolve())
                        for path in dependency.public_include_directories
                    ),
                    defines=dependency.public_defines,
                    link_args=(
                        self.layout.static_library(
                            dependency.name,
                            self.toolchain,
                        ),
                    ),
                )
                values.append(UsageRequirements.merge(own, self._usage_for_dependencies(dependency.dependencies)))
            elif isinstance(dependency, HeaderOnlyLibrary):
                source_directory = self.sources.source_path(dependency.name, dependency.source)
                own = UsageRequirements(
                    includes=tuple(
                        IncludeDirectory((source_directory / path).resolve())
                        for path in dependency.public_include_directories
                    ),
                    defines=dependency.public_defines,
                )
                values.append(UsageRequirements.merge(own, self._usage_for_dependencies(dependency.dependencies)))
        return UsageRequirements.merge(*values)

    def _add_static_library(
        self,
        target: CxxStaticLibrary,
        source_node: str,
        dependency_nodes: tuple[str, ...],
    ) -> str:
        source_directory = self.sources.source_path(target.name, target.source)
        dependency_usage = self._usage_for_dependencies(target.dependencies)
        own_usage = UsageRequirements(
            includes=tuple(
                IncludeDirectory((source_directory / path).resolve())
                for path in (*target.public_include_directories, *target.private_include_directories)
            ),
            defines=(*target.public_defines, *target.private_defines),
        )
        compile_usage = UsageRequirements.merge(own_usage, dependency_usage)
        compile_nodes: list[str] = []
        suffix = ".obj" if self.toolchain.driver_style == DriverStyle.MSVC else ".o"
        for relative_source in target.sources:
            identifier = f"compile:{sanitize_name(target.name)}:{sanitize_name(relative_source)}"
            object_path = self.layout.object_directory(target.name) / f"{sanitize_name(relative_source)}{suffix}"
            command = self.native.compile_command(
                source_directory / relative_source,
                object_path,
                compile_usage,
            )
            self.nodes[identifier] = ActionNode(
                identifier,
                f"Compile {target.name}: {relative_source}",
                display_command(command),
                (source_node, *dependency_nodes),
            )
            compile_nodes.append(identifier)

        archive_id = f"archive:{sanitize_name(target.name)}"
        library = self.layout.static_library(target.name, self.toolchain)
        object_paths = [
            self.layout.object_directory(target.name) / f"{sanitize_name(source)}{suffix}"
            for source in target.sources
        ]
        self.nodes[archive_id] = ActionNode(
            archive_id,
            f"Archive static library {target.name}",
            display_command(self.native.archive_command(library, object_paths)),
            tuple(compile_nodes),
        )
        return archive_id

    def _add_executable(
        self,
        target: CxxExecutable,
        source_node: str,
        dependency_nodes: tuple[str, ...],
    ) -> str:
        source_directory = self.sources.source_path(target.name, target.source)
        dependency_usage = self._usage_for_dependencies(target.dependencies)
        own_usage = UsageRequirements(
            includes=tuple(
                IncludeDirectory((source_directory / path).resolve())
                for path in target.include_directories
            ),
            defines=target.defines,
        )
        compile_usage = UsageRequirements.merge(own_usage, dependency_usage)
        sources = [source_directory / item for item in target.sources]
        output = self.layout.executable(target.name, self.toolchain)
        identifier = f"link:{sanitize_name(target.name)}"
        command = self.native.executable_command(
            sources,
            output,
            compile_usage,
            dependency_usage,
        )
        cached_missing = list(dependency_usage.unresolved)
        label = f"Compile/link executable {target.name}"
        if cached_missing:
            label += "\n(usage flags finalized after CMake probes: " + ", ".join(cached_missing) + ")"
        self.nodes[identifier] = ActionNode(
            identifier,
            label,
            display_command(command),
            (source_node, *dependency_nodes),
        )
        return identifier


def write_dot_graph(path: Path, nodes: Sequence[ActionNode]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    lines = [
        "digraph mgmake_prototype {",
        "  rankdir=LR;",
        '  graph [fontname="Consolas"];',
        '  node [shape=box, fontname="Consolas"];',
        '  edge [fontname="Consolas"];',
    ]
    for node in nodes:
        label = node.label
        if node.command:
            label += "\n\n" + node.command
        lines.append(f'  "{dot_escape(node.identifier)}" [label="{dot_escape(label)}"];')
    for node in nodes:
        for dependency in node.dependencies:
            lines.append(
                f'  "{dot_escape(dependency)}" -> "{dot_escape(node.identifier)}";'
            )
    lines.append("}")
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def dot_escape(value: str) -> str:
    return (
        value.replace("\\", "\\\\")
        .replace('"', '\\"')
        .replace("\n", "\\n")
    )
