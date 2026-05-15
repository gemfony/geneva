# Implementation Prompt: Linux-Only Build System Simplification

## What you are doing and why

You are working on the Geneva library (Grid-Enabled Evolutionary Algorithms), a C++20
optimization library. The repository root is `/home/rberlich/ClionProjects/geneva`.
The active branch is `catch2-migration`. The active build directory is `/home/rberlich/build`.

Geneva has been simplified: static linking was recently removed, and all five shared-library
`ADD_LIBRARY` calls are now explicit `SHARED`. Windows and macOS support is being dropped
permanently for the current reengineering phase. This commit implements that removal plus
two pieces of confirmed dead-code removal. No functional behaviour changes.

The work is split into two sequential scopes. **Do Scope A first, verify, commit. Then do
Scope B, verify, commit.** Both scopes are described in full below with exact before/after
content for every file.

Do not implement Scope C (the pre-C++20 / Boost migration). That is a separate task with
its own plan in `PRE_CPP20_MIGRATION_PLAN.md`.

---

## Critical constraints

1. **Read every file before editing it.** The Edit tool will fail otherwise.
2. **The four `CMakeModules/` trees are kept in sync.** After editing the root copy of a
   file, `cp` it to the three example copies:
   - Root: `CMakeModules/IdentifySystemParameters.cmake`
   - Copies: `examples/common/CMakeModules/IdentifySystemParameters.cmake`
             `examples/geneva/CMakeModules/IdentifySystemParameters.cmake`
             `examples/hap/CMakeModules/IdentifySystemParameters.cmake`
   The same applies to `CommonGenevaBuild.cmake`.
3. **Do not touch** `FindGeneva.cmake`, `GenevaConfig.cmake`, `CMakeModules/CMakeLists.txt`,
   `CMakeModules/IndependentBuild.cmake`, `CMakeModules/CleanCmakeTemporaries.cmake`,
   Catch2/test infrastructure, CUDA build paths, MPI paths, or any file not listed below.
4. **Do not modify `CHANGES` in Scope A.** Update it only in the Scope B commit.
5. After the sed script in B1, run the grep verification command before proceeding.

---

## Scope A — Dead-code removal (5 files, zero behavioural change)

### A1: `include/common/GGlobalDefines.hpp`

Read the file. Find the block that begins with:
```cpp
#if defined GEM_DYNAMIC && (GEM_DYNAMIC != 0)
```
and ends with:
```cpp
#endif /* GEM_DYNAMIC */
```

This outer `#if defined GEM_DYNAMIC` / `#else /* GEM_DYNAMIC undefined */` / `#endif` wrapper
is dead code: `GEM_DYNAMIC` is always defined by the CMake build system. Remove only the three
structural lines of the wrapper:
- the opening `#if defined GEM_DYNAMIC && (GEM_DYNAMIC != 0)` line
- the `#else /* GEM_DYNAMIC undefined */` line and the five empty `#define G_API_*` lines below it
- the closing `#endif /* GEM_DYNAMIC */` line

Keep all five inner `#ifdef GEM_*_EXPORTS` / `#define G_API_* BOOST_SYMBOL_EXPORT` /
`#else` / `#define G_API_* BOOST_SYMBOL_IMPORT` / `#endif` blocks intact — they are not
dead code yet (Scope B removes them).

The result for the Common block (representative; same pattern for HAP, COURTIER, GENEVA,
INDIVIDUALS) should look like:

```cpp
#ifdef GEM_COMMON_EXPORTS
#define G_API_COMMON BOOST_SYMBOL_EXPORT
#else
#define G_API_COMMON BOOST_SYMBOL_IMPORT
#endif /* GEM_COMMON_EXPORTS */
```

### A2: `CMakeModules/CommonGenevaBuild.cmake` (root copy)

Read the file. In the "Geneva only supports shared libraries" block, remove exactly the line:
```cmake
SET (BUILD_SHARED_LIBS ON)
```
That single line is a no-op because all five `ADD_LIBRARY` calls are already explicit `SHARED`.
Do not touch any other line in this file.

