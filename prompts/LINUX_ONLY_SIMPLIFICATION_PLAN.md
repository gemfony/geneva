# Plan: Linux-Only Simplification of Geneva Build System and Headers

## Background

Three related questions drive this plan:

1. **`BUILD_SHARED_LIBS` is now always ON** — is the `SET(BUILD_SHARED_LIBS ON)` call in
   `CommonGenevaBuild.cmake` still needed?
2. **`GEM_DYNAMIC` is always defined** — is the `#if defined GEM_DYNAMIC` outer guard in
   `GGlobalDefines.hpp` dead code? Can the `G_API_*` macros be simplified?
3. **`G_API_*` macros** — are these Windows-specific? Can they be removed or simplified for
   a Linux-only build?

The answers are: yes to all three. This plan covers both the targeted simplification (just the
dead-code removal) and the full Windows/macOS removal. Both are described so you can choose the
scope.

---

## Analysis of the Current State

### `BUILD_SHARED_LIBS ON` (redundant)

All five `ADD_LIBRARY` calls in `src/*/CMakeLists.txt` are now explicit `SHARED`:

```cmake
ADD_LIBRARY ( ${COMMON_LIBNAME} SHARED ${COMMONSOURCES} )
```

The `SET(BUILD_SHARED_LIBS ON)` in `CommonGenevaBuild.cmake` only affects `ADD_LIBRARY` calls
that omit the `STATIC`/`SHARED` keyword. Since all five are explicit, this line is a no-op.
It can be removed from all four copies of `CommonGenevaBuild.cmake`.

### `GEM_DYNAMIC` outer guard (dead code)

`CommonGenevaBuild.cmake` always calls:

```cmake
ADD_DEFINITIONS("-DGEM_DYNAMIC")
```

This makes the `#else /* GEM_DYNAMIC undefined */` branch in `GGlobalDefines.hpp` unreachable.
Lines 131–136 (the five empty `#define G_API_*`) are dead code. The outer `#if defined GEM_DYNAMIC`
guard can be removed, collapsing the block.

### `G_API_*` macros and Windows

`BOOST_SYMBOL_EXPORT` and `BOOST_SYMBOL_IMPORT` are defined by Boost per-platform:

| Platform | `BOOST_SYMBOL_EXPORT` | `BOOST_SYMBOL_IMPORT` |
|---|---|---|
| Windows (GCC/Cygwin) | `__attribute__((__dllexport__))` | `__attribute__((__dllimport__))` |
| Linux/macOS (GCC, non-Windows) | `__attribute__((__visibility__("default")))` | *(empty)* |

On Linux **without** `-fvisibility=hidden` (which Geneva never sets), every symbol is already
exported by default. `__attribute__((__visibility__("default")))` is therefore a no-op — it
changes nothing. This means the `G_API_*` macros are currently complete no-ops on Linux.

On Windows the `__declspec(dllexport/dllimport)` split is essential for DLL ABI. Without it,
Windows shared libraries fail to link. So `G_API_*` is a Windows-only concern.

The `GEM_*_EXPORTS` macros (e.g. `-DGEM_COMMON_EXPORTS` added per-library in
`src/common/CMakeLists.txt`) only matter for the export/import direction on Windows. On Linux
they select between two no-ops.

**Conclusion:** For a Linux-only build, all five `G_API_*` macros can be defined as empty.
The `GEM_*_EXPORTS` `-D` flags in the library `CMakeLists.txt` files become meaningless
and can be removed.

---

## Scope A: Minimal Dead-Code Removal (No platform removal yet)

This scope removes only the confirmed dead code. It is a pure simplification with no
behavioral change, suitable as a standalone commit before the full platform removal.

### A1 — `GGlobalDefines.hpp`: collapse the `GEM_DYNAMIC` guard

**File:** `include/common/GGlobalDefines.hpp`

Current block (lines ~99–137):

