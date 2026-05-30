# GHapVsHap2 baselines

Versioned TSV results, one per rework phase. Produced with:

```bash
cd <build>
make GHapVsHap2 -j$(nproc)
./hap2/benchmarks/GHapVsHap2/GHapVsHap2 --out <repo>/hap2/benchmarks/GHapVsHap2/baselines/phase-NN-<label>.tsv
```

| File | Phase | Hap2 engine |
|---|---|---|
| `phase-02-clone.tsv`   | 2 — namespace clone        | mt19937 (== Hap) |
| `phase-03-mt64.tsv`    | 3 — A-1                    | mt19937_64 |
| `phase-04-xoshiro.tsv` | 4 — xoshiro256++           | xoshiro256++ |
| `phase-05-simd.tsv`    | 5 — A-3 SIMD bulk-refill   | xoshiro256++ (SIMD refill) |

Numbers are machine-specific; compare Hap vs Hap2 on the same host and the
trend phase N → N+1, not absolute values across hosts.
