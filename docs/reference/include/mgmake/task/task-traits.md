# `include/mgmake/task/task_traits.hxx`

**Source:** `include/mgmake/task/task_traits.hxx`  
**Status:** Task protocol introspection.

`task_traits` extracts a task's option metadata, validates its handler shape, and provides common name/description/matching operations.

## Handler concept

The intended protocol is:

```cpp
template<typename config_t>
static std::expected<sys::exit_code, std::string>
handle(const sys::shell& cmd, const options_type& opts);
```

`task_handler<task_t, config_t>` checks that `task_t::handle<config_t>(cmd, opts)` has exactly the expected return type.

The concept currently forms its check with a forward-declared `cli::options` type, while the actual parser passes a `cli::option_storage<...>` specialization. Built-in handlers use abbreviated `auto` parameters, so they satisfy the expression shape; more narrowly typed custom handlers may not.

## `task_traits`

For a task type:

- `task_type` aliases the original type.
- `option_type` aliases `task_type::option_type`.
- `valid_handler<config_t>` exposes the concept result.
- `name()` and `description()` return string views from option metadata.
- `match(arg)` delegates to `option_type::match(arg)`.

The dispatcher and help task consume this interface rather than depending on each task's implementation details.
