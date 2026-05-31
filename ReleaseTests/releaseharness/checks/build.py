"""Build checks (ReleaseTestplan.md: "Build system").

Drives an out-of-source build inside the guest. Two strategies:

1. ``prepareBuild.sh`` driven by a generated ``.gcfg`` (primary; matches how
   releases are actually built).
2. a direct ``cmake`` invocation (fallback / used implicitly via the same flow).

Targets exercised: ``make core``, ``make gemfony-build-all``, ``make doc``.
"""

from __future__ import annotations

import re
import time

from ..model import BuildType, CheckResult, Status, Tier
from ..runner import GUEST_BUILD, GUEST_SRC, JobContext

# The .gcfg is written into the writable build dir so prepareBuild.sh finds it.
_GCFG_GUEST_PATH = f"{GUEST_BUILD}/release.gcfg"

# A build can exit 0 yet have actually failed (wrapper/notification exit, a
# non-relinked target, a sub-make whose error was swallowed). Defend against
# that false-green class by scanning the output for genuine compiler/linker
# diagnostics. The pattern is deliberately specific to GCC/Clang diagnostics
# ("file:line[:col]: error:") and linker failures, so it does NOT match benign
# tool noise such as Doxygen's "error: Problems running latex ..." (which has no
# file:line prefix) that legitimately appears in a successful build's output.
_BUILD_ERROR = re.compile(r":\d+:(?:\d+:)?\s*error:|undefined reference to|ld returned",
                          re.MULTILINE)

# prepareBuild.sh can exit 0 even when its cmake step fails (e.g. a missing CUDA
# toolkit makes ENABLE_LANGUAGE(CUDA) error out), leaving no Makefile. Catch that
# false-green at configure time instead of as a confusing "No rule to make
# target 'core'" later.
_CONFIGURE_ERROR = re.compile(r"Configuring incomplete|CMake Error")


def _scan_build_errors(result: CheckResult, res) -> CheckResult:
    """Downgrade a PASS to FAIL when the build output shows compiler/linker
    errors despite a zero exit code."""
    if result.status is Status.PASS:
        if _BUILD_ERROR.search(res.stdout + res.stderr):
            result.status = Status.FAIL
            result.message = ("exit 0 but compiler/linker 'error:' found in output "
                              "(suspected swallowed build failure)")
    return result


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
    #
    # --clean is ALWAYS passed (including the SHORT/--quick tier): a release
    # qualification build must start from a pristine directory so no stale
    # artifact from a previous run of this cell can mask a problem. prepareBuild
    # --clean wipes the build dir but keeps *.gcfg files, so the release.gcfg we
    # just wrote survives. The per-cell dirs are already namespaced by job slug
    # (os-compiler-buildtype), so different build types / compilers never share
    # a directory anyway; --clean additionally guarantees a from-scratch compile
    # on re-runs. The compile time this costs is accepted; --quick stays "quick"
    # by skipping the LONG-tier checks and long-running benchmarks, not by
    # reusing build artifacts.
    started = time.monotonic()
    configure_cmd = (
        _write_gcfg(ctx)
        + f"{GUEST_SRC}/scripts/prepareBuild.sh -y --clean {_GCFG_GUEST_PATH}"
    )
    res = ctx.exec_in_guest(["bash", "-lc", configure_cmd],
                            workdir=GUEST_BUILD, timeout=900)
    configure_res = ctx.record("build/configure", tier, res, started=started)
    # Guard the prepareBuild-exited-0-but-cmake-failed false-green.
    if configure_res.status is Status.PASS and not ctx.dry_run:
        if _CONFIGURE_ERROR.search(res.stdout + res.stderr):
            configure_res.status = Status.FAIL
            configure_res.message = ("cmake configuration failed "
                                     "(prepareBuild.sh exited 0 anyway)")
    results.append(configure_res)
    if configure_res.status is Status.FAIL or (res.returncode != 0 and not ctx.dry_run):
        return results  # no point building if configure failed

    # Step 2: make core (all libraries, no examples/tests).
    started = time.monotonic()
    res = ctx.exec_in_guest(["bash", "-lc", "make -j$(nproc) core"],
                            workdir=GUEST_BUILD, timeout=3600)
    results.append(_scan_build_errors(
        ctx.record("build/core", tier, res, started=started), res))

    # Step 3: gemfony-build-all (core + benchmarks + examples + tests).
    started = time.monotonic()
    res = ctx.exec_in_guest(["bash", "-lc", "make -j$(nproc) gemfony-build-all"],
                            workdir=GUEST_BUILD, timeout=7200)
    results.append(_scan_build_errors(
        ctx.record("build/gemfony-build-all", tier, res, started=started), res))

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
