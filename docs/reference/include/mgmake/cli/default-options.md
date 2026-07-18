# `include/mgmake/cli/default_options.hxx`

**Source:** `include/mgmake/cli/default_options.hxx`  
**Status:** Public default CLI option declarations.

This header defines the option types that are installed into `mgmake::config` unless a caller supplies a replacement option list.

## Options

### `task_option`

A storage-only pseudo-option:

```cpp
option::name<"task">
      ::storage<"task", std::size_t>
      ::build
```

It does not match command-line text. Its purpose is to reserve a `std::size_t` entry named `"task"` in `option_storage`. Task-specific options write the selected dispatcher index into this slot.

### `verbose_option`

Matches `--verbose` and `-v`. Its callback sets the `"verbose"` key to `true`. Because the key is introduced by the option's own `storage_pair`, the runtime value is a default-initialized `bool` until the switch is seen.

### `dry_run_option`

Matches `--dry-run` and sets `"dry_run"` to `true`.

### `build_dir_option`

Matches `--build-dir=<path>` or `--build-dir <path>`. The `value_parser<std::filesystem::path>` specialization converts relative paths to paths rooted at `std::filesystem::current_path()`.

## `default_options`

The exported list is:

```cpp
meta::type_list<
    task_option,
    verbose_option,
    dry_run_option,
    build_dir_option
>
```

Task options are not listed here. `config_builder::build` derives those from the configured task list and prepends them automatically.

## Extending the defaults

```cpp
using options = mgmake::cli::default_options::append<my_option>;
using configured = mgmake::config::options<options>;
```

A replacement list must preserve every storage key required by tasks. In particular, the dispatcher expects a `"task"` key.
