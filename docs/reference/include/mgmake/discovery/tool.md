# `include/mgmake/discovery/tool.hxx`

**Source:** `include/mgmake/discovery/tool.hxx`  
**Status:** Experimental compile-time tool builder; not currently standalone-compilable.

This header is intended to represent one logical discovered tool and the role for which it is selected.

## Intended fields

`tool_builder` defines:

- `logical<"...">`: the logical executable name, such as `clang`.
- `role<...>`: the tool role, such as `cxx_role`.
- `build`: finalize as `tool_impl<storage_t>`.

`tool_impl` is currently empty, so finalized fields are not yet exposed through consumer aliases.

## Current source issue

The role declaration currently invokes:

```cpp
MGMAKE_TYPE_BUILDER_TYPE_FIELD(tool_builder, role, meta::static_string);
```

`MGMAKE_TYPE_BUILDER_TYPE_FIELD` accepts only the wrapper and alias names. A type field should be declared with two arguments, while a value field would use the three-argument value macro. The uses in `default_tools.hxx` pass a role type, which indicates this should be a type field.

Until that declaration is corrected and `tool_impl` consumes its fields, this header is an unfinished schema rather than usable API.
