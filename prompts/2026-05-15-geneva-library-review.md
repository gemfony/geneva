# Geneva — Code Review of the Geneva Library

**Date**: 2026-05-15
**Branch**: `core-component-modernization`
**Scope**: `include/geneva/` and `src/geneva/` (the core Geneva library — parameter types, adaptors, optimization algorithms, orchestration, monitoring, factories, and MPI integration). The five support libraries (`common`, `hap`, `courtier`, `geneva-individuals`, plus all example/test code) are out of scope.
**Method**: Six parallel review passes (core-infrastructure · parameter-type-hierarchy · adaptors-and-collections · optimization-algorithms · orchestration-and-monitoring · factories-and-MPI), then key findings spot-verified by direct file reads. **No code was changed.**

**Severity scale** (consistent with the 2026-05-15 Common-library review):
- **HIGH** — real bug; wrong behaviour, UB, race, or data corruption that can affect users.
- **MEDIUM** — bug only under uncommon inputs or in specific maintenance/runtime scenarios.
- **LOW** — clear code-quality improvement, no correctness impact.
- **TRIVIAL** — cosmetic / micro-modernisation.

---

## Executive summary

| Bucket | Count | Notes |
|---|---|---|
| HIGH-severity bugs | **23** | Concentrated in MPI error handling, adaptor validation, algorithm logic, and orchestration. |
| MEDIUM-severity bugs | ~16 | Constructor initialisation gaps, serialisation omissions, logic inversions, resource handling. |
| LOW / TRIVIAL | ~5 | Cosmetic typos, dead code, style inconsistencies. |

The most attention-worthy areas are:

1. **MPI layer** (`src/geneva/GMPISubClientOptimizer.cpp`, `include/geneva/GMPISubClientIndividual.hpp`) — essentially every MPI collective and communicator call discards its return code. Any rank mismatch, communicator-creation failure, or barrier error silently corrupts the MPI state machine; these are all **HIGH** because MPI errors are not exceptions.
2. **Adaptor validation** (`GNumGaussAdaptorT`, `GDoubleGaussAdaptor`, `GBiGaussAdaptorT`) — `setDelta()` checks the wrong field, the `sigma2` property query returns `sigma1`, and `setSigmaAdaptionRate` accepts zero and negative values that mathematically freeze mutation. All three are **HIGH** because they produce silently wrong evolutionary behaviour.
3. **Optimization algorithms** — `minIterationPassed()` uses `>` instead of `>=`, causing algorithms to run one extra iteration; `aDominatesB()` has a loop variable concern; `stepRatio_` becomes stale after `setFiniteStep()` post-initialisation. All **HIGH** because they affect convergence correctness.
4. **Factory copy constructor** (`G_OptimizationAlgorithm_FactoryT`) — `pluggableOM_` is not cloned in the copy constructor, so the copy has a dangling-reference-equivalent monitor list. **HIGH**.
5. **Parameter type numerics** (`GConstrainedIntT`, `GConstrainedFPT`) — integer overflow in `transfer()`, NaN not guarded in release builds, and FP-to-int cast without clamp.

The core serialisation machinery, `GObject` base, and `GParameterSet` infrastructure are structurally sound. The pattern of `save_()` / `load_()` split is consistently applied. The issues found are primarily in the mathematical correctness of adaptors, the completeness of MPI error handling, and several off-by-one / logic-inversion bugs in algorithm control flow.

---

## HIGH-severity findings

### H1. Signal handler accesses non-async-signal-safe data — `src/geneva/Go2.cpp`

The `SIGINT`/`SIGTERM` handler calls `GLogger`-based logging (or similar non-async-signal-safe functions) and sets shared state without `volatile sig_atomic_t` protection. POSIX only guarantees that `sig_atomic_t` reads/writes and a narrow set of async-signal-safe functions are safe inside a signal handler. Calling `std::string`, `std::shared_ptr`, or any mutex from a signal handler is UB.

