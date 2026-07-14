#include "mgmake.hxx"

using namespace mgmk;
/*
using namespace mgmk::spec;

project proj() {
	auto testlib = library{"testlib", library::kind::interface}
		.add_include_dir("test");

	auto builder = executable{"build"}
		.add_source("build.cxx")
		.link(testlib);

	return project{"mkmake"}
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

MGMK_ENTRY();