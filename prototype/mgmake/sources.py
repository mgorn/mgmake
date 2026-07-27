from __future__ import annotations

from pathlib import Path
from typing import Sequence

from .core import BuildError, BuildLayout, ProcessRunner
from .model import GitFetch, LocalSource, SourceSpec, Target
from .tools import ToolRegistry, ToolRole
from .traversal import source_for_target, topological_targets


class SourceManager:
    def __init__(
        self,
        layout: BuildLayout,
        tools: ToolRegistry,
        runner: ProcessRunner,
    ) -> None:
        self.layout = layout
        self.tools = tools
        self.runner = runner
        self._materialized: dict[tuple[int, str], Path] = {}

    def source_path(self, owner_name: str, source: SourceSpec) -> Path:
        if isinstance(source, LocalSource):
            return source.path.resolve()
        if isinstance(source, GitFetch):
            return self.layout.fetched_source(owner_name)
        raise BuildError(f"unsupported source specification: {type(source).__name__}")

    def materialize(self, owner_name: str, source: SourceSpec) -> Path:
        key = (id(source), owner_name)
        if key in self._materialized:
            return self._materialized[key]

        result = self.source_path(owner_name, source)
        if isinstance(source, LocalSource):
            if not result.is_dir() and not self.runner.options.dry_run:
                raise BuildError(f"local source directory does not exist: {result}")
        elif isinstance(source, GitFetch):
            self._fetch_git(source, result)

        self._materialized[key] = result
        return result

    def fetch_targets(self, targets: Sequence[Target]) -> None:
        seen: set[tuple[int, str]] = set()
        for target in topological_targets(targets):
            owner_name, source = source_for_target(target)
            key = (id(source), owner_name)
            if key in seen:
                continue
            seen.add(key)
            self.materialize(owner_name, source)

    def git_commands(self, source: GitFetch, destination: Path) -> tuple[list[str], ...]:
        git = self.tools.require(ToolRole.GIT)
        if not destination.exists():
            command = git.with_arguments("clone")
            if source.shallow:
                command.extend(["--depth", "1"])
            if source.ref is not None:
                command.extend(["--branch", source.ref, "--single-branch"])
            command.extend([source.url, str(destination)])
            return (command,)

        remote_ref = source.ref if source.ref is not None else "HEAD"
        fetch = git.with_arguments("-C", destination, "fetch")
        if source.shallow:
            fetch.extend(["--depth", "1"])
        fetch.extend(["origin", remote_ref])
        checkout = git.with_arguments("-C", destination, "checkout", "--detach", "FETCH_HEAD")
        return (fetch, checkout)

    def _fetch_git(self, source: GitFetch, destination: Path) -> None:
        destination.parent.mkdir(parents=True, exist_ok=True)

        if destination.exists() and not (destination / ".git").is_dir():
            raise BuildError(
                f"fetch destination exists but is not a Git repository: {destination}"
            )

        if destination.exists() and not self.runner.options.dry_run:
            git = self.tools.require(ToolRole.GIT)
            status = self.runner.capture(
                git.with_arguments("-C", destination, "status", "--porcelain")
            )
            if status:
                raise BuildError(
                    f"refusing to update modified dependency repository: {destination}\n{status}"
                )

        print(f"Fetching source '{destination.name}'...")
        for command in self.git_commands(source, destination):
            self.runner.run(command)
