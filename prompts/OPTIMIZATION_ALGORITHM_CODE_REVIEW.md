# Code Review: Geneva Optimization Algorithm Classes

**Date:** 2026-05-08
**Reviewer:** Claude Sonnet 4.6
**Scope:** Optimization algorithm base and concrete algorithm classes

---

## Executive Summary

The optimization algorithm subsystem is architecturally sound and shows clear, consistent design intent throughout a deep inheritance chain (`G_Interface_OptimizerT` → `G_OptimizationAlgorithm_Base` → algorithm-specific classes). The codebase is feature-complete and well-commented. That said, the review found several bugs of varying severity, numerous C++20 modernization opportunities, and a handful of meaningful design/API inconsistencies.

**Most critical findings:**

1. **Critical bug — wrong variable checked in `setMaxIteration` and `setMinIteration`:** The validation guard compares the member variable against itself instead of comparing the new argument, making the guard useless.
2. **Critical bug — `setFiniteStep`/`setStepSize` validate the stale member instead of the argument:** The range checks fire on the old value, not the value being set, so invalid arguments slip through.
3. **Critical bug — `aDominatesB` loop body ignores the loop variable `i`:** The Pareto dominance test always checks criterion 0, making multi-criterion Pareto selection incorrect.
4. **Major bug — `getBestGlobalIndividual_` diverges between DEBUG and RELEASE:** DEBUG returns a non-cloned pointer into the priority queue; RELEASE clones it. The returned pointer in DEBUG mode is therefore not owned by the caller.
5. **Major bug — `neighborhoodsHaveNominalValues` has inverted logic:** It returns `false` when the condition is satisfied and `true` otherwise.
6. **Major serialization gap — `GGradientDescent::load_` omits several computed members** that are serialized in `serialize()`, creating divergence between the two code paths.

---

## File-by-File Review

---

### 1. `G_Interface_OptimizerT.hpp`

#### Bugs / Logic

**Line 125–126 / 181–182 — Redundant `std::move` on a return value:**
```cpp
return std::move(bestIndividuals);
```
This prevents NRVO (Named Return Value Optimization). The `std::move` here forces a move instead of allowing the compiler to construct in-place. Remove `std::move` from both `getBestGlobalIndividuals` and `getBestIterationIndividuals`.

**Lines 88–89 — `dynamic_pointer_cast` silently returns null if the cast fails:**
`getBestGlobalIndividual` and `getBestIterationIndividual` call `std::dynamic_pointer_cast` without checking the result. A wrong `individual_type` argument leads to a null `shared_ptr` returned to the caller with no diagnostic.
**Fix:** Add a null check after the cast and throw `geneva_exception` if it is null.

#### C++20 / Modernization

**Lines 63–65 — Explicitly deleted move operations should use `= delete` with a comment, not rely on custom `noexcept`:**
Deleting moves is intentional and documented in the comment. However, the explicit `noexcept` on deleted functions is redundant (deleted functions have no exception specification that matters). Consider clarifying the comment.

#### API / Design

**`get_best_mutex_` is not serialized and not copyable:** The `std::mutex` member makes `G_Interface_OptimizerT` non-copyable and non-movable. The deleted move operators reflect this, but the copy constructor is defaulted (line 213), which is silently deleted by the compiler because `std::mutex` is not copyable. The class should either explicitly delete copy construction/assignment with a comment explaining why, or use a wrapper that is copy-constructible.

---

### 2. `G_OptimizationAlgorithm_Base.hpp` / `G_OptimizationAlgorithm_Base.cpp`

#### Bugs / Logic

**Lines 885–894 (`.cpp`) — `setMaxIteration`: validation checks stale member, not the argument:**
```cpp
void G_OptimizationAlgorithm_Base::setMaxIteration(std::uint32_t maxIteration) {
    if(maxIteration_ > 0 && maxIteration_ <= minIteration_) { // BUG: should be `maxIteration`, not `maxIteration_`
        ...
    }
    maxIteration_ = maxIteration;
}
```
The guard uses `maxIteration_` (the old value) instead of `maxIteration` (the new value being set). If `maxIteration_` was already valid, the check always passes; if it was already invalid, the check fires even when setting a valid new value. Same bug at lines 919–928 in `setMinIteration`.
**Fix:** Change both checks to use the parameter names (`maxIteration` / `minIteration`), not the member names.

