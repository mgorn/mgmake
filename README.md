# Michael Gorney's Make
A CMake-inspired C++2X build system using C++2X as it's DSL. The inspiration is that I don't like using CMake.

## Disclaimer
This project is still in its infancy. I don't even use this myself *yet*, but keep an eye out I'm working on it!

## Have issues?
Please open an issue on GitHub!!! I want this project to really be the best it can, so I'll be making sure to help out however I can!!

# Usage
1) Download the `mgmake.hxx` file.
2) Copy this snippet to the top of your build script (call it `build.cxx`).
```c++
//usr/bin/env mkdir .mgmake && clang++ "$0" -std=c++2c -o .mgmake/build && exec ./.mgmake/build "$@" && exit 0
#if 0 //WINDOWS
echo "[BUILD] Entering Developer Command Prompt"
rd /S /Q .mgmake
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" && echo "[BUILD] Building!" && md .mgmake && clang-cl "build.cxx" /std:c++latest /Fe".mgmake\build.exe" && ".mgmake\build.exe" %*
exit /b
#endif

#include "mgmake.hxx"
```
(Optional) add this using statement for convenience:
```c++
using namespace mgmk;
```
3) Create your project and target(s).
4) Run the source file (`build.cxx`) as a script

Mac/Linux/Unix-like
```shell 
chmod +x build.cxx
./build.cxx
```
Windows
```commandline 
cmd < build.cxx
```

# Files
```
/ - Project root
    include/ - mgmake library headers
        mgmake/
            ext/ - Extension headers
                cmake.hxx - Extension for using CMake projects & targets as dependencies
    mgmake.hxx - mgmake header
    build.cxx - The build script that builds the build script
```