#include <mgmake.hxx>

using Project = mgmake::Project<"MGMake", [] {
	return mgmake::Target.name<"mgmake">().sources<"build.cxx">();
}>;

BUILD_ENTRY(Project);