**Lines 1665–1679 (`.cpp`) — `getBestGlobalIndividual_`: DEBUG and RELEASE diverge on ownership semantics:**
```cpp
#ifdef DEBUG
    std::shared_ptr<GParameterSet> p = bestGlobalIndividuals_pq_.best();
    if(p) return p;              // returns a shared pointer INTO the queue — no clone
    else throw ...
#else
    return bestGlobalIndividuals_pq_.best()->clone<GParameterSet>(); // clones
#endif
```
In DEBUG mode the function returns a shared pointer to the live object in the priority queue (aliasing the internal queue). In RELEASE mode it returns an independent clone. Callers that modify the returned individual will silently corrupt the best-queue in DEBUG builds. The correct behaviour is consistently to clone. At minimum, this inconsistency must be documented, but the safer fix is to always clone.
Same asymmetry exists in `getBestIterationIndividual_` (lines 1703–1718).

**Lines 1964–1979 — `stallHalt`: off-by-one — threshold equals stalls triggers halt:**
```cpp
if(stallCounter_ > maxStallIteration_) { return true; }
```
The documentation in `setMaxStallIteration` says "maximum number of generations without improvement". With `>`, setting `maxStallIteration_ = 5` halts only after 6 stalls. Most users will expect `>=`. This is a semantic disagreement between name and behaviour; at minimum document it prominently. If `>=` is intended, change accordingly.

**Lines 257–279 (`.cpp`) — `checkpoint`: race-free removal of old file relies on `cp_last_` being correctly named `"empty"` initially:**
If a previous run crashed after `saveCheckpoint` but before `cp_last_` was updated, `cp_last_` may hold a stale path after deserialization. This is already handled (the code checks `std::filesystem::exists` before removing), but the sentinel value `"empty"` as a string is fragile. A `std::optional<std::filesystem::path>` would be cleaner and type-safe.

**Lines 748 — `optimize_` loop: `informationUpdate(INFOPROCESSING)` fires at iteration 0 only if `iteration_ % reportIteration_ == 0`:**
When `reportIteration_` is set to 1, iteration 0 emits processing info before any evaluation has happened (because `markIteration()` and `cycleLogic_()` have already been called, but `iteration_` increments at line 755 **after** the report check). The report at iteration 0 is therefore the correct iteration, but given that the first evaluation results are not yet folded into globals when `INFOINIT` fires (line 699), there may be a window where monitors see stale fitness data. Consider documenting this ordering precisely.

#### C++20 / Modernization

**Lines 236–237 (`.cpp`) — `new` in `createExecutor`:**
```cpp
executor_ptr = std::shared_ptr<Gem::Courtier::GBaseExecutorT<GParameterSet>>(
    new Gem::Courtier::GSerialExecutorT<GParameterSet>()
);
```
All three `case` branches use explicit `new`. Should use `std::make_shared`:
```cpp
executor_ptr = std::make_shared<Gem::Courtier::GSerialExecutorT<GParameterSet>>();
```

**Line 714 (`.hpp`) — `std::atomic<bool>` data member has redundant brace-initializer causing copy-constructor issues:**
`halted_{true}` initialises via the atomic's value constructor. The copy constructor manually calls `halted_.store(cp.halted_.load())` (line 222 of `.cpp`), which is correct but verbose. Consider a helper or using `std::atomic_ref` pattern.

**`setDefaultPopulationSize` takes `const std::size_t&` (line 530 `.hpp` / line 1757 `.cpp`):** Scalar types should be passed by value, not by const-reference.

**`minOnly_transformed_fitness` is called in many sort lambdas:** These lambdas all re-enter the individual through a virtual-dispatch chain on every comparison. Hoisting the fitness values into a temporary `std::vector<double>` before calling `std::sort` would be a significant speedup for large populations.

#### Serialization

**`bestIterationIndividuals_pq_` is not serialized (`.hpp` load/save):** The `load` and `save` functions serialize `bestGlobalIndividuals_pq_` but not `bestIterationIndividuals_pq_`. After deserialization from a checkpoint file, the iteration-best queue will be empty. This is likely intentional (it is per-iteration transient state), but it is not documented.

**`startTime_` and `file_startTime_` are not serialized:** After loading a checkpoint, the timed-halt and touch-halt features will behave incorrectly because `startTime_` (initialized in `optimize_` at line 715) and `file_startTime_` (initialized at line 718) are not restored. This is acceptable only if checkpoints are always resumed by calling `optimize()` again — which is how they are used — and the comment in the code clarifies that.

---

