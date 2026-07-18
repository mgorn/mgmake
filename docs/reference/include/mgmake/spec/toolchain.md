# `include/mgmake/spec/toolchain.hxx`

**Source:** `include/mgmake/spec/toolchain.hxx`  
**Status:** Experimental toolchain builder; not exported and missing direct dependencies.

This file sketches a simple named toolchain whose compiler, archiver, and linker programs are represented by compile-time strings.

## Intended fields

```cpp
toolchain::name<"...">
         ::cc<"...">
         ::cxx<"...">
         ::ar<"...">
         ::linker<"...">
         ::build
```

Every field is a `meta::static_string` value stored through `meta::type_builder`.

## Finalized type

`toolchain_impl<storage_t>` is currently empty, so finalized fields are not exposed and no validation is performed.

## Current integration status

The header does not include the meta headers it directly uses. It only compiles when those declarations have already been made available transitively. It is also absent from the public umbrella and generated single-header file.

The discovery subsystem is evolving toward role-based tools, so this direct string-based toolchain schema may be replaced or become a higher-level selection layer.
