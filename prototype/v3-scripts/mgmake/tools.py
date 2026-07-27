from __future__ import annotations

import json
import os
import platform
import re
import shutil
import subprocess
from dataclasses import dataclass
from enum import Enum
from pathlib import Path
from typing import Mapping, Sequence, TYPE_CHECKING

from .core import BuildError, TOOL_CACHE_SCHEMA, display_command

if TYPE_CHECKING:
    from .core import BuildLayout, ProcessRunner


class ToolRole(Enum):
    CC = "cc"
    CXX = "cxx"
    OBJC = "objcc"
    OBJCXX = "objcxx"
    ASM = "asm"
    CUDA = "cuda"
    HIP = "hip"
    RC = "rc"
    MIDL = "midl"
    AR = "ar"
    RANLIB = "ranlib"
    LIB = "lib"
    LINKER = "linker"
    SHARED_LINKER = "shared-linker"
    DLLTOOL = "dlltool"
    MT = "mt"
    STRIP = "strip"
    OBJCOPY = "objcopy"
    OBJDUMP = "objdump"
    NM = "nm"
    READELF = "readelf"
    DEBUG_SYMBOL_TOOL = "debug-symbol-tool"
    LIPO = "lipo"
    INSTALL_NAME_TOOL = "install-name-tool"
    CODESIGN = "codesign"
    NINJA = "ninja"
    MAKE = "make"
    MSBUILD = "msbuild"
    XCODEBUILD = "xcodebuild"
    CMAKE = "cmake"
    PKG_CONFIG = "pkg-config"
    GIT = "git"
    CURL = "curl"
    WGET = "wget"
    UNZIP = "unzip"
    TAR = "tar"
    EXE_WRAPPER = "exe-wrapper"
    EMULATOR = "emulator"


TOOL_LABELS: dict[ToolRole, str] = {
    ToolRole.CC: "C Compiler",
    ToolRole.CXX: "C++ Compiler",
    ToolRole.OBJC: "Objective-C Compiler",
    ToolRole.OBJCXX: "Objective-C++ Compiler",
    ToolRole.ASM: "Assembler",
    ToolRole.CUDA: "CUDA Compiler",
    ToolRole.HIP: "HIP Compiler",
    ToolRole.RC: "Resource Compiler",
    ToolRole.MIDL: "MIDL Compiler",
    ToolRole.AR: "Archiver",
    ToolRole.RANLIB: "Ranlib",
    ToolRole.LIB: "Librarian",
    ToolRole.LINKER: "Linker",
    ToolRole.SHARED_LINKER: "Shared Linker",
    ToolRole.DLLTOOL: "DLL Tool",
    ToolRole.MT: "Manifest Tool",
    ToolRole.STRIP: "Strip",
    ToolRole.OBJCOPY: "Objcopy",
    ToolRole.OBJDUMP: "Objdump",
    ToolRole.NM: "NM",
    ToolRole.READELF: "Readelf",
    ToolRole.DEBUG_SYMBOL_TOOL: "Debug Symbol Tool",
    ToolRole.LIPO: "Lipo",
    ToolRole.INSTALL_NAME_TOOL: "Install Name Tool",
    ToolRole.CODESIGN: "Codesign",
    ToolRole.NINJA: "Ninja",
    ToolRole.MAKE: "Make",
    ToolRole.MSBUILD: "MSBuild",
    ToolRole.XCODEBUILD: "Xcodebuild",
    ToolRole.CMAKE: "CMake",
    ToolRole.PKG_CONFIG: "pkg-config",
    ToolRole.GIT: "Git",
    ToolRole.CURL: "Curl",
    ToolRole.WGET: "Wget",
    ToolRole.UNZIP: "Unzip",
    ToolRole.TAR: "Tar",
    ToolRole.EXE_WRAPPER: "Executable Wrapper",
    ToolRole.EMULATOR: "Emulator",
}


