#include "mgmake.hxx"

/*
using namespace mgmk;
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

MGMK_ENTRY();