```cpp
#if defined GEM_DYNAMIC && (GEM_DYNAMIC != 0)
#ifdef GEM_COMMON_EXPORTS
#define G_API_COMMON BOOST_SYMBOL_EXPORT
#else
#define G_API_COMMON BOOST_SYMBOL_IMPORT
#endif /* GEM_COMMON_EXPORTS */
// ... same for HAP, COURTIER, GENEVA, INDIVIDUALS ...
#else /* GEM_DYNAMIC undefined */
#define G_API_COMMON
#define G_API_HAP
#define G_API_COURTIER
#define G_API_GENEVA
#define G_API_INDIVIDUALS
#endif /* GEM_DYNAMIC */
```

**Change:** Remove the outer `#if defined GEM_DYNAMIC` / `#else` / `#endif` wrapper.
Keep the five inner `#ifdef GEM_*_EXPORTS` blocks as-is.

Result:

```cpp
#ifdef GEM_COMMON_EXPORTS
#define G_API_COMMON BOOST_SYMBOL_EXPORT
#else
#define G_API_COMMON BOOST_SYMBOL_IMPORT
#endif /* GEM_COMMON_EXPORTS */
// ... same for HAP, COURTIER, GENEVA, INDIVIDUALS ...
```

No call sites change. The `G_API_*` macros still expand to `BOOST_SYMBOL_EXPORT` or
`BOOST_SYMBOL_IMPORT` (which are no-ops on Linux, but ready for Windows if re-added).

### A2 — `CommonGenevaBuild.cmake` (4 copies): remove `SET(BUILD_SHARED_LIBS ON)`

**Files:**
- `CMakeModules/CommonGenevaBuild.cmake`
- `examples/common/CMakeModules/CommonGenevaBuild.cmake`
- `examples/geneva/CMakeModules/CommonGenevaBuild.cmake`
- `examples/hap/CMakeModules/CommonGenevaBuild.cmake`

Remove the line `SET (BUILD_SHARED_LIBS ON)` from the "Geneva only supports shared libraries"
block in each copy. The block becomes just:

```cmake
################################################################################
# Geneva only supports shared libraries
ADD_DEFINITIONS("-DGEM_DYNAMIC")
```

### A3 — `ADD_DEFINITIONS("-DGEM_DYNAMIC")` — keep for now

Even after Scope A, `GEM_DYNAMIC` still gates the `BOOST_SYMBOL_EXPORT`/`IMPORT` dispatch
in the Windows re-add path. Keep the `ADD_DEFINITIONS("-DGEM_DYNAMIC")` until Scope B or C.

**Effort for Scope A:** ~30 minutes, 5 files, zero call-site changes.

---

## Scope B: Full Windows and macOS Removal

This removes all non-Linux platform code. Grouped by file category.

### B1 — Remove `G_API_*` macros entirely (definitions and all 2266 call sites)

Keeping the macros as empty definitions is pure noise: every class and function declaration
carries a meaningless token, and readers must chase the macro definition to confirm it does
nothing. Since we are Linux-only and the macros are no-ops, they should be removed entirely —
both the definitions in `GGlobalDefines.hpp` and every usage across the codebase.

When Windows support is re-added during the reengineering it can be re-annotated then, possibly
using a different mechanism (e.g. a single `#pragma GCC visibility push(default)` block per
header, or the CMake `GenerateExportHeader` module).

**Step 1 — Remove all call sites** with a sed script (run from the repository root):

```bash
find include src tests examples benchmarks -name "*.hpp" -o -name "*.cpp" -o -name "*.h" \
  | xargs sed -i \
      -e 's/\bG_API_COMMON\b[[:space:]]*//' \
      -e 's/\bG_API_HAP\b[[:space:]]*//' \
      -e 's/\bG_API_COURTIER\b[[:space:]]*//' \
      -e 's/\bG_API_GENEVA\b[[:space:]]*//' \
      -e 's/\bG_API_INDIVIDUALS\b[[:space:]]*//'
```

Verify no occurrences remain:
```bash
grep -rn "G_API_" include/ src/ tests/ examples/ benchmarks/
```

**Step 2 — Remove the macro definitions from `GGlobalDefines.hpp`**: delete the entire
`#if defined GEM_DYNAMIC` block (lines ~99–137). Nothing remains to define.

