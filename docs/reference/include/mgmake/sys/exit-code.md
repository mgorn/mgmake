# `include/mgmake/sys/exit_code.hxx`

**Source:** `include/mgmake/sys/exit_code.hxx`  
**Status:** Public process-result enum.

`sys::exit_code` is the typed result returned by task handlers and the CLI entry point.

## Values

```cpp
enum struct exit_code : int {
    success,       // 0
    task_failure,  // 1
    usage_error    // 2
};
```

Because no explicit enumerators are assigned, values begin at zero and increment in declaration order.

`MGMK_ENTRY` converts the enum to its underlying `int` with `std::to_underlying` before returning from `main`.

## Meaning

- `success`: parsing and task execution succeeded.
- `task_failure`: a task returned an error.
- `usage_error`: command-line parsing failed.

Additional build-tool-specific exit categories can be added later without changing task-handler signatures.
