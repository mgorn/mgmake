# `include/mgmake/detail/index_bit.hxx`

**Source:** `include/mgmake/detail/index_bit.hxx`  
**Status:** Internal bitset helper.

`detail::index_bit` returns the position of the only set bit in a `std::bitset<N>`.

## Contract

```cpp
template<std::size_t N>
constexpr std::size_t index_bit(std::bitset<N> bits) noexcept;
```

Requirements:

- `N` must be greater than zero.
- Exactly one bit must be set.

The second requirement is enforced with `mgmkassert`.

## Algorithm

The function processes the bitset in `unsigned long long`-sized chunks:

1. Mask the lowest chunk.
2. If that chunk has a set bit, convert it with `to_ullong`.
3. Use `std::countr_zero` to locate the bit within the chunk.
4. Add the accumulated chunk offset.
5. Otherwise shift the bitset and continue.

Masking before `to_ullong` prevents overflow when `N` exceeds the width of `unsigned long long`.

## Uses

The CLI parser uses this helper after asserting that an argument matched exactly one option. It converts a match bitset into the runtime index consumed by `type_list::type_switch`.