### 3. `G_OptimizationAlgorithm_EvolutionaryAlgorithm.hpp` / `.cpp`

#### Bugs / Logic

**Lines 1255–1261 — `aDominatesB`: loop iterates over `nCriteriaX` but body uses index-independent `isWorse(x_ptr, y_ptr)` without `i`:**
```cpp
for(std::size_t i = 0; i < nCriteriaX; i++) {
    if(isWorse(x_ptr, y_ptr))   // BUG: i is never used — always checks criterion 0
        return false;
}
```
The variable `i` is computed but the comparison `isWorse(x_ptr, y_ptr)` does not accept a criterion index. This means only criterion 0 is ever checked, and the function returns `true` (dominates) as long as `x` is not worse than `y` on criterion 0, regardless of all other criteria. **This is a fundamental correctness bug in multi-objective optimization.**
**Fix:** `isWorse` must accept a criterion index. Inspect the `isWorse` signature; if it uses only criterion 0, this entire loop collapses to one check and the Pareto computation is broken for any `nCriteriaX > 1`.

**Lines 412–443 (`populationSanityChecks_`) — TODO comment acknowledges missing PARETO mode checks:**
```cpp
if ( // TODO: Why are PARETO modes missing here ?
    ((sorting_mode_ == sortingMode::MUCOMMANU_SINGLEEVAL || ...
```
The PARETO modes are not validated against the population-size constraint. In MUCOMMANU_PARETO mode it is theoretically possible to have fewer individuals than parents if the pareto front selection empties children before the resize at line 779 of selectBest_, though `fixAfterJobSubmission` should prevent it. The TODO should be resolved.

**Lines 460–493 — `adaptChildren_`: after `tp_ptr_->wait()`, futures are checked only in DEBUG:**
```cpp
tp_ptr_->wait();
#ifdef DEBUG
    for(auto &f : futures_cnt) { f.get(); ... }
#endif
```
The futures are never consumed in RELEASE mode, meaning exceptions thrown from worker threads are silently discarded. Even if threads use the thread pool's internal exception capture mechanism, the pattern of ignoring futures in RELEASE is fragile. Consider always consuming them (after `wait()` they are immediately ready and non-blocking).

**Line 78 (`compare_`): copy-paste error in the comment:**
```cpp
// Check that we are dealing with a GBaseSwarm::GSwarmOptimizationMonitor reference...
```
This comment was copy-pasted from `GSwarmAlgorithm`. It should reference `GEvolutionaryAlgorithm`.

#### C++20 / Modernization

**Lines 597–608 (`fixAfterJobSubmission`) — `std::remove_if` + `erase` idiom should be `std::erase_if`:**
```cpp
old_work_items.erase(
    std::remove_if(old_work_items.begin(), old_work_items.end(), lambda),
    old_work_items.end()
);
```
C++20 provides `std::erase_if` for vectors. This is already used correctly in `runFitnessCalculation_` (line 550), so there is an inconsistency.

**Lines 617–628 (`fixAfterJobSubmission`) — `std::sort` with comparison on `bool` from `isParent()`:**
The sort uses `>` on `bool` return from `isParent()`, which is technically valid C++ but semantically fragile. A named comparator or `std::stable_partition` on the parent/child boundary would be clearer and equivalent.

**`tp_ptr_` is not serialized:** The thread pool is a transient object created in `init()` and destroyed in `finalize()`. It must not be serialized. This is correct, but the member is declared in the header (`.hpp` line 221) with no comment explaining why it is excluded from `serialize()`. Add such a comment to aid future maintainers.

#### Serialization

The `serialize()` function (`.hpp` line 75–83) serializes `sorting_mode_` and `n_threads_` and the parent class. `load_` (`.cpp` line 366–378) mirrors this exactly. No issues found.

**`tp_ptr_` correctly omitted from serialization** — not a bug, but should be documented with an inline comment.

---

### 4. `G_OptimizationAlgorithm_SimulatedAnnealing.hpp` / `.cpp`

#### Bugs / Logic

