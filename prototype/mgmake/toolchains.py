from __future__ import annotations

import os
import platform
import sys
from pathlib import Path

from .tools import ArchiverStyle, DriverStyle, ToolRole, ToolchainBootstrap, ToolchainSpec


def android_toolchain() -> ToolchainSpec:
    ndk = os.environ.get("ANDROID_NDK_HOME") or os.environ.get("ANDROID_NDK_ROOT")
    abi = os.environ.get("ANDROID_ABI", "arm64-v8a")
    api = os.environ.get("ANDROID_API", "21")
    host_system = {
        "win32": "windows-x86_64",
        "darwin": "darwin-x86_64",
        "linux": "linux-x86_64",
    }.get(sys.platform, "linux-x86_64")
    triples = {
        "arm64-v8a": "aarch64-linux-android",
        "armeabi-v7a": "armv7a-linux-androideabi",
        "x86": "i686-linux-android",
        "x86_64": "x86_64-linux-android",
    }
    triple = triples.get(abi, "aarch64-linux-android")
    candidates: dict[ToolRole, tuple[str, ...]] = {}
    cmake_options: list[str] = []
    if ndk:
        ndk_path = Path(ndk)
        bin_dir = ndk_path / "toolchains" / "llvm" / "prebuilt" / host_system / "bin"
        suffix = ".cmd" if os.name == "nt" else ""
        candidates = {
            ToolRole.CC: (str(bin_dir / f"{triple}{api}-clang{suffix}"),),
            ToolRole.CXX: (str(bin_dir / f"{triple}{api}-clang++{suffix}"),),
            ToolRole.LINKER: (str(bin_dir / f"{triple}{api}-clang++{suffix}"),),
            ToolRole.AR: (str(bin_dir / ("llvm-ar.exe" if os.name == "nt" else "llvm-ar")),),
            ToolRole.RANLIB: (str(bin_dir / ("llvm-ranlib.exe" if os.name == "nt" else "llvm-ranlib")),),
            ToolRole.CMAKE: ("cmake",),
            ToolRole.NINJA: ("ninja",),
            ToolRole.GIT: ("git",),
        }
        cmake_options = [
            f"-DCMAKE_TOOLCHAIN_FILE:FILEPATH={ndk_path / 'build' / 'cmake' / 'android.toolchain.cmake'}",
            f"-DANDROID_ABI:STRING={abi}",
            f"-DANDROID_PLATFORM:STRING=android-{api}",
        ]
    else:
        candidates = {
            ToolRole.CC: (f"{triple}{api}-clang",),
            ToolRole.CXX: (f"{triple}{api}-clang++",),
            ToolRole.LINKER: (f"{triple}{api}-clang++",),
            ToolRole.AR: ("llvm-ar",),
            ToolRole.RANLIB: ("llvm-ranlib",),
            ToolRole.CMAKE: ("cmake",),
            ToolRole.NINJA: ("ninja",),
            ToolRole.GIT: ("git",),
        }

    return ToolchainSpec(
        name="Android",
        description="Android NDK Clang toolchain (ANDROID_NDK_HOME, ANDROID_ABI, ANDROID_API)",
        candidates=candidates,
        cmake_options=tuple(cmake_options),
        preferred_generators=(ToolRole.NINJA,),
    )