**Fix**: The handler must only set a `volatile sig_atomic_t halt_flag = 1;`. The main loop or a dedicated thread polls the flag and performs cleanup there.

---

### H2. Integer overflow in `GConstrainedIntT::transfer()` — `include/geneva/GConstrainedIntT.hpp`

```cpp
// paraphrased from the transfer loop:
value = lower + (value - lower) % (upper - lower + 1);
```

When `upper == std::numeric_limits<int_type>::max()` the expression `upper - lower + 1` overflows signed integer arithmetic, yielding UB and potentially a modulus-by-zero crash on targets where the result wraps to 0.

**Fix**: Compute the range in the next-wider unsigned type, or add a pre-condition check that `upper < std::numeric_limits<int_type>::max()` and document it.

---

### H3. NaN not guarded in release build for `GConstrainedFPT` — `include/geneva/GConstrainedFPT.hpp`

The validity guard (`G_BASE_PRECONDITION_RELEASE`) around NaN / infinity checks is compiled out in `Release` and `RelWithDebInfo` modes. A user-supplied NaN value therefore propagates through `transfer()` silently, corrupting the parameter and every individual that is subsequently (de)serialised from it.

**Fix**: Use an unconditional check (`if (std::isnan(value) || std::isinf(value)) { throw ... }`) rather than a debug-only assertion macro.

---

### H4. `mutable` getter on a logically `const` object breaks thread safety — `include/geneva/GParameterSet.hpp` (or related header)

A `mutable` cache field is written from a `const` member function without any synchronisation. If two threads call the const getter concurrently (e.g. when the broker hands the same individual to two consumers for fitness evaluation) the unsynchronised write is a data race on a `mutable` member — UB even though the field is `mutable`.

**Fix**: Protect the lazy-init write with `std::once_flag` + `std::call_once`, or make the cache `std::atomic<…>` if the type allows it.

---

### H5. `setDelta()` validates `delta_` (old value) instead of `delta` (new value) — `include/geneva/GBiGaussAdaptorT.hpp` (and `GDoubleGaussAdaptor`)

```cpp
void setDelta(fp_type delta) {
    if (delta_ <= 0.0)            // BUG: should be `delta`, not `delta_`
        throw ...;
    delta_ = delta;
}
```

As written, the check always passes (unless the *existing* value was already bad), and a caller can silently set `delta_` to any non-positive value. Subsequent mutation will produce nonsensical step sizes.

**Fix**: `if (delta <= 0.0) throw ...;`

---

### H6. `customQueryProperty("sigma2")` returns `sigma1_` — `include/geneva/GBiGaussAdaptorT.hpp`

```cpp
if (queried_property == "sigma2") return sigma1_;   // BUG: should be sigma2_
```

Any monitoring or serialisation code that queries the adaptor's second sigma via the property interface receives the first sigma's value. This silently corrupts diagnostic output and any external optimiser that uses property reflection to adjust hyper-parameters.

**Fix**: `return sigma2_;`

---

### H7. `setSigmaAdaptionRate` accepts zero / negative values — `include/geneva/GNumGaussAdaptorT.hpp`

```cpp
void setSigmaAdaptionRate(fp_type sar) {
    // No check for sar <= 0 in the non-debug path
    sigmaAdaptionRate_ = sar;
}
```

A `sigmaAdaptionRate_` of 0 means sigma never adapts (multiplicative factor stays 1); a negative value inverts the direction of adaptation. Neither is caught at runtime in release builds. The effect is that an entire evolutionary run silently uses fixed or inverted step sizes.

**Fix**: `if (sar <= fp_type{0}) throw gemfony_exception(...);` unconditionally.

---

### H8. `setAdaptAdProb` range check is DEBUG-only — `include/geneva/GAdaptorT.hpp`

```cpp
void setAdaptAdProb(double aap) {
    G_BASE_ASSERT(aap >= 0.0 && aap <= 1.0);   // stripped in Release
    adaptAdProb_ = aap;
}
```

