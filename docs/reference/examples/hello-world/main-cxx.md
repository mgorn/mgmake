# `examples/hello_world/main.cxx`

**Source:** `examples/hello_world/main.cxx`  
**Status:** Complete hello-world program used as example target input.

This is the source file the hello-world build description intends to compile.

## Program

```cpp
#include <print>

int main() {
    std::print("Hello, world!\n");
    return 0;
}
```

It requires a standard library implementation that provides `<print>`. The explicit zero return indicates successful process completion.

This file is independent of mgmake; it is ordinary target source used to demonstrate source-file collection.
