//usr/bin/env mkdir .mgmake && clang++ "$0" -I include -std=c++2c -o .mgmake/build && exec ./.mgmake/build "$@" && exit 0
#include <mgmake/mgmake.hxx>
using namespace mgmk;

using Builder = Target::name<"build">
	::sources<Sources.add<"build.cxx">()>
	::includes<Includes.add<"include">()>;

using MgMake = Project
	::name<"mgmake">
	::standard<"c++2c">
	::add_target<Builder>;

BUILD_ENTRY(MgMake);