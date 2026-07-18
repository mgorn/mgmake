# `include/mgmake/meta/type_value.hxx`

**Source:** `include/mgmake/meta/type_value.hxx`  
**Status:** Core adapter from compile-time values to types.

`type_value<value>` wraps an NTTP so it can participate in type-only containers such as `type_list` and `type_map`.

## `type_value`

```cpp
template<auto value_v>
struct type_value {
    static inline constexpr auto value = value_v;
};
```

Common uses include wrapping `meta::static_string` keys, source paths, enum values, booleans, and callback values.

## `type_value_or`

```cpp
template<typename value_t, auto default_v>
struct type_value_or;
```

For a normal wrapper type, `value` forwards `value_t::value`. The specialization for `void` uses `default_v`.

This is the value-level analogue of `type_or`, although most current builder consumers implement defaults through the field macros instead.

## Example

```cpp
using key = meta::type_value<meta::static_string{"verbose"}>;
static_assert(key::value == "verbose");
```
