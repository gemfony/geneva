# Geneva — Code Review of the Geneva Library (verified & corrected)

**Date**: 2026-05-15
**Branch**: `core-component-modernization`
**Scope**: `include/geneva/` and `src/geneva/` (the core Geneva library — parameter types, adaptors, optimization algorithms, orchestration, monitoring, factories, and MPI integration). The five support libraries (`common`, `hap`, `courtier`, `geneva-individuals`, plus all example/test code) are out of scope.

**Method & status**: The first draft of this review was produced by parallel passes that read only code *excerpts*. Every finding was subsequently re-verified by reading the **full source files**. The original draft had a high false-positive rate (≈43 % of HIGH findings were not real). This document contains **only the verified findings**, with corrected file/line locations and real (not paraphrased) code. A record of the dismissed findings is kept at the end so they are not re-investigated.

**Severity scale**:
- **HIGH** — real bug; wrong behaviour, UB, race, or data corruption that affects users on a normal path.
- **MEDIUM** — bug only under uncommon inputs, post-init API misuse, or specific maintenance scenarios.
- **LOW** — code-quality / robustness improvement, little or no correctness impact in practice.
- **TRIVIAL** — cosmetic.

---

## Executive summary

| Bucket | Count | Notes |
|---|---|---|
| MEDIUM | **6** | `load_()`/clone derived-state gaps, factory copy omission, MPI handle init, DEBUG-only validity assert, unchecked numeric cast, constructor ignoring args. |
| LOW | **3** | Documented-as-intentional setter, mode-handling inconsistency, partial differentiability guard. |
| **Dismissed after verification** | **20** | Findings whose described code does not exist or is already correct — listed at the end. |

The originally-identified HIGH findings are not part of this document and are tracked separately.

---

## MEDIUM-severity findings

### M-A. `load_()` skips derived members → clone consistent only after `init()`

- `src/geneva/G_OptimizationAlgorithm_GradientDescent.cpp:255-273` — copies raw members but explicitly comments out `stepRatio_`, `dbl_lower/upper_parameter_boundaries_cnt_`, `adjustedFiniteStep_` (lines 269-272).
- `src/geneva/G_OptimizationAlgorithm_SwarmAlgorithm.cpp:128-216` — does not copy `velocities_cnt_` / `last_iteration_individuals_cnt_`, and re-assigns neighbourhood arrays only on the size-mismatch branch.

These are recomputed in `init()`, so a loaded clone is inconsistent until `init()` is next called. By design, but fragile for any code path that uses a loaded object as a continuation checkpoint without re-`init()`.

**Fix**: copy all members in `load_()`, or assert/document the `init()`-after-`load_()` requirement.

---

### M-B. `G_OptimizationAlgorithm_FactoryT` copy constructor drops `pluggableOM_` — `include/geneva/G_OptimizationAlgorithm_FactoryT.hpp:96-113`