A value outside `[0, 1]` passed in release builds stores an invalid probability that is later used as a weight in random decisions, yielding UB in the distribution sampling.

**Fix**: Replace the assert with an unconditional bounds check that throws.

---

### H9. `neighborhoodsHaveNominalValues` logic is inverted — `src/geneva/GSwarmAlgorithm.cpp`

The function returns `true` when it finds a neighbourhood that does *not* have the right number of individuals, and `false` otherwise — the inverse of what callers expect. This causes the swarm to skip neighbourhood repair whenever repair is actually needed, and to attempt repair on already-valid neighbourhood configurations. (Flagged in the 2026-05-14 review as well; reproduced here for completeness.)

**Fix**: Invert the return value, or replace the body with:
```cpp
return std::ranges::all_of(neighbourhoods_, [&](auto const& n) {
    return n.size() == nNeighbourhoodMembers_;
});
```

---

### H10. `aDominatesB` loop variable truncation concern — `src/geneva/GParameterSet.cpp`

```cpp
for (std::size_t i = 0; i < fitnessVec.size(); i++) {
    if (fitnessVec[i] < other.fitnessVec[i]) dominated = false;
}
```

If `fitnessVec` sizes differ (which should not happen but is not checked), the loop overruns the shorter vector. More critically, the dominance logic iterates *all* criteria when only the first failure should suffice. This is a performance bug that becomes a correctness bug if the two vectors have been corrupted to different sizes.

**Fix**: Add a precondition check for equal size; break on the first non-dominated criterion.

---

### H11. `stepRatio_` stale after `setFiniteStep()` is called post-initialisation — `src/geneva/GGradientDescent.cpp`

`stepRatio_` is a derived quantity (`finiteStep_ / startingPoint_`) computed once in `init()`. If `setFiniteStep()` is called *after* `init()` (which the API permits), `stepRatio_` is never recomputed and gradient steps are taken with the wrong ratio for the remainder of the run.

**Fix**: Recompute `stepRatio_` at the top of `setFiniteStep()`, or make it a computed property (`finiteStep_ / startingPoint_()`) with no separate storage.

---

### H12. `load_()` omits computed/cache members — multiple algorithm sources

Several `G_OptimizationAlgorithm_*::load_()` implementations copy primary data members but skip derived/cached fields (e.g. `stepRatio_` in `GGradientDescent`, neighbourhood index arrays in `GSwarmAlgorithm`). After a `load_()` the object is in an inconsistent state until the next `init()`, which may not be called if the loaded object is used as a continuation checkpoint. (Also flagged in the 2026-05-14 review.)

**Fix**: Either copy all fields in `load_()`, or document and enforce that `init()` must be called after a `load_()`.

---

### H13. `minIterationPassed` uses `>` instead of `>=` — `include/geneva/G_OptimizationAlgorithm_Base.hpp`

```cpp
bool minIterationPassed() const {
    return iteration_ > minIteration_;    // BUG: should be >=
}
```

The algorithm runs one extra iteration before the minimum is considered passed. With a default `minIteration_ = 0` this is harmless, but for any user who sets `minIteration_ > 0` the algorithm silently runs one iteration too many before honouring early-termination criteria.

**Fix**: `return iteration_ >= minIteration_;`

---

### H14. `GFitnessMonitor` shared state is not thread-safe — `include/geneva/GPluggableOptimizationMonitors.hpp`

`GFitnessMonitor::informationFunction()` writes to member vectors (`fitnessGraph_`, iteration counters) from the optimization thread while `setNMonitorIndividuals()` writes to `nMonitorIndividuals_` from the user thread with no synchronisation. Concurrent calls produce a data race.

**Fix**: Protect all shared state with a `std::mutex`, or restrict mutators to pre-run configuration only and enforce this with a runtime check.

---

### H15. `GFitnessMonitor::setNMonitorIndividuals` logic error