DEFAULT_TOOL_CANDIDATES: dict[ToolRole, tuple[str, ...]] = {
    ToolRole.CC: ("clang", "gcc", "cl"),
    ToolRole.CXX: ("clang++", "g++", "clang-cl", "cl"),
    ToolRole.OBJC: ("clang",),
    ToolRole.OBJCXX: ("clang++",),
    ToolRole.ASM: ("clang", "gcc", "as"),
    ToolRole.RC: ("llvm-rc", "rc"),
    ToolRole.MIDL: ("midl",),
    ToolRole.AR: ("llvm-ar", "ar"),
    ToolRole.RANLIB: ("llvm-ranlib", "ranlib"),
    ToolRole.LIB: ("llvm-lib", "lib"),
    ToolRole.LINKER: ("clang++", "g++", "clang-cl", "cl"),
    ToolRole.SHARED_LINKER: ("clang++", "g++", "clang-cl", "cl"),
    ToolRole.DLLTOOL: ("llvm-dlltool", "dlltool"),
    ToolRole.MT: ("llvm-mt", "mt"),
    ToolRole.STRIP: ("llvm-strip", "strip"),
    ToolRole.OBJCOPY: ("llvm-objcopy", "objcopy"),
    ToolRole.OBJDUMP: ("llvm-objdump", "objdump"),
    ToolRole.NM: ("llvm-nm", "nm"),
    ToolRole.READELF: ("llvm-readelf", "readelf"),
    ToolRole.DEBUG_SYMBOL_TOOL: ("dsymutil", "llvm-pdbutil"),
    ToolRole.LIPO: ("llvm-lipo", "lipo"),
    ToolRole.INSTALL_NAME_TOOL: ("install_name_tool",),
    ToolRole.CODESIGN: ("codesign",),
    ToolRole.NINJA: ("ninja", "ninja-build"),
    ToolRole.MAKE: ("make", "gmake", "mingw32-make"),
    ToolRole.MSBUILD: ("MSBuild.exe", "msbuild"),
    ToolRole.XCODEBUILD: ("xcodebuild",),
    ToolRole.CMAKE: ("cmake",),
    ToolRole.PKG_CONFIG: ("pkg-config",),
    ToolRole.GIT: ("git",),
    ToolRole.CURL: ("curl",),
    ToolRole.WGET: ("wget",),
    ToolRole.UNZIP: ("unzip",),
    ToolRole.TAR: ("tar",),
}


class DriverStyle(Enum):
    GNU = "gnu"
    MSVC = "msvc"


class ArchiverStyle(Enum):
    AR = "ar"
    LIB = "lib"


@dataclass(frozen=True)
class ToolchainSpec:
    name: str
    description: str
    candidates: Mapping[ToolRole, tuple[str, ...]]
    driver_style: DriverStyle = DriverStyle.GNU
    archiver_style: ArchiverStyle = ArchiverStyle.AR
    compile_options: tuple[str, ...] = ()
    link_options: tuple[str, ...] = ()
    cmake_options: tuple[str, ...] = ()
    preferred_generators: tuple[ToolRole, ...] = (ToolRole.NINJA, ToolRole.MAKE)

    def candidates_for(self, role: ToolRole) -> tuple[str, ...]:
        return self.candidates.get(role, ())


@dataclass(frozen=True)
class Tool:
    role: ToolRole
    command: tuple[str, ...]
    source: str

    def with_arguments(self, *arguments: str | Path) -> list[str]:
        return [*self.command, *(str(argument) for argument in arguments)]

    def as_path(self) -> str:
        if len(self.command) != 1:
            raise BuildError(
                f"{TOOL_LABELS[self.role]} cannot be used as a compiler path: "
                f"{display_command(self.command)}"
            )
        return self.command[0]


@dataclass(frozen=True)
class ToolRequirements:
    required: frozenset[ToolRole] = frozenset()
    optional: frozenset[ToolRole] = frozenset()

    @staticmethod
    def merge(*values: ToolRequirements) -> ToolRequirements:
        required = set(role for value in values for role in value.required)
        optional = set(role for value in values for role in value.optional)
        optional.difference_update(required)
        return ToolRequirements(frozenset(required), frozenset(optional))

