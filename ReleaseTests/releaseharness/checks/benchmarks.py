"""Benchmark smoke checks (ReleaseTestplan.md: long-running benchmarks).

The medium tier builds the benchmark programs and *starts each one once* with a
per-program wall-clock limit. A benchmark that runs to completion OR is aborted
by the limit counts as a healthy start (PASS); only a benchmark that fails to
launch (binary missing / not executable) or dies within the first few seconds
counts as FAIL. This honours "start every benchmark at least once; aborting is
fine" without paying the full multi-hour benchmark runtime.

CUDA benchmarks are excluded here (they need a GPU and are covered by the CUDA
example/feature path).
"""

from __future__ import annotations

import time

from ..model import CheckResult, Tier
from ..runner import GUEST_BUILD, JobContext

# Per-benchmark wall-clock limit (seconds): enough to prove a healthy start
# before the program is aborted.
_LIMIT_S = 60


def smoke_start(ctx: JobContext) -> CheckResult:
    """Start every built benchmark once, aborting after _LIMIT_S; abort == PASS."""
    tier = Tier.SHORT
    if not ctx.job.spec.build_benchmarks:
        return ctx.skipped("benchmark/smoke-start", tier, "benchmarks not built")
    if not ctx.should_run(tier):
        return ctx.skipped("benchmark/smoke-start", tier, "skipped")
    started = time.monotonic()
    script = (
        # Post-Lager-A there is no top-level benchmarks/ dir; benchmark
        # executables live under per-library trees (common/benchmarks/...,
        # hap/benchmarks/..., courtier/..., geneva/...). Search the whole build
        # tree restricted to */benchmarks/* paths.
        f"cd {GUEST_BUILD}\n"
        "mapfile -t exes < <(find . -type f -perm -u+x -path '*/benchmarks/*' "
        "! -name '*.so*' ! -name '*.sh' ! -name '*.cmake' "
        "! -path '*CMakeFiles*' | grep -vi cuda | sort)\n"
        "if [ ${#exes[@]} -eq 0 ]; then "
        "echo 'no benchmark executables found'; exit 1; fi\n"
        "fail=0\n"
        'for exe in "${exes[@]}"; do\n'
        '  d=$(dirname "$exe"); b=$(basename "$exe")\n'
        f'  echo "=== starting $exe (limit {_LIMIT_S}s) ==="\n'
        "  t0=$SECONDS\n"
        f'  ( cd "$d" && timeout -k 5 -s INT {_LIMIT_S}s "./$b" </dev/null )\n'
        "  rc=$?; dt=$((SECONDS - t0))\n"
        '  if [ "$rc" = 0 ] || [ "$rc" = 124 ]; then\n'
        '    echo "OK    $exe rc=$rc ran=${dt}s"\n'
        '  elif [ "$dt" -ge 5 ]; then\n'
        '    echo "OK    $exe rc=$rc ran=${dt}s (nonzero exit after running -> started)"\n'
        "  else\n"
        '    echo "FAIL  $exe rc=$rc ran=${dt}s (failed to start)"; fail=1\n'
        "  fi\n"
        "done\n"
        "exit $fail\n"
    )
    res = ctx.exec_in_guest(["bash", "-lc", script],
                            workdir=GUEST_BUILD, timeout=_LIMIT_S * 8 + 120)
    return ctx.record("benchmark/smoke-start", tier, res, started=started)
