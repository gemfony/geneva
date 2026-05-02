# Phase 8: Hygiene — Explicit Catch2 Includes, Remove Transitive Include via GObject.hpp

**Branch:** catch2-migration  
**Commit:** `543b159f`  
**Status:** COMPLETE — 22/22 non-benchmark CTest tests pass

---

## Scope

Phase 8 is a build-hygiene phase. After the migration (Phases 0–7), `GObject.hpp` carried a `#ifdef GEM_TESTING` block that included the Catch2 headers transitively into every Geneva translation unit. This pulled the test framework into all consumers at compile time, even units that contain no test assertions. Phase 8 moves the Catch2 include to each file that actually uses it.

---

## Approach

1. Identified all files that directly use Catch2 assertion macros (`CHECK`, `REQUIRE`, `CHECK_NOTHROW`, `CHECK_THROWS_AS`, `INFO`, `CAPTURE`, `FAIL`, etc.) via:
   ```bash
   grep -rln '\bCHECK\b|\bREQUIRE\b|CHECK_NOTHROW|CHECK_THROWS' \
     include/geneva src/geneva include/geneva-individuals src/geneva-individuals
   ```
   Result: 14 headers + 26 source files = **40 files**.

2. Verified none of the affected files use floating-point matchers (`WithinRel`/`WithinAbs`) — so only `catch_test_macros.hpp` is needed, not the matcher headers.

3. Added the following block after the last `#include` in each of the 40 files (via `/tmp/add_catch2_includes.py`):
   ```cpp
   #ifdef GEM_TESTING
   #include <catch2/catch_test_macros.hpp>
   #endif /* GEM_TESTING */
   ```

4. Removed the `#ifdef GEM_TESTING` Catch2 include block from `GObject.hpp` entirely:
   ```cpp
   // REMOVED from GObject.hpp:
   #ifdef GEM_TESTING
   #include <catch2/catch_test_macros.hpp>
   #include <catch2/matchers/catch_matchers.hpp>
   #include <catch2/matchers/catch_matchers_floating_point.hpp>
   #endif /* GEM_TESTING */
   ```

---

## Files Modified

**14 headers** (template classes with inline test method bodies):

| File |
|---|
| `include/geneva/GAdaptorT.hpp` |
| `include/geneva/GConstrainedDoubleCollection.hpp` |
| `include/geneva/GConstrainedFPT.hpp` |
| `include/geneva/GConstrainedIntT.hpp` |
| `include/geneva/GConstrainedNumT.hpp` |
| `include/geneva/GFPNumCollectionT.hpp` |
| `include/geneva/GIntNumCollectionT.hpp` |
| `include/geneva/GNumCollectionT.hpp` |
| `include/geneva/GNumFPT.hpp` |
| `include/geneva/GNumGaussAdaptorT.hpp` |
| `include/geneva/GNumIntT.hpp` |
| `include/geneva/GNumT.hpp` |
| `include/geneva/G_OptimizationAlgorithm_FactoryT.hpp` |
| `include/geneva/GParameterBaseWithAdaptorsT.hpp` |

**26 source files**:

| File |
|---|
| `src/geneva/GBooleanAdaptor.cpp` |
| `src/geneva/GBooleanCollection.cpp` |
| `src/geneva/GBooleanObject.cpp` |
| `src/geneva/GBooleanObjectCollection.cpp` |
| `src/geneva/GConstrainedDoubleCollection.cpp` |
| `src/geneva/GConstrainedDoubleObject.cpp` |
| `src/geneva/GConstrainedDoubleObjectCollection.cpp` |
| `src/geneva/GConstrainedInt32ObjectCollection.cpp` |
| `src/geneva/GDoubleBiGaussAdaptor.cpp` |
| `src/geneva/GDoubleCollection.cpp` |
| `src/geneva/GDoubleGaussAdaptor.cpp` |
| `src/geneva/GDoubleObject.cpp` |
| `src/geneva/GDoubleObjectCollection.cpp` |
| `src/geneva/GInt32FlipAdaptor.cpp` |
| `src/geneva/GInt32GaussAdaptor.cpp` |
| `src/geneva/GInt32Object.cpp` |
| `src/geneva/GInt32ObjectCollection.cpp` |
| `src/geneva/GObject.cpp` |
| `src/geneva/G_OptimizationAlgorithm_EvolutionaryAlgorithm.cpp` |
| `src/geneva/G_OptimizationAlgorithm_ParChildT_PersonalityTraits.cpp` |
| `src/geneva/G_OptimizationAlgorithm_SwarmAlgorithm_PersonalityTraits.cpp` |
| `src/geneva/GParameterBase.cpp` |
| `src/geneva/GParameterObjectCollection.cpp` |
| `src/geneva/GParameterSet.cpp` |
| `src/geneva/GTestIndividual1.cpp` |
| `src/geneva-individuals/GTestIndividual3.cpp` |

**1 header cleaned:**

- `include/geneva/GObject.hpp` — `#ifdef GEM_TESTING` Catch2 block removed

---

## Note on Non-Asserting Files

Several files have `#ifdef GEM_TESTING` blocks but contain no CHECK/REQUIRE calls (e.g., `GConstrainedInt32Object.cpp`). These only declare or delegate to parent test methods; the assertions live in the parent template headers (which got their own explicit includes). These files compile correctly without a direct Catch2 include because no macros are expanded in their translation units directly.

---

## Verification

**Build:**
```bash
cd /home/rberlich/build && make -j$(nproc)
```
Result: clean build, zero errors.

**CTest:** 22/22 tests passed (excluding long-running benchmarks).

**Residual check:**
```bash
grep -n 'catch2' include/geneva/GObject.hpp   # → no output (correct)
```
