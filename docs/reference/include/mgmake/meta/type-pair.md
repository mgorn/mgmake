# `include/mgmake/meta/type_pair.hxx`

**Source:** `include/mgmake/meta/type_pair.hxx`  
**Status:** Minimal compile-time key/value pair.

`type_pair<key_t, value_t>` is the storage entry used by `meta::type_map`.

## Interface

```cpp
template<typename key_t, typename value_t>
struct type_pair {
    using key_type = key_t;
    using value_type = value_t;
};
```

It has no runtime state and performs no validation itself. `type_map` validates key uniqueness, while higher-level users such as `static_dict` validate the form of keys and properties of values.

## Example

```cpp
using name_key = meta::type_value<meta::static_string{"name"}>;
using entry = meta::type_pair<name_key, std::string>;
```
