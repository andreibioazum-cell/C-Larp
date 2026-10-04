#!/usr/bin/env python3
"""Copy game assets into the directory packaged by aapt."""

from __future__ import annotations

import argparse
import shutil
import sys
from pathlib import Path


def stage_assets(source: Path, destination: Path) -> list[Path]:
    source = source.resolve()
    destination = destination.resolve()
    if not source.is_dir():
        raise ValueError(f"asset directory not found: {source}")
    if source == destination or source in destination.parents or destination in source.parents:
        raise ValueError("the source and destination directories must not overlap")

    shutil.rmtree(destination, ignore_errors=True)
    destination.mkdir(parents=True)
    staged: list[Path] = []
    for path in sorted(source.rglob("*")):
        relative = path.relative_to(source)
        target = destination / relative
        if path.is_symlink():
            raise ValueError(f"asset symlinks are not supported: {relative}")
        if path.is_dir():
            target.mkdir(parents=True, exist_ok=True)
        else:
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(path, target)
            staged.append(relative)
    return staged


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", nargs="?", default="game/assets")
    parser.add_argument("destination", nargs="?", default="staging/assets")
    args = parser.parse_args(argv)
    try:
        staged = stage_assets(Path(args.source), Path(args.destination))
    except (OSError, ValueError) as error:
        print(f"Error: {error}", file=sys.stderr)
        return 1
    print(f"Staged {len(staged)} asset(s) in {args.destination}")
    for path in staged:
        print(f"  {path.as_posix()}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
