#include "mgmake.hxx"

using namespace mgmk;
using namespace mgmk::spec;

/* Define the project spec */
static constexpr auto glmproj = cmake.name<"glm">()
	.fetch<fetch.git()
		.name<"glm-src">()
		.url<"https://github.com/g-truc/glm.git">()
	>()
	.define<"GLM_BUILD_LIBRARY", "ON">()
	.define<"GLM_BUILD_TESTS", "OFF">()
	.define<"GLM_BUILD_INSTALL", "ON">()
	.install();
static constexpr auto glm = glmproj.library().name<"glm">();

static constexpr auto sdlproj = cmake.name<"sdl">()
	.fetch<fetch.git()
		.name<"sdl-src">()
		.url<"https://github.com/libsdl-org/SDL.git">()
		.tag<"release-3.4.x">()
	>()
	.define<"SDL_EXAMPLES", "OFF">()
	.define<"SDL_WERROR", "OFF">()
	.define<"SDL_SHARED_DEFAULT", "OFF">()
	.define<"BUILD_SHARED_LIBS", "OFF">()
	.install();
static constexpr auto sdl = sdlproj.library().name<"SDL3-static">();

static constexpr auto testlib = library.name<"testlib">()
	.type<library_type::interface>()
	.include_dirs<"test">()
	.link<sdl>()
	.link<glm>();

static constexpr auto builder = executable.name<"build">()
	.sources<"build.cxx">()
	.link<testlib>();

static constexpr auto proj = project
	.name<"mgmake">()
	.targets<builder>();

// Configure MGMake
static constexpr auto c = config.project<proj>();

// Define entrypoint
MGMK_ENTRY(c);

/*
project proj() {
	auto testlib = library{"testlib", library::kind::interface}
		.add_include_dir("test");

	auto builder = executable{"build"}
		.add_source("build.cxx")
		.link(testlib);

	return project{"mgmake"}
		.add_target(testlib)
		.add_target(builder);
}
*/

/*
using custom_option = cli::option
	::name<"custom">
	::description<"Users can define CLI options in their build scripts!">
	::callback<[](auto& opts) {
		std::println("Custom option used!");
	}>
	::build;

using my_options = cli::default_options::append<custom_option>;
using my_config = config::options<my_options>;

MGMK_ENTRY(my_config);
*/