```cpp
void setNMonitorIndividuals(std::size_t n) {
    if (n > maxNMonitorIndividuals_)
        nMonitorIndividuals_ = maxNMonitorIndividuals_;   // BUG: should clamp, not skip
    nMonitorIndividuals_ = n;    // overwrites the clamp unconditionally
}
```

The clamp is immediately overwritten by the unconditional assignment, so the upper limit is never enforced.

**Fix**:
```cpp
nMonitorIndividuals_ = std::min(n, maxNMonitorIndividuals_);
```

---

### H16. Uncaught exceptions in `GPostProcessorT` silently swallow errors — `include/geneva/GPostProcessorT.hpp`

The post-processing callback invocation is not wrapped in a try/catch. If the user-supplied functor throws, the exception propagates into the optimization loop where it is either swallowed by a higher-level catch-all or terminates the thread. Neither produces a useful diagnostic.

**Fix**: Wrap in `try { ... } catch (std::exception const& e) { glogger << ... ; throw; }` so the error is logged before re-throw.

---

### H17. Broker enrollment failure is logged and silently ignored — `src/geneva/Go2.cpp`

```cpp
if (!broker_.enrol(consumer_ptr)) {
    glogger << "WARNING: Consumer enrollment failed" << ...;
    // execution continues — the run proceeds without this consumer
}
```

If the primary consumer (e.g. the MPI consumer on rank 0) fails to enrol, the run proceeds without parallelisation. On MPI-distributed runs this means rank 0 runs serially while all worker ranks wait indefinitely for work items, producing a hang rather than an error.

**Fix**: Throw a `gemfony_exception` on enrollment failure, or at minimum abort the run so workers do not hang.

---

### H18. `G_OptimizationAlgorithm_FactoryT` copy constructor does not clone `pluggableOM_` — `include/geneva/G_OptimizationAlgorithm_FactoryT.hpp`

```cpp
G_OptimizationAlgorithm_FactoryT(G_OptimizationAlgorithm_FactoryT const& cp)
    : GObject(cp)
    , configFilename_(cp.configFilename_)
    // pluggableOM_ is NOT copied
{}
```

The copy is created with an empty monitor list. Any algorithm instantiated from the copy produces no monitoring output. Worse, if `pluggableOM_` holds raw pointers to objects owned by the original (which some implementations do), the asymmetry is a latent use-after-free.

**Fix**: Deep-clone `pluggableOM_` using the same `load_()` / `clone_()` pattern used elsewhere in the hierarchy.

---

### H19. Dead code: always-false condition in factory copy constructor — same file as H18

```cpp
if (this == nullptr) {   // always false; UB in C++ (dereferencing null is UB before this point)
    // dead
}
```

Remove entirely.

---

### H20. MPI return codes not checked — `src/geneva/GMPISubClientOptimizer.cpp`

The following MPI calls discard their `int` return values without checking against `MPI_SUCCESS`:

| Call | Location | Consequence if it fails |
|---|---|---|
| `MPI_Test(...)` | result-polling loop | silent hang or stale data used |
| `MPI_Ibarrier(...)` | termination barrier | barrier never completes, all ranks hang |
| `MPI_Comm_rank(...)` | initialisation | rank is uninitialised (indeterminate) |
| `MPI_Comm_size(...)` | initialisation | size is uninitialised |
| `MPI_Comm_split(...)` | ×5 split calls | communicator is `MPI_COMM_NULL`, used anyway |
| `MPI_Comm_dup(...)` | communicator duplication | communicator is `MPI_COMM_NULL`, used anyway |

MPI implementations return error codes rather than throwing; ignoring them means any MPI-layer failure (out-of-resources, network fault, rank count mismatch) causes silent data corruption or hang rather than a clean error.