**Lines 714–716 — `saProb` always returns a value that may be `> 1`:**
```cpp
double GSimulatedAnnealing::saProb(const double &qParent, const double &qChild) {
    return exp(-(fMinOnlyChild - fMinOnlyParent) / t_);
}
```
When a child is better than the parent (`fMinOnlyChild < fMinOnlyParent`), the exponent is positive and `exp(...)` exceeds 1. The calling code handles this correctly (lines 677–678: `if(pPass >= 1.) { ... }`). However, as `t_` approaches 0, `exp(large negative / very_small)` evaluates to 0 (underflows gracefully). The floor in `updateTemperature_` (line 728) prevents `t_` from reaching zero, which is correct. But when `fMinOnlyChild == fMinOnlyParent` and `t_` is the minimum normalised double, `saProb` computes `exp(0) = 1.0`, so an equal child always replaces the parent. This may not be the intended behaviour late in an annealing run. **Minor algorithm concern; at least document the limiting behaviour.**

**Line 72 (`.cpp`) — copy-paste comment error:**
```cpp
// Check that we are dealing with a GBaseSwarm::GSwarmOptimizationMonitor reference
```
The comment was copy-pasted. Should reference `GSimulatedAnnealing`.

**`adaptChildren_`: same issue as EA regarding futures not consumed in RELEASE (lines 356–379).**

#### C++20 / Modernization

**Line 631 (`.cpp`) — `new` in `init()` for thread pool:**
```cpp
tp_ptr_.reset(new Gem::Common::GThreadPool(n_threads_));
```
Should use `std::make_shared` / `std::make_unique` (whichever owns the pool). The EA equivalent uses `std::make_shared` (line 811 of EA `.cpp`), creating an inconsistency.

**`saProb` and `updateTemperature` are private but non-virtual:** These helper methods are declared in the header and defined in the `.cpp`. Since they are private and the class has `NOLINT(cppcoreguidelines-special-member-functions)`, marking these `[[nodiscard]]` (for `saProb`) would improve call-site hygiene, though the only call-site does use the result.

#### Serialization

`serialize()` covers `t0_`, `t_`, `alpha_`, `n_threads_` plus the base class. `load_` covers the same set. No gaps found.

---

### 5. `G_OptimizationAlgorithm_SwarmAlgorithm.hpp` / `.cpp`

#### Bugs / Logic

**Lines 958–963 — `neighborhoodsHaveNominalValues` has inverted return logic:**
```cpp
bool GSwarmAlgorithm::neighborhoodsHaveNominalValues() const {
    for(std::size_t n = 0; n < n_neighborhoods_; n++) {
        if(n_neighborhood_members_cnt_[n] == default_n_neighborhood_members_)
            return false;  // BUG: returns false when size IS nominal
    }
    return true;
}
```
The loop returns `false` when a neighborhood **has** the nominal value — the condition is exactly backwards. The function name says "have nominal values"; it should return `false` when a neighborhood does **not** have the nominal value. This means every call-site that uses this function will behave incorrectly. 
**Fix:** Change `==` to `!=` in the comparison.

**Line 209 — `load_`: clones `p_load` instead of `p_load->global_best_ptr_`:**
```cpp
global_best_ptr_ = p_load->GObject::clone<GParameterSet>();
```
This clones `p_load` (the whole `GSwarmAlgorithm` object) via `GObject::clone<GParameterSet>()` — which will almost certainly return the wrong type or throw. It should be:
```cpp
global_best_ptr_ = p_load->global_best_ptr_->clone<GParameterSet>();
```
This is a critical correctness bug in checkpoint loading.

**Lines 540–553 — `updatePersonalBestIfBetter` has inverted comparison sense:**
```cpp
if(isBetter(
       std::get<G_TRANSFORMED_FITNESS>(
           ind_ptr->getPersonalityTraits<GSwarmAlgorithm_PersonalityTraits>()
               ->getPersonalBestQuality()
       ),
       ind_ptr->transformed_fitness(0),
       m
   )) {
    ind_ptr->getPersonalityTraits<GSwarmAlgorithm_PersonalityTraits>()->registerPersonalBest(ind_ptr);
}
```
The condition checks `isBetter(personalBest, current)`. If the personal best is better than the current position, it updates the personal best... to the current position. This replaces a better personal best with a worse current value. The condition should be `isBetter(current, personalBest)`:
```cpp
if(isBetter(ind_ptr->transformed_fitness(0),
            std::get<G_TRANSFORMED_FITNESS>(personal_best_quality), m)) {
    // current is better than personal best, so update
}
```
**This is a major algorithmic bug in the swarm algorithm.**

