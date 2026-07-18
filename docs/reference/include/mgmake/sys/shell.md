# `include/mgmake/sys/shell.hxx`

**Source:** `include/mgmake/sys/shell.hxx`  
**Status:** Public owned command-line representation and shell-command utility.

`sys::shell` stores already-tokenized arguments, exposes the program/user argument split, safely renders individual arguments, and can execute the resulting command through `std::system`.

## Storage and views

The object owns `std::vector<std::string> m_args`.

- `program_name()` returns argument zero or an empty view.
- `user_args()` returns a span excluding argument zero.
- `view()` returns a span over all arguments.
- `from_args(argc, argv)` copies C arguments into owned strings.

Because views refer to owned strings, they remain valid while the `shell` object is alive and its vector is not modified.

## Rendering

`full_command()` transforms every argument with `arg_escape`, joins them with spaces, and returns one command string.

`arg_needs_escape` detects empty strings or platform-specific shell metacharacters.

### POSIX

Arguments needing escaping are wrapped in single quotes. Embedded single quotes use the standard close/escaped-quote/reopen sequence.

### Windows

Arguments are wrapped in double quotes. Backslashes preceding quotes and trailing backslashes are doubled according to Windows command-line quoting rules. `invoke` adds another outer pair because `cmd.exe /c` strips outer quotes.

## Invocation

```cpp
auto status = command.invoke(verbose, dry_run);
```

The rendered command is printed when either flag is true. A dry run returns zero without executing. Otherwise `std::system` is called.

The returned `int` is the raw `std::system` result; on POSIX it may encode signal and exit-status information rather than being the child exit code directly.

## Scope of escaping

`arg_escape` is for one argument. It is not a parser or escaping function for an already-composed command containing pipelines, redirections, or multiple arguments.
