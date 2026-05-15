# Geneva Release Testplan — Implementation Prompt

**Date**: 2026-05-14  
**Branch**: catch2-migration (or new branch `release-testplan`)  
**Target version**: 1.11 "Hendaye"

---

## Goal

Implement the 25 test points in `ReleaseTestplan.md` as automated, reproducible checks that can be run in CI or locally by a release engineer. Where a test point is already covered, document it. Where it is not, create the missing infrastructure.

---

## Current State

The following test points from `ReleaseTestplan.md` are **already covered**:

| # | Point | How |
|---|---|---|
| 1 | Build with GCC/Clang | Verified 2026-05-14: GCC 15 Debug+Release, Clang 21 Debug+Release (4-way matrix, all pass) |
| 7 | `ctest` full suite | 28/28 pass with both GCC 15 and Clang 21 as of 2026-05-14 |
| 8 | `GenevaStandardTests` directly | 3.4M+ assertions, all pass |
| 14 | EA algorithm | Covered by `GBrokerSanityChecks` and `GBrokerOverhead` |
| 17 | `10_GStarter` builds | Included in `gemfony-build-all` |

**Build matrix requirement**: Points 1–8 must be verified with all four combinations:
- `g++` + `Debug`
- `g++` + `Release`
- `clang++` + `Debug`
- `clang++` + `Release`

The `release-check.sh` script (see below) automates this matrix via CMake presets.

The following are **not yet automated** and require new work:

---

## Build System Tests (points 1–6, 8)

### What to implement

**1. CMake presets** (`CMakePresets.json` at repo root):

Create a `CMakePresets.json` with configure presets for all required build matrices:

```json
{
  "version": 6,
  "configurePresets": [
    { "name": "gcc-debug",  "cacheVariables": { "CMAKE_CXX_COMPILER": "g++",   "GENEVA_BUILD_TYPE": "Debug"   } },
    { "name": "gcc-release","cacheVariables": { "CMAKE_CXX_COMPILER": "g++",   "GENEVA_BUILD_TYPE": "Release" } },
    { "name": "clang-debug","cacheVariables": { "CMAKE_CXX_COMPILER": "clang++","GENEVA_BUILD_TYPE": "Debug"  } },
    { "name": "clang-release","cacheVariables": {"CMAKE_CXX_COMPILER": "clang++","GENEVA_BUILD_TYPE": "Release"} },
    { "name": "sanitize",   "cacheVariables": { "CMAKE_CXX_COMPILER": "clang++","GENEVA_BUILD_TYPE": "Sanitize"} },
    { "name": "minimal",    "cacheVariables": { "GENEVA_BUILD_TESTS": "FALSE", "GENEVA_BUILD_EXAMPLES": "FALSE", "GENEVA_BUILD_BENCHMARKS": "FALSE" } },
    { "name": "full",       "cacheVariables": { "GENEVA_BUILD_TESTS": "TRUE",  "GENEVA_BUILD_EXAMPLES": "TRUE",  "GENEVA_BUILD_BENCHMARKS": "TRUE", "GENEVA_BUILD_WITH_MPI_CONSUMER": "TRUE" } }
  ]
}
```

**2. Shell script `scripts/release-check.sh`**:

A single script that:
- Configures and builds each preset in a temporary directory
- Runs `ctest` for each build
- Logs pass/fail per preset to a summary file
- Exits non-zero if any preset fails

**Implementation notes**:
- Use `cmake --preset <name>` and `cmake --build --preset <name>`
- Store build artifacts in `$TMPDIR/geneva-release-<preset>` to avoid clobbering
- Use `ctest --test-dir <build_dir>` to run tests per preset

---

## Sanitize Build — Thread Sanitizer (point 9)

Currently `GENEVA_BUILD_TYPE=Sanitize` is defined in CMake but the exact flags are not verified. Check:

1. Open `cmake/GenevaOptions.cmake` (or wherever `Sanitize` is handled) and verify:
   - `-fsanitize=thread` is added for Sanitize builds
   - `-fsanitize=address,undefined` variant is also available
2. Run `GenevaStandardTests` under TSan and capture its output
3. Run `GBrokerSanityChecks` under TSan (it exercises multi-threaded consumers)

**Known issue**: TSan and CUDA do not mix. The `USECUDARNG` flag must be `FALSE` for Sanitize builds. Ensure CMake warns if both are enabled.

---

## Consumer Tests (points 10–13)

### Serial consumer
Already exercised by single-threaded runs. No new code needed.

### Multithreaded consumer — explicit N=2 and N=system_threads test (point 11)

Add a CTest entry to `benchmarks/geneva/GBrokerSanityChecks/CMakeLists.txt`:

