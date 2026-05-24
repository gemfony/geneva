"""Parallelization-consumer checks (ReleaseTestplan.md: "Parallelization consumers").

Runs a simple optimization under each consumer via example 01
(GSimpleOptimizer). The consumer is selected with ``-c``:
  sc    -> GSerialConsumerT (serial)
  stc   -> GStdThreadConsumerT (multithreaded)
  beast -> GWebsocketConsumerT
  mpi   -> GMPIConsumerT
The websocket and MPI splits are structured stubs with the intended commands.
The StdThread consumer's thread count is taken from the JSON config, not a CLI
flag, so the threaded check simply runs the stc consumer to completion.
"""

from __future__ import annotations

import time

from ..model import CheckResult, Tier
from ..runner import GUEST_BUILD, JobContext

# Example 01 is the canonical simple optimizer; built under examples/geneva.
# It writes/reads ./config/Go2.json relative to the CWD, so run it from its own
# build directory where the config/ folder is installed.
_EX01_DIR = "examples/geneva/01_GSimpleOptimizer"
_EX01 = "./GSimpleOptimizer"


def serial(ctx: JobContext) -> CheckResult:
    """Run one optimization to completion with the serial consumer (sc)."""
    tier = Tier.SHORT
    if not ctx.job.spec.build_examples:
        return ctx.skipped("consumer/serial", tier, "examples not built")
    if not ctx.should_run(tier):
        return ctx.skipped("consumer/serial", tier, "skipped")
    started = time.monotonic()
    res = ctx.exec_in_guest(
        ["bash", "-lc", f"cd {GUEST_BUILD}/{_EX01_DIR} && {_EX01} -c sc"],
        workdir=GUEST_BUILD, timeout=900,
    )
    return ctx.record("consumer/serial", tier, res, started=started)


def threaded(ctx: JobContext) -> list[CheckResult]:
    """Run with the multithreaded StdThread consumer (stc) to completion."""
    tier = Tier.SHORT
    if not ctx.job.spec.build_examples:
        return [ctx.skipped("consumer/threaded", tier, "examples not built")]
    if not ctx.should_run(tier):
        return [ctx.skipped("consumer/threaded", tier, "skipped")]
    started = time.monotonic()
    res = ctx.exec_in_guest(
        ["bash", "-lc", f"cd {GUEST_BUILD}/{_EX01_DIR} && {_EX01} -c stc"],
        workdir=GUEST_BUILD, timeout=900,
    )
    return [ctx.record("consumer/threaded", tier, res, started=started)]


def websocket(ctx: JobContext) -> CheckResult:
    """Client/server split on localhost via GWebsocketConsumerT (beast) — STUB.

    Intended flow: start the server example backgrounded (``-c beast`` in server
    mode), start a client pointing at localhost, wait for completion, compare
    results. Requires process orchestration and a free port; left as a TODO.
    """
    tier = Tier.LONG
    if not ctx.job.spec.build_examples:
        return ctx.skipped("consumer/websocket", tier, "examples not built")
    return ctx.skipped(
        "consumer/websocket", tier,
        "STUB: orchestrate `-c beast` server+client on localhost (TODO)",
    )


def mpi(ctx: JobContext) -> CheckResult:
    """GMPIConsumerT with >=2 ranks — STUB, only when MPI is built.

    Intended flow: `mpirun -np 2 ./examples/geneva/16_GMPIConsumer/...`.
    """
    tier = Tier.LONG
    if not ctx.job.spec.with_mpi:
        return ctx.skipped("consumer/mpi", tier, "MPI not enabled in this build")
    return ctx.skipped(
        "consumer/mpi", tier,
        "STUB: run example 16 via `mpirun -np 2 ...` (TODO)",
    )
