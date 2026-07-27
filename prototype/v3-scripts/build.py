#!/usr/bin/env python3
from pathlib import Path

from mgmake.cli import main
from project import define_project


ROOT = Path(__file__).resolve().parent


if __name__ == "__main__":
    raise SystemExit(main(define_project(ROOT), ROOT))
