from __future__ import annotations

from pathlib import Path

from mgmake import BuildProject, CMakeProject, CxxExecutable, CxxStaticLibrary, GitFetch, LocalSource
from mgmake.toolchains import define_toolchains


def define_project(project_root: Path) -> BuildProject:
    glm_project = CMakeProject(
        name="glm",
        source=GitFetch(url="https://github.com/g-truc/glm.git"),
        languages=("CXX",),
        options={
            "GLM_BUILD_LIBRARY:BOOL": True,
            "GLM_BUILD_TESTS:BOOL": False,
            "GLM_BUILD_INSTALL:BOOL": True,
            "BUILD_SHARED_LIBS:BOOL": False,
        },
    )
    glm = glm_project.library(
        name="glm",
        package="glm",
        target="glm::glm",
    )

    sdl_project = CMakeProject(
        name="sdl",
        source=GitFetch(
            url="https://github.com/libsdl-org/SDL.git",
            ref="release-3.4.x",
        ),
        languages=("C",),
        options={
            "SDL_EXAMPLES:BOOL": False,
            "SDL_WERROR:BOOL": False,
            "SDL_SHARED_DEFAULT:BOOL": False,
            "BUILD_SHARED_LIBS:BOOL": False,
            "SDL_SHARED:BOOL": False,
            "SDL_STATIC:BOOL": True,
            "SDL_INSTALL:BOOL": True,
            "SDL_TEST_LIBRARY:BOOL": False,
            "SDL_TESTS:BOOL": False,
            "SDL_INSTALL_TESTS:BOOL": False,
        },
    )
    sdl = sdl_project.library(
        name="sdl",
        package="SDL3",
        target="SDL3::SDL3",
        components=("SDL3-static",),
    )

    imgui = CxxStaticLibrary(
        name="imgui",
        source=GitFetch(
            url="https://github.com/ocornut/imgui.git",
            ref="v1.92.9",
        ),
        sources=(
            "imgui.cpp",
            "imgui_demo.cpp",
            "imgui_draw.cpp",
            "imgui_tables.cpp",
            "imgui_widgets.cpp",
            "backends/imgui_impl_sdl3.cpp",
            "backends/imgui_impl_sdlrenderer3.cpp",
        ),
        public_include_directories=(".", "backends"),
        dependencies=(sdl,),
    )

    main = CxxExecutable(
        name="main",
        source=LocalSource(project_root),
        sources=("main.cxx",),
        dependencies=(glm, imgui),
    )

    return BuildProject(
        name="glm-sdl-imgui-example",
        targets=(glm, sdl, imgui, main),
        default_target=main,
        toolchains=define_toolchains(),
        default_toolchain="LLVM",
    )
