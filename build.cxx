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

MGMK_BUILD_ENTRY(void);