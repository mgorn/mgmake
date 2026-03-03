#include <mgmake.hxx>
//$include <"https://mwg.codes/mgmake.hxx">

using namespace mgmake;

using MyProject = Project<
	"MGMake",
	Target.name<"mgmake">().with<Sources.sources<"build.cxx">()>()
>;

BUILD_ENTRY(MyProject);