#include "mgmake.hxx"

using namespace mgmk;

/*
using Builder = Target::name<"build">
	::sources<Sources.add<"build.cxx">()>;

using MgMake = Project
	::name<"mgmake">
	::standard<"c++2c">
	::add_target<Builder>;
*/

//MGMK_BUILD_ENTRY(void);

/*
int main() {
	dag::graph build_graph{};

	auto build_src = build_graph.create_artifact(dag::artifact::kind::source, "build.cxx");
	auto build_exe = build_graph.create_artifact(dag::artifact::kind::generated, ".build/build");
	auto compile_action = build_graph.create_action(
		std::string{ "Compile build.cxx" }, std::string{ "Compiles build.cxx as the build program." },
		std::vector<dag::artifact::id>{ build_src }, std::vector<dag::artifact::id>{ build_exe },
		true, sys::command_line{std::vector<std::string>{"clang-mg++", "build.cxx", "-o", ".build/build" }});

	backend::graphviz viz;
	viz.generate(build_graph);
	backend::ninja back;
	back.build(build_graph);

	return 0;
}
*/

#include <iostream>

int main(int argc, const char** argv) {
	auto args = sys::args_from_utf8(argc, argv);
	auto parsed = cli::parse(args);

	if (!parsed) {
		std::cerr << "mgmake: error: " << parsed.m_error << std::endl;
		return 2;
	}

	cli::options opts = std::move(parsed.m_value);

	if (opts.m_show_help) {
		cli::print_help(args.program_name());
		return 0;
	}

	build::request req{ build::tc_clang_mg, opts.m_build_dir, { "build" }, opts.target() };
	auto testlib = spec::library{"testlib", spec::library::kind::interface}.add_include_dir("test");
	auto builder = spec::executable{"build"}.add_source("build.cxx").link(testlib);
	auto proj = spec::project{"mkmake"}.add_target(builder).add_target(testlib);
	auto graph = proj.graph(req);

	backend::graphviz viz;
	viz.generate(graph, req);
	
	backend::ninja be;
	auto result = be.build(graph, req);
	if (!result) {
		std::cerr << result.error() << std::endl;
	}

	return 0;
}
