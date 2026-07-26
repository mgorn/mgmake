#!/usr/bin/env python3

from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
INCLUDE_DIR = ROOT / "include"
INPUT = INCLUDE_DIR / "mgmake" / "mgmake.hxx"
OUTPUT = ROOT / "mgmake.hxx"

include_pattern = re.compile(r'^\s*#\s*include\s+([<"])([^>"]+)[>"]')
version_define_pattern = re.compile(
    r'^\s*#\s*define\s+(MGMK_VERSION_COMMIT|MGMK_VERSION_DIRTY|MGMK_VERSION)\b'
)

visited: set[Path] = set()


def display_path(path: Path) -> str:
    return path.relative_to(ROOT).as_posix()


def run_git(*args: str) -> str:
    result = subprocess.run(
        ["git", "-C", str(ROOT), *args],
        capture_output=True,
        text=True,
        check=False,
    )

    if result.returncode != 0:
        message = result.stderr.strip() or result.stdout.strip() or "unknown Git error"
        raise RuntimeError(f"Git command failed: {message}")

    return result.stdout.strip()


def read_define_value(path: Path, name: str) -> str:
    pattern = re.compile(rf'^\s*#\s*define\s+{re.escape(name)}\s+(.+?)\s*$')

    for line in path.read_text().splitlines():
        match = pattern.match(line)

        if match:
            return match.group(1)

    raise RuntimeError(f"{display_path(path)} does not define {name}")


def make_version_defines() -> dict[str, str]:
    commit = run_git("rev-parse", "--short=7", "HEAD")
    dirty = bool(run_git("status", "--porcelain", "--untracked-files=normal"))

    return {
        "MGMK_VERSION": read_define_value(INPUT, "MGMK_VERSION"),
        "MGMK_VERSION_COMMIT": f'"{commit}"',
        "MGMK_VERSION_DIRTY": "true" if dirty else "false",
    }


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


def emit_file(path: Path, out: list[str], version_defines: dict[str, str]) -> None:
    path = path.resolve()

    if path in visited:
        out.append(f"// skipped duplicate include: {display_path(path)}\n")
        return

    visited.add(path)

    out.append("\n")
    out.append(f"// ===== begin {display_path(path)} =====\n")

    for line in path.read_text().splitlines(keepends=True):
        if path == INPUT:
            define_match = version_define_pattern.match(line)

            if define_match:
                name = define_match.group(1)
                out.append(f"#define {name} {version_defines[name]}\n")
                continue

        include_match = include_pattern.match(line)

        if not include_match:
            out.append(line)
            continue

        _, include_name = include_match.groups()
        project_header = resolve_project_include(include_name, path)

        if project_header is None:
            out.append(line)
            continue

        emit_file(project_header, out, version_defines)

    out.append(f"// ===== end {display_path(path)} =====\n")
    out.append("\n")


def main() -> int:
    try:
        version_defines = make_version_defines()
    except RuntimeError as error:
        print(f"error: {error}", file=sys.stderr)
        return 1

    out: list[str] = []

    out.append("// This file is generated. Do not edit directly.\n")
    out.append("// Source: include/mgmake/mgmake.hxx\n\n")
    out.append("#ifndef MGMAKE_SINGLE_HEADER_HXX\n")
    out.append("#define MGMAKE_SINGLE_HEADER_HXX\n\n")

    emit_file(INPUT, out, version_defines)

    out.append("\n#endif // MGMAKE_SINGLE_HEADER_HXX\n")

    OUTPUT.write_text("".join(out))

    print(f"wrote {OUTPUT.relative_to(ROOT)}")
    print(f"version: {version_defines['MGMK_VERSION']}")
    print(f"commit: {version_defines['MGMK_VERSION_COMMIT']}")
    print(f"dirty: {version_defines['MGMK_VERSION_DIRTY']}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())