# `include/mgmake/task/dispatcher.hxx`

**Source:** `include/mgmake/task/dispatcher.hxx`  
**Status:** Public compile-time task matcher and runtime dispatcher.

`task::dispatcher<config_t>` maps task names to bit positions and invokes the task type stored at the selected runtime index.

## Configuration

```cpp
using list_type = config_type::tasks_type;
```

The task order is semantically important because the option storage contains a numeric index into this list.

## `invoke`

`invoke(cmd, opts)` first verifies at compile time that the storage type has a `"task"` key. It then calls:

```cpp
list_type::type_switch(callable, opts.get<"task">());
```

For the selected task type, it:

1. Instantiates `task_traits<task_t>`.
2. Asserts that `valid_handler<config_type>` is true.
3. Calls `task_t::handle<config_type>(cmd, opts)`.

The common result type is `std::expected<sys::exit_code, std::string>`.

## `match`

```cpp
using matches_type = std::bitset<list_type::size()>;
static constexpr matches_type match(std::string_view arg);
```

Each bit corresponds to one task. Matching delegates to `task_traits<task_t>::match`, which delegates to the task's option metadata.

This function allows a task option to derive its index from the actual configured list instead of hard-coding it.

## Failure modes

- Missing `"task"` storage returns an error.
- An out-of-range stored index trips `type_list::type_switch`'s assertion.
- A task lacking the required handler triggers a compile-time assertion.
- A task handler can return a runtime error string.
