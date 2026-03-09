# Michael Gorney's Make
A CMake-inspired C++ build system using C++ as it's DSL. The inspiration is that I don't like using CMake.

## Disclaimer
This project is still in it's infancy. I don't even use this myself *yet*, but keep an eye out I'm working on it!

# Usage
1) Use a `#include` directive to include the `mgmake.hxx` file.
2) Set up your build (see examples)
3) Compile your build binary using your favorite compiler
4) Run the build executable to build your program

# Files
```
/ - Project root
    include/ - mgmake library headers
        mgmake.hxx - mgmake header
    build.cxx - The build script that builds the build script
    CMakeLists.txt - The cmake project file to compile the build script without the build script
```