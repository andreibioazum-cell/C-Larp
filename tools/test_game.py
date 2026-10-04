#!/usr/bin/env python3
"""Compile and run the pure-C menu/arena/punch flow on the host."""

from __future__ import annotations

import os
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main() -> int:
    compiler = os.environ.get("CC", "cc")
    with tempfile.TemporaryDirectory(prefix="cb4-test-") as directory:
        executable = Path(directory) / "test_game"
        subprocess.run(
            [
                compiler,
                "-std=c99",
                "-Wall",
                "-Wextra",
                "-Werror",
                "-pedantic",
                "-I",
                str(ROOT),
                "-I",
                str(ROOT / "tools/host_test/stub"),
                str(ROOT / "tools/test_game.c"),
                str(ROOT / "game/game.c"),
                "-lm",
                "-o",
                str(executable),
            ],
            check=True,
            cwd=ROOT,
        )
        subprocess.run([str(executable)], check=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
