# `include/mgmake/cli/parser.hxx`

**Source:** `include/mgmake/cli/parser.hxx`  
**Status:** Public parser implementation parameterized by an `option_storage` type.

The parser matches command-line tokens against a compile-time list of option types and fills a runtime `option_storage` instance.

## Option partitions

`list_type` is the complete list from the storage type. Two filtered lists are derived:

- `tasks_type`: options with `task_value == true`.
- `switches_type`: all remaining options.

`switches_type` is currently metadata only; matching uses the complete list.

## Parsing rules

`parse<dispatcher_t>(cmd)` iterates over `cmd.user_args()`:

- A token beginning with `-` is a switch.
- The first non-switch token is a task.
- Any later non-switch token is invalid unless it was consumed as the value of the preceding value-taking option.

Every token must match exactly one option. A `std::bitset` records matching option indices; `detail::index_bit` converts the sole set bit to an index; and `type_list::type_switch` instantiates the selected handler.

## Value-taking options

Both forms are supported:

```text
--build-dir=out
--build-dir out
```

The second form advances the iterator past the consumed value. A missing value produces an error.

The parser currently handles one value per occurrence. Container-valued or repeatable options are noted as future work.

## Diagnostics

For an unprefixed token in an invalid position, the parser tries to produce a hint:

1. Test the token as `-<token>`.
2. Test it as `--<token>`.
3. Test whether it is a known task.

Unknown or ambiguous matches produce errors rather than silently selecting an option.

## Handler order

For the selected option, the parser checks:

1. `parse_value`
2. `is_callback`
3. `task_value`

This is significant for task options built with `option::set`: their callback runs before the generic task handler, so the hard-coded value supplied to `set` becomes the dispatcher index.

## Matching API

```cpp
using matches_type = std::bitset<list_type::size()>;
static constexpr matches_type match(std::string_view arg);
```

This API is also useful for diagnostics. The bit position corresponds directly to the option's position in `list_type`.
