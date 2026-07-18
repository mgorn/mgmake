# `include/mgmake/meta/type_builder.hxx`

**Source:** `include/mgmake/meta/type_builder.hxx`  
**Status:** Core fluent type-building framework and field macros.

`type_builder` stores named compile-time fields in a `type_map` and finalizes them into a consumer template.

## `type_builder`

```cpp
template<typename storage_t = type_map<>>
struct type_builder;
```

Operations:

- `get<key, check>` returns the type stored under a compile-time string key.
- `set<key, value_t>` returns a new builder whose map contains or replaces that key.
- `build<consumer_t>` instantiates `consumer_t<storage_t>`.

Values are represented as types with `meta::type_value`.

## Builder-field macros

### Type fields

```cpp
MGMAKE_TYPE_BUILDER_TYPE_FIELD(wrapper_t, field)
MGMAKE_TYPE_BUILDER_TYPE_FIELD_AS(wrapper_t, field, key)
```

These generate `template<typename field_t> using field = ...`. The `_AS` form decouples the visible alias from the stored key.

### Value fields

```cpp
MGMAKE_TYPE_BUILDER_VALUE_FIELD(wrapper_t, field, value_type)
MGMAKE_TYPE_BUILDER_VALUE_FIELD_AS(wrapper_t, field, value_type, key)
```

These generate an intermediate type field and a value-taking alias that wraps the NTTP in `type_value`.

## Consumer-field macros

Type consumers expose a resolved `<field>` alias and an intermediate `<field>_type`. Missing entries fall back to the supplied default type.

Value consumers use the type consumer internally and expose `<field>_value`, falling back to `type_value<default>`.

## Minimal pattern

```cpp
template<typename storage_t = meta::type_map<>>
struct item_impl {
    MGMAKE_TYPE_CONSUMER_VALUE_FIELD(name, meta::static_string{""});
};

template<typename builder_t = meta::type_builder<>>
struct item_builder {
    MGMAKE_TYPE_BUILDER_VALUE_FIELD(item_builder, name, meta::static_string);
    using build = typename builder_t::template build<item_impl>;
};

using item = item_builder<>;
using configured = item::name<"example">::build;
```

## Replacement semantics

`type_builder::set` delegates to `type_map::emplace`, so setting the same key again replaces the previous value type rather than creating a duplicate.
