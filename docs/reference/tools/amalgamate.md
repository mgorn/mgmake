# `tools/amalgamate.py`

**Source:** `tools/amalgamate.py`  
**Status:** Maintainer tool for generating the root single-header distribution.

This Python script recursively expands mgmake's project-local includes, beginning at the modular umbrella header.

## Paths

The script derives paths relative to its own location:

- project root: parent of `tools/`
- include root: `include/`
- input: `include/mgmake/mgmake.hxx`
- output: `mgmake.hxx`

## Include recognition

A regular expression recognizes preprocessor include lines with either quotes or angle brackets. `resolve_project_include` considers:

1. `include/<name>` when the include begins with `mgmake/`;
2. a path relative to the current file.

A candidate is expanded only when its resolved path remains under the include directory and exists. Standard-library and other external includes are copied unchanged.

## Duplicate handling

A process-global `visited` set tracks resolved header paths. The first include emits the full file; subsequent includes emit a comment instead. This prevents recursive duplication while preserving evidence of the dependency.

## Output structure

The script writes:

- a generated-file warning;
- a top-level include guard;
- begin/end comments around each source header;
- the expanded umbrella;
- the closing guard.

## Running

```sh
python3 tools/amalgamate.py
```

The script overwrites `mgmake.hxx` and prints the relative output path.

## Maintenance considerations

`visited` is global and is not cleared inside `main`. That is harmless for normal one-shot command execution, but calling `main()` twice in the same Python process would produce an incomplete second output unless the set were reset.

The script preserves each source header's own `#pragma once` and include guards. They are redundant inside the amalgamation but harmless.