**Lines 1595–1602 — `adjustPopulation_` MUST FIX comments confirm known incomplete logic:**
```cpp
// TODO: This is catastrophic if work items didn't return in GSwarmAlgorithm,
// as it uses adjustPopulation to fix the population.
// MUST FIX
```
The path where `currentSize > n_neighborhoods_ && currentSize < defaultPopSize` simply truncates to `n_neighborhoods_` individuals and re-fills. This discards all but the first `n_neighborhoods_` individuals silently and is documented internally as catastrophic. This should be treated as a known critical defect.

**Lines 1613–1617 — `adjustPopulation_` when `currentSize > defaultPopSize`:**
```cpp
n_neighborhood_members_cnt_[n_neighborhoods_ - 1] =
    default_n_neighborhood_members_ + (currentSize - defaultPopSize);
```
This assigns all surplus individuals to the last neighborhood without actually removing them. The neighbourhood count is updated but the population vector is not truncated. `adjustNeighborhoods()` will later prune, but `adjustPopulation_()` is also called from `optimize_()` before the main loop, where `adjustNeighborhoods` hasn't been called yet. Another TODO comment at line 1637 acknowledges this.

#### Inefficiencies

**`getFirstNIPosVec` (lines 444–451) accumulates a sum for each call in O(n):**
```cpp
std::size_t nPreviousMembers = 0;
for(std::size_t n = 0; n < neighborhood; n++) {
    nPreviousMembers += vec[n];
}
```
With variable-sized neighborhoods this is unavoidable, but with `default_n_neighborhood_members_` fixed for the normal (nominal) case, this could be `return neighborhood * default_n_neighborhood_members_` in the common path. A comment explaining the two-path opportunity would be useful.

**`updatePositions()` and `findBests()` iterate the entire population multiple times in a single cycle.** These could be merged into a single pass, though this is a secondary concern.

#### Serialization

The `serialize()` function (`hpp` lines 68–86) covers all persistent members. Notably `velocities_cnt_` and `last_iteration_individuals_cnt_` are **not** serialized — these are transient optimization-loop state reinitialised in `init()` and `updatePositions()` respectively. This is correct. However, the `velocities_cnt_` field is declared in the protected section of the header (line 282) but is not listed in `serialize()`. A comment should make this exclusion explicit.

---

### 6. `G_OptimizationAlgorithm_GradientDescent.hpp` / `.cpp`

#### Bugs / Logic

**Lines 98–111 — `setFiniteStep` checks `finiteStep_` (old member) instead of `finiteStep` (parameter):**
```cpp
void GGradientDescent::setFiniteStep(double finiteStep) {
    if(finiteStep_ <= 0. || finiteStep_ > 1000.) { // BUG: should be `finiteStep`
        ...
    }
    finiteStep_ = finiteStep;
}
```
The validation guard fires on the existing (old) value of `finiteStep_`, not on the value being set. Any value of `finiteStep` passes as long as `finiteStep_` is currently valid. Same bug at lines 129–142 in `setStepSize`.
**Fix:** Change `finiteStep_` → `finiteStep` and `stepSize_` → `stepSize` in both guard expressions.

**Lines 269–273 — `load_` omits computed/derived members that are covered by `serialize()`:**
```cpp
// stepRatio_ = p_load->stepRatio_; // temporary parameter
// dbl_lower_parameter_boundaries_cnt_ = p_load->dbl_lower_parameter_boundaries_cnt_; // temporary parameter
...
```
These fields are **serialized** in the `serialize()` template (`.hpp` lines 83–85: `nFPParmsFirst_`, `finiteStep_`, `stepSize_` are explicit; but `serialize()` does not include `stepRatio_`, `dblLowerParameterBoundaries_`, `dblUpperParameterBoundaries_`, or `adjustedFiniteStep_`). The comments in `load_` call them "temporary parameters" but they appear in `compare_()` (lines 215–218), which compares them for equality testing. The result is that `compare_()` will always find differences for these fields after a serialize/deserialize cycle, making round-trip equality tests fail silently.

**Recommendation:** Either serialize these fields (adding them to `serialize()` alongside the base class call) and uncomment the `load_` assignments, or remove them from `compare_()`.

**Lines 399–418 — `updateParentIndividuals` uses `boost::numeric_cast` where `static_cast<double>` suffices:**
```cpp
parmVec[j] -= boost::numeric_cast<double>(
    stepRatio_ * (boost::numeric_cast<long double>(...))
);
```
`boost::numeric_cast<double>` from `long double` performs a runtime range check. The range of `long double` subsumes `double`, so the cast can never fail. This adds overhead and exception-handling code for no benefit. Use `static_cast<double>` directly.

#### C++20 / Modernization

