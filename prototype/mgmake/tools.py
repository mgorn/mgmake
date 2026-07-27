from __future__ import annotations

import json
import os
import platform
import re
import shutil
import subprocess
import tempfile
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


class ToolchainBootstrap(Enum):
    NONE = "none"
    VISUAL_STUDIO = "visual-studio"


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
    cmake_generator_toolset: str | None = None
    bootstrap: ToolchainBootstrap = ToolchainBootstrap.NONE
    target_architecture: str | None = None
    host_architecture: str | None = None

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
        self._prepare_toolchain_environment()
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
            xcrun = shutil.which("xcrun", path=self.runner.environment_value("PATH"))
            if xcrun is None:
                return None
            try:
                resolved = subprocess.run(
                    [xcrun, "--sdk", sdk, "--find", tool],
                    check=True,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    text=True,
                    env=self.runner.environment,
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

        found = shutil.which(expanded, path=self.runner.environment_value("PATH"))
        if found is None:
            return None
        # Preserve argv[0] as found. In particular, do not resolve clang++
        # symlinks to clang because driver behavior depends on its invoked name.
        return (str(Path(found).absolute()),)

    def _prepare_toolchain_environment(self) -> None:
        if self.toolchain.bootstrap != ToolchainBootstrap.VISUAL_STUDIO:
            return
        if self.runner.options.dry_run:
            return
        if os.name != "nt":
            raise BuildError("Visual Studio-backed toolchains are only available on Windows")

        path_value = self.runner.environment_value("PATH")
        if (
            shutil.which("cl.exe", path=path_value) is not None
            and self.runner.environment_value("INCLUDE")
            and self.runner.environment_value("LIB")
        ):
            return

        original_environment = self.runner.environment
        bootstrap_environment = self._clean_visual_studio_environment(original_environment)
        vswhere = self._find_vswhere(bootstrap_environment)
        if vswhere is None:
            raise BuildError(
                "could not find vswhere.exe; install Visual Studio or the Visual Studio "
                "Build Tools with the Desktop development with C++ workload"
            )

        try:
            result = subprocess.run(
                [
                    vswhere,
                    "-latest",
                    "-prerelease",
                    "-products",
                    "*",
                    "-requires",
                    "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
                    "-property",
                    "installationPath",
                ],
                check=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                errors="replace",
                env=bootstrap_environment,
            )
            installation = result.stdout.strip()
        except subprocess.CalledProcessError as error:
            details = self._format_process_failure(
                "vswhere.exe failed while locating Visual C++",
                error.returncode,
                error.stdout,
                error.stderr,
            )
            raise BuildError(details) from error

        if not installation:
            raise BuildError(
                "no Visual Studio installation with the Visual C++ x64/x86 tools was found"
            )

        target_arch = self._normalize_windows_architecture(
            self.toolchain.target_architecture or "x64"
        )
        host_arch = self._normalize_windows_architecture(
            self.toolchain.host_architecture or "x64"
        )
        installation_path = Path(installation)
        attempts: list[tuple[Path, tuple[str, ...], str]] = []

        vsdevcmd = installation_path / "Common7" / "Tools" / "VsDevCmd.bat"
        if vsdevcmd.is_file():
            attempts.append(
                (
                    vsdevcmd,
                    ("-no_logo", f"-arch={target_arch}", f"-host_arch={host_arch}"),
                    "VsDevCmd.bat",
                )
            )

        vcvarsall = installation_path / "VC" / "Auxiliary" / "Build" / "vcvarsall.bat"
        vcvars_arch = self._vcvarsall_architecture(host_arch, target_arch)
        if vcvarsall.is_file() and vcvars_arch is not None:
            attempts.append((vcvarsall, (vcvars_arch,), "vcvarsall.bat"))

        if not attempts:
            raise BuildError(
                "the selected Visual Studio installation contains neither VsDevCmd.bat "
                "nor a compatible vcvarsall.bat"
            )

        failures: list[str] = []
        for batch_file, arguments, label in attempts:
            discovered, failure = self._capture_batch_environment(
                batch_file,
                arguments,
                bootstrap_environment,
            )
            if discovered is None:
                failures.append(f"{label}:\n{failure}")
                continue

            candidate_environment = dict(original_environment)
            self._merge_environment(candidate_environment, discovered)
            candidate_path = self._environment_value(candidate_environment, "PATH")
            missing: list[str] = []
            if shutil.which("cl.exe", path=candidate_path) is None:
                missing.append("cl.exe")
            if not self._environment_value(candidate_environment, "INCLUDE"):
                missing.append("INCLUDE")
            if not self._environment_value(candidate_environment, "LIB"):
                missing.append("LIB")

            if missing:
                failures.append(
                    f"{label} completed but did not provide: {', '.join(missing)}"
                )
                continue

            self.runner.update_environment(discovered)
            return

        joined = "\n\n".join(failures)
        raise BuildError(
            f"failed to initialize the Visual Studio {target_arch} developer environment.\n\n"
            f"{joined}"
        )

    @classmethod
    def _clean_visual_studio_environment(
        cls,
        environment: Mapping[str, str],
    ) -> dict[str, str]:
        result = dict(environment)
        exact_names = {
            "devengdir",
            "extension_sdk_dir",
            "framework40version",
            "frameworkdir",
            "frameworkdir32",
            "frameworkversion",
            "frameworkversion32",
            "include",
            "lib",
            "libpath",
            "netfxsdkdir",
            "platform",
            "universalcrtsdkdir",
            "vcideinstallDir".casefold(),
            "vcinstallDir".casefold(),
            "vctoolsinstallDir".casefold(),
            "vctoolsredistDir".casefold(),
            "visualstudioedition",
            "visualstudioversion",
            "vsinstallDir".casefold(),
            "windowlibpath",
            "windowssdkbinpath",
            "windowssdkdir",
            "windowssdkverbinpath",
            "windowssdkversion",
        }
        prefixes = ("vscmd_", "__vscmd_", "vscomntools", "vsappidrdir")
        for key in tuple(result):
            folded = key.casefold()
            if folded in exact_names or folded.startswith(prefixes):
                del result[key]
        return result

    @staticmethod
    def _merge_environment(destination: dict[str, str], values: Mapping[str, str]) -> None:
        existing = {key.casefold(): key for key in destination}
        for key, value in values.items():
            previous = existing.get(key.casefold())
            if previous is not None and previous != key:
                del destination[previous]
            destination[key] = value
            existing[key.casefold()] = key

    @staticmethod
    def _normalize_windows_architecture(value: str) -> str:
        normalized = value.strip().casefold().replace("-", "_")
        aliases = {
            "amd64": "x64",
            "x86_64": "x64",
            "x64": "x64",
            "i386": "x86",
            "i486": "x86",
            "i586": "x86",
            "i686": "x86",
            "win32": "x86",
            "x86": "x86",
            "aarch64": "arm64",
            "arm64": "arm64",
            "arm": "arm",
        }
        try:
            return aliases[normalized]
        except KeyError as error:
            choices = ", ".join(sorted(set(aliases.values())))
            raise BuildError(
                f"unsupported Visual Studio architecture '{value}'; expected one of: {choices}"
            ) from error

    @staticmethod
    def _vcvarsall_architecture(host_arch: str, target_arch: str) -> str | None:
        architectures = {
            ("x86", "x86"): "x86",
            ("x86", "x64"): "x86_amd64",
            ("x86", "arm"): "x86_arm",
            ("x86", "arm64"): "x86_arm64",
            ("x64", "x86"): "amd64_x86",
            ("x64", "x64"): "amd64",
            ("x64", "arm"): "amd64_arm",
            ("x64", "arm64"): "amd64_arm64",
            ("arm64", "arm64"): "arm64",
            ("arm64", "x64"): "arm64_x64",
            ("arm64", "x86"): "arm64_x86",
        }
        return architectures.get((host_arch, target_arch))

    @classmethod
    def _capture_batch_environment(
        cls,
        batch_file: Path,
        arguments: Sequence[str],
        environment: Mapping[str, str],
    ) -> tuple[dict[str, str] | None, str | None]:
        marker_begin = "__MGMK_ENVIRONMENT_BEGIN__"
        marker_end = "__MGMK_ENVIRONMENT_END__"
        invocation = subprocess.list2cmdline([str(batch_file), *arguments])
        wrapper_contents = (
            "@echo off\n"
            "setlocal DisableDelayedExpansion\n"
            f"call {invocation}\n"
            "set _MGMK_VS_EXIT=%ERRORLEVEL%\n"
            "if not %_MGMK_VS_EXIT%==0 exit /b %_MGMK_VS_EXIT%\n"
            f"echo {marker_begin}\n"
            "set\n"
            f"echo {marker_end}\n"
            "exit /b 0\n"
        )

        with tempfile.TemporaryDirectory(prefix="mgmake-vsenv-") as temporary_directory:
            wrapper = Path(temporary_directory) / "capture.cmd"
            wrapper.write_text(wrapper_contents, encoding="utf-8")
            comspec = cls._environment_value(environment, "COMSPEC") or "cmd.exe"
            command_line = cls._batch_wrapper_command_line(comspec, wrapper)
            # Pass a raw command-line string on Windows. subprocess.list2cmdline(),
            # which Python uses for a sequence of arguments, escapes embedded
            # quotes with backslashes. cmd.exe does not treat backslashes as
            # quote escapes, so it would try to execute a filename containing
            # literal quote characters (\"C:\\...\").
            result = subprocess.run(
                command_line,
                executable=comspec,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                errors="replace",
                env=dict(environment),
            )

        if result.returncode != 0:
            return None, cls._format_process_failure(
                f"command: call {invocation}",
                result.returncode,
                result.stdout,
                result.stderr,
            )

        begin = result.stdout.find(marker_begin)
        end = result.stdout.find(marker_end, begin + len(marker_begin))
        if begin < 0 or end < 0:
            return None, cls._format_process_failure(
                "the environment capture markers were not emitted",
                result.returncode,
                result.stdout,
                result.stderr,
            )

        environment_text = result.stdout[begin + len(marker_begin):end]
        discovered: dict[str, str] = {}
        for line in environment_text.splitlines():
            name, separator, value = line.partition("=")
            if separator and name and not name.startswith("="):
                discovered[name] = value
        return discovered, None

    @staticmethod
    def _batch_wrapper_command_line(comspec: str, wrapper: Path) -> str:
        # /S makes cmd.exe apply its special quote-stripping rules. The extra
        # outer quote pair is therefore intentional:
        #
        #   cmd.exe /D /S /C ""C:\path with spaces\capture.cmd""
        #
        # Supplying this as one raw string avoids Python's C-runtime argv
        # quoting, which is not compatible with cmd.exe's parser.
        executable = subprocess.list2cmdline([comspec])
        return f'{executable} /d /s /c ""{wrapper}""'

    @staticmethod
    def _format_process_failure(
        heading: str,
        returncode: int,
        stdout: str | None,
        stderr: str | None,
    ) -> str:
        parts = [heading, f"exit code: {returncode}"]
        clean_stdout = (stdout or "").strip()
        clean_stderr = (stderr or "").strip()
        if clean_stdout:
            parts.append(f"stdout:\n{clean_stdout}")
        if clean_stderr:
            parts.append(f"stderr:\n{clean_stderr}")
        return "\n".join(parts)

    @staticmethod
    def _environment_value(environment: Mapping[str, str], name: str) -> str | None:
        folded = name.casefold()
        for key, value in environment.items():
            if key.casefold() == folded:
                return value
        return None

    @classmethod
    def _find_vswhere(cls, environment: Mapping[str, str]) -> str | None:
        path_value = cls._environment_value(environment, "PATH")
        found = shutil.which("vswhere.exe", path=path_value) or shutil.which(
            "vswhere", path=path_value
        )
        if found is not None:
            return str(Path(found).absolute())

        roots = (
            cls._environment_value(environment, "ProgramFiles(x86)"),
            cls._environment_value(environment, "ProgramFiles"),
        )
        for root in roots:
            if not root:
                continue
            candidate = Path(root) / "Microsoft Visual Studio" / "Installer" / "vswhere.exe"
            if candidate.is_file():
                return str(candidate.absolute())
        return None

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
        if data.get("target_architecture") != self.toolchain.target_architecture:
            return {}
        if data.get("host_architecture") != self.toolchain.host_architecture:
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
            "target_architecture": self.toolchain.target_architecture,
            "host_architecture": self.toolchain.host_architecture,
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