The copy constructor initialises the base, `contentCreatorPtr_` (with a clone/load), and the CL members, but never copies `pluggableOM_` (declared line 420). A copied factory therefore produces algorithms with **no monitor**. (The draft's "dangling pointers" wording was wrong — `pluggableOM_` is a `std::shared_ptr`, so the copy is empty, not dangling.)

**Fix**: clone `pluggableOM_` in the copy constructor.

---

### M-C. `MPI_Request` member value-initialised instead of `MPI_REQUEST_NULL` — `include/geneva/GMPISubClientIndividual.hpp:144`

```cpp
inline static MPI_Request clientStatusRequest_{};
```

`{}` does not guarantee the well-defined `MPI_REQUEST_NULL` sentinel for the opaque handle. `getClientStatus()` calls `MPI_Test(&clientStatusRequest_, …)` (`GMPISubClientIndividual.cpp:65`); if reached before `setClientStatusRequest()` it tests an indeterminate handle. The sibling member `communicator_{MPI_COMM_NULL}` (line 139) is correctly initialised, making the omission stand out.

**Fix**: `inline static MPI_Request clientStatusRequest_{MPI_REQUEST_NULL};`

---

### M-D. `getCrossOverPos` validity check is `assert()` (DEBUG-only) — `src/geneva/GParameterSet.cpp:652-662`

The function uses `assert(lower > 0)` / `assert(upper > lower)` (lines 654, 656), stripped under `NDEBUG`. With `upper == lower` (e.g. a single-element collection), the subsequent `std::uniform_int_distribution(lower, upper-1)` is given an inverted/invalid range — UB, **not** the "division by zero" the draft claimed. All current call sites guard with a non-empty check, so it is latent rather than active.

**Fix**: replace the asserts with an unconditional `throw`.

---

### M-E. Unchecked FP→int `static_cast` — `include/geneva/GConstrainedFPT.hpp:347-350`

```cpp
region = static_cast<std::int64_t>(std::floor((localVal - lowerBoundary) / (upperBoundary - lowerBoundary)));
```

Raw `static_cast`, no `std::clamp`; the DEBUG-only guard at lines 327-345 (which range-checks the value) is compiled out in Release. If the quotient exceeds `int64` range (extreme `val`, narrow range) the cast is UB in Release. (The draft mislocated this to `GConstrainedIntT.hpp`, which contains no FP→int cast; the surrounding `narrow_cast` calls *are* checked — only this one raw cast is the gap.)

**Fix**: `std::clamp` the value (or use the checked `narrow_cast`) before the cast, unconditionally.

---

### M-F. 5-argument `GNumGaussAdaptorT` constructor ignores all of its arguments — `include/geneva/GNumGaussAdaptorT.hpp:134-142`

The `GNumGaussAdaptorT(sigma, sigmaSigma, minSigma, maxSigma, probability)` constructor has an empty body and only delegates `: GAdaptorT<num_type>(probability)`. The four sigma-related parameters are silently dropped; the object falls back to `DEFAULTSIGMA` etc. (Members are not uninitialised — default member initialisers exist at lines 824-829 — so the draft's "indeterminate values" wording, and its claim that the *4-arg* constructor was at fault, were both wrong. The 4-arg constructor at lines 108-122 correctly sets `sigma_reset_ = sigma_`.)

**Fix**: have the 5-arg constructor call the same setters as the 4-arg one.

---

## LOW-severity findings

### L-A. `setSigmaAdaptionRate` performs no validation — `include/geneva/GNumGaussAdaptorT.hpp:318-320`

```cpp
void setSigmaAdaptionRate(const fp_type &sigmaSigma) { sigmaSigma_ = sigmaSigma; }
```

No range check at all. However, the Doxygen explicitly documents that values `<= 0` mean "do not adapt sigma", so accepting them is (at least partly) **intentional contract**, not a bug. (Downgraded from HIGH.) Worth at most a `> 0` sanity note for clearly-invalid (e.g. NaN) inputs.

---

### L-B. `setExecMode` vs. constructor handle `BROKER` mode inconsistently — `src/geneva/GPostProcessorT.cpp`

The **constructor** logs a warning and falls back to SERIAL for `execMode::BROKER` (lines 51-70); the **setter** `setExecMode()` **throws** for the same value (lines 128-134). Inconsistent, but the inconsistency is the *reverse* of what the draft stated, and it is in `GPostProcessorT.cpp` (the `GEvolutionaryAlgorithmPostOptimizer`), not `Go2.cpp`.

**Fix**: make both paths behave the same (preferably both throw).

---

### L-C. Gradient descent only rejects *pure* non-differentiable individuals — `src/geneva/G_OptimizationAlgorithm_GradientDescent.cpp:646-655`

`adjustPopulation()` throws if there are zero active `double` parameters, so an individual containing **only** `GBooleanObject`/`GInt32Object` is correctly rejected (contradicting the draft's "runs and produces meaningless results"). A **mixed** individual (doubles + ints/bools) passes, and the integer/boolean parameters are silently ignored by the gradient with no warning.

**Fix (optional)**: warn when non-differentiable parameters are present in a mixed individual.

---

## Dismissed after verification (recorded so they are not re-investigated)

Each of these was checked against the full source and found to be **not a bug** — the described code does not exist, is located elsewhere and correct, or already does the right thing.

| Draft ID | Subject | Why dismissed |
|---|---|---|
| H1 | Signal handler unsafe | Real handler `GObject::sigHupHandler` (`GObject.hpp:134-138`) only sets a `volatile std::sig_atomic_t` — correctly async-signal-safe. No handler in Go2.cpp. |
| H2 | `GConstrainedIntT::transfer()` modulo overflow | No modulo used (`GConstrainedIntT.hpp:166-215` uses division/folding); int32 boundaries clamped to `±INT32_MAX/10` and enforced by throwing setters. |
| H9 | `neighborhoodsHaveNominalValues` inverted | `G_OptimizationAlgorithm_SwarmAlgorithm.cpp:957-963` logic is correct; function additionally has zero callers. |
| H10 | `aDominatesB` no early break / overrun | `G_OptimizationAlgorithm_EvolutionaryAlgorithm.cpp:1225-1251` breaks on first failure and has a DEBUG size-precondition. |
| H14 | `GFitnessMonitor` data race | `informationFunction()` called from a single synchronous site (`G_OptimizationAlgorithm_Base.cpp:802`); no concurrent path; member names in draft don't exist. |
| H16 | `GPostProcessorT` no try/catch | Functor invoked at `GProcessingContainerT.hpp:1000`, inside a `try/catch(...)` block (lines 283-321). |
| H17 | Broker enrol failure ignored | `enrol_consumer` returns `void` (`GBrokerT.hpp:274`); the log path is a deliberate "first consumer wins" policy, not a failure path. |
| H19 | Dead `if(this==nullptr)` | Does not exist anywhere in the factory header. |
| H22 | Factory `init()` not thread-safe | `GFactoryT::globalInit()` (`include/common/GFactoryT.hpp:374-380`) guards `init_()` with `std::scoped_lock`; `std::once_flag` deliberately avoided (serialisation). |
| H23 | `load_()` calls wrong base | `GMultiConstraintT.hpp:369-382` correctly calls the immediate parent `GPreEvaluationValidityCheckT::load_`. (`GValidityCheckContainerT.hpp` does not exist.) |
| M2 | `assignValueVector` missing `isActive()` | No such branched function; the template (`GParameterSet.hpp:618-645`) delegates uniformly with no type-specific branches. |
| M3 | Garbled grammar token | Spirit rules (`GParameterPropertyParser.cpp:85-106`) are all well-formed. |
| M4 | Property-tree key missing dot | All keys in `GParameterSet::toPropertyTree` are correctly dotted; Go2.cpp uses no property_tree. |
| M5 | int32×int32 width overflow | No such unwidened product; `range()` is a subtraction. |
| M7 | `GNumBiGaussAdaptorT` ctor uninitialised members | All members have non-static default initialisers (lines 554-568). |
| M9 | `sortMuCommaNuMode` incomplete swap | Real functions (`…EvolutionaryAlgorithm.cpp:829-917`) operate on one `data_cnt_` array; no temp-buffer/aux-structure asymmetry. |
| M11 | `sSpecVec.empty()` dead loop check | Loop iterates `variableDescriptions`; the `.empty()` guards are live and correct. |
| M13 | Check-objects collection not restored | `GMultiConstraintT.hpp:381` restores `validityChecks_` via `copyCloneableSmartPointerContainer`. |
| M14 | Spec vector not cleared between parses | `parse()` is one-shot (`parsed_` flag); reruns go through `setNewParameterDescription()` which clears all five vectors. |
| M15 | Factory init redundantly re-parses config | `init_()` is guarded and unrelated to config parsing; per-`get_()` config re-read is intentional. |

---

*Review verified and corrected 2026-05-15. Net: 6 MEDIUM, 3 LOW findings; 20 draft findings dismissed.*