### B2 — Library `CMakeLists.txt` (5 files): remove `GEM_*_EXPORTS` and dead blocks

**Files:** `src/common/`, `src/hap/`, `src/courtier/`, `src/geneva/`, `src/geneva-individuals/`

Per-library changes:
- Remove `ADD_DEFINITIONS("-DGEM_*_EXPORTS")` (e.g., `-DGEM_COMMON_EXPORTS`) — no longer
  needed since `G_API_*` are now empty unconditionally.
- Remove `IF(PLATFORM_NEEDS_LIBRARY_LINKING) TARGET_LINK_LIBRARIES(...) ENDIF()` blocks —
  `PLATFORM_NEEDS_LIBRARY_LINKING` is always `FALSE` on Linux. (See B4.)
- Remove `MACOSX_RPATH` from `SET_TARGET_PROPERTIES(... PROPERTIES ...)` calls.

Example (current `src/common/CMakeLists.txt`, condensed):

```cmake
SET_TARGET_PROPERTIES( ${COMMON_LIBNAME} PROPERTIES
    ...
    MACOSX_RPATH ${INSTALL_PREFIX_LIBS}   # <-- remove
)
ADD_DEFINITIONS("-DGEM_COMMON_EXPORTS")   # <-- remove
IF ( PLATFORM_NEEDS_LIBRARY_LINKING )     # <-- remove entire block
    TARGET_LINK_LIBRARIES(...)
ENDIF ()
```

### B3 — `CommonGenevaBuild.cmake` (4 copies): remove all WIN32 blocks

In each of the 4 copies:

**Remove:** `IF(WIN32) ... ENDIF()` Boost extras block:
```cmake
IF(WIN32)
    SET (GENEVA_BOOST_LIBS ${GENEVA_BOOST_LIBS} chrono date_time)
ENDIF()
```

**Remove:** `IF(WIN32) ADD_DEFINITIONS("-DBOOST_ALL_DYN_LINK") ENDIF()`:
```cmake
SET (Boost_USE_STATIC_LIBS OFF)
IF(WIN32)
    ADD_DEFINITIONS("-DBOOST_ALL_DYN_LINK")   # <-- remove
ENDIF()
```

**Remove:** `IF(WIN32 AND CMAKE_VERBOSE_MAKEFILE) ADD_DEFINITIONS(...) ENDIF()`:
```cmake
IF(WIN32 AND CMAKE_VERBOSE_MAKEFILE)          # <-- remove entire block
    ADD_DEFINITIONS(${Boost_LIB_DIAGNOSTIC_DEFINITIONS})
ENDIF()
```

**Remove:** `SET(BUILD_SHARED_LIBS ON)` (same as Scope A2).

**Remove:** `ADD_DEFINITIONS("-DGEM_DYNAMIC")` (no longer needed, `G_API_*` are empty).

**Simplify:** two `IF(UNIX)` guards — on Linux, `UNIX` is always true, so the guards become
unconditional. While doing that, also remove the dead `FIND_LIBRARY(PTHREAD_LIBRARY ...)` call:
`PTHREAD_LIBRARY` is found here but never referenced in any `TARGET_LINK_LIBRARIES` call across
the entire source tree (pthread is already pulled in by the `-pthread` compiler flag). The
`clean-cmake` target just loses its `UNIX AND` prefix:

```cmake
# Remove entirely (PTHREAD_LIBRARY is never consumed):
IF(UNIX)
    FIND_LIBRARY( PTHREAD_LIBRARY NAMES pthread ... )
ENDIF()

# Simplify to (drop the UNIX guard; always true on Linux):
IF (NOT TARGET "clean-cmake")
    ADD_CUSTOM_TARGET( "clean-cmake" ... )
ENDIF()
```

### B4 — `IdentifySystemParameters.cmake` (4 copies): strip to Linux-only

This is the largest single change. All 4 copies are identical so edit the root
`CMakeModules/IdentifySystemParameters.cmake` and then copy to the three example dirs.

