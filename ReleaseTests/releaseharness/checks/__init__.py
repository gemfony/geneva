"""Individual release checks, grouped by the sections of ReleaseTestplan.md.

Each module exposes functions that take a :class:`~releaseharness.runner.JobContext`
and return one or more :class:`~releaseharness.model.CheckResult` objects.
"""

from . import (algorithms, benchmarks, build, consumers, ctest, examples,
               install, metadata, outoftree)

__all__ = [
    "build", "ctest", "consumers", "algorithms",
    "examples", "benchmarks", "install", "outoftree", "metadata",
]