### A3–A5: Three example copies of `CommonGenevaBuild.cmake`

Apply the identical single-line removal (`SET (BUILD_SHARED_LIBS ON)`) to:
- `examples/common/CMakeModules/CommonGenevaBuild.cmake`
- `examples/geneva/CMakeModules/CommonGenevaBuild.cmake`
- `examples/hap/CMakeModules/CommonGenevaBuild.cmake`

Read each file before editing.

### Scope A verification

Run from `/home/rberlich/build`:
```bash
cmake /home/rberlich/ClionProjects/geneva 2>&1 | tail -20
```
The configure step must complete without errors. You do not need to run `make`.

### Scope A commit

Stage and commit only the five files changed in Scope A:
- `include/common/GGlobalDefines.hpp`
- `CMakeModules/CommonGenevaBuild.cmake`
- `examples/common/CMakeModules/CommonGenevaBuild.cmake`
- `examples/geneva/CMakeModules/CommonGenevaBuild.cmake`
- `examples/hap/CMakeModules/CommonGenevaBuild.cmake`

Commit message (use this verbatim):
```
build: remove dead GEM_DYNAMIC outer guard and redundant BUILD_SHARED_LIBS ON

GEM_DYNAMIC is unconditionally defined by ADD_DEFINITIONS in CommonGenevaBuild.cmake,
making the #else branch in GGlobalDefines.hpp unreachable. Remove the outer
#if defined GEM_DYNAMIC / #else / #endif wrapper; keep the inner GEM_*_EXPORTS
export/import dispatch blocks intact.

BUILD_SHARED_LIBS ON is a no-op now that all five ADD_LIBRARY calls are explicit
SHARED. Remove from all four CommonGenevaBuild.cmake copies.

Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>
```

---

## Scope B — Full Windows and macOS removal

### B1: Remove all `G_API_*` macro call sites (sed script)

Run from the repository root `/home/rberlich/ClionProjects/geneva`:

```bash
find include src tests examples benchmarks -type f \( -name "*.hpp" -o -name "*.cpp" -o -name "*.h" \) \
  | xargs sed -i \
      -e 's/\bG_API_COMMON\b[[:space:]]*//' \
      -e 's/\bG_API_HAP\b[[:space:]]*//' \
      -e 's/\bG_API_COURTIER\b[[:space:]]*//' \
      -e 's/\bG_API_GENEVA\b[[:space:]]*//' \
      -e 's/\bG_API_INDIVIDUALS\b[[:space:]]*//'
```

Then verify zero occurrences remain (this command must produce no output):
```bash
grep -rn "G_API_" include/ src/ tests/ examples/ benchmarks/ --include="*.hpp" --include="*.cpp" --include="*.h"
```

If any occurrences remain they are in unexpected locations; investigate before continuing.
Note: the macro *definitions* in `GGlobalDefines.hpp` will still exist at this point —
they are removed in B2 below.

### B2: `include/common/GGlobalDefines.hpp`

Read the file. Now that all call sites are gone, remove the entire remaining macro block:
the five `#ifdef GEM_*_EXPORTS` / `#define G_API_* BOOST_SYMBOL_EXPORT` / `#else` /
`#define G_API_* BOOST_SYMBOL_IMPORT` / `#endif` blocks (all five, for COMMON, HAP,
COURTIER, GENEVA, INDIVIDUALS). Also remove `ADD_DEFINITIONS("-DGEM_DYNAMIC")` — wait,
that is in CMake, not this file. In this file, remove only the preprocessor macro definitions.

Also remove any `#include` of Boost symbol-visibility headers if they exist and are only
used by the `G_API_*` block (check the surrounding includes before removing).

### B3: Five library `CMakeLists.txt` files

For each of the five files listed below, read the file and make the following three removals.
The exact lines differ slightly per library (different macro names, different library names)
but the pattern is identical.

**Files:**
- `src/common/CMakeLists.txt`
- `src/hap/CMakeLists.txt`
- `src/courtier/CMakeLists.txt`
- `src/geneva/CMakeLists.txt`
- `src/geneva-individuals/CMakeLists.txt`

