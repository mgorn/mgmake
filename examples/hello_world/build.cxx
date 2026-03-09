#include <mgmake/mgmake.hxx>

using namespace mgmk;

using HelloTarget = Target::name<"hello_world">
	::sources<Sources.add<"main.cxx">()>;

using HelloProj = Project
	::name<"hello_world">
	::standard<"c++2c">
	::add_target<HelloTarget>;

BUILD_ENTRY(HelloProj);