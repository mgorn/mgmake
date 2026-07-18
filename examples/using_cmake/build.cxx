#include "mgmake.hxx"

using namespace mgmk;
using namespace mgmk::spec;

/* Define the project spec */
using glmproj = cmake::name<"glm">
	::fetch<fetch::git
		::name<"glm-src">
		::url<"https://github.com/g-truc/glm.git">>
	::define<"GLM_BUILD_LIBRARY", "ON">
	::define<"GLM_BUILD_TESTS", "OFF">
	::define<"GLM_BUILD_INSTALL", "ON">
	::install;
using glm = glmproj::library<"glm">;

using sdlproj = cmake::name<"sdl">
	::fetch<fetch::git
		::name<"sdl-src">
		::url<"https://github.com/libsdl-org/SDL.git">
		::tag<"release-3.4.x">>
	::define<"SDL_EXAMPLES", "OFF">
	::define<"SDL_WERROR", "OFF">
	::define<"SDL_SHARED_DEFAULT", "OFF">
	::define<"BUILD_SHARED_LIBS", "OFF">
	::install;
using sdl = sdlproj::library<"SDL3-static">;

using tafw = library::name<"tafw">
	::type<library_type::static_lib>
	::include_dir<"include">
	::source<"src/platform/sdl3/sdl3_window.cxx">
	::link<glm>
	::link<sdl>;

using hello_world = executable::name<"hello_world">
	::source<"example/hello_world.cxx">
	::link<tafw>;

using proj = project::name<"mgmake">
	::external<glmproj>
	::external<sdlproj>
	::target<glm>
	::target<sdl>
	::target<tafw>
	::target<hello_world>;

// Configure MGMake
using c = config::project<proj>;

// Define entrypoint
MGMK_ENTRY(c);