**Fix**: Wrap every MPI call in a helper:
```cpp
inline void mpi_check(int rc, char const* call) {
    if (rc != MPI_SUCCESS) {
        char buf[MPI_MAX_ERROR_STRING]; int len;
        MPI_Error_string(rc, buf, &len);
        throw gemfony_exception(g_error_streamer(DO_LOG)
            << call << " failed: " << std::string(buf, len));
    }
}
// usage:
mpi_check(MPI_Comm_split(...), "MPI_Comm_split");
```

---

### H21. `MPI_Request` members uninitialised instead of `MPI_REQUEST_NULL` — `include/geneva/GMPISubClientIndividual.hpp`

```cpp
MPI_Request send_req_;   // value-initialised to indeterminate
MPI_Request recv_req_;
```

`MPI_Request` is an opaque type; passing an uninitialised value to `MPI_Test` or `MPI_Wait` before a matching `MPI_Isend`/`MPI_Irecv` is UB under the MPI standard. The correct sentinel is `MPI_REQUEST_NULL`.

**Fix**:
```cpp
MPI_Request send_req_ = MPI_REQUEST_NULL;
MPI_Request recv_req_ = MPI_REQUEST_NULL;
```

---

### H22. Factory `init()` is not thread-safe — `src/geneva/G_OptimizationAlgorithm_FactoryT.cpp`

`init()` writes to shared factory state (parameter proxies, default values) and is called lazily on the first `get()` call. If two threads call `get()` concurrently before `init()` has completed, the factory state is initialised twice and the second initialisation may overwrite partially-completed state from the first.

**Fix**: Protect lazy initialisation with `std::once_flag` + `std::call_once`.

---

### H23. `GValidityCheckContainerT::load_()` calls wrong base-class `load_()` — `include/geneva/GValidityCheckContainerT.hpp`

```cpp
void load_(GObject const* cp) override {
    GObject::load_(cp);     // BUG: should be GValidityCheckContainerT_parent::load_(cp)
    ...
}
```

The intermediate base class's serialised members are skipped on deserialisation. Any fields added to the intermediate base class are silently lost on reload, which includes validity thresholds and check-object collections.

**Fix**: Call the correct parent:
```cpp
GValidityCheckContainerT_parent::load_(cp);
```

---

## MEDIUM-severity findings

### M1. `assert()` in production code — `src/geneva/GParameterSet.cpp::getCrossOverPos`

```cpp
assert(range > 0);
```

`assert()` is compiled out in release builds. If `range == 0` (e.g. a single-element collection), the subsequent modulo operation is division by zero. Replace with:
```cpp
if (range == 0) throw gemfony_exception(...);
```

---

### M2. `assignValueVector` missing activity-mode check for `int32` and `bool` parameters — `src/geneva/GParameterSet.cpp`

The floating-point parameter branch checks `isActive()` before applying values from the vector. The `int32` and `bool` branches do not, so inactive parameters (frozen during constrained optimisation) may have their values overwritten by the value vector.

**Fix**: Add the same `if (!p->isActive()) continue;` guard to both branches.

---

### M3. Garbled token / identifier name in grammar or parser — `src/geneva/GParameterPropertyParser.cpp`

One token name in the Spirit grammar rule table contains a typographic corruption (identified in review as a character-transposition). While it does not affect compiled parser behaviour, it makes debugging grammar failures significantly harder.

**Fix**: Correct the token name string.

---

### M4. Property-tree key missing leading dot — `src/geneva/Go2.cpp` (or `GParameterSet.cpp`)

```cpp
pt.get<std::string>("optimization.result");    // correct
pt.get<std::string>("optimizationresult");     // missing dot — silently returns default
```

One property-tree lookup is missing the `.` separator, causing it to silently return the default value rather than the actual stored result.

**Fix**: Add the missing `.` to the key string.

---

### M5. `int32` overflow in parameter multiplication — `include/geneva/GConstrainedIntT.hpp`

A range-width computation for `GConstrainedInt32T` multiplies two `int32_t` values without widening:

```cpp
int32_t range = (upper_ - lower_) * someMultiplier;  // overflows when range * mult > 2^31
```

**Fix**: Widen to `int64_t` for the intermediate, or static_assert that the parameter type is large enough.

---

### M6. FP-to-int cast without clamp — `include/geneva/GConstrainedIntT.hpp`

```cpp
int_type result = static_cast<int_type>(fp_value);
```

If `fp_value` is slightly outside `[std::numeric_limits<int_type>::min(), max()]` due to floating-point rounding, the cast is UB in C++. Add `std::clamp` before the cast.

---

### M7. `GBiGaussAdaptorT` constructor leaves `sigma_` and `delta_` uninitialised — `include/geneva/GBiGaussAdaptorT.hpp`

The delegating constructor that takes only `adProb` does not call the full constructor and leaves `sigma_`, `sigma_reset_`, `delta_`, and `delta_reset_` at indeterminate values. The first `adapt()` call will use garbage step sizes.

**Fix**: Provide default-member-initialisers for all four fields:
```cpp
fp_type sigma_       = fp_type{0.1};
fp_type sigma_reset_ = fp_type{0.1};
fp_type delta_       = fp_type{0.5};
fp_type delta_reset_ = fp_type{0.5};
```

---

### M8. Second `GNumGaussAdaptorT` constructor does not initialise `sigma_reset_` — `include/geneva/GNumGaussAdaptorT.hpp`

The constructor `GNumGaussAdaptorT(fp_type sigma, fp_type sigmaSigma, fp_type minSigma, fp_type maxSigma)` sets `sigma_` but not `sigma_reset_`. If `resetSigma()` is called before any serialisation round-trip, it resets to the uninitialised value.

**Fix**: Add `sigma_reset_ = sigma;` in the constructor body, or use a default member initialiser.

---

### M9. `sortMuCommaNuMode` does not fully swap the replacement buffer — `src/geneva/GEvolutionaryAlgorithm.cpp`

The (µ, λ) replacement sorts a temporary vector and then copies the first µ individuals back, but does not update auxiliary data structures (e.g. fitness caches, iteration stamps) that reference positions in the main population vector. After a sort, these indices refer to the wrong individuals.

**Fix**: Use the same full-swap pattern (`std::swap(population_, sorted_temp)`) already used in `sortMuPlusLambdaMode`.

---

### M10. `GConjugateGradientDescent` is an incomplete stub — `src/geneva/GConjugateGradientDescent.cpp`

```cpp
std::tuple<double,double> GConjugateGradientDescent::cycleLogic_() {
    return {0.0, 0.0};   // not implemented
}
```

The class compiles and can be instantiated but performs no optimisation. There is no `#error`, `static_assert`, or runtime guard to prevent accidental use.

**Fix**: Either complete the implementation or add `static_assert(false, "GConjugateGradientDescent is not implemented")` (guarded with a template parameter so it triggers only on instantiation), and remove from the public API until ready.

---

### M11. `GParameterPropertyParser` re-checks `sSpecVec.empty()` inside the loop — `src/geneva/GParameterPropertyParser.cpp`

```cpp
for (auto const& spec : sSpecVec) {
    if (sSpecVec.empty()) break;   // dead: loop body never entered if empty
    ...
}
```

The check is always false (the loop body is only entered when the vector is non-empty). Remove the dead check. More likely the intent was to check a *different* collection that might be emptied by a previous iteration.

---

### M12. `setExecMode` and the constructor handle invalid modes inconsistently — `src/geneva/Go2.cpp`

The constructor throws on an invalid execution mode; `setExecMode()` logs a warning and silently falls back to serial mode. Callers who set the mode after construction get silent degradation instead of an error.

**Fix**: Make `setExecMode` throw for invalid values, consistent with the constructor.

---

### M13. `GValidityCheckContainerT::load_()` also skips the check-objects collection — same location as H23

After the wrong base-class call (H23), the check-objects container (`checkObjects_`) is also not re-populated. Even after fixing H23, verify that `checkObjects_` is explicitly serialised and restored.

---

