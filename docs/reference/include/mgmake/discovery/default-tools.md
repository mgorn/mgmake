# `include/mgmake/discovery/default_tools.hxx`

**Source:** `include/mgmake/discovery/default_tools.hxx`  
**Status:** Experimental discovery declarations; not exported by the public umbrella.

This header sketches logical tool declarations for Clang and assigns compiler roles to them.

## `default_tools`

The struct begins with a logical tool builder:

```cpp
using clang = tool::logical<"clang">;
```

It then intends to build role-specific tool types:

```cpp
using clang_c   = clang::role<cc_role>::build;
using clang_cxx = clang::role<cxx_role>::build;
```

The model separates a logical executable family (`clang`) from the role in which it is used (`cc_role` or `cxx_role`). This would allow one discovered program to satisfy multiple tool roles while retaining role-specific configuration.

## Current status

The discovery subsystem is not included by `include/mgmake/mgmake.hxx`, and the current `tool.hxx` role-field macro declaration is inconsistent with the macro API. Treat these aliases as design scaffolding rather than a working public interface.