**`FIND_HOST_OS`:** Remove APPLE, FreeBSD, CYGWIN, WIN32 branches. Keep Linux only:

```cmake
FUNCTION (FIND_HOST_OS GENEVA_OS_NAME_OUT GENEVA_OS_VERSION_OUT)
    execute_process(COMMAND uname -r OUTPUT_VARIABLE LINUX_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
    SET(${GENEVA_OS_NAME_OUT} "Linux" PARENT_SCOPE)
    SET(${GENEVA_OS_VERSION_OUT} "${LINUX_VERSION}" PARENT_SCOPE)
ENDFUNCTION()
```

**`SET_COMPILER_FLAGS`:** Remove Intel, AppleClang, MSVC branches. Remove the
`IF(CYGWIN)` guard inside the GCC branch. Keep Clang (Linux Clang) and GCC.
Remove the `CLANG_DEF_IDENTIFIER` `MacOSX` inner branch:

```cmake
FUNCTION (SET_COMPILER_FLAGS GENEVA_OS_NAME_IN GENEVA_OS_VERSION_IN GENEVA_BUILD_MODE_IN)
    SET(FLAGS_LOCAL "${CMAKE_CXX_FLAGS}")

    IF(CMAKE_CXX_COMPILER_ID MATCHES ${CLANG_DEF_IDENTIFIER})
        SET(FLAGS_LOCAL "${FLAGS_LOCAL} -Wall -Wno-unused -Wno-attributes -Wno-parentheses-equality")
        SET(FLAGS_LOCAL "${FLAGS_LOCAL} -ftemplate-depth=512 -pthread")
        SET(CMAKE_CXX_FLAGS_SANITIZE "${CMAKE_CXX_FLAGS_SANITIZE} -fsanitize=thread" PARENT_SCOPE)
    ELSEIF(CMAKE_CXX_COMPILER_ID MATCHES ${GNU_DEF_IDENTIFIER})
        SET(FLAGS_LOCAL "${FLAGS_LOCAL} -fno-unsafe-math-optimizations -fno-finite-math-only")
        SET(FLAGS_LOCAL "${FLAGS_LOCAL} -fmessage-length=0 -ftemplate-depth=1024 -pthread")
        SET(CMAKE_CXX_FLAGS_SANITIZE "${CMAKE_CXX_FLAGS_SANITIZE} -fsanitize=thread" PARENT_SCOPE)
    ELSE()
        MESSAGE(FATAL_ERROR "Unsupported compiler ${CMAKE_CXX_COMPILER_ID}")
    ENDIF()

    SET(CMAKE_CXX_FLAGS "${FLAGS_LOCAL}" PARENT_SCOPE)
ENDFUNCTION()
```

**`SET_LINKER_FLAGS`:** Remove the Clang/macOS `-stdlib=libc++` block. Keep only the
GCC `< 9.0` `lstdc++fs` block (for completeness, even though GCC 13+ no longer needs it):

```cmake
FUNCTION (SET_LINKER_FLAGS GENEVA_OS_NAME_IN GENEVA_OS_VERSION_IN GENEVA_BUILD_MODE_IN)
    IF(CMAKE_CXX_COMPILER_ID MATCHES ${GNU_DEF_IDENTIFIER})
        IF(${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 9.0)
            SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lstdc++fs" PARENT_SCOPE)
            SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lstdc++fs" PARENT_SCOPE)
        ENDIF()
    ENDIF()
ENDFUNCTION()
```

**`GET_BUILD_FLAGS`:** Remove entirely (always returns `FALSE` on Linux) OR replace with a
one-liner that hardcodes `FALSE`:

```cmake
FUNCTION (GET_BUILD_FLAGS GENEVA_OS_NAME_IN GENEVA_OS_VERSION_IN GENEVA_BUILD_MODE_IN
          PLATFORM_NEEDS_LIBRARY_LINKING_OUT)
    SET(${PLATFORM_NEEDS_LIBRARY_LINKING_OUT} FALSE PARENT_SCOPE)
ENDFUNCTION()
```

