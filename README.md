# Michael Gorney's Make
A CMake-inspired C++2X build system using C++2X as it's DSL. The inspiration is that I don't like using CMake.

## Disclaimer
This project is still in its infancy. I don't even use this myself *yet*, but keep an eye out I'm working on it!

## Have issues?
Please open an issue on GitHub!!! I want this project to really be the best it can, so I'll be making sure to help out however I can!!

[Join the GornCord](https://discord.gg/ECDfZGTzVQ)

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
			backend/ - Backend implementations
				graphviz.hxx - Graphviz backend, great for debugging and visualizing your build
				ninja.hxx - Ninja backend, uses the Ninja build tool to compiler your project
				traits.hxx - Traits for conditional compilation of backend features
			cli/ - Command line interface & related utilities
				action.hxx - Command line actions/options (build, generate, clean, etc)
				backend.hxx - Backend options
				options.hxx - Actual user configured state based on command line arguments
				parse.hxx - Parser & utilities for parsing the cli arguments
				util.hxx - Other command line utilities such as printing the help menu
			dag/ - Direct Acyclic Graph, a common structure among other build systems
				action.hxx - An action in the DAG (e.g. a command to run)
				artifact.hxx - An item in the graph (usually some file) that is consumed, produced, or both
				graph.hxx - The main graph structure that collects actions, artifacts & targets
				target.hxx - A target that produces any number of artifacts
			detail/ - Internal helpers & utilities
				convert.hxx - Converts Windows "wide strings" to normal utf8 strings (std::wstring -> std::string)
				static_string.hxx - Compile-time string type, should std::string not be suitable (soon to be deprecated)
            ext/ - Extension headers
                cmake.hxx - Extension for using CMake projects & targets as dependencies
			spec/ - Project specification tools
			sys/ - Platform/system specific utilities
				command_line.hxx - A structure for command line data
				platform.hxx - Util header for determining the host platform, includes <windows.h> unless MGMK_NO_WINDOWS is defined
				util.hxx - Platform specific utilities
    mgmake.hxx - mgmake header
    build.cxx - The build script that builds the build script
```