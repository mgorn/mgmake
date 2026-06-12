#include "mgmake.hxx"

auto project() -> mgmk::spec::project {
	auto testlib =
		mgmk::spec::library{
			"testlib",
			mgmk::spec::library::kind::interface
		}.add_include_dir("test");

	auto builder =
		mgmk::spec::executable{"build"}
			.add_source("build.cxx")
			.link(testlib);

	return mgmk::spec::project{"mkmake"}
		.add_target(testlib)
		.add_target(builder);
}

MGMK_ENTRY(project);