**`DEFAULTGDSTARTINGPOINTS`, `DEFAULTFINITESTEP`, `DEFAULTSTEPSIZE` declared as `const` variables at namespace scope (lines 59–61):**
```cpp
const std::size_t DEFAULTGDSTARTINGPOINTS = 1;
const double DEFAULTFINITESTEP = 0.001;
const double DEFAULTSTEPSIZE = 0.1;
```
These should be `constexpr` and arguably belong in the class or a dedicated constants header, not at namespace scope in a public header. As namespace-scope non-`inline` constants they create a definition in every translation unit that includes the header (though this is fine because `const` at namespace scope has internal linkage in C++).

**`stepRatio_` is declared as `long double` (`.hpp` line 204):** This is used to preserve precision in the gradient step computation. The comment "NOTE: long double; Will be recalculated in init()" is present. This is intentional. However, mixing `long double` with `double` parameters through `boost::numeric_cast` adds complexity that could be simplified by using `double` throughout and increasing the precision in the intermediate calculation only where needed.

#### Serialization

`serialize()` (`.hpp` lines 77–85) serializes `nStartingPoints_`, `nFPParmsFirst_`, `finiteStep_`, `stepSize_`. The derived/cached fields `stepRatio_`, `dblLowerParameterBoundaries_`, `dblUpperParameterBoundaries_`, `adjustedFiniteStep_` are intentionally excluded (they are recomputed in `init()`). This is consistent, but creates the asymmetry with `compare_()` noted above.

---

### 7. `G_OptimizationAlgorithm_ParameterScan.hpp` / `.cpp`

#### Bugs / Logic

**Lines 234–239 — `baseScanParT::goToNextItem`: after wrapping, `step_` resets to 0 but `isAtTerminalPosition` checks `step_ >= nSteps_`:**
```cpp
bool goToNextItem() override {
    if(++step_ >= nSteps_) {
        step_ = 0;
        return true; // wrapped
    }
    return false;
}
```
After `step_` is reset to 0 and the function returns `true` (indicating wrap), the next call to `isAtTerminalPosition()` returns `false` because `step_ == 0 < nSteps_`. In `switchToNextParameterSet` (lines 925–943) the wrap return value (`true`) is what drives the counter-increment. This logic appears correct for the grid scan. However, if `nSteps_` is 0, `++step_` overflows to 1 which is `>= 0`, causing instant wrap and `step_` resets to 0 forever. The constructor sets `nSteps_(pps.nSteps)` without validating against 0. The `fillWithData` specializations require at least 2 steps for float/double types, but the template base class has no such guard. Add validation that `nSteps >= 1` in the `baseScanParT` constructor.

**Lines 746–747 (`randomShuffle`) — `scansPerformed_` incremented twice per iteration (once inside the while loop body, and potentially again):**
```cpp
if(++indPos >= this->getDefaultPopulationSize())
    break;

if(++scansPerformed_ >= simpleScanItems_) {
```
Note that `indPos` is incremented first, then `scansPerformed_`. If `indPos` reaches `getDefaultPopulationSize()` before `scansPerformed_` reaches `simpleScanItems_`, the loop breaks without setting `cycleLogicHalt_`. This means the optimization continues when there are more scans to perform — correct behaviour. But `scansPerformed_` is also incremented in the outer call path, so the counting depends on the relative magnitudes of population size and total scans. This logic is subtle and should be documented.

**Line 788 — `resetParameterObjects` resets `simpleScanItems_` to 0:**
```cpp
simpleScanItems_ = std::size_t(0);
```
If the user set `simpleScanItems_` via `setNSimpleScans`, calling `resetParameterObjects` (which is called from `resetToOptimizationStart_`) silently erases that setting. A subsequent `optimize()` call will therefore use simple-scan mode without any scans. This is a state-management issue.

**`cycleLogicHalt_` is not serialized** (not in `serialize()`) but it is in `compare_()` (line 393). After a checkpoint load, `cycleLogicHalt_` will be `false` (its default), but if the serialized object had it `true` (i.e., scan was nearly finished), the resumed run will re-scan what has already been done. The `serialize()` function at `.hpp` lines 578–589 does serialize `scansPerformed_` and `simpleScanItems_`, so partial state is preserved, but `cycleLogicHalt_` itself is excluded. This is likely intentional (re-enter a fresh cycle), but `compare_()` testing it without it being serialized is misleading.

#### C++20 / Modernization

