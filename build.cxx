//usr/bin/env | rmdir ".mgmake" && mkdir ".mgmake" && c++ build.cxx -std=c++26 -o .mgmake/build && exec ./.mgmake/build "$@" && exit 0 && exit 0
#if 0 //WINDOWS
rd /S /Q .mgmake
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" && echo "[BUILD] Building!" && md .mgmake && clang-cl "build.cxx" /std:c++latest /Fe".mgmake\build.exe" && ".mgmake\build.exe" %*
exit /b
#endif

#include "mgmake.hxx"
using namespace mgmk;

using Builder = Target::name<"build">
	::sources<Sources.add<"build.cxx">()>;

using MgMake = Project
	::name<"mgmake">
	::standard<"c++2c">
	::add_target<Builder>;

BUILD_ENTRY(MgMake);