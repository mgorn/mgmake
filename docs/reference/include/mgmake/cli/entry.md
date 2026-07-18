# `include/mgmake/cli/entry.hxx`

**Source:** `include/mgmake/cli/entry.hxx`  
**Status:** Public runtime entry point and `main`-generation macro.

`cli::entry` is the bridge from a compile-time configuration to runtime command-line parsing and task dispatch.

## Overloads

```cpp
template<typename config_builder_t = config>
sys::exit_code entry(sys::shell cmd);

template<typename config_builder_t = config>
sys::exit_code entry(int argc, char* argv[]);
```

The `argc`/`argv` overload copies the arguments into `sys::shell` and delegates to the primary overload.

## Execution flow

The primary overload:

1. Finalizes `config_builder_t` as `config_builder_t::build`.
2. Selects the generated `option_storage_type`.
3. Instantiates `task::dispatcher<config_type>`.
4. Parses the command line with `cli::parser<option_storage_type>`.
5. Invokes the selected task.
6. Prints parse or task errors to `stderr`.
7. Maps failures to `sys::exit_code::usage_error` or `task_failure`.

Successful tasks return their own `sys::exit_code`.

## `MGMK_ENTRY`

```cpp
#define MGMK_ENTRY(...) \
int main(int argc, char* argv[]) { \
    return std::to_underlying(::mgmake::cli::entry<__VA_ARGS__>(argc, argv)); \
}
```

Use it once in a build script:

```cpp
using configured = mgmake::config::project<my_project>;
MGMK_ENTRY(configured);
```

With no argument, `MGMK_ENTRY()` uses the default `mgmake::config`. The variadic macro form allows an empty template argument list to be produced.

## Ownership

`entry(sys::shell)` takes the shell object by value. The parser and task handlers receive references to that owned object for the duration of the call.
