# `examples/using_cmake/main.cxx`

**Source:** `examples/using_cmake/main.cxx`  
**Status:** Incomplete example target source.

This file is intended to be the executable source for the future CMake-integration example.

## Intended program

It includes `<print>` and writes a message indicating that an external CMake project is in use.

## Current syntax issue

The `std::print(...)` statement is missing its terminating semicolon:

```cpp
std::print("Hello, world! Using an external CMake project.")
```

The file therefore does not compile as written. Add `;` before using it as an example target.
