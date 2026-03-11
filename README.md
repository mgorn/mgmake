# Michael Gorney's Make
A CMake-inspired C++2X build system using C++2X as it's DSL. The inspiration is that I don't like using CMake.

## Disclaimer
This project is still in its infancy. I don't even use this myself *yet*, but keep an eye out I'm working on it!

## Have issues?
Please open an issue on GitHub!!! I want this project to really be the best it can, so I'll be making sure to help out however I can!!

# Usage
1) Download the `mgmake.hxx` file.
2) Copy this segment into your build script (typically `build.cxx`).
```c++
//usr/bin/env mkdir .mgmake && clang++ "$0" -I include -std=c++2c -o .mgmake/build && exec ./.mgmake/build "$@" && exit 0
#include <mgmake/mgmake.hxx>
```
3) Optionally add this using statement for convenience:
```c++
using namespace mgmk;
```
4) Run the source file (`build.cxx`) as a script
```shell
chmod +x build.cxx
./build.cxx
```

# Files
```
/ - Project root
    include/ - mgmake library headers
        mgmake.hxx - mgmake header
    build.cxx - The build script that builds the build script
```