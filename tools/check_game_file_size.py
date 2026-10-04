#!/usr/bin/env python3
"""Enforce the 450-line limit for maintained native game source files."""

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
MAX_LINES = 450
ROOT_SOURCES = {"graphics.c", "main.c", "runtime.c", "runtime.h", "ttf_font.c"}
SOURCE_SUFFIXES = {".c", ".h", ".inc", ".xml"}


def game_sources():
    for name in sorted(ROOT_SOURCES):
        yield ROOT / name
    for directory in (ROOT / "game", ROOT / "native"):
        for path in sorted(directory.rglob("*")):
            if path.is_file() and path.suffix.lower() in SOURCE_SUFFIXES:
                yield path


def line_count(path: Path) -> int:
    with path.open("rb") as source:
        return sum(1 for _ in source)


def main() -> int:
    oversized = []
    for path in game_sources():
        count = line_count(path)
        if count > MAX_LINES:
            oversized.append((path.relative_to(ROOT), count))
    if oversized:
        print(f"Game source files must not exceed {MAX_LINES} lines:", file=sys.stderr)
        for path, count in oversized:
            print(f"  {path}: {count}", file=sys.stderr)
        return 1
    print(f"Game source size check: ok (maximum {MAX_LINES} lines)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
