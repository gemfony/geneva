"""Out-of-tree application build (ReleaseTestplan.md: "Out-of-tree application
build") — STUBS.

Depends on a completed install; LONG and left as structured stubs.
"""

from __future__ import annotations

from ..model import CheckResult, Tier
from ..runner import JobContext


def build_against_install(ctx: JobContext) -> CheckResult:
    """Build a tiny app against the installed tree using FindGeneva.

    Intended flow: a minimal CMake project with `find_package(Geneva)` pointing
    at the install prefix, configure + build, run.
    """
    return ctx.skipped(
        "outoftree/findgeneva", Tier.LONG,
        "STUB: build a sample app against the installed tree via FindGeneva (TODO)",
    )


def build_tests_mismatch(ctx: JobContext) -> CheckResult:
    """Confirm a GENEVA_BUILD_TESTS mismatch is caught with a useful error.

    Intended flow: install with tests OFF, then configure the sample app
    expecting tests ON (or vice versa); assert the FindGeneva error fires.
    """
    return ctx.skipped(
        "outoftree/build-tests-mismatch", Tier.LONG,
        "STUB: assert GENEVA_BUILD_TESTS mismatch produces a useful error (TODO)",
    )
