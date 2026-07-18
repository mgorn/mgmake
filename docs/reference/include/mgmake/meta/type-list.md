# `include/mgmake/meta/type_list.hxx`

**Source:** `include/mgmake/meta/type_list.hxx`  
**Status:** Core compile-time type-sequence and runtime type-dispatch utility.

`type_list<...>` names a variadic type pack and provides algorithms for querying, transforming, folding, sorting, and runtime-selecting its elements.

## Queries

```cpp
list::size()
list::type_at<I>
list::count<T>()
list::has<T>()
list::unique<T>()
list::index<T>()
```

`index<T>()` requires exactly one match and returns its zero-based position.

## Construction

The list can append or prepend individual types, packs, or another `type_list`:

```cpp
append<T>
append_types<A, B>
append_list<other_list>
prepend<T>
```

Unique variants either skip duplicates or, for `append_unique`/`prepend_unique`, optionally assert that the requested type was absent.

## `apply`

```cpp
template<template<typename...> typename pack_t>
using apply = pack_t<types_t...>;
```

This rebinds the stored pack to another variadic template. `static_dict` uses it to create a `std::tuple` of value types.

## `fold`

`fold<operation, initial_t>` processes types left to right. The operation is a compile-time callable of the form:

```cpp
[]<typename state_t, typename type_t>() consteval {
    return std::type_identity<next_state_t>{};
}
```

Both `filter` and `sort` are built on this primitive.

## `filter` and `sort`

`filter<predicate>` retains types for which `predicate.operator()<T>()` is true.

`sort<compare>` performs insertion sorting. The comparator returns true when its left type should precede its right type.

## Runtime `type_switch`

```cpp
list::type_switch(callable, index);
```

This creates a static dispatch table with one function per stored type and invokes:

```cpp
callable.template operator()<selected_type>()
```

All instantiations must return exactly the same type. The list must be non-empty, and the runtime index must be in range.

The CLI parser and task dispatcher use `type_switch` to convert a parsed numeric index back into compile-time option or task behavior.