**Removal 1:** Remove the `MACOSX_RPATH` property from the `SET_TARGET_PROPERTIES` call.
The property and its value are on one line like:
```cmake
    MACOSX_RPATH ${INSTALL_PREFIX_LIBS}
```
Remove that line. The surrounding `SET_TARGET_PROPERTIES` block and other properties stay.

**Removal 2:** Remove the `ADD_DEFINITIONS("-DGEM_*_EXPORTS")` line. The exact macro name
differs per library:
- `src/common`: `-DGEM_COMMON_EXPORTS`
- `src/hap`: `-DGEM_HAP_EXPORTS`
- `src/courtier`: `-DGEM_COURTIER_EXPORTS`
- `src/geneva`: `-DGEM_GENEVA_EXPORTS`
- `src/geneva-individuals`: `-DGEM_INDIVIDUALS_EXPORTS`

**Removal 3:** Remove the entire `IF(PLATFORM_NEEDS_LIBRARY_LINKING)` / `ENDIF()` block
(including the `TARGET_LINK_LIBRARIES` call inside it). `PLATFORM_NEEDS_LIBRARY_LINKING`
is always `FALSE` on Linux; this block never executes.

### B4: `CMakeModules/CommonGenevaBuild.cmake` (root copy)

Read the file. Make the following removals/simplifications in order:

**Removal 1:** Remove the `IF(WIN32)` Boost-extras block (chrono, date_time):
```cmake
IF(WIN32)
    # Boost.Thread requires Boost.Chrono, required for linking in Windows
    SET (
            GENEVA_BOOST_LIBS
            ${GENEVA_BOOST_LIBS}
            chrono
            date_time
    )
ENDIF()
```

**Removal 2:** Inside the Boost static/dynamic block, remove the WIN32 inner block:
```cmake
IF(WIN32)
    # Disable auto-linking
    ADD_DEFINITIONS("-DBOOST_ALL_DYN_LINK")
ENDIF()
```
Keep the surrounding `SET (Boost_USE_STATIC_LIBS OFF)` line — it is not platform-specific.

**Removal 3:** Remove the WIN32 verbose-diagnostics block:
```cmake
# Add compile-time debug information about Boost's linked libraries
IF(WIN32 AND CMAKE_VERBOSE_MAKEFILE)
    ADD_DEFINITIONS(${Boost_LIB_DIAGNOSTIC_DEFINITIONS})
ENDIF()
```

**Removal 4:** Remove `ADD_DEFINITIONS("-DGEM_DYNAMIC")` from the shared-libraries block.
After Scope A already removed `SET(BUILD_SHARED_LIBS ON)`, the block should now contain
only the comment and this define. Remove the define; keep the section comment.

**Simplification 5:** Remove the `IF(UNIX)` / `FIND_LIBRARY(PTHREAD_LIBRARY...)` / `ENDIF()`
block entirely. `PTHREAD_LIBRARY` is found here but never referenced in any
`TARGET_LINK_LIBRARIES` call anywhere in the source tree — the `-pthread` flag is injected
via the compiler flags instead. The variable is discovered and then silently ignored.

**Simplification 6:** In the `clean-cmake` custom target block, remove the `UNIX AND`
condition prefix so it reads:
```cmake
IF (NOT TARGET "clean-cmake")
```
instead of:
```cmake
IF (UNIX AND NOT TARGET "clean-cmake")
```

### B5: Three example copies of `CommonGenevaBuild.cmake`

Apply the identical six changes from B4 to:
- `examples/common/CMakeModules/CommonGenevaBuild.cmake`
- `examples/geneva/CMakeModules/CommonGenevaBuild.cmake`
- `examples/hap/CMakeModules/CommonGenevaBuild.cmake`

Read each file before editing.

### B6: `CMakeModules/IdentifySystemParameters.cmake` (root copy)

This is the largest single file change. Read the full file first. Then make the following
changes in order. After completing all changes to this file, copy it to the three example
directories (B7).

