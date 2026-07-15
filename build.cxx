#include "mgmake.hxx"

using namespace mgmk;
using namespace mgmk::spec;

/* Define the project spec */
using testlib = library::name<"testlib">
	::type<library_type::interface>
	::include_dirs<"test">
	::build;

using builder = executable::name<"build">
	::sources<"build.cxx">
	::link<testlib>
	::build;

using proj = project::name<"mgmake">
	::target<builder>
	::build;

// Configure MGMake
using c = config::project<proj>;

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