class ToolRegistry:
    def __init__(
        self,
        layout: BuildLayout,
        toolchain: ToolchainSpec,
        runner: ProcessRunner,
        overrides: Mapping[ToolRole, str],
    ) -> None:
        self.layout = layout
        self.toolchain = toolchain
        self.runner = runner
        self.overrides = dict(overrides)
        self._tools: dict[ToolRole, Tool] = {}

    def discover(self, requirements: ToolRequirements, *, allow_cache: bool = True) -> None:
        cached = self._load_cache() if allow_cache else {}
        all_roles = sorted(
            requirements.required | requirements.optional,
            key=lambda role: role.value,
        )

        for role in all_roles:
            required = role in requirements.required
            override = self.overrides.get(role)
            if override is not None:
                tool = self._resolve_tool(role, (override,), "command-line override", required=True)
            elif role in cached:
                tool = cached[role]
            else:
                toolchain_candidates = self.toolchain.candidates_for(role)
                candidates = (
                    toolchain_candidates
                    if toolchain_candidates
                    else DEFAULT_TOOL_CANDIDATES.get(role, ())
                )
                tool = self._resolve_tool(role, candidates, "discovered", required=required)

            if tool is not None:
                self._tools[role] = tool

        if ToolRole.CMAKE in self._tools and not self.runner.options.dry_run:
            self._check_cmake_version(self.require(ToolRole.CMAKE))

        if not self.runner.options.dry_run:
            self._save_cache()

    def require(self, role: ToolRole) -> Tool:
        try:
            return self._tools[role]
        except KeyError as error:
            raise BuildError(f"required tool was not discovered: {TOOL_LABELS[role]}") from error

    def optional(self, role: ToolRole) -> Tool | None:
        return self._tools.get(role)

    def print_tools(self) -> None:
        print(f"Toolchain: {self.toolchain.name}")
        if not self._tools:
            print("  <no tools required>")
            return
        for role in sorted(self._tools, key=lambda item: item.value):
            tool = self._tools[role]
            print(
                f"  {TOOL_LABELS[role]:24} "
                f"{display_command(tool.command)} [{tool.source}]"
            )

    def _resolve_tool(
        self,
        role: ToolRole,
        candidates: Sequence[str],
        source: str,
        *,
        required: bool,
    ) -> Tool | None:
        for candidate in candidates:
            command = self._resolve_candidate(candidate)
            if command is not None:
                return Tool(role=role, command=command, source=source)

        if self.runner.options.dry_run and candidates:
            return Tool(role=role, command=(candidates[0],), source="dry-run placeholder")
        if not required:
            return None

        rendered = ", ".join(repr(candidate) for candidate in candidates)
        raise BuildError(f"could not find {TOOL_LABELS[role]}; tried: {rendered}")

    def _resolve_candidate(self, candidate: str) -> tuple[str, ...] | None:
        expanded = os.path.expandvars(os.path.expanduser(candidate))

        if expanded.startswith("xcrun://"):
            _, remainder = expanded.split("xcrun://", 1)
            sdk, tool = remainder.split("/", 1)
            xcrun = shutil.which("xcrun")
            if xcrun is None:
                return None
            try:
                resolved = subprocess.run(
                    [xcrun, "--sdk", sdk, "--find", tool],
                    check=True,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    text=True,
                ).stdout.strip()
            except subprocess.CalledProcessError:
                return None
            return (resolved,)

        path = Path(expanded)
        has_directory = path.is_absolute() or path.parent != Path(".")
        if has_directory:
            if path.is_file():
                return (str(path.absolute()),)
            if os.name == "nt" and path.with_suffix(".exe").is_file():
                return (str(path.with_suffix(".exe").absolute()),)
            return None

        found = shutil.which(expanded)
        if found is None:
            return None
        # Preserve argv[0] as found. In particular, do not resolve clang++
        # symlinks to clang because driver behavior depends on its invoked name.
        return (str(Path(found).absolute()),)

    def _load_cache(self) -> dict[ToolRole, Tool]:
        path = self.layout.tool_cache
        if not path.is_file():
            return {}
        try:
            data = json.loads(path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError):
            return {}

        if data.get("schema") != TOOL_CACHE_SCHEMA:
            return {}
        if data.get("toolchain", "").casefold() != self.toolchain.name.casefold():
            return {}
        if data.get("host") != platform.platform():
            return {}

        result: dict[ToolRole, Tool] = {}
        for role_name, value in data.get("tools", {}).items():
            try:
                role = ToolRole(role_name)
                command = tuple(str(item) for item in value["command"])
            except (ValueError, KeyError, TypeError):
                continue
            if command and self._cached_command_is_valid(command):
                result[role] = Tool(role, command, "cache")
        return result

    @staticmethod
    def _cached_command_is_valid(command: tuple[str, ...]) -> bool:
        executable = command[0]
        path = Path(executable)
        if path.is_absolute() or path.parent != Path("."):
            return path.is_file()
        return shutil.which(executable) is not None

    def _save_cache(self) -> None:
        path = self.layout.tool_cache
        path.parent.mkdir(parents=True, exist_ok=True)
        data = {
            "schema": TOOL_CACHE_SCHEMA,
            "toolchain": self.toolchain.name,
            "host": platform.platform(),
            "tools": {
                role.value: {"command": list(tool.command)}
                for role, tool in sorted(self._tools.items(), key=lambda item: item[0].value)
            },
        }
        path.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")

    def _check_cmake_version(self, cmake: Tool) -> None:
        first_line = self.runner.capture(cmake.with_arguments("--version")).splitlines()[0]
        match = re.search(r"(\d+)\.(\d+)(?:\.(\d+))?", first_line)
        if match is None:
            raise BuildError(f"could not parse CMake version from: {first_line}")
        version = tuple(int(part or 0) for part in match.groups())
        minimum = (3, 16, 0)
        if version < minimum:
            raise BuildError(
                f"CMake {minimum[0]}.{minimum[1]} or newer is required; found {first_line}"
            )