```cmake
ADD_TEST(GBrokerSanityChecks_2threads GBrokerSanityChecks -p 1 --nEvaluationThreads 2)
SET_TESTS_PROPERTIES(GBrokerSanityChecks_2threads PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
```

### Websocket consumer (point 12)

A new minimal test program is needed. Create `tests/geneva/ManualTests/GWebsocketConsumerTest/`:
- `GWebsocketConsumerTest.cpp`: launches server (background thread) + client in the same process
- Runs a 2-iteration EA, verifies completion
- Add to CTest with a 30-second timeout

**Key complication**: `GWebsocketConsumerT` requires a network port. Use a fixed port (e.g., 10000) and guard with `GENEVA_BUILD_NETWORK_TESTS` CMake option (off by default, enabled for release checks).

### MPI consumer (point 13)

Add to `tests/geneva/ManualTests/GMPIConsumerTest/` (only built with `GENEVA_BUILD_WITH_MPI_CONSUMER`):
- Launch 1 server rank + 1 worker rank via `mpirun -n 2`
- Guard with `MPIEXEC_EXECUTABLE` being available
- CTest registration:
  ```cmake
  if(GENEVA_BUILD_WITH_MPI_CONSUMER AND MPIEXEC_EXECUTABLE)
    ADD_TEST(GMPIConsumerTest ${MPIEXEC_EXECUTABLE} -n 2 GMPIConsumerTest)
  endif()
  ```

---

## Algorithm Coverage (points 14–16)

### Each algorithm runs to completion (point 14)

`GenevaStandardTests` exercises parameter types but not full optimization runs. Create `tests/geneva/IntegrationTests/GAlgorithmRunTest/`:

```cpp
// For each algorithm type (EA, SA, Swarm, GD, ParameterScan):
// 1. Create a GTestIndividual1 with 5 double parameters
// 2. Configure Go2 with that algorithm
// 3. Run for maxIterations=10
// 4. Verify result has lower fitness than initial random individual
```

This test must be deterministic (seed the RNG) so it can be run in CI without flakiness.

### Go2 JSON config round-trip (point 15)

Add a test in `tests/geneva/IntegrationTests/Go2ConfigRoundtrip/`:
1. Instantiate `Go2` with EA algorithm
2. Call `Go2::writeConfigFile("test_config.json")`
3. Create a new `Go2` instance, call `Go2::readConfigFile("test_config.json")`
4. Verify key parameters match (population size, max iterations, etc.)

### Checkpoint save and restore (point 16)

Add to the integration tests:
1. Run EA for 5 iterations, save checkpoint to `checkpoints/`
2. Start a new run, restore from checkpoint
3. Verify the restored run starts from iteration 5 (not 0) and reaches the same fitness values

**Required code changes**: Checkpoint serialization is already implemented in `G_OptimizationAlgorithm_Base`. Verify that `checkpointBaseName_` is set and the checkpoint files are written. The test needs a deterministic individual.

### Go2 algorithm chaining (supplementary — not in original 25 points)

`Go2` can be used sequentially: run one algorithm to completion, extract the best individual, then seed a second `Go2` instance with it. Test this EA → SA → GD chain:

```cpp
// tests/geneva/IntegrationTests/Go2ChainingTest/Go2ChainingTest.cpp
//
// 1. Create a GTestIndividual1 with 5 double parameters, seed RNG to fixed value.
// 2. Run Go2 with EA for 5 iterations; record best fitness F_ea.
// 3. Extract best individual from Go2 result set.
// 4. Create new Go2 with SA algorithm; add the EA best individual as seed.
// 5. Run SA for 5 iterations; record best fitness F_sa.
// 6. REQUIRE(F_sa <= F_ea) — SA must not regress the EA result.
// 7. Extract best individual from SA; create new Go2 with GD algorithm.
// 8. Run GD for 5 iterations; record best fitness F_gd.
// 9. REQUIRE(F_gd <= F_sa) — GD must not regress the SA result.
//
// The test verifies that algorithm hand-off via GParameterSet copy/clone
// works correctly end-to-end without resetting fitness to a worse value.
```

Add to `tests/geneva/IntegrationTests/CMakeLists.txt` in the same pattern as other integration tests, with `TIMEOUT 120`.

---

## Examples (points 17–19)

### 10_GStarter (point 17)

Already builds. Add a CTest entry that runs it for 5 iterations:

```cmake
ADD_TEST(GStarter_smoke GStarter --maxIterations 5 --maxMinutes 0)
SET_TESTS_PROPERTIES(GStarter_smoke PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} TIMEOUT 60)
```

### Networked example (point 18)

