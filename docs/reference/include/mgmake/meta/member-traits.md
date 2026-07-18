# `include/mgmake/meta/member_traits.hxx`

**Source:** `include/mgmake/meta/member_traits.hxx`  
**Status:** Low-level public metaprogramming utility.

This header extracts the declaring class, member type, return type, argument list, qualifiers, and `noexcept` state from member pointers.

## Member objects

For `member_t class_t::*`, `member_traits` exposes:

```cpp
using class_type = class_t;
using member_type = member_t;
static constexpr bool is_function = false;
static constexpr bool is_object = true;
```

Function types are excluded from this specialization.

## Member functions

Member-function pointers delegate to `member_function_traits`. A macro generates specializations for all combinations of:

- no cv qualifier, `const`, `volatile`, and `const volatile`;
- no ref qualifier, `&`, and `&&`;
- `noexcept(true)` and `noexcept(false)` through a deduced boolean.

Each specialization exposes:

- `class_type`
- `member_type` as the reconstructed function type
- `return_type`
- `arg_types` as `meta::type_list<...>`
- `is_function`
- `is_object`
- `is_noexcept`

## Example

```cpp
struct object {
    int read(double) const noexcept;
};

using traits =
    mgmake::meta::member_traits<decltype(&object::read)>;

static_assert(std::same_as<traits::return_type, int>);
static_assert(traits::arg_types::size() == 1);
static_assert(traits::is_noexcept);
```

Static and free functions are outside this trait's scope because they are not member pointers.
