#!/usr/bin/env python3

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
INCLUDE_DIR = ROOT / "include"
INPUT = INCLUDE_DIR / "mgmake" / "mgmake.hxx"
OUTPUT = ROOT / "mgmake.hxx"

include_pattern = re.compile(r'^\s*#\s*include\s+([<"])([^>"]+)[>"]')

visited: set[Path] = set()


def resolve_project_include(name: str, current_file: Path) -> Path | None:
    candidates = []

    if name.startswith("mgmake/"):
        candidates.append(INCLUDE_DIR / name)

    candidates.append(current_file.parent / name)

    for candidate in candidates:
        candidate = candidate.resolve()
        try:
            candidate.relative_to(INCLUDE_DIR.resolve())
        except ValueError:
            continue

        if candidate.exists():
            return candidate

    return None


def emit_file(path: Path, out: list[str]) -> None:
    path = path.resolve()

    if path in visited:
        out.append(f"// skipped duplicate include: {path.relative_to(ROOT)}\n")
        return

    visited.add(path)

    out.append("\n")
    out.append(f"// ===== begin {path.relative_to(ROOT)} =====\n")

    for line in path.read_text().splitlines(keepends=True):
        match = include_pattern.match(line)

        if not match:
            out.append(line)
            continue

        delimiter, include_name = match.groups()

        project_header = resolve_project_include(include_name, path)

        if project_header is None:
            out.append(line)
            continue

        emit_file(project_header, out)

    out.append(f"// ===== end {path.relative_to(ROOT)} =====\n")
    out.append("\n")


def main() -> int:
    out: list[str] = []

    out.append("// This file is generated. Do not edit directly.\n")
    out.append("// Source: include/mgmake/mgmake.hxx\n\n")
    out.append("#ifndef MGMAKE_SINGLE_HEADER_HXX\n")
    out.append("#define MGMAKE_SINGLE_HEADER_HXX\n\n")

    emit_file(INPUT, out)

    out.append("\n#endif // MGMAKE_SINGLE_HEADER_HXX\n")

    OUTPUT.write_text("".join(out))
    print(f"wrote {OUTPUT.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())