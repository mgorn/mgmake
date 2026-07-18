# `include/mgmake/meta/static_string.hxx`

**Source:** `include/mgmake/meta/static_string.hxx`  
**Status:** Core structural compile-time string type.

`static_string<N>` stores a null-terminated character array in a structural type suitable for non-type template parameters.

## Construction and access

String literals convert implicitly:

```cpp
template<mgmake::meta::static_string value>
struct named {};

using example = named<"hello">;
```

The literal's array extent becomes `N`. `size()` returns `N - 1`, excluding the terminator. `view()` and the conversion operator provide a `std::string_view` over the characters.

The default constructor leaves the array zero-initialized.

## Comparison and concatenation

`operator==` compares extents first, then every stored character including the terminator.

`operator+` produces a new `static_string<N1 + N2 - 1>`, copying the first string without its terminator and then the complete second string.

```cpp
constexpr auto name = meta::static_string{"hello "}
                    + meta::static_string{"world"};
```

## Hashing

`hash<hash_t>()` implements FNV-1a-style hashing. It selects 32-bit constants for integer types of four bytes or less and 64-bit constants otherwise. `std::hash<static_string<N>>` delegates to the default `std::size_t` hash.

## Structural-type role

Public `m_data` storage and value semantics make the type usable as an NTTP. It is used for builder keys, option names, target names, source paths, and dictionary keys.

## Header dependencies

The implementation uses `std::unsigned_integral` and specializes `std::hash`, but the header currently does not directly include `<concepts>` or `<functional>`. It works in the present umbrella through transitive includes; standalone inclusion should add those direct dependencies in the source.