**Lines 237–293 (`operator<<` for `parSet`) — pre-ranged-for iterator loops:**
All four loops use `std::vector<...>::const_iterator cit` patterns. Use range-for with structured bindings:
```cpp
for(const auto &[val, mode, name, pos] : pS.bParVec) { ... }
```

**Lines 315–333 (`GParameterScan` copy constructor) — manual clone loops using old-style iterators:**
```cpp
for(b_it = cp.b_cnt_.begin(); b_it != cp.b_cnt_.end(); ++b_it) {
    b_cnt_.push_back((*b_it)->clone());
}
```
Use range-for:
```cpp
for(const auto &p : cp.b_cnt_) b_cnt_.push_back(p->clone());
```
Same pattern in `load_` (lines 476–497).

**`scanParInterface` (lines 117–126) — pure virtual interface class without `= 0` on destructor body:**
The destructor is `virtual G_API_GENEVA ~scanParInterface() = default;` which is correct for a pure interface. However, the other virtual pure functions lack a default implementation note. Minor style point.

**`fillWithData<T>` generic trap function throws inside the function body (lines 73–91):** For C++20, this pattern should use `static_assert(false, ...)` with a `requires` constraint to get a compile-time error instead of a runtime throw. However, `static_assert(false)` triggers even when the template is not instantiated in standard C++23 (`if consteval` offers a path); for now a `[[noreturn]]` attribute and a `static_assert` in a `requires` clause would be improvements.

#### Serialization

`serialize()` covers `scanRandomly_`, `nMonitorInds_`, `b_cnt_`, `int32_cnt_`, `d_cnt_`, `f_cnt_`, `simpleScanItems_`, `scansPerformed_`. The transient `all_par_cnt_` is correctly excluded (it is rebuilt in `init()`). `cycleLogicHalt_` is not serialized (see Bugs section above).

`baseScanParT::serialize()` covers `var_`, `step_`, `nSteps_`, `lower_`, `upper_`, `randomScan_`, `typeDescription_`. The per-type distribution objects (`uniform_bool_`, `uniform_float_distribution_`, etc.) are private and not serialized — they are stateless (or their state is irrelevant for re-runs), which is correct.

---

## Cross-Cutting Findings

### Consistent patterns across EA and SA

Both `GEvolutionaryAlgorithm` and `GSimulatedAnnealing` have nearly identical `fixAfterJobSubmission`, `adaptChildren_`, and `runFitnessCalculation_` bodies. The code duplication is substantial. Abstracting the common fixup logic into `G_OptimizationAlgorithm_ParChild` (the shared parent) would eliminate this redundancy. This is a refactoring opportunity, not a bug.

### Halt helper functions are all private

All halt helpers (`timedHalt`, `stallHalt`, `iterationHalt`, `minIterationPassed`, etc.) are private in `G_OptimizationAlgorithm_Base`. This is correct design. However, `customHalt_` is `virtual private` with a default implementation returning `false`. It is correctly overridden in `GParameterScan`. No inconsistencies found here across the algorithm classes.

### `std::make_shared` vs `new` inconsistency

EA `init()` uses `std::make_shared` for the thread pool (`.cpp` line 811). SA `init()` uses `tp_ptr_.reset(new ...)` (`.cpp` line 631). These do the same thing but the inconsistency is confusing. All object creation should use `std::make_shared`.

Similarly, `getPersonalityTraits_()` in Swarm, SA, and GD uses `std::shared_ptr<X>(new X())` while EA uses `std::make_shared<X>()`. Standardize to `std::make_shared` throughout.

### All algorithms call `this->at(0)` without checking for empty population

In `qualityHalt()`, `updateStallCounter()`, `findBests()`, `cycleLogic_()` etc., `this->at(0)` is called without guarding against an empty population. By the time these are called (after `adjustPopulation_`), the population is guaranteed non-empty, but this implicit precondition should be documented, or a `DEBUG` assert added.

---

## Findings by Severity

### Critical

| ID | File | Lines | Description |
|----|------|-------|-------------|
| C1 | `G_OA_Base.cpp` | 885–894, 919–928 | `setMaxIteration`/`setMinIteration`: validation guard checks old member value, not the new argument — guard is always a no-op or incorrectly fires |
| C2 | `G_OA_GradientDescent.cpp` | 98–111, 129–142 | `setFiniteStep`/`setStepSize`: guard checks old member value, not the argument — invalid values pass through silently |
| C3 | `G_OA_EvolutionaryAlgorithm.cpp` | 1255–1261 | `aDominatesB`: loop index `i` never used — only criterion 0 is ever compared, making multi-criterion Pareto dominance completely wrong |
| C4 | `G_OA_SwarmAlgorithm.cpp` | 209 | `load_`: clones the entire source algorithm object instead of `global_best_ptr_` — checkpoint loading for Swarm is broken |

