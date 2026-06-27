"""Logging helpers: a console logger plus per-job log files in the work area."""

from __future__ import annotations

import logging
import sys
from pathlib import Path

_CONSOLE_NAME = "geneva.release"


def get_console_logger(verbose: bool = False) -> logging.Logger:
    logger = logging.getLogger(_CONSOLE_NAME)
    if logger.handlers:
        return logger
    logger.setLevel(logging.DEBUG if verbose else logging.INFO)
    handler = logging.StreamHandler(sys.stderr)
    handler.setFormatter(logging.Formatter("%(asctime)s %(levelname)-7s %(message)s",
                                            datefmt="%H:%M:%S"))
    logger.addHandler(handler)
    return logger


def ensure_workdir(workdir: Path) -> dict[str, Path]:
    """Create the standard sub-directories of the work area; return their paths."""
    layout = {
        "root": workdir,
        "logs": workdir / "logs",
        "builds": workdir / "builds",
        "images": workdir / "images",
        "reports": workdir / "reports",
    }
    for p in layout.values():
        p.mkdir(parents=True, exist_ok=True)
    return layout


def job_log_path(logs_dir: Path, job_slug: str, check: str) -> Path:
    safe = check.replace("/", "_").replace(" ", "_")
    return logs_dir / f"{job_slug}__{safe}.log"