**Step 1 — Remove top-level compiler-ID constants for non-Linux compilers.**

Remove these three constant blocks (lines ~42–69 of the current file):

```cmake
# Intel settings
SET(INTEL_DEF_IDENTIFIER "Intel")
SET(INTEL_DEF_MIN_CXX14_VERSION "16.0")

# Apple Clang settings
SET(APPLECLANG_DEF_IDENTIFIER "AppleClang")
SET(APPLECLANG_DEF_MIN_VERSION "7.3")
SET(APPLECLANG_DEF_CXX14_STANDARD_FLAG "")

# MSVC settings
SET(MSVC_DEF_IDENTIFIER "MSVC")
SET(MSVC_DEF_MIN_CXX14_VERSION "21.0")    # Aka MS Visual C++ 14.0, MS Visual Studio 2015
SET(MSVC_DEF_CXX14_STANDARD_FLAG "")
```

Keep the GCC and Clang constant blocks intact:
```cmake
# Clang settings  (keep)
SET(CLANG_DEF_IDENTIFIER "Clang")
...
# GCC settings    (keep)
SET(GNU_DEF_IDENTIFIER "GNU")
...
```

**Step 2 — Rewrite `FIND_HOST_OS`.**

Replace the entire body of the `FIND_HOST_OS` function (everything between the opening
`FUNCTION(` line and `ENDFUNCTION()`) with Linux-only detection:

```cmake
FUNCTION (
    FIND_HOST_OS
    GENEVA_OS_NAME_OUT
    GENEVA_OS_VERSION_OUT
)
    execute_process(COMMAND uname -r OUTPUT_VARIABLE LINUX_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
    SET(${GENEVA_OS_NAME_OUT} "Linux" PARENT_SCOPE)
    SET(${GENEVA_OS_VERSION_OUT} "${LINUX_VERSION}" PARENT_SCOPE)
ENDFUNCTION()
```

**Step 3 — Rewrite `SET_COMPILER_FLAGS`.**

Replace the entire function body with a GCC + Linux-Clang only version. Remove the Intel,
AppleClang/Clang-on-MacOSX, and MSVC branches. Remove the `IF(CYGWIN)` guard inside the
GCC branch. Keep the sanitize flag. The new body:

```cmake
FUNCTION (
    SET_COMPILER_FLAGS
    GENEVA_OS_NAME_IN
    GENEVA_OS_VERSION_IN
    GENEVA_BUILD_MODE_IN
)
    SET(FLAGS_LOCAL "${CMAKE_CXX_FLAGS}")

    IF(CMAKE_CXX_COMPILER_ID MATCHES ${CLANG_DEF_IDENTIFIER})
        SET(FLAGS_LOCAL "${FLAGS_LOCAL} -Wall -Wno-unused -Wno-attributes -Wno-parentheses-equality -Wno-deprecated-register")
        SET(FLAGS_LOCAL "${FLAGS_LOCAL} -ftemplate-depth=512 -pthread")
        SET(CMAKE_CXX_FLAGS_SANITIZE "${CMAKE_CXX_FLAGS_SANITIZE} -fsanitize=thread" PARENT_SCOPE)
    ELSEIF(CMAKE_CXX_COMPILER_ID MATCHES ${GNU_DEF_IDENTIFIER})
        SET(FLAGS_LOCAL "${FLAGS_LOCAL} -fno-unsafe-math-optimizations -fno-finite-math-only")
        SET(FLAGS_LOCAL "${FLAGS_LOCAL} -fmessage-length=0 -ftemplate-depth=1024 -pthread")
        SET(CMAKE_CXX_FLAGS_SANITIZE "${CMAKE_CXX_FLAGS_SANITIZE} -fsanitize=thread" PARENT_SCOPE)
    ELSE()
        MESSAGE(FATAL_ERROR "Unsupported compiler: ${CMAKE_CXX_COMPILER_ID}. Geneva requires GCC >= 13 or Clang >= 18 on Linux.")
    ENDIF()

    SET(CMAKE_CXX_FLAGS "${FLAGS_LOCAL}" PARENT_SCOPE)
ENDFUNCTION()
```