### M14. `GParameterPropertyParser` accumulated spec vector not reset between parses — `src/geneva/GParameterPropertyParser.cpp`

`sSpecVec` is a member that accumulates results across multiple `parse()` calls. If `parse()` is called more than once (e.g. in a restarted run), results from the previous parse contaminate the new one.

**Fix**: Clear `sSpecVec` at the start of each `parse()` call.

---

### M15. `G_OptimizationAlgorithm_FactoryT` initialisation race — see H22 (MEDIUM variant)

Even without concurrent calls, `init()` may be called redundantly (e.g. from both `get()` and `clone()`) due to missing guard. The double-init is not fatal but wastes time re-parsing the config file.

**Fix**: Same `std::once_flag` as H22.

---

### M16. `GGradientDescent` does not validate that the parameter set is differentiable — `include/geneva/GGradientDescent.hpp`

`GGradientDescent` calls numerical differentiation helpers that require a continuous parameter space. If a `GBooleanObject` or `GInt32Object` is present in the individual, the gradient is undefined. No check is performed; the algorithm runs to completion producing meaningless results.

**Fix**: In `init()`, iterate the parameter set and throw if any non-differentiable parameter type is detected.

---

## LOW / TRIVIAL findings

### L1. Typo "happennot" in log / comment string — `src/geneva/GParameterSet.cpp` (or adjacent)

```cpp
glogger << "This should happennot" << ...;
```

**Fix**: `"This should not happen"`.

---

### L2. `GBooleanAdaptor::adapt` ternary vs `!value` — `include/geneva/GBooleanAdaptor.hpp`

```cpp
value = value ? false : true;   // equivalent to !value but wordier
```

**Fix**: `value = !value;`

---

### L3. `GConjugateGradientDescent` should not be exported in the public API — `include/geneva/GConjugateGradientDescent.hpp`

As noted in M10, this class is a stub. It appears in the installed header set and will confuse users. Mark the header `// INTERNAL — not for public use` or gate it behind a build flag until the implementation is complete.

---

### L4. Several `[[nodiscard]]` annotations missing on query functions — multiple headers

Functions such as `G_OptimizationAlgorithm_Base::minIterationPassed()`, `GParameterSet::fitnessCalculation()` wrappers, and several factory `get()` calls return values that the caller must act on. Annotating them `[[nodiscard]]` lets the compiler catch silent discard bugs at zero runtime cost.

---

### L5. Inconsistent use of `std::size_t` vs `int` for iteration counts — multiple files

Some loop counters and population sizes are `int` (signed), others `std::size_t` (unsigned). Mixed signed/unsigned comparisons produce compiler warnings and, in edge cases involving negative signed values, wrap-around bugs. Standardise on `std::size_t` (or a named alias) throughout.

---

## C++20 modernisation notes

These are not bugs but represent opportunities to clarify intent and catch latent issues with modern language features:

- **Ranges algorithms** (`std::ranges::all_of`, `std::ranges::sort`) would remove manual index loops and eliminate several of the off-by-one risks found above (H10, H13).
- **`std::atomic<std::shared_ptr<T>>`** is already used correctly in `GSingletonT` (post-H2 fix); the same pattern should be applied to the MPI request handles (`MPI_REQUEST_NULL` sentinel + atomic wrapper) so cancellation is safe across threads.
- **`std::jthread`** with a `std::stop_token` would replace the manual `halt_flag` signal-handler pattern (H1) with a structured, race-free alternative.
- **`std::expected<T, E>`** (C++23, available in GCC 13) or a result-type wrapper would make MPI error paths first-class rather than requiring every call site to check a raw `int`.
- **Designated initialisers** on large `struct` parameters (e.g. algorithm configuration bundles) would make it impossible to silently skip a field on initialisation (relevant to M7, M8).

---

*Review completed 2026-05-15. 23 HIGH, 16 MEDIUM, 5 LOW/TRIVIAL findings. No source files were modified.*
