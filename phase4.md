# Phase 4: Geneva Library — Catch2 Migration

**Branch:** catch2-migration  
**Status:** COMPLETE — all 25 non-benchmark tests pass (100%)

---

## Scope

Phase 4 migrated the largest library: the `geneva` library (~1,500 Boost.Test macros across ~115 header and source files).

### 4a — `Geneva_tests.hpp` (internal test header)

`include/geneva/tests/Geneva_tests.hpp` was completely rewritten:
- Removed `#include <boost/test/unit_test.hpp>`, `#include <boost/mpl/list.hpp>`, and Boost-specific types
- Added `#include <catch2/catch_test_macros.hpp>` and matcher headers
- Wrapped everything in `namespace Gem::Tests`
- `StandardTests_no_failure_expected<T>` and `StandardTests_failures_expected<T>` converted to plain template functions

### 4b — `GenevaStandardTests.cpp` (unit test driver)

`tests/geneva/UnitTests/GenevaStandardTests.cpp` was completely rewritten:
- Replaced `BOOST_AUTO_TEST_SUITE` / `BOOST_TEST_CASE_TEMPLATE` with `TEMPLATE_TEST_CASE`
- All 58 test cases (12 TEMPLATE_TEST_CASE pairs × adaptors, parameters, collections, algorithms) now use Catch2

`tests/geneva/UnitTests/CMakeLists.txt` updated to link `Catch2::Catch2WithMain`.

### 4c — `GObject.hpp` (root of all Geneva header includes)

Switched the `#ifdef GEM_TESTING` block from Boost.Test headers to Catch2:
```cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
```

### 4d — Bulk conversion (~115 files)

All `include/geneva/*.hpp` and `src/geneva/*.cpp` files with `#ifdef GEM_TESTING` blocks were migrated:

**Automated conversions (sed + Python scripts):**
- 83 `BOOST_CHECK_MESSAGE(cond, msg)` → `INFO(msg); CHECK(cond)` via `convert_boost_check_message.py`
- All remaining `BOOST_CHECK_*` / `BOOST_REQUIRE_*` macros replaced via `sed`
- All `using boost::unit_test_framework::test_suite/test_case` removed
- All `#include <boost/test/unit_test.hpp>` removed

**Manual fixes for subtle Catch2 differences:**

| Issue | Files | Fix |
|---|---|---|
| Multi-line `CHECK_NOTHROW(expr;)` — trailing `;` inside | GAdaptorT.hpp (×6), GConstrainedFPT.hpp (×8), others | `fix_nothrow_semicolons.py` + manual edit |
| `CHECK_THROWS_AS((expr, ExcType))` — both args in outer parens | GConstrainedNumT.hpp, GNumGaussAdaptorT.hpp, GNumT.hpp, GConstrainedFPT.hpp, GBooleanCollection.cpp, GTestIndividual1.cpp, GInt32Object.cpp, G_OptimizationAlgorithm_ParChildT_PersonalityTraits.cpp (30 occurrences) | `fix_throws_as.py` |
| Multi-line `CHECK_THROWS_AS(\n  expr;\n, ExcType\n)` — semicolon in expr | GAdaptorT.hpp (×4) | Manual edit |
| `CHECK(x = f())` — assignment inside CHECK | GNumGaussAdaptorT.hpp (×1) | Converted to plain assignment |
| `CHECK(expr)  // comment with );` — closing `)` inside comment | GConstrainedFPT.hpp:646 | Fixed comment placement |
| Multi-line `CHECK(expr)` missing trailing `;` | GNumFPT.hpp, GTestIndividual1.cpp | Manual edit |
| `CHECK(a && b)` needing extra parens | GHap_tests.hpp (already fixed in Phase 2), others via `fix_catch2_issues.py` | Extra parens added |

### CMake fix — Catch2 RTTI symbols in shared library

**Problem:** `libgemfony-geneva.so` is compiled with `GEM_TESTING`, so all in-class test methods reference Catch2 symbols (specifically `_ZTIN5Catch20ITransientExpressionE`, the typeinfo for `Catch::ITransientExpression`). The system Catch2 is static-only (no `-fPIC`), so it can't be linked directly into the `.so`.

**Solution:** In `src/geneva/CMakeLists.txt`:
```cmake
IF ( GENEVA_BUILD_TESTS )
    TARGET_LINK_LIBRARIES(${GENEVA_LIBNAME} INTERFACE Catch2::Catch2)
ENDIF ()
```

The `INTERFACE` keyword propagates `Catch2::Catch2` to all downstream executables without embedding it in the `.so`. When those executables run, their linked Catch2 provides the typeinfo symbols that the shared library's test code references at runtime.

---

## Results

```
All tests passed (3 479 244 assertions in 58 test cases)
```

Non-benchmark CTest results:
```
100% tests passed, 0 tests failed out of 25
```

(The 3 excluded tests — GOptimizationBenchmark, GParallelisationOverhead, GSerializationOverhead — always time out; they are long-running optimization runs, unrelated to the Catch2 migration.)

---

## Phase 5: geneva-individuals (completed as part of this commit)

Four `.cpp` files in `src/geneva-individuals/` retained `using boost::unit_test_framework::test_suite/test_case` declarations. These were removed with `sed`. GTestIndividual3.cpp had 4 unconverted `BOOST_CHECK_NO_THROW`/`BOOST_CHECK` macros; these were converted to `CHECK_NOTHROW`/`CHECK`.

---

## Phase 6: Examples (completed as part of this commit)

**10_GStarter:**
- `GStarterIndividual.cpp`: 5 BOOST_ macros converted (`BOOST_CHECK_NO_THROW` → `CHECK_NOTHROW`, `BOOST_CHECK_CLOSE` → `CHECK_THAT(...WithinRel(...))`, `BOOST_CHECK` → `CHECK`); `using` declarations removed
- `Tests/UnitTests/GStarterIndividualUnitTests.cpp`: completely rewritten — `BOOST_TEST_CASE_TEMPLATE` → `TEMPLATE_TEST_CASE` (2 test cases)
- `Tests/UnitTests/CMakeLists.txt`: added `Catch2::Catch2WithMain`

**15_GCUDAWorker:**
- `GImageIndividual.cpp`: removed `using` declarations, converted 1 `BOOST_REQUIRE` → `REQUIRE`
- `GImagePOM.hpp`: removed `using` declarations