### Major

| ID | File | Lines | Description |
|----|------|-------|-------------|
| M1 | `G_OA_Base.cpp` | 1665–1679, 1703–1718 | `getBestGlobalIndividual_`/`getBestIterationIndividual_`: DEBUG returns a raw aliased pointer; RELEASE returns a clone — broken ownership semantics in DEBUG |
| M2 | `G_OA_SwarmAlgorithm.cpp` | 540–553 | `updatePersonalBestIfBetter`: comparison is inverted — replaces a better personal best with a worse current position |
| M3 | `G_OA_SwarmAlgorithm.cpp` | 958–963 | `neighborhoodsHaveNominalValues`: return value is inverted — returns `false` when sizes ARE nominal |
| M4 | `G_OA_GradientDescent.cpp` | 269–273 | `load_` silently skips fields that are included in `compare_()`, causing round-trip equality tests to fail |
| M5 | `G_Interface_OptimizerT.hpp` | 88–89, 121–125, 177–179 | `dynamic_pointer_cast` results unchecked — null returned to caller on type mismatch |
| M6 | `G_Interface_OptimizerT.hpp` | 125, 181 | `std::move` on return value defeats NRVO |

### Minor

| ID | File | Lines | Description |
|----|------|-------|-------------|
| m1 | `G_OA_Base.cpp` | 1964 | `stallHalt` uses `>` instead of `>=` — halts one iteration later than the user-specified maximum |
| m2 | `G_OA_EA.cpp` | 460–493 | `adaptChildren_` futures never consumed in RELEASE mode — exceptions from threads are silently lost |
| m3 | `G_OA_SA.cpp` | 356–379 | Same as m2 for SA |
| m4 | `G_OA_SwarmAlgorithm.cpp` | 1595–1617 | `adjustPopulation_` has acknowledged MUST FIX TODOs for under-population and over-population cases |
| m5 | `G_OA_ParameterScan.cpp` | 788 | `resetParameterObjects` zeros `simpleScanItems_` — erases user-configured scan count |
| m6 | `G_OA_Base.cpp` | 271 | Sentinel value `"empty"` for `cp_last_` is fragile; prefer `std::optional<std::filesystem::path>` |
| m7 | `G_OA_Base.hpp` | 530 | `setDefaultPopulationSize` takes `const std::size_t&` — scalar should be by value |

### Style / C++20

| ID | File | Description |
|----|------|-------------|
| S1 | All files | ~~`std::enable_if` SFINAE → C++20 `requires` constraints~~ **Not applicable:** CUDA targets include these headers and NVCC does not support `requires` syntax unless `CMAKE_CUDA_STANDARD 20` and CUDA ≥ 13.1 are in use. CUDA 13.1 has a known glibc 2.39 incompatibility on the current build host. Keep `std::enable_if` until a compatible CUDA toolchain is available. |
| S2 | `G_OA_Base.cpp` | `new` in `createExecutor` should use `std::make_shared` |
| S3 | `G_OA_SA.cpp` L631 | `tp_ptr_.reset(new ...)` inconsistency with EA's `make_shared` |
| S4 | All algorithms | `getPersonalityTraits_` uses `shared_ptr<X>(new X())` — should use `make_shared<X>()` |
| S5 | `G_OA_EA.cpp`, `G_OA_SA.cpp` | Old-style `remove_if`/`erase` should use C++20 `std::erase_if` (already used in `runFitnessCalculation_` — inconsistency) |
| S6 | `G_OA_ParameterScan.cpp` | Old-style iterator loops throughout; replace with range-for / structured bindings |
| S7 | `G_OA_GradientDescent.cpp` | `DEFAULTGDSTARTINGPOINTS` etc. should be `constexpr` |
| S8 | `G_OA_EA.cpp` L78, `G_OA_SA.cpp` L72 | Copy-paste comment errors referencing `GBaseSwarm::GSwarmOptimizationMonitor` |
| S9 | All algorithms | `std::sort` lambdas call `minOnly_transformed_fitness` on every comparison — consider pre-computing a key vector for large populations |
