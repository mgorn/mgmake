from __future__ import annotations

import os
import re
import shlex
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, Sequence, TYPE_CHECKING, TypeVar

if TYPE_CHECKING:
    from .tools import ToolchainSpec

VERSION_TEXT = "MGMake prototype script 0.2"
TOOL_CACHE_SCHEMA = 1
USAGE_CACHE_SCHEMA = 1


class BuildError(RuntimeError):
    pass


@dataclass(frozen=True)
class BuildLayout:
    project_root: Path
    build_root: Path

    @property
    def fetch_root(self) -> Path:
        return self.build_root / "fetch"

    @property
    def project_build_root(self) -> Path:
        return self.build_root / "projects"

    @property
    def install_root(self) -> Path:
        return self.build_root / "ext"

    @property
    def object_root(self) -> Path:
        return self.build_root / "objects"

    @property
    def artifact_root(self) -> Path:
        return self.build_root / "out"

    @property
    def probe_root(self) -> Path:
        return self.build_root / "probes"

    @property
    def state_root(self) -> Path:
        return self.build_root / "state"

    @property
    def tool_cache(self) -> Path:
        return self.state_root / "tools.json"

    @property
    def graph_file(self) -> Path:
        return self.build_root / "graph.dot"

    def fetched_source(self, owner_name: str) -> Path:
        return self.fetch_root / owner_name

    def cmake_build(self, project_name: str) -> Path:
        return self.project_build_root / "cmake" / project_name

    def install_prefix(self, project_name: str) -> Path:
        return self.install_root / project_name

    def object_directory(self, target_name: str) -> Path:
        return self.object_root / target_name

    def static_library(self, target_name: str, toolchain: ToolchainSpec) -> Path:
        cross_unix_archive = toolchain.name in {"Android", "Emscripten", "iOS"}
        if toolchain.archiver_style.value == "lib" or (os.name == "nt" and not cross_unix_archive):
            return self.artifact_root / target_name / f"{target_name}.lib"
        return self.artifact_root / target_name / f"lib{target_name}.a"

    def executable(self, target_name: str, toolchain: ToolchainSpec) -> Path:
        if toolchain.name == "Emscripten":
            suffix = ".html"
        elif toolchain.name in {"Android", "iOS"}:
            suffix = ""
        else:
            suffix = ".exe" if os.name == "nt" else ""
        return self.artifact_root / target_name / f"{target_name}{suffix}"

    def probe_source(self, target_name: str) -> Path:
        return self.probe_root / target_name / "source"

    def probe_build(self, target_name: str) -> Path:
        return self.probe_root / target_name / "build"

    def usage_cache(self, target_name: str) -> Path:
        return self.state_root / "usage" / f"{sanitize_name(target_name)}.json"


@dataclass(frozen=True)
class BuildOptions:
    config: str
    jobs: int | None
    cxx_standard: str
    verbose: bool
    short: bool
    dry_run: bool


class ProcessRunner:
    def __init__(self, options: BuildOptions) -> None:
        self.options = options

    def run(self, args: Sequence[str | Path], *, cwd: Path | None = None) -> None:
        command = [str(argument) for argument in args]
        if self.options.verbose or self.options.dry_run:
            self._print_command(command, cwd)
        if self.options.dry_run:
            return

        if self.options.short:
            result = subprocess.run(
                command,
                cwd=cwd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
            )
            if result.returncode != 0:
                if result.stdout:
                    print(result.stdout, end="", file=sys.stderr)
                if result.stderr:
                    print(result.stderr, end="", file=sys.stderr)
                raise subprocess.CalledProcessError(result.returncode, command)
            return

        subprocess.run(command, cwd=cwd, check=True)

    def capture(self, args: Sequence[str | Path], *, cwd: Path | None = None) -> str:
        command = [str(argument) for argument in args]
        if self.options.verbose or self.options.dry_run:
            self._print_command(command, cwd)
        if self.options.dry_run:
            return ""

        result = subprocess.run(
            command,
            cwd=cwd,
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        return result.stdout.strip()

    @staticmethod
    def _print_command(command: Sequence[str], cwd: Path | None) -> None:
        prefix = ""
        if cwd is not None:
            prefix = f"(cd {display_command((str(cwd),))}) "
        print(f"$ {prefix}{display_command(command)}", flush=True)


def display_command(args: Sequence[str]) -> str:
    if os.name == "nt":
        return subprocess.list2cmdline(list(args))
    return shlex.join(args)


T = TypeVar("T")


def unique_preserving_order(values: Iterable[T]) -> tuple[T, ...]:
    result: list[T] = []
    seen: set[T] = set()
    for value in values:
        if value not in seen:
            seen.add(value)
            result.append(value)
    return tuple(result)


def sanitize_name(value: str) -> str:
    sanitized = re.sub(r"[^A-Za-z0-9_.-]+", "_", value)
    return sanitized.strip("._-") or "target"



def resolve_build_root(value: str | None, project_root: Path) -> Path:
    if value is None:
        return project_root / ".build"
    path = Path(value).expanduser()
    if not path.is_absolute():
        path = project_root / path
    return path.resolve()
