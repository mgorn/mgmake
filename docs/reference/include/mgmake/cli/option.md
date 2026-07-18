# `include/mgmake/cli/option.hxx`

**Source:** `include/mgmake/cli/option.hxx`  
**Status:** Public compile-time CLI option definition API.

This header defines the fluent `cli::option` builder and the finalized `option_impl` type used by the parser, help task, and option-storage generator.

## Builder fields

`option_builder` supports these direct fields:

| Builder alias | Stored value | Purpose |
|---|---|---|
| `name<"...">` | `meta::static_string` | Long name and positional task name. |
| `description<"...">` | `meta::static_string` | Help text. |
| `short_name<'x'>` | `char` | Single-character short switch. |
| `callback<callable>` | value | Runs during parsing. |
| `storage_pair<pair>` | type | Internal key/value declaration. |
| `parse_value<true>` | `bool` | Marks an option as value-taking. |
| `task<true>` | `bool` | Allows a positional task match. |
| `flag<false>` | `bool` | Disables `-`/`--` matching. |

Convenience aliases build common forms:

```cpp
option::set<"verbose", true>
option::parse<"build_dir", std::filesystem::path>
option::storage<"task", std::size_t>
```

Always finish a declaration with `::build`.

## Finalized metadata

`option_impl` exposes values such as `name_value`, `description_value`, `short_name_value`, `parse_value`, `task_value`, and `flag_value`. It also derives:

- `has_storage`
- `storage_key()`
- `storage_value_type`
- `is_callback`

These are consumed at compile time by `option_storage`, `parser`, and the help printer.

## Matching

`match(arg)` checks positional task syntax first when `task_value` is true. If `flag_value` is true, it also accepts:

- `--<name>` through `match_long`
- `-<short_name>` through `match_short`

The current `match_long` uses `starts_with(name_value)`. This intentionally permits `--name=value`, but it also means text such as `--verbose-extra` can match `verbose`. Exact-name or `=` boundary validation is not yet implemented.

## Handling

- `handle_callback` invokes the stored callback.
- `handle_parse` converts a text value with `value_parser<storage_value_type>` and stores it.
- `handle_task` asks the dispatcher for the matching task index and stores it under `"task"`.

The parser currently checks callback handling before task handling. Consequently, task options created with `set<"task", index>` use their callback and do not reach `handle_task`.

## Example

```cpp
using jobs_option = mgmake::cli::option
    ::name<"jobs">
    ::short_name<'j'>
    ::description<"Maximum parallel jobs.">
    ::parse<"jobs", int>
    ::build;
```

The resulting runtime value is available as `opts.get<"jobs">()`.
