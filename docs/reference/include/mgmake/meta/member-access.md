# `include/mgmake/meta/member_access.hxx`

**Source:** `include/mgmake/meta/member_access.hxx`  
**Status:** Low-level public metaprogramming utility.

`member_access` binds a pointer to a non-static data member into a type that can uniformly read or assign that member.

## Valid specialization

```cpp
template<auto member_ptr>
struct member_access;
```

For a non-null member-object pointer, the type exposes:

- `pointer_type`
- `traits`
- `class_type`
- `value_type`
- `pointer`
- `valid == true`

`get(object)` uses `std::invoke` and preserves the member expression's reference category through `decltype(auto)`.

`set(object, value)` is constrained so the member expression must be assignable from the supplied value.

Both operations propagate conditional `noexcept` based on the corresponding invocation and assignment traits.

## Null specialization

```cpp
member_access<nullptr>
```

Only exposes `valid == false`. This makes `nullptr` useful as a sentinel without instantiating the member-pointer assertions.

## Example

```cpp
struct settings {
    bool verbose{};
};

using verbose_access =
    mgmake::meta::member_access<&settings::verbose>;

settings value;
verbose_access::set(value, true);
bool& ref = verbose_access::get(value);
```

The current CLI storage design uses string keys instead of member access, but this utility remains available for APIs that bind configuration directly to object members.