def ios_toolchain() -> ToolchainSpec:
    sdk = os.environ.get("IOS_SDK", "iphoneos")
    arch = os.environ.get("IOS_ARCH", "arm64")
    deployment = os.environ.get("IOS_DEPLOYMENT_TARGET", "13.0")
    return ToolchainSpec(
        name="iOS",
        description="Apple iOS toolchain selected through xcrun",
        candidates={
            ToolRole.CC: (f"xcrun://{sdk}/clang",),
            ToolRole.CXX: (f"xcrun://{sdk}/clang++",),
            ToolRole.LINKER: (f"xcrun://{sdk}/clang++",),
            ToolRole.AR: (f"xcrun://{sdk}/ar",),
            ToolRole.RANLIB: (f"xcrun://{sdk}/ranlib",),
            ToolRole.CMAKE: ("cmake",),
            ToolRole.XCODEBUILD: ("xcodebuild",),
            ToolRole.NINJA: ("ninja",),
            ToolRole.GIT: ("git",),
        },
        compile_options=("-arch", arch, f"-mios-version-min={deployment}"),
        link_options=("-arch", arch, f"-mios-version-min={deployment}"),
        cmake_options=(
            "-DCMAKE_SYSTEM_NAME:STRING=iOS",
            f"-DCMAKE_OSX_SYSROOT:STRING={sdk}",
            f"-DCMAKE_OSX_ARCHITECTURES:STRING={arch}",
            f"-DCMAKE_OSX_DEPLOYMENT_TARGET:STRING={deployment}",
        ),
        preferred_generators=(ToolRole.XCODEBUILD, ToolRole.NINJA),
    )


def windows_tool_architectures() -> tuple[str, str]:
    machine = platform.machine().casefold()
    default_arch = {
        "amd64": "x64",
        "x86_64": "x64",
        "arm64": "arm64",
        "aarch64": "arm64",
        "x86": "x86",
        "i386": "x86",
        "i686": "x86",
    }.get(machine, "x64")
    return (
        os.environ.get("MGMK_MSVC_ARCH", default_arch),
        os.environ.get("MGMK_MSVC_HOST_ARCH", default_arch),
    )


def msvc_toolchain() -> ToolchainSpec:
    target_arch, host_arch = windows_tool_architectures()
    return ToolchainSpec(
        name="MSVC",
        description=(
            "Microsoft Visual C++ from Visual Studio/Build Tools "
            "(MGMK_MSVC_ARCH, MGMK_MSVC_HOST_ARCH)"
        ),
        candidates={
            ToolRole.CC: ("cl.exe",),
            ToolRole.CXX: ("cl.exe",),
            ToolRole.LINKER: ("cl.exe",),
            ToolRole.SHARED_LINKER: ("cl.exe",),
            ToolRole.LIB: ("lib.exe",),
            ToolRole.ASM: ("ml64.exe", "ml.exe"),
            ToolRole.RC: ("rc.exe",),
            ToolRole.MIDL: ("midl.exe",),
            ToolRole.MT: ("mt.exe",),
            ToolRole.CMAKE: ("cmake.exe", "cmake"),
            ToolRole.NINJA: ("ninja.exe", "ninja"),
            ToolRole.MSBUILD: ("MSBuild.exe", "msbuild"),
            ToolRole.GIT: ("git.exe", "git"),
        },
        driver_style=DriverStyle.MSVC,
        archiver_style=ArchiverStyle.LIB,
        compile_options=("/EHsc",),
        preferred_generators=(ToolRole.NINJA, ToolRole.MSBUILD),
        bootstrap=ToolchainBootstrap.VISUAL_STUDIO,
        target_architecture=target_arch,
        host_architecture=host_arch,
    )