This keeps the call site in `CommonGenevaBuild.cmake` working without change.

**`FLAG_UNSUPPORTED_SETUPS`:** Remove MacOSX, FreeBSD, Cygwin, Windows branches.
Keep Linux (`# No restrictions at the moment`). Update compiler version minimums to
GCC 13 / Clang 18. Remove MSVC, Intel, AppleClang from the version-check block:

```cmake
FUNCTION (FLAG_UNSUPPORTED_SETUPS GENEVA_OS_NAME_IN GENEVA_OS_VERSION_IN GENEVA_BUILD_MODE_IN)
    IF(NOT ${GENEVA_OS_NAME_IN} STREQUAL "Linux")
        MESSAGE(FATAL_ERROR "Geneva only supports Linux.")
    ENDIF()

    IF(${CMAKE_CXX_COMPILER_ID} STREQUAL ${CLANG_DEF_IDENTIFIER})
        SET(COMPILER_MIN_VER 18.0)
    ELSEIF(${CMAKE_CXX_COMPILER_ID} STREQUAL ${GNU_DEF_IDENTIFIER})
        SET(COMPILER_MIN_VER 13.0)
    ELSE()
        MESSAGE(FATAL_ERROR "Unsupported compiler ${CMAKE_CXX_COMPILER_ID}")
    ENDIF()

    IF(${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS ${COMPILER_MIN_VER})
        MESSAGE(FATAL_ERROR "Compiler version ${CMAKE_CXX_COMPILER_VERSION} too old; need >= ${COMPILER_MIN_VER}")
    ENDIF()
ENDFUNCTION()
```

**Remove from the top-level constants:** `MSVC_DEF_*`, `INTEL_DEF_*`, `APPLECLANG_DEF_*`
(lines 42–69 of the current file). Keep `CLANG_DEF_*` and `GNU_DEF_*`.

### B5 — C++ source: one `#if defined(_WIN32)` guard

**File:** `benchmarks/geneva/GCUDAOptBenchmark/GBenchmarkResultWriter.cpp:53`

Current:
```cpp
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
```

Change to:
```cpp
localtime_r(&t, &tm);
```

This is the **only** `#ifdef WIN32`/`#ifdef __APPLE__` guard in the entire C++ source tree.
All platform specificity was in the CMake layer and `GGlobalDefines.hpp`.

### B6 — `scripts/prepareBuild.sh`: minor cleanup

Remove any remaining `WIN32`/`MACOS` references in comments or conditionals. Audit is needed
(file is large), but expected to be comment-only mentions.

### B7 — `CHANGES`: document as breaking change in 1.12 INCOMPATIBILITIES

Add an entry noting Windows and macOS support removal, listing removed macros (`GEM_*_EXPORTS`,
`GEM_DYNAMIC`) and that `G_API_*` are now empty no-ops on Linux.

---

## File Inventory

| File | Scope A | Scope B |
|---|---|---|
| `include/common/GGlobalDefines.hpp` | Remove outer `GEM_DYNAMIC` guard | Further: define `G_API_*` as empty, remove inner `GEM_*_EXPORTS` blocks |
| `CMakeModules/CommonGenevaBuild.cmake` | Remove `BUILD_SHARED_LIBS ON` | Also: remove all WIN32 blocks, `GEM_DYNAMIC` define |
| `examples/common/CMakeModules/CommonGenevaBuild.cmake` | Same | Same |
| `examples/geneva/CMakeModules/CommonGenevaBuild.cmake` | Same | Same |
| `examples/hap/CMakeModules/CommonGenevaBuild.cmake` | Same | Same |
| `CMakeModules/IdentifySystemParameters.cmake` | — | Strip to Linux-only |
| `examples/common/CMakeModules/IdentifySystemParameters.cmake` | — | Copy from root |
| `examples/geneva/CMakeModules/IdentifySystemParameters.cmake` | — | Copy from root |
| `examples/hap/CMakeModules/IdentifySystemParameters.cmake` | — | Copy from root |
| `src/common/CMakeLists.txt` | — | Remove `GEM_COMMON_EXPORTS`, `MACOSX_RPATH`, `PLATFORM_NEEDS_LIBRARY_LINKING` block |
| `src/hap/CMakeLists.txt` | — | Same pattern |
| `src/courtier/CMakeLists.txt` | — | Same pattern |
| `src/geneva/CMakeLists.txt` | — | Same pattern |
| `src/geneva-individuals/CMakeLists.txt` | — | Same pattern |
| `benchmarks/geneva/GCUDAOptBenchmark/GBenchmarkResultWriter.cpp` | — | Remove `_WIN32` guard, keep `localtime_r` |
| `scripts/prepareBuild.sh` | — | Audit and remove Windows/macOS comments/conditionals |
| `CHANGES` | — | Document as 1.12 breaking change |

