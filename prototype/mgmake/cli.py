from __future__ import annotations

import argparse
import json
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Sequence

from .core import VERSION_TEXT, BuildError, BuildLayout, BuildOptions, ProcessRunner, resolve_build_root, unique_preserving_order
from .engine import BuildEngine
from .graph import CommandGraphBuilder, write_dot_graph
from .model import BuildProject, Target
from .tools import TOOL_LABELS, ToolRegistry, ToolRole
from .traversal import required_tools_for


TASKS: tuple[tuple[str, str], ...] = (
    ("build", "Build the project."),
    ("help", "Show help."),
    ("clean", "Delete all build files."),
    ("tools", "Discover and cache required tools for the project."),
    ("fetch", "Fetch all project dependencies."),
    ("configure", "Configure build-system projects."),
    ("graph", "Export the complete build command graph as Graphviz DOT."),
    ("version", "Display the version of MGMake."),
)


def parse_arguments(argv: Sequence[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(add_help=False, allow_abbrev=False)
    parser.add_argument("task", nargs="?", default="help")
    parser.add_argument("-h", "--help", action="store_true")
    parser.add_argument("--version", action="store_true")
    parser.add_argument("-v", "--verbose", action="store_true")
    parser.add_argument("-s", "--short", action="store_true")
    parser.add_argument("-d", "--dry-run", action="store_true")
    parser.add_argument("-b", "--build-dir")
    parser.add_argument("-t", "--targets", nargs="+", action="append", default=[])
    parser.add_argument("--toolchain")
    parser.add_argument("--config", default="Release")
    parser.add_argument("--jobs", type=int, default=None)
    parser.add_argument("--std", default="c++26")

    for role in ToolRole:
        parser.add_argument(f"--{role.value}", dest=f"tool_{role.value.replace('-', '_')}")

    return parser.parse_args(argv)


def selected_targets(project: BuildProject, arguments: argparse.Namespace) -> tuple[Target, ...]:
    names = [name for group in arguments.targets for name in group]
    if not names:
        return (project.default_target,)
    return unique_preserving_order(project.find_target(name) for name in names)


def tool_overrides(arguments: argparse.Namespace) -> dict[ToolRole, str]:
    result: dict[ToolRole, str] = {}
    for role in ToolRole:
        value = getattr(arguments, f"tool_{role.value.replace('-', '_')}")
        if value:
            result[role] = value
    return result


def print_help(project: BuildProject) -> None:
    program = str(Path(sys.argv[0]).resolve())
    print("Usage:")
    print(f"        {program} [task] [options]")
    print("\nTasks:")
    width = max(len(name) for name, _ in TASKS)
    for name, description in TASKS:
        print(f"        {name:<{width}}  {description}")

    options: list[tuple[str, str]] = [
        ("-h, --help", "Show help."),
        ("--version", "Display the version of MGMake."),
        ("-v, --verbose", "Print commands before executing them."),
        ("-s, --short", "Keep command output short."),
        ("-d, --dry-run", "Print commands without executing them."),
        ("-b, --build-dir=<path>", "Set the build directory."),
        ("-t, --targets=<strings...>", "Build specific targets. May be passed multiple times."),
        ("--toolchain=<toolchain>", "Select a declared toolchain."),
        ("--config=<name>", "Set the build configuration (default: Release)."),
        ("--jobs=<count>", "Set the parallel CMake build job count."),
        ("--std=<standard>", "Set the C++ language standard (default: c++26)."),
    ]
    options.extend(
        (f"--{role.value}=<path>", f"Override the {TOOL_LABELS[role]} tool.")
        for role in ToolRole
    )
    print("\nOptions:")
    option_width = max(len(name) for name, _ in options)
    for name, description in options:
        print(f"        {name:<{option_width}}  {description}")

    print("\nTargets:")
    for target in project.targets:
        default = " (default)" if target is project.default_target else ""
        print(f"        {target.name:<20} {type(target).__name__}{default}")

    print("\nToolchains:")
    toolchain_width = max(len(toolchain.name) for toolchain in project.toolchains)
    for toolchain in project.toolchains:
        default = " (default)" if toolchain.name == project.default_toolchain else ""
        print(
            f"        {toolchain.name:<{toolchain_width}}  "
            f"{toolchain.description}{default}"
        )


def run_task(
    project: BuildProject,
    project_root: Path,
    arguments: argparse.Namespace,
) -> int:
    task = arguments.task.casefold()
    valid_tasks = {name for name, _ in TASKS}
    if arguments.help:
        task = "help"
    if arguments.version:
        task = "version"
    if task not in valid_tasks:
        raise BuildError(
            f"unknown task '{arguments.task}'; available tasks: "
            + ", ".join(name for name, _ in TASKS)
        )

    if task == "help":
        print_help(project)
        return 0
    if task == "version":
        print(VERSION_TEXT)
        return 0

    layout = BuildLayout(project_root, resolve_build_root(arguments.build_dir, project_root))
    if task == "clean":
        if layout.build_root.exists():
            print(f"Removing {layout.build_root}")
            if not arguments.dry_run:
                shutil.rmtree(layout.build_root)
        else:
            print(f"Build directory does not exist: {layout.build_root}")
        return 0

    targets = selected_targets(project, arguments)
    toolchain = project.find_toolchain(arguments.toolchain)
    options = BuildOptions(
        config=arguments.config,
        jobs=arguments.jobs,
        cxx_standard=arguments.std,
        verbose=arguments.verbose,
        short=arguments.short,
        dry_run=arguments.dry_run,
    )
    runner = ProcessRunner(options)
    registry = ToolRegistry(
        layout,
        toolchain,
        runner,
        tool_overrides(arguments),
    )
    requirements = required_tools_for(targets, task, toolchain)
    registry.discover(requirements)

    if task == "tools":
        registry.print_tools()
        return 0

    engine = BuildEngine(layout, registry, runner, options, toolchain)
    if task == "fetch":
        engine.fetch(targets)
        print("Fetch complete.")
        return 0
    if task == "configure":
        engine.configure(targets)
        print("Configure complete.")
        return 0
    if task == "graph":
        nodes = CommandGraphBuilder(layout, registry, options, toolchain).build(targets)
        write_dot_graph(layout.graph_file, nodes)
        print(f"Wrote Graphviz build graph: {layout.graph_file}")
        return 0
    if task == "build":
        results = engine.build_many(targets)
        artifacts = [artifact for result in results for artifact in result.artifacts]
        if artifacts:
            print("Dry run complete; planned artifacts:" if arguments.dry_run else "Build complete:")
            for artifact in artifacts:
                print(f"  {artifact}")
        else:
            print("Dry run complete." if arguments.dry_run else "Selected targets are ready.")
        return 0

    raise BuildError(f"unhandled task: {task}")


def main(project: BuildProject, project_root: Path, argv: Sequence[str] | None = None) -> int:
    try:
        arguments = parse_arguments(argv)
        return run_task(project, project_root, arguments)
    except (BuildError, subprocess.CalledProcessError, OSError, json.JSONDecodeError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
