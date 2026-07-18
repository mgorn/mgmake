# `include/mgmake/meta/type_or.hxx`

**Source:** `include/mgmake/meta/type_or.hxx`  
**Status:** Small compile-time fallback utility.

`type_or` selects a default type when an input type equals a configurable sentinel type.

## Interface

```cpp
template<
    typename type_t,
    typename default_t,
    typename none_t = void
>
struct type_or;

template<typename type_t, typename default_t, typename none_t = void>
using type_or_t = typename type_or<type_t, default_t, none_t>::type;
```

When `type_t` and `none_t` are the same type, the result is `default_t`; otherwise the result is `type_t`.

The implementation uses a `consteval` lambda returning `std::type_identity` so only the selected branch needs to form its result type.

## Validation

`default_t` may not equal `none_t`, because such a fallback would not change the outcome.

## Uses

Builders often query a missing key with `type_map::at<key, false>`, which yields `void`, then use `type_or_t` to supply an empty list or default configuration:

```cpp
using targets = meta::type_or_t<
    typename builder_t::template get<"targets", false>,
    meta::type_list<>
>;
```