**Step 4 — Rewrite `SET_LINKER_FLAGS`.**

Remove the Clang/AppleClang macOS `-stdlib=libc++` block entirely. Keep only the GCC
`< 9.0` filesystem library block (it is harmless and documents a past requirement):

```cmake
FUNCTION (
    SET_LINKER_FLAGS
    GENEVA_OS_NAME_IN
    GENEVA_OS_VERSION_IN
    GENEVA_BUILD_MODE_IN
)
    IF(CMAKE_CXX_COMPILER_ID MATCHES ${GNU_DEF_IDENTIFIER})
        # For GCC version < 9.0 add the filesystem library explicitly
        IF(${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 9.0)
            SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lstdc++fs" PARENT_SCOPE)
            SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lstdc++fs" PARENT_SCOPE)
        ENDIF()
    ENDIF()
ENDFUNCTION()
```

**Step 5 — Rewrite `GET_BUILD_FLAGS`.**

Replace the OS-branching logic with a single hardcoded `FALSE` (Linux never needs library
linking). Keep the function signature identical so call sites are unchanged:

```cmake
FUNCTION (
    GET_BUILD_FLAGS
    GENEVA_OS_NAME_IN
    GENEVA_OS_VERSION_IN
    GENEVA_BUILD_MODE_IN
    PLATFORM_NEEDS_LIBRARY_LINKING_OUT
)
    # Linux does not need explicit library linking in ADD_LIBRARY targets
    SET(${PLATFORM_NEEDS_LIBRARY_LINKING_OUT} FALSE PARENT_SCOPE)
ENDFUNCTION()
```

**Step 6 — Rewrite `FLAG_UNSUPPORTED_SETUPS`.**

Replace the multi-OS, multi-compiler branching with a Linux-only guard and updated minimum
compiler versions (GCC 13, Clang 18 — the actual requirement for C++20 + concepts):

```cmake
FUNCTION (
    FLAG_UNSUPPORTED_SETUPS
    GENEVA_OS_NAME_IN
    GENEVA_OS_VERSION_IN
    GENEVA_BUILD_MODE_IN
)
    IF(NOT ${GENEVA_OS_NAME_IN} STREQUAL "Linux")
        MESSAGE(FATAL_ERROR "Geneva only supports Linux.")
    ENDIF()

    IF(${CMAKE_CXX_COMPILER_ID} STREQUAL ${CLANG_DEF_IDENTIFIER})
        SET(COMPILER_MIN_VER 18.0)
    ELSEIF(${CMAKE_CXX_COMPILER_ID} STREQUAL ${GNU_DEF_IDENTIFIER})
        SET(COMPILER_MIN_VER 13.0)
    ELSE()
        MESSAGE(FATAL_ERROR "Unsupported compiler: ${CMAKE_CXX_COMPILER_ID}. Geneva requires GCC >= 13 or Clang >= 18.")
    ENDIF()

    IF(${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS ${COMPILER_MIN_VER})
        MESSAGE(FATAL_ERROR "Compiler version ${CMAKE_CXX_COMPILER_VERSION} is too old. Need >= ${COMPILER_MIN_VER}.")
    ENDIF()
ENDFUNCTION()
```

### B7: Copy `IdentifySystemParameters.cmake` to the three example directories

After completing all edits in B6:
```bash
cp /home/rberlich/ClionProjects/geneva/CMakeModules/IdentifySystemParameters.cmake \
   /home/rberlich/ClionProjects/geneva/examples/common/CMakeModules/IdentifySystemParameters.cmake

cp /home/rberlich/ClionProjects/geneva/CMakeModules/IdentifySystemParameters.cmake \
   /home/rberlich/ClionProjects/geneva/examples/geneva/CMakeModules/IdentifySystemParameters.cmake

cp /home/rberlich/ClionProjects/geneva/CMakeModules/IdentifySystemParameters.cmake \
   /home/rberlich/ClionProjects/geneva/examples/hap/CMakeModules/IdentifySystemParameters.cmake
```