def define_toolchains() -> tuple[ToolchainSpec, ...]:
    windows_target_arch, windows_host_arch = windows_tool_architectures()
    llvm = ToolchainSpec(
        name="LLVM",
        description="LLVM tools with the Clang C and C++ drivers",
        candidates={
            ToolRole.CC: ("clang",),
            ToolRole.CXX: ("clang++",),
            ToolRole.LINKER: ("clang++",),
            ToolRole.SHARED_LINKER: ("clang++",),
            ToolRole.AR: ("llvm-ar",),
            ToolRole.RANLIB: ("llvm-ranlib",),
            ToolRole.LIB: ("llvm-lib",),
            ToolRole.RC: ("llvm-rc",),
            ToolRole.CMAKE: ("cmake",),
            ToolRole.NINJA: ("ninja",),
            ToolRole.GIT: ("git",),
        },
    )
    clang = ToolchainSpec(
        name="Clang",
        description="Clang drivers with host archive and build tools",
        candidates={
            ToolRole.CC: ("clang",),
            ToolRole.CXX: ("clang++",),
            ToolRole.LINKER: ("clang++",),
            ToolRole.AR: ("ar", "llvm-ar"),
            ToolRole.RANLIB: ("ranlib", "llvm-ranlib"),
            ToolRole.CMAKE: ("cmake",),
            ToolRole.NINJA: ("ninja",),
            ToolRole.MAKE: ("make",),
            ToolRole.GIT: ("git",),
        },
    )
    clang_cl = ToolchainSpec(
        name="Clang-CL",
        description="clang-cl, llvm-lib, and a Windows-compatible linker",
        candidates={
            ToolRole.CC: ("clang-cl",),
            ToolRole.CXX: ("clang-cl",),
            ToolRole.LINKER: ("clang-cl",),
            ToolRole.SHARED_LINKER: ("clang-cl",),
            ToolRole.LIB: ("llvm-lib", "lib"),
            ToolRole.RC: ("llvm-rc", "rc"),
            ToolRole.CMAKE: ("cmake",),
            ToolRole.NINJA: ("ninja",),
            ToolRole.MSBUILD: ("MSBuild.exe", "msbuild"),
            ToolRole.GIT: ("git",),
        },
        driver_style=DriverStyle.MSVC,
        archiver_style=ArchiverStyle.LIB,
        preferred_generators=(ToolRole.NINJA, ToolRole.MSBUILD),
        cmake_generator_toolset="ClangCL",
        bootstrap=ToolchainBootstrap.VISUAL_STUDIO,
        target_architecture=windows_target_arch,
        host_architecture=windows_host_arch,
    )
    gcc = ToolchainSpec(
        name="GCC",
        description="GNU Compiler Collection toolchain",
        candidates={
            ToolRole.CC: ("gcc",),
            ToolRole.CXX: ("g++",),
            ToolRole.LINKER: ("g++",),
            ToolRole.AR: ("ar",),
            ToolRole.RANLIB: ("ranlib",),
            ToolRole.CMAKE: ("cmake",),
            ToolRole.NINJA: ("ninja",),
            ToolRole.MAKE: ("make",),
            ToolRole.GIT: ("git",),
        },
    )
    apple = ToolchainSpec(
        name="AppleClang",
        description="Apple Clang for macOS",
        candidates={
            ToolRole.CC: ("xcrun://macosx/clang",),
            ToolRole.CXX: ("xcrun://macosx/clang++",),
            ToolRole.LINKER: ("xcrun://macosx/clang++",),
            ToolRole.AR: ("xcrun://macosx/ar",),
            ToolRole.RANLIB: ("xcrun://macosx/ranlib",),
            ToolRole.CMAKE: ("cmake",),
            ToolRole.NINJA: ("ninja",),
            ToolRole.XCODEBUILD: ("xcodebuild",),
            ToolRole.GIT: ("git",),
        },
        preferred_generators=(ToolRole.NINJA, ToolRole.XCODEBUILD),
    )
    emscripten = ToolchainSpec(
        name="Emscripten",
        description="Emscripten WebAssembly toolchain",
        candidates={
            ToolRole.CC: ("emcc",),
            ToolRole.CXX: ("em++",),
            ToolRole.LINKER: ("em++",),
            ToolRole.AR: ("emar",),
            ToolRole.RANLIB: ("emranlib",),
            ToolRole.CMAKE: ("cmake",),
            ToolRole.NINJA: ("ninja",),
            ToolRole.GIT: ("git",),
            ToolRole.EMULATOR: ("node",),
        },
        cmake_options=("-DCMAKE_SYSTEM_NAME:STRING=Emscripten",),
        preferred_generators=(ToolRole.NINJA,),
    )
    return (
        llvm,
        clang,
        clang_cl,
        msvc_toolchain(),
        gcc,
        apple,
        android_toolchain(),
        ios_toolchain(),
        emscripten,
    )
