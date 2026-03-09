#include <mgmake.hxx>

struct Toolchain {
	std::string mCompiler = "clang++";
};

using namespace mgmk;

using Builder = Target::name<"builder">
	::sources<Sources.add<"build.cxx">()>
	::includes<Includes.add<"include">()>;

using MgMake = Project
	::name<"mgmake">
	::standard<"c++2c">
	::add_target<Builder>;

int main() {
	const Toolchain toolchain;
	const MgMake project;
	return project.build(toolchain);
}