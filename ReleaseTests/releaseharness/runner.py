"""Job execution: wires a backend, a job and the work area together.

A :class:`JobContext` is passed to each check. Checks issue commands through
``ctx.exec_in_guest(...)`` which routes to the active backend with the standard
mounts (source read-only at ``/work/src``, a writable build dir at ``/work/build``)
and writes per-check logs into the work area.
"""

from __future__ import annotations

import time
from dataclasses import dataclass
from pathlib import Path

from .backends.base import CommandResult, ContainerBackend, Mount
from .logging_util import job_log_path
from .model import BuildType, CheckResult, Job, Status, Tier

# Standard in-guest paths.
GUEST_SRC = "/work/src"
GUEST_BUILD = "/work/build"


@dataclass
class JobContext:
    job: Job
    backend: ContainerBackend
    source_dir: Path
    builds_dir: Path
    logs_dir: Path
    quick: bool
    dry_run: bool

    @property
    def host_build_dir(self) -> Path:
        return self.builds_dir / self.job.slug

    def mounts(self) -> list[Mount]:
        self.host_build_dir.mkdir(parents=True, exist_ok=True) if not self.dry_run else None
        return [
            Mount(host=self.source_dir, guest=GUEST_SRC, read_only=True),
            Mount(host=self.host_build_dir, guest=GUEST_BUILD, read_only=False),
        ]

    def exec_in_guest(self, argv: list[str], *, workdir: str = GUEST_BUILD,
                      use_gpu: bool = False, timeout: int | None = None) -> CommandResult:
        # Sanitizer-instrumented binaries refuse to start under high ASLR entropy
        # ("Please rerun with lower ASLR entropy"). Disable ASLR per-process via
        # `setarch -R` for the whole Sanitize cell (harmless for build commands).
        # This needs the personality() syscall, which the podman backend unblocks
        # with --security-opt seccomp=unconfined for Sanitize builds.
        if self.job.spec.build_type is BuildType.SANITIZE:
            argv = ["setarch", "-R", *argv]
        return self.backend.run(
            self.job.guest, self.job.spec, argv,
            mounts=self.mounts(), workdir=workdir,
            use_gpu=use_gpu, timeout=timeout,
        )

    def should_run(self, tier: Tier) -> bool:
        """SHORT checks always run; LONG checks only under --full (not quick)."""
        return tier is Tier.SHORT or not self.quick

    def record(self, check: str, tier: Tier, result: CommandResult,
               *, started: float, skip_reason: str | None = None) -> CheckResult:
        """Turn a backend command result into a CheckResult and persist the log."""
        log_path = job_log_path(self.logs_dir, self.job.slug, check)
        if not self.dry_run:
            try:
                log_path.write_text(
                    f"$ {' '.join(result.argv)}\n\n--- stdout ---\n{result.stdout}\n"
                    f"--- stderr ---\n{result.stderr}\n",
                    encoding="utf-8",
                )
            except OSError:
                pass
        if skip_reason is not None:
            status = Status.SKIP
            message = skip_reason
        elif result.returncode == 127:
            status = Status.ERROR
            message = "command/backend not found"
        elif result.ok:
            status = Status.PASS
            message = ""
        else:
            status = Status.FAIL
            message = (result.stderr or result.stdout).strip().splitlines()[-1:] and \
                (result.stderr or result.stdout).strip().splitlines()[-1] or \
                f"exit {result.returncode}"
        return CheckResult(
            job_slug=self.job.slug, check=check, tier=tier, status=status,
            duration_s=time.monotonic() - started, message=message,
            log_path=str(log_path),
        )

    def skipped(self, check: str, tier: Tier, reason: str) -> CheckResult:
        return CheckResult(job_slug=self.job.slug, check=check, tier=tier,
                           status=Status.SKIP, message=reason)