### B8: `benchmarks/geneva/GCUDAOptBenchmark/GBenchmarkResultWriter.cpp`

Read the file. At line 53, replace:
```cpp
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
```
with:
```cpp
    localtime_r(&t, &tm);
```

This is the only `#ifdef WIN32` / `#ifdef __APPLE__` guard in the entire C++ source tree.

### B9: `scripts/prepareBuild.sh`

Read the file. Scan for any remaining Windows- or macOS-specific comments, variables, or
conditionals that reference `WIN32`, `MACOS`, `Windows`, `Darwin`, or `Cygwin`. Remove them
or replace with a Linux-only note. Expected to be comment-only references.

### B10: `CHANGES`

Read the file. In the `INCOMPATIBILITIES` section under version 1.12, add the following
entry after the existing static-linking removal entry:

```
- Windows and macOS support has been removed. Geneva now targets Linux exclusively.
  The G_API_COMMON, G_API_HAP, G_API_COURTIER, G_API_GENEVA, and G_API_INDIVIDUALS
  symbol-visibility macros have been removed from all 2266 call sites and from
  GGlobalDefines.hpp entirely (they were no-ops on Linux without -fvisibility=hidden).
  The GEM_*_EXPORTS preprocessor definitions and the GEM_DYNAMIC definition have been
  removed from the CMake build system. The PLATFORM_NEEDS_LIBRARY_LINKING variable
  always evaluates to FALSE and its conditional TARGET_LINK_LIBRARIES blocks have been
  removed from all five library CMakeLists.txt files. IdentifySystemParameters.cmake
  now supports only GCC >= 13 and Clang >= 18 on Linux.
```

In the "Noteworthy Changes" section under version 1.12, add:

```
- Build system stripped to Linux-only: IdentifySystemParameters.cmake, all four
  CommonGenevaBuild.cmake copies, and the five library CMakeLists.txt files have been
  cleaned of Windows (MSVC, Cygwin), macOS (AppleClang, Darwin), Intel compiler, and
  FreeBSD branches. The cmake configure step is now ~150 lines shorter with no dead
  platform-conditional branches.
```

---

## Scope B verification

After completing all B steps but before committing:

**Check 1 — No G_API_ tokens remain in C++ files:**
```bash
cd /home/rberlich/ClionProjects/geneva
grep -rn "G_API_" include/ src/ tests/ examples/ benchmarks/ --include="*.hpp" --include="*.cpp" --include="*.h"
```
Must produce no output.

**Check 2 — No GEM_*_EXPORTS or GEM_DYNAMIC in CMake files:**
```bash
grep -rn "GEM_COMMON_EXPORTS\|GEM_HAP_EXPORTS\|GEM_COURTIER_EXPORTS\|GEM_GENEVA_EXPORTS\|GEM_INDIVIDUALS_EXPORTS\|GEM_DYNAMIC" \
  CMakeModules/ examples/*/CMakeModules/ src/*/CMakeLists.txt
```
Must produce no output.

**Check 3 — No WIN32 / APPLE / CYGWIN / MSVC / Darwin in CMake files (except FindGeneva / GenevaConfig):**
```bash
grep -rn "WIN32\|APPLE\|CYGWIN\|MSVC\|Darwin\|MacOSX\|Cygwin\|Windows" \
  CMakeModules/ examples/*/CMakeModules/ src/*/CMakeLists.txt \
  | grep -v "FindGeneva\|GenevaConfig\|CleanCmake\|Independent"
```
Must produce no output.

**Check 4 — CMake configure step succeeds:**
```bash
cd /home/rberlich/build
cmake /home/rberlich/ClionProjects/geneva 2>&1 | grep -i "error\|fatal\|not found"
```
Must produce no error lines.

### Scope B commit

Stage all modified files (the list is everything touched in B1–B10 that is not covered by
the Scope A commit) and create this commit:

```
build: remove Windows and macOS support; drop G_API_* macros and platform CMake branches

Geneva is now Linux-only for the current reengineering phase. Removed:
- 2266 G_API_COMMON/HAP/COURTIER/GENEVA/INDIVIDUALS call sites (sed script)
- G_API_* macro definitions from GGlobalDefines.hpp
- GEM_*_EXPORTS -D flags from five library CMakeLists.txt files
- GEM_DYNAMIC ADD_DEFINITIONS from all four CommonGenevaBuild.cmake copies
- PLATFORM_NEEDS_LIBRARY_LINKING conditional TARGET_LINK_LIBRARIES blocks
- MACOSX_RPATH from SET_TARGET_PROPERTIES in all five library CMakeLists.txt files
- IdentifySystemParameters.cmake: removed APPLE, WIN32, CYGWIN, FreeBSD, Intel,
  AppleClang, MSVC branches from all four copies; updated compiler minimums to
  GCC 13 / Clang 18
- CommonGenevaBuild.cmake: removed WIN32 Boost extras, BOOST_ALL_DYN_LINK,
  FIND_LIBRARY(PTHREAD_LIBRARY), verbose Boost diagnostics block
- benchmarks/GBenchmarkResultWriter.cpp: removed _WIN32 localtime_s guard
- CHANGES: documented as 1.12 breaking change

Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>
```

---

## Files changed summary

**Scope A (5 files):**
1. `include/common/GGlobalDefines.hpp` — remove outer `GEM_DYNAMIC` guard
2. `CMakeModules/CommonGenevaBuild.cmake` — remove `SET(BUILD_SHARED_LIBS ON)`
3. `examples/common/CMakeModules/CommonGenevaBuild.cmake` — same
4. `examples/geneva/CMakeModules/CommonGenevaBuild.cmake` — same
5. `examples/hap/CMakeModules/CommonGenevaBuild.cmake` — same

**Scope B (~18 files + sed across ~hundreds of headers):**
1. C++ headers/sources (sed script) — remove 2266 `G_API_*` tokens
2. `include/common/GGlobalDefines.hpp` — remove `G_API_*` macro definitions block
3. `src/common/CMakeLists.txt` — remove MACOSX_RPATH, GEM_COMMON_EXPORTS, PLATFORM block
4. `src/hap/CMakeLists.txt` — same pattern
5. `src/courtier/CMakeLists.txt` — same pattern
6. `src/geneva/CMakeLists.txt` — same pattern
7. `src/geneva-individuals/CMakeLists.txt` — same pattern
8. `CMakeModules/CommonGenevaBuild.cmake` — remove WIN32 blocks, GEM_DYNAMIC, PTHREAD, UNIX guards
9. `examples/common/CMakeModules/CommonGenevaBuild.cmake` — same
10. `examples/geneva/CMakeModules/CommonGenevaBuild.cmake` — same
11. `examples/hap/CMakeModules/CommonGenevaBuild.cmake` — same
12. `CMakeModules/IdentifySystemParameters.cmake` — rewrite to Linux-only
13. `examples/common/CMakeModules/IdentifySystemParameters.cmake` — copy of root
14. `examples/geneva/CMakeModules/IdentifySystemParameters.cmake` — copy of root
15. `examples/hap/CMakeModules/IdentifySystemParameters.cmake` — copy of root
16. `benchmarks/geneva/GCUDAOptBenchmark/GBenchmarkResultWriter.cpp` — remove `_WIN32` guard
17. `scripts/prepareBuild.sh` — remove platform comments/conditionals
18. `CHANGES` — document breaking changes

**Do not touch:**
- `CMakeModules/FindGeneva.cmake`
- `CMakeModules/GenevaConfig.cmake` (if it exists)
- `CMakeModules/IndependentBuild.cmake`
- `CMakeModules/CleanCmakeTemporaries.cmake`
- Any file under `tests/`, `include/`, or `src/` except via the sed script and `GGlobalDefines.hpp`
- `PRE_CPP20_MIGRATION_PLAN.md`, `LINUX_ONLY_SIMPLIFICATION_PLAN.md`, `LINUX_ONLY_IMPL_PROMPT.md`
- The top-level `CMakeLists.txt`
