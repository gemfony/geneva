"""Test checks (ReleaseTestplan.md: "Unit and integration tests").

Runs the CTest suite and the GenevaStandardTests executable directly, and
scans for unexpected skips. The thread-sanitizer run is gated on the build
type being Sanitize and is LONG-tier.
"""

from __future__ import annotations

import re
import time

from ..model import BuildType, CheckResult, Status, Tier
from ..runner import GUEST_BUILD, JobContext

_CTEST_SUMMARY = re.compile(r"(\d+)% tests passed,\s*(\d+)\s+tests failed out of\s*(\d+)")


def run_ctest(ctx: JobContext) -> CheckResult:
    """Run the full CTest suite; PASS only on zero failures."""
    if not ctx.job.spec.build_tests:
        return ctx.skipped("test/ctest", Tier.SHORT, "tests not built (minimal)")
    # Sanitize runs are slow -> LONG; otherwise SHORT.
    tier = Tier.LONG if ctx.job.spec.build_type is BuildType.SANITIZE else Tier.SHORT
    if not ctx.should_run(tier):
        return ctx.skipped("test/ctest", tier, "LONG tier skipped in --quick")

    started = time.monotonic()
    res = ctx.exec_in_guest(["bash", "-lc", "ctest --output-on-failure"],
                            workdir=GUEST_BUILD, timeout=7200)
    result = ctx.record("test/ctest", tier, res, started=started)
    m = _CTEST_SUMMARY.search(res.stdout + res.stderr)
    if m and not ctx.dry_run:
        failed = int(m.group(2))
        total = int(m.group(3))
        result.message = f"{total - failed}/{total} passed"
        result.status = Status.PASS if failed == 0 else Status.FAIL
    return result


def run_standard_tests(ctx: JobContext) -> CheckResult:
    """Run GenevaStandardTests directly and flag unexpected skips."""
    if not ctx.job.spec.build_tests:
        return ctx.skipped("test/standard", Tier.SHORT, "tests not built (minimal)")
    tier = Tier.SHORT
    if not ctx.should_run(tier):
        return ctx.skipped("test/standard", tier, "skipped")
    started = time.monotonic()
    res = ctx.exec_in_guest(
        ["bash", "-lc", "./geneva/tests/UnitTests/GenevaStandardTests"],
        workdir=GUEST_BUILD, timeout=3600,
    )
    result = ctx.record("test/standard", tier, res, started=started)
    if result.status is Status.PASS:
        lower = (res.stdout + res.stderr).lower()
        if "skipped" in lower:
            result.message = "completed with skipped cases (review log)"
    return result