Similar to the websocket consumer test above. Use `01_GSimpleOptimizer` (example `14_GSimpleOptimizer` does not exist) or create a local client+server test. `01_GSimpleOptimizer` supports a `--client` flag and can be launched twice (server + client) in the same CI job via background process.

### CUDA example (point 19)

`15_GCUDAWorker`: already builds if `USECUDARNG=TRUE`. Add a smoke-run CTest that:
- Only registers when `CMAKE_CUDA_COMPILER` is found
- Runs for 3 iterations on the CUDA device
- Verifies non-zero output

---

## Out-of-tree Build (points 20–21)

### Install and use FindGeneva (point 20)

Add `cmake/tests/FindGenevaTest/` with a minimal `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.27)
project(GenevaFindTest)
find_package(Geneva REQUIRED)
add_executable(hello_geneva hello_geneva.cpp)
target_link_libraries(hello_geneva Geneva::geneva)
```

And a script in `scripts/test-install.sh` that:
1. Installs Geneva to a temp prefix (`cmake --install . --prefix /tmp/geneva-test`)
2. Configures the test project against that install
3. Builds and runs it

### FindGeneva mismatch detection (point 21)

In `cmake/FindGeneva.cmake`, add a check for `GENEVA_BUILD_TESTS`. If the consuming project passes `-DGENEVA_BUILD_TESTS=TRUE` but the installed tree was built without tests, emit a `cmake_path_error(FATAL_ERROR ...)` with a clear message.

---

## Installation and Runtime Linking (points 22–23)

### ldconfig and LD_LIBRARY_PATH (point 22)

In `scripts/release-check.sh`:

```bash
# After make install:
ldd /opt/geneva/bin/GStarter | grep "not found" && echo "ERROR: unresolved libs" && exit 1
```

### ldd check (point 23)

Add to `scripts/release-check.sh` — iterate all installed executables and check `ldd` for "not found" entries.

---

## Documentation and Release Metadata (points 24–25)

### CHANGES entry (point 24)

Add to `scripts/release-check.sh`:

```bash
grep -q "1.11" CHANGES || { echo "ERROR: CHANGES missing 1.11 entry"; exit 1; }
```

### Version string in CMakeLists.txt (point 24)

```bash
grep -q "VERSION 1.11" CMakeLists.txt || { echo "ERROR: CMakeLists.txt version mismatch"; exit 1; }
```

### make doc (point 25)

```bash
make doc 2>&1 | grep -c "warning:" | xargs -I{} sh -c 'if [ {} -gt 0 ]; then echo "Doxygen warnings: {}"; fi'
```

---

## Required Code Changes Summary

| Change | Location | Effort |
|---|---|---|
| `CMakePresets.json` | repo root | Low |
| `scripts/release-check.sh` | scripts/ | Low |
| Websocket consumer test | tests/geneva/ManualTests/ | Medium |
| MPI consumer test | tests/geneva/ManualTests/ | Medium (only with MPI) |
| Algorithm integration tests | tests/geneva/IntegrationTests/ | Medium |
| Go2 config round-trip test | tests/geneva/IntegrationTests/ | Low |
| Checkpoint save/restore test | tests/geneva/IntegrationTests/ | Medium |
| Go2 chaining test (EA→SA→GD) | tests/geneva/IntegrationTests/ | Medium |
| GStarter smoke CTest | examples/geneva/10_GStarter/CMakeLists.txt | Trivial |
| FindGeneva test | cmake/tests/ | Low |
| FindGeneva mismatch check | cmake/FindGeneva.cmake | Low |
| ldd check in release script | scripts/release-check.sh | Trivial |
| CHANGES/version grep | scripts/release-check.sh | Trivial |
| TSan guard for CUDA | cmake/GenevaOptions.cmake | Low |

**Total effort estimate**: 3–5 days for a single developer familiar with Geneva's test infrastructure.

---

## Non-automated Points (require human judgment)

- **Websocket/MPI examples on separate machines**: Can only be fully tested on a cluster or with separate processes. The automated test uses localhost.
- **CUDA example on GPU**: CI must have a GPU node; local GPU test is manual.
- **CHANGES editorial review**: Content of the CHANGES entry must be human-reviewed.
- **Doxygen undocumented symbols**: The check is mechanical but fixing warnings requires manual documentation work.

---

## Suggested Implementation Order

1. `CMakePresets.json` + `scripts/release-check.sh` (build matrix) — highest leverage  
2. GStarter smoke CTest + algorithm integration tests — catches regressions quickly  
3. Go2 config round-trip + checkpoint test + Go2 chaining test — validates serialization and algorithm hand-off  
4. FindGeneva test — validates install  
5. Websocket/MPI tests — last, requires network-aware test infrastructure  
