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

int main() {

	dag::graph build_graph{};

	auto build_src = build_graph.create_artifact(dag::artifact::kind::source, "build.cxx");
	auto build_exe = build_graph.create_artifact(dag::artifact::kind::generated, "build.exe");
	auto compile_action = build_graph.create_action(
		std::string{ "Compile build.cxx" }, std::string{ "Compiles build.cxx as the build program." },
		std::vector<dag::artifact::id>{ build_src }, std::vector<dag::artifact::id>{ build_exe }, true);

	backend::graphviz back;
	back.generate(build_graph);

	return 0;
}