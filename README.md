# mgmake
A C++ build system using C++ as it's DSL

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