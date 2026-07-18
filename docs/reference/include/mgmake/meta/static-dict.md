# `include/mgmake/meta/static_dict.hxx`

**Source:** `include/mgmake/meta/static_dict.hxx`  
**Status:** Core typed runtime storage with compile-time string keys.

`static_dict` combines a compile-time `type_map` schema with a runtime `std::tuple` of values.

## Schema

The expected map shape is:

```cpp
meta::type_map<
    meta::type_list<
        meta::type_pair<
            meta::type_value<key>,
            value_type
        >
    >
>
```

Each key is a `meta::static_string` wrapped in `meta::type_value`. Each value is an ordinary default-constructible runtime type.

The class exposes the original map, pair list, key list, value list, and tuple `storage_type`.

## Queries and access

```cpp
static_assert(dict_t::has<"name">());
constexpr auto index = dict_t::key_index<"name">();

dict.get<"name">();
dict.set<"name">(value);
```

`key_index` fails at compile time for a missing key. `get` and the internal `storage` overloads preserve references with `decltype(auto)`.

`set` verifies assignability at compile time and forwards the supplied value into tuple assignment.

## Validation

A private fold instantiates `entry_check` for every map pair. It verifies:

- the key has exactly the required `type_value<static_string>` form;
- each value type is default-constructible.

`type_map` separately guarantees unique key types.

## Runtime layout

Values are stored in map order inside a `std::tuple`. Lookup has no runtime string search: the key is resolved to a tuple index during compilation.

## Use in mgmake

`cli::option_storage` constructs a static dictionary schema from all configured options and uses it to hold parsed option values.
