#include "mgmake.hxx"

using namespace mgmk;
using namespace mgmk::spec;
using namespace mgmk::find;

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
static constexpr auto c = config
	.project<proj>();

// Define entrypoint
MGMK_ENTRY(c);

/*
int main(int argc, char* argv[]) {
	auto cmd = sys::shell::from_args(argc, argv);

	using opt_storage_type = decltype(config_v.option_storage());
	using dispatcher_type = task::dispatcher<config_v>;
	using parser_type = cli::parser<opt_storage_type>;

	auto parse_result = parser_type::template parse<dispatcher_type>(cmd);
	if (not parse_result) {
		std::println(stderr, "{}", parse_result.error());
		return std::to_underlying(sys::exit_code::usage_error);
	}

	auto opts = std::move(parse_result).value();

	//
	// Normally:
	//
	// auto dispatch_result = dispatcher_type::invoke(cmd, opts);
	// if (not dispatch_result) {
	//     std::println(stderr, "{}", dispatch_result.error());
	//     return std::to_underlying(sys::exit_code::task_failure);
	// }
	//
	// return std::to_underlying(dispatch_result.value());
	//
	// Everything below represents what the build task would eventually do
	// for the example specification.
	//

	//
	// Resolve paths and tools.
	//

	auto build_dir = opts.template get<"build_dir">();

	if (build_dir.empty()) {
		build_dir = ".build";
	}

	const auto ext_dir = build_dir / "ext";
	const auto obj_dir = build_dir / "obj";
	const auto bin_dir = build_dir / "bin";

	const auto glm_source_dir = ext_dir / "glm-src";
	const auto glm_build_dir = ext_dir / "glm-build";
	const auto glm_install_dir = ext_dir / "glm-install";

	const auto sdl_source_dir = ext_dir / "sdl-src";
	const auto sdl_build_dir = ext_dir / "sdl-build";
	const auto sdl_install_dir = ext_dir / "sdl-install";

	// Pseudocode: use the eventual mgmake tool-resolution API.
	auto git = tools.find<git_tool>();
	auto cmake = tools.find<cmake_tool>();
	auto cxx = tools.find<cxx_tool>();

	std::filesystem::create_directories(ext_dir);
	std::filesystem::create_directories(obj_dir);
	std::filesystem::create_directories(bin_dir);

	//
	// Fetch GLM.
	//

	if (not std::filesystem::exists(glm_source_dir)) {
		auto clone_glm_cmd = sys::shell{
			git,
			"clone",
			"--depth", "1",
			"https://github.com/g-truc/glm.git",
			glm_source_dir
		};

		if (clone_glm_cmd.invoke() != 0) {
			return std::to_underlying(sys::exit_code::task_failure);
		}
	}

	//
	// Fetch SDL.
	//

	if (not std::filesystem::exists(sdl_source_dir)) {
		auto clone_sdl_cmd = sys::shell{
			git,
			"clone",
			"--branch", "release-3.4.x",
			"--depth", "1",
			"https://github.com/libsdl-org/SDL.git",
			sdl_source_dir
		};

		if (clone_sdl_cmd.invoke() != 0) {
			return std::to_underlying(sys::exit_code::task_failure);
		}
	}

	//
	// Configure GLM.
	//
	// Produced from:
	//
	//     cmake
	//         .define<"GLM_BUILD_LIBRARY", "ON">()
	//         .define<"GLM_BUILD_TESTS", "OFF">()
	//         .define<"GLM_BUILD_INSTALL", "ON">()
	//         .install();
	//

	auto configure_glm_cmd = sys::shell{
		cmake,
		"-S", glm_source_dir,
		"-B", glm_build_dir,
		"-DGLM_BUILD_LIBRARY=ON",
		"-DGLM_BUILD_TESTS=OFF",
		"-DGLM_BUILD_INSTALL=ON",
		std::format("-DCMAKE_INSTALL_PREFIX={}", glm_install_dir.string())
	};

	if (configure_glm_cmd.invoke() != 0) {
		return std::to_underlying(sys::exit_code::task_failure);
	}

	//
	// Build GLM.
	//

	auto build_glm_cmd = sys::shell{
		cmake,
		"--build", glm_build_dir,
		"--parallel"
	};

	if (build_glm_cmd.invoke() != 0) {
		return std::to_underlying(sys::exit_code::task_failure);
	}

	//
	// Install GLM.
	//

	auto install_glm_cmd = sys::shell{
		cmake,
		"--install", glm_build_dir
	};

	if (install_glm_cmd.invoke() != 0) {
		return std::to_underlying(sys::exit_code::task_failure);
	}

	//
	// Configure SDL.
	//
	// Produced from:
	//
	//     cmake
	//         .define<"SDL_EXAMPLES", "OFF">()
	//         .define<"SDL_WERROR", "OFF">()
	//         .define<"SDL_SHARED_DEFAULT", "OFF">()
	//         .define<"BUILD_SHARED_LIBS", "OFF">()
	//         .install();
	//

	auto configure_sdl_cmd = sys::shell{
		cmake,
		"-S", sdl_source_dir,
		"-B", sdl_build_dir,
		"-DSDL_EXAMPLES=OFF",
		"-DSDL_WERROR=OFF",
		"-DSDL_SHARED_DEFAULT=OFF",
		"-DBUILD_SHARED_LIBS=OFF",
		std::format("-DCMAKE_INSTALL_PREFIX={}", sdl_install_dir.string())
	};

	if (configure_sdl_cmd.invoke() != 0) {
		return std::to_underlying(sys::exit_code::task_failure);
	}

	//
	// Build SDL.
	//

	auto build_sdl_cmd = sys::shell{
		cmake,
		"--build", sdl_build_dir,
		"--parallel"
	};

	if (build_sdl_cmd.invoke() != 0) {
		return std::to_underlying(sys::exit_code::task_failure);
	}

	//
	// Install SDL.
	//

	auto install_sdl_cmd = sys::shell{
		cmake,
		"--install", sdl_build_dir
	};

	if (install_sdl_cmd.invoke() != 0) {
		return std::to_underlying(sys::exit_code::task_failure);
	}

	//
	// Resolve testlib.
	//
	// testlib is an interface library, so no command is executed for it.
	// Its usage requirements are inherited by `builder`:
	//
	//     include_dirs:
	//         test/
	//         glm-install/include/
	//         sdl-install/include/
	//
	//     link libraries:
	//         SDL3-static
	//         glm
	//

	const auto builder_source = std::filesystem::path{ "build.cxx" };
	const auto builder_object = obj_dir / "build.cxx.o";
	const auto builder_output = bin_dir / "build";

	//
	// Compile build.cxx.
	//
	// The exact flags come from the selected toolchain and configuration.
	//

	auto compile_builder_cmd = sys::shell{
		cxx,
		"-std=c++2d",
		"-Itest",
		std::format("-I{}", glm_install_dir / "include"),
		std::format("-I{}", sdl_install_dir / "include"),
		"-c", builder_source,
		"-o", builder_object
	};

	if (compile_builder_cmd.invoke() != 0) {
		return std::to_underlying(sys::exit_code::task_failure);
	}

	//
	// Resolve the installed library files.
	//
	// These names are illustrative. The CMake integration should eventually
	// resolve the actual imported target locations and transitive link flags.
	//

	const auto glm_library = glm_install_dir / "lib/libglm.a";
	const auto sdl_library = sdl_install_dir / "lib/libSDL3.a";

	//
	// Link the builder executable.
	//
	// Any platform libraries required by SDL would also be appended here by
	// the toolchain/CMake target resolver.
	//

	auto link_builder_cmd = sys::shell{
		cxx,
		builder_object,
		sdl_library,
		glm_library,

		// Pseudocode: transitive platform-specific SDL dependencies.
		// resolved_sdl_link_options...,

		"-o", builder_output
	};

	if (link_builder_cmd.invoke() != 0) {
		return std::to_underlying(sys::exit_code::task_failure);
	}

	return std::to_underlying(sys::exit_code::success);
}
*/

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
