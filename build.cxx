#include <mgmake.hxx>

using namespace mgmk;

using Builder = Target::name<"builder">
	::sources<Sources.add<"build.cxx">()>
	::includes<Includes.add<"include">()>;

using MgMake = Project
	::name<"mgmake">
	::standard<"c++2c">
	::add_target<Builder>;

BUILD_ENTRY(MgMake);