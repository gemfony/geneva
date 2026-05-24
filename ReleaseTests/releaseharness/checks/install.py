"""Install / runtime-linking checks (ReleaseTestplan.md: "Installation and
runtime linking") — STUBS.

These require a completed build and are side-effecting (they write under an
install prefix), so they are LONG and left as structured stubs with the
intended command flow.
"""

from __future__ import annotations

from ..model import CheckResult, Tier
from ..runner import JobContext


def make_install(ctx: JobContext) -> CheckResult:
    """`make install` into a prefix and verify headers/libs/cmake config land.

    Intended flow:
      make install (prefix under /work/build or a dedicated mount), then
      check $PREFIX/include/geneva, $PREFIX/lib/libgeneva*, and the exported
      CMake package config exist.
    """
    return ctx.skipped(
        "install/make-install", Tier.LONG,
        "STUB: make install + verify layout under prefix (TODO)",
    )


def ldd_check(ctx: JobContext) -> CheckResult:
    """ldd the main executables for unresolved symbols after ldconfig/LD path."""
    return ctx.skipped(
        "install/ldd", Tier.LONG,
        "STUB: ldconfig/LD_LIBRARY_PATH + `ldd` unresolved-symbol scan (TODO)",
    )
