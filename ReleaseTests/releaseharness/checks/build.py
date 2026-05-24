"""Build checks (ReleaseTestplan.md: "Build system").

Drives an out-of-source build inside the guest. Two strategies:

1. ``prepareBuild.sh`` driven by a generated ``.gcfg`` (primary; matches how
   releases are actually built).
2. a direct ``cmake`` invocation (fallback / used implicitly via the same flow).

Targets exercised: ``make core``, ``make gemfony-build-all``, ``make doc``.
"""

from __future__ import annotations

import time

from ..model import BuildType, CheckResult, Tier
from ..runner import GUEST_BUILD, GUEST_SRC, JobContext

# The .gcfg is written into the writable build dir so prepareBuild.sh finds it.
_GCFG_GUEST_PATH = f"{GUEST_BUILD}/release.gcfg"


def _tier_for(ctx: JobContext) -> Tier:
    # Sanitize / RelWithDebInfo builds are slow -> LONG. Debug/Release -> SHORT.
    if ctx.job.spec.build_type in (BuildType.SANITIZE, BuildType.REL_WITH_DEB_INFO):
        return Tier.LONG
    return Tier.SHORT


def _write_gcfg(ctx: JobContext) -> str:
    """Produce the shell snippet that writes the .gcfg inside the guest.

    The .gcfg content is generated from the BuildSpec so it is fully determined
    by the matrix cell.
    """
    body = ctx.job.spec.gcfg_text()
    # Use a heredoc so quoting survives the container boundary.
    return f"cat > {_GCFG_GUEST_PATH} <<'GCFG'\n{body}GCFG\n"


def configure_and_build(ctx: JobContext) -> list[CheckResult]:
    """Configure with prepareBuild.sh, then build core and gemfony-build-all."""
    tier = _tier_for(ctx)
    results: list[CheckResult] = []
    if not ctx.should_run(tier):
        return [ctx.skipped("build", tier, "LONG tier skipped in --quick")]

    # Step 1: configure via prepareBuild.sh using the generated .gcfg.
    started = time.monotonic()
    configure_cmd = (
        _write_gcfg(ctx)
        + f"{GUEST_SRC}/scripts/prepareBuild.sh -y {_GCFG_GUEST_PATH}"
    )
    res = ctx.exec_in_guest(["bash", "-lc", configure_cmd],
                            workdir=GUEST_BUILD, timeout=900)
    results.append(ctx.record("build/configure", tier, res, started=started))
    if res.returncode != 0 and not ctx.dry_run:
        return results  # no point building if configure failed

    # Step 2: make core (all libraries, no examples/tests).
    started = time.monotonic()
    res = ctx.exec_in_guest(["bash", "-lc", "make -j$(nproc) core"],
                            workdir=GUEST_BUILD, timeout=3600)
    results.append(ctx.record("build/core", tier, res, started=started))

    # Step 3: gemfony-build-all (core + benchmarks + examples + tests).
    started = time.monotonic()
    res = ctx.exec_in_guest(["bash", "-lc", "make -j$(nproc) gemfony-build-all"],
                            workdir=GUEST_BUILD, timeout=7200)
    results.append(ctx.record("build/gemfony-build-all", tier, res, started=started))

    return results


def build_docs(ctx: JobContext) -> CheckResult:
    """make doc (Doxygen). SHORT tier; scans output for warnings is best-effort."""
    tier = Tier.SHORT
    if not ctx.should_run(tier):
        return ctx.skipped("build/doc", tier, "skipped")
    started = time.monotonic()
    res = ctx.exec_in_guest(["bash", "-lc", "make doc"],
                            workdir=GUEST_BUILD, timeout=1800)
    result = ctx.record("build/doc", tier, res, started=started)
    # Warning-scan is advisory; do not fail the build solely on warnings here.
    if result.status.value == "pass" and "warning" in (res.stdout + res.stderr).lower():
        result.message = "completed with Doxygen warnings (see log)"
    return result
