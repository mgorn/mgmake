# `include/mgmake/meta/type_map.hxx`

**Source:** `include/mgmake/meta/type_map.hxx`  
**Status:** Core compile-time map from types to types.

`type_map` stores a `type_list` of `type_pair<key_t, value_t>` entries and enforces unique key types.

## Shape

```cpp
using map = meta::type_map<
    meta::type_list<
        meta::type_pair<key_a, value_a>,
        meta::type_pair<key_b, value_b>
    >
>;
```

The default map is empty.

Exposed aliases:

- `storage_type`
- `key_types`
- `value_types`

## Lookup

```cpp
map::size()
map::key_index<key_t>()
map::has<key_t>()
map::at<key_t>
map::at<key_t, false>
```

`key_index` returns `size()` for a missing key. `at` normally asserts that the key exists; with `check == false`, a missing key resolves to `void`.

## Mutation by type

The map is immutable at runtime; mutation aliases produce new map types:

- `emplace<key_t, value_t>` replaces an existing value or appends a new pair.
- `emplace_unique<key_t, value_t>` requires the key to be absent.

This replacement behavior is what allows later fluent-builder calls to override earlier fields.

## Key identity

Keys are compared with `std::same_as`. String-keyed users wrap `meta::static_string` values in `meta::type_value`, turning each compile-time string value into a distinct key type.
