# `include/mgmake/cli/value_parser.hxx`

**Source:** `include/mgmake/cli/value_parser.hxx`  
**Status:** Public customization point for parsing option values.

`value_parser<T>` converts one command-line string into a typed value and provides a short help-menu label for that type.

## Primary template

The unspecialized template only defines:

```cpp
static inline constexpr std::string_view help_hint = "value";
```

It does not provide `parse`. Attempting to use `option::parse<key, T>` for an unsupported type therefore fails at compile time when the parser tries to call `value_parser<T>::parse`.

## Built-in specializations

### `std::string`

Copies the complete input and uses the help hint `text`.

### `int`

Uses `std::stoi` after copying the `std::string_view` into a `std::string`. Empty input and conversion exceptions become `std::unexpected<std::string>` errors. The current implementation does not verify that `std::stoi` consumed the entire string, so a numeric prefix followed by trailing text may be accepted.

### `std::filesystem::path`

Constructs a path from the text. Absolute paths are returned unchanged; relative paths are prefixed with `std::filesystem::current_path()`. The result is not canonicalized and need not exist.

## Custom parser

```cpp
template<>
struct mgmake::cli::value_parser<my_type> {
    static inline constexpr std::string_view help_hint = "my-type";

    static std::expected<my_type, std::string>
    parse(std::string_view text) {
        // ...
    }
};
```

The parser should not throw for ordinary invalid input; return a descriptive `std::unexpected` instead.
