//usr/bin/env mkdir .mgmake && clang++ "$0" -std=c++2c -o .mgmake/build && exec ./.mgmake/build "$@" && exit 0
#if 0 //WINDOWS
echo "[BUILD] Entering Developer Command Prompt"
rd /S /Q .mgmake
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" && echo "[BUILD] Building!" && md .mgmake && clang-cl "build.cxx" /std:c++latest /Fe".mgmake\build.exe" && ".mgmake\build.exe" %*
exit /b
#endif

#include "mgmake.hxx"
using namespace mgmk;

using Builder = Target::name<"build">
	::sources<Sources.add<"build.cxx">()>
	::includes<Includes.add<"include">()>;

using MgMake = Project
	::name<"mgmake">
	::standard<"c++2c">
	::add_target<Builder>;

BUILD_ENTRY(MgMake);