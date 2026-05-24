"""Optimization-algorithm checks (ReleaseTestplan.md: "Optimization algorithms").

Runs each algorithm to completion on a simple individual via example 01, which
selects the algorithm with ``--algorithm`` (ea/sa/swarm/gd/ps). The Go2 config
round-trip and checkpoint save/restore identity are structured stubs.
"""

from __future__ import annotations

import time

from ..model import CheckResult, Tier
from ..runner import GUEST_BUILD, JobContext

# Run from the example's own build dir so it finds/creates ./config/Go2.json.
_EX01_DIR = "examples/geneva/01_GSimpleOptimizer"
_EX01 = "./GSimpleOptimizer"
_ALGOS = {
    "ea": "Evolutionary Algorithm",
    "sa": "Simulated Annealing",
    "swarm": "Swarm",
    "gd": "Gradient Descent",
    "ps": "Parameter Scan",
}


def run_all_algorithms(ctx: JobContext) -> list[CheckResult]:
    tier = Tier.LONG  # running all five end-to-end is LONG
    if not ctx.job.spec.build_examples:
        return [ctx.skipped("algorithm/all", tier, "examples not built")]
    if not ctx.should_run(tier):
        return [ctx.skipped("algorithm/all", tier, "LONG tier skipped in --quick")]
    out: list[CheckResult] = []
    for key in _ALGOS:
        started = time.monotonic()
        res = ctx.exec_in_guest(
            ["bash", "-lc",
             f"cd {GUEST_BUILD}/{_EX01_DIR} && "
             f"{_EX01} -a {key} -c sc"],
            workdir=GUEST_BUILD, timeout=1800,
        )
        out.append(ctx.record(f"algorithm/{key}", tier, res, started=started))
    return out


def go2_config_roundtrip(ctx: JobContext) -> CheckResult:
    """Write default Go2 config, reload it, run — STUB.

    Intended flow: run with `--writeConfig`, then re-run pointing at the written
    JSON and confirm it parses and completes. Needs the example's config flags
    wired; left as a TODO.
    """
    tier = Tier.LONG
    return ctx.skipped(
        "algorithm/go2-config-roundtrip", tier,
        "STUB: write default Go2 config, reload, run (TODO)",
    )


def checkpoint_identity(ctx: JobContext) -> CheckResult:
    """Checkpoint save then restore produces identical results — STUB.

    Intended flow: run with checkpointing on; interrupt/restart from the
    checkpoint; compare the final objective (and ideally serialized state)
    to an uninterrupted run. Left as a TODO (needs an objective comparison).
    """
    tier = Tier.LONG
    return ctx.skipped(
        "algorithm/checkpoint-identity", tier,
        "STUB: compare checkpoint-restored run against baseline (TODO)",
    )
