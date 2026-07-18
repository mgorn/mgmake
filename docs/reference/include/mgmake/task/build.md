# `include/mgmake/task/build.hxx`

**Source:** `include/mgmake/task/build.hxx`  
**Status:** Built-in task stub.

The `task::build` type defines the positional `build` command and the placeholder handler that will eventually lower and execute the configured project.

## Option metadata

```cpp
using option_type = cli::option
    ::name<"build">
    ::description<"Build the project.">
    ::set<"task", std::size_t{1}>
    ::task<true>
    ::flag<false>
    ::build;
```

`task<true>` permits positional matching. `flag<false>` means `--build` is not accepted.

The option uses a callback to write task index `1`.

## Handler

```cpp
template<typename config_t>
static std::expected<sys::exit_code, std::string>
handle(auto& cmd, auto& opts);
```

The current implementation prints `Build task`, prints the configured build directory, and returns success. It does not yet inspect `config_t::project`, discover tools, build a graph, or invoke a compiler.

Because the `build_dir` value is default-constructed when no option is supplied, printing it may yield an empty path.

## Current index mismatch

`task::default_tasks` orders tasks as `<build, help>`, making `build` index `0`. This option writes index `1`, which dispatches `help`. Since callback handling precedes generic task handling in the parser, the mismatch is observable. Either the hard-coded values or the task ordering should be corrected; deriving the index through `dispatcher::match` would avoid this coupling.
