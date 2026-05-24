"""Top-level orchestration: provision, run the job matrix, collect results.

Sequences the checks for each job in the order of ReleaseTestplan.md and
aggregates everything into a RunReport.
"""

from __future__ import annotations

import datetime as _dt
from pathlib import Path

from .backends.base import ContainerBackend
from .checks import (algorithms, build, consumers, ctest, examples, install,
                     metadata, outoftree)
from .logging_util import ensure_workdir, get_console_logger
from .model import CheckResult, Job, RunReport, Status
from .runner import JobContext


def _now() -> str:
    return _dt.datetime.now().isoformat(timespec="seconds")


def _run_job(ctx: JobContext, log) -> list[CheckResult]:
    """Run the full check sequence for one job, in test-plan order."""
    results: list[CheckResult] = []

    # Build system
    build_results = build.configure_and_build(ctx)
    results.extend(build_results)
    results.append(build.build_docs(ctx))

    # If the build failed outright, skip the rest with a clear note.
    build_ok = all(r.status is not Status.FAIL for r in build_results) or ctx.dry_run
    if not build_ok:
        log.warning("  %s: build failed; skipping downstream checks", ctx.job.slug)
        return results

    # Unit / integration tests
    results.append(ctest.run_ctest(ctx))
    results.append(ctest.run_standard_tests(ctx))

    # Parallelization consumers
    results.append(consumers.serial(ctx))
    results.extend(consumers.threaded(ctx))
    results.append(consumers.websocket(ctx))
    results.append(consumers.mpi(ctx))

    # Optimization algorithms
    results.extend(algorithms.run_all_algorithms(ctx))
    results.append(algorithms.go2_config_roundtrip(ctx))
    results.append(algorithms.checkpoint_identity(ctx))

    # Examples
    results.append(examples.gstarter(ctx))
    results.append(examples.networked(ctx))
    results.append(examples.cuda(ctx))

    # Install / out-of-tree / linking
    results.append(install.make_install(ctx))
    results.append(install.ldd_check(ctx))
    results.append(outoftree.build_against_install(ctx))
    results.append(outoftree.build_tests_mismatch(ctx))

    return results


def provision(backend: ContainerBackend, jobs: list[Job], layout: dict[str, Path],
              *, verbose: bool = False) -> int:
    """Build the base images/instances for the distinct (OS, compiler) pairs."""
    log = get_console_logger(verbose)
    seen: set[str] = set()
    rc = 0
    for job in jobs:
        tag = backend.image_tag(job.guest, job.spec)
        if tag in seen:
            continue
        seen.add(tag)
        log.info("provisioning %s (%s)", tag, backend.name)
        res = backend.build_image(job.guest, job.spec, images_dir=layout["images"])
        if not res.ok:
            log.error("  failed: %s", (res.stderr or res.stdout).strip()[:300])
            rc = 1
        else:
            log.info("  ok")
            if res.stdout.startswith("[dry-run]"):
                log.debug(res.stdout)
    return rc


def run_matrix(backend: ContainerBackend, jobs: list[Job], layout: dict[str, Path],
               source_dir: Path, *, quick: bool, dry_run: bool,
               verbose: bool = False) -> RunReport:
    """Execute every job and return an aggregate report."""
    log = get_console_logger(verbose)
    report = RunReport(started_at=_now(), quick=quick, backend=backend.name)

    # Host-side static metadata checks run once, not per job.
    report.results.extend(metadata.run_host_checks(source_dir))

    if not dry_run and not backend.is_available():
        log.error("backend %s is not available on this host; aborting run. "
                  "Use --dry-run to preview, or install/repair the backend.",
                  backend.name)
        report.finished_at = _now()
        return report

    for i, job in enumerate(jobs, 1):
        log.info("[%d/%d] job %s", i, len(jobs), job.slug)
        ctx = JobContext(
            job=job, backend=backend, source_dir=source_dir,
            builds_dir=layout["builds"], logs_dir=layout["logs"],
            quick=quick, dry_run=dry_run,
        )
        for res in _run_job(ctx, log):
            report.add(res)
            level = log.info if res.status in (Status.PASS, Status.SKIP) else log.error
            level("    %-32s %-5s %s", res.check, res.status.value, res.message)

    report.finished_at = _now()
    return report