**Counts:**
- Scope A: 5 files, ~10 lines removed, zero call-site changes
- Scope B adds: ~17 files changed by hand + sed script across all headers/sources for `G_API_*`
  removal; ~2266 token deletions in C++ files, ~150–200 CMake lines removed

**`G_API_*` call sites:** 2266 occurrences across `include/` and `src/`. All are removed by the
sed script in B1 — no manual editing required.

---

---

## Scope C: Pre-C++20 Construct Migration

Full survey and migration path are in `PRE_CPP20_MIGRATION_PLAN.md`. Summary:

**18 items surveyed.** Three already done (typedef, NULL, boost::assign — count: 0).
The remaining items are ordered by CUDA safety and impact:

- **Safe everywhere (Priorities 1–12):** `boost::noncopyable`, `boost::optional`,
  `boost::tuple`, `boost::any`, `shared_ptr(new T)`, `boost::xpressive`,
  `boost::accumulators`, `boost::logic::tribool`, `boost::lexical_cast`,
  `boost::numeric_cast`, `boost::iterator_facade`, `boost::variant` (non-recursive cases).

- **Requires CUDA-zone care (Priority 14):** `std::enable_if` / SFINAE → C++20 concepts.
  Only migrate headers **not** transitively included by `.cu` files. Zone 1 headers
  (`hap/`, transitively-included `common/`) must keep `std::enable_if` since CUDA 13.1
  does not support `requires` in device code.

- **CUDA-zone milestone:** After completing Priorities 1–11, `boost/cast.hpp`,
  `boost/lexical_cast.hpp`, and `boost/utility.hpp` can be removed from
  `hap/GRandomFactory.hpp`, fully eliminating Boost from Zone 1.

**Remaining Boost components after full migration (no std replacement exists):**
Boost.Serialization, Boost.Asio/Beast, Boost.Program_options, Boost.Math,
Boost.Spirit.Qi, Boost.Property_tree, Boost.UUID.

---

## Recommended Order of Implementation

1. **Commit A:** Scope A only (5 files, fast, zero risk). Collapse the `GEM_DYNAMIC` guard and
   remove the redundant `BUILD_SHARED_LIBS ON`. Verify build still works.

2. **Commit B:** Scope B in two steps:
   a. `IdentifySystemParameters.cmake` + `CommonGenevaBuild.cmake` (CMake layer only).
      Verify `cmake` configure step runs cleanly.
   b. `GGlobalDefines.hpp` + 5 library `CMakeLists.txt` + `GBenchmarkResultWriter.cpp`
      (C++ + build definitions). Verify full build.

3. **CHANGES** update in the same commit as step 2b.

---

## What Stays Unchanged

- All `FindGeneva.cmake`, `GenevaConfig.cmake`, and install logic.
- The `GEM_TESTING` definition and the CMake configure-step summary print.
- Test infrastructure and Catch2 integration.
- CUDA and MPI build paths (already Linux-only in practice).

Note: the 2266 `G_API_*` call sites **are** removed — by the sed script in B1. They are
empty no-ops on Linux (BOOST_SYMBOL_EXPORT without -fvisibility=hidden is a no-op), so
removing them is pure dead-code cleanup with no behavioural effect.
