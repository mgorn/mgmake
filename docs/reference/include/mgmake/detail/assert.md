# `include/mgmake/detail/assert.hxx`

**Source:** `include/mgmake/detail/assert.hxx`  
**Status:** Internal assertion utilities used by runtime and constant-evaluation code.

This header provides mgmake's invariant-checking macros and diagnostic path.

## Runtime failure

`detail::assertion_failed` prints:

- the failed condition text,
- the caller-provided message,
- source file and line,
- function name,

then calls `std::abort()`.

`mgmk_assert_impl` is the small runtime wrapper used by the macro.

## Constant evaluation

When an enabled assertion fails in a constant-evaluated context, `mgmkassert` calls the `consteval` `constexpr_assert_failed` function. That function throws an internal marker type, causing constant evaluation to fail and producing a compile-time diagnostic.

## Macros

### `mgmkstaticassert(condition, message)`

A spelling wrapper around `static_assert`.

### `mgmkassert(condition, message)`

Enabled when `MGMK_ENABLE_ASSERTS` is nonzero. If the user does not define it, assertions are enabled unless `NDEBUG` is defined.

In disabled builds, the condition is placed in `sizeof` so it is type-checked but not evaluated. Side effects in assertion conditions must therefore never be required for correctness.

## Intended use

`mgmkassert` is for internal preconditions and impossible states, not recoverable user input. Command-line errors should use `std::expected` so the entry point can report them without aborting.
