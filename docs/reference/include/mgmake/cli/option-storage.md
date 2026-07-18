# `include/mgmake/cli/option_storage.hxx`

**Source:** `include/mgmake/cli/option_storage.hxx`  
**Status:** Public runtime storage generated from an option type list.

`option_storage` turns compile-time option metadata into a strongly typed runtime dictionary whose keys remain compile-time strings.

## Type construction

```cpp
template<typename opts_t = meta::type_list<>>
struct option_storage;
```

`list_type` preserves the complete option list for parsing and help generation.

`storage_map_type` folds over that list. Each option with `has_storage == true` contributes its `storage_pair`. Keys are compile-time `meta::type_value<meta::static_string>` types; values are ordinary runtime types.

When two options declare the same key:

- Identical value types are accepted and only one map entry is retained.
- Different value types trigger a compile-time assertion.

A `void` value type is currently treated as a deferred declaration and skipped, with a TODO to verify that another option eventually defines the key.

## Runtime representation

`storage_type` is `meta::static_dict<storage_map_type>`, which stores the values in a `std::tuple`. All values are default-constructed.

Access by key:

```cpp
opts.get<"verbose">();
opts.set<"verbose">(true);
```

Access by option type:

```cpp
opts.get<cli::build_dir_option>();
```

The option-type overload resolves `opt_t::storage_key()` and asserts that the key exists.

## Compile-time query

```cpp
static_assert(option_storage_t::has<"task">());
```

`has` is `consteval`, so it is intended for compile-time branching and validation.

## Design consequence

Storage contains values, not “was supplied” state. A default-initialized value can be indistinguishable from an explicitly supplied value unless the stored type models presence itself, such as `std::optional<T>`.
