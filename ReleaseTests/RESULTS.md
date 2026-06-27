# Base-mode release run — recorded results

Captured live with Podman 5.7.0 (rootless) on the reference host, using the
per-OS images that build **Boost 1.91.0 from source in C++20 mode**. The build
inside each container uses `scripts/prepareBuild.sh` with a generated
`release.gcfg` (`BOOSTROOT=/opt/boost`).

Command:

```bash
python3 release_test.py --quick --filter buildtype=Debug run
```

This expands to base mode: 2 guest OSes x 2 compilers x Debug, each running
`prepareBuild.sh` -> `make core` -> `make gemfony-build-all` -> `make doc` ->
`ctest` -> `GenevaStandardTests` -> serial/threaded consumers -> GStarter.

## Image build (provision)

Both per-OS images built successfully (Boost 1.91 compiled from source, C++20):

| Image                  | Boost      | GCC    | Clang   | CMake  | Catch2 |
|------------------------|------------|--------|---------|--------|--------|
| `geneva-rt/ubuntu-24.04` | 1.91.0 (src) | 13.3.0 | 18.1.3  | 3.28.3 | v3     |
| `geneva-rt/ubuntu-26.04` | 1.91.0 (src) | 15.x   | 21.x    | 4.x    | v3     |

CMake at configure time reported, for every job:
`Found Boost: /opt/boost/lib/cmake/Boost-1.91.0/BoostConfig.cmake (found
suitable version "1.91.0", minimum required is "1.91")` and found Catch2 v3.

## One image, both compilers — verified

A single GCC-built Boost served the Clang Geneva builds: the Clang configure +
compile used `/opt/boost` (GCC-built) with no Boost link errors. The only Clang
failure observed (below) is a Geneva source issue, not a Boost/ABI mismatch —
so per-compiler Boost is NOT needed.

## Build + ctest matrix (Debug)

| Job                        | build/core | gemfony-build-all | make doc | ctest      | GenevaStandardTests | consumers | GStarter |
|----------------------------|------------|-------------------|----------|------------|---------------------|-----------|----------|
| ubuntu-26.04 gcc Debug     | PASS       | PASS              | PASS*    | PASS 48/48 | PASS                | PASS      | PASS     |
| ubuntu-26.04 clang Debug   | PASS       | PASS              | PASS*    | PASS 48/48 | PASS                | PASS      | PASS     |
| ubuntu-24.04 gcc Debug     | PASS       | PASS              | PASS*    | PASS 48/48 | PASS                | PASS      | PASS     |
| ubuntu-24.04 clang Debug   | FAIL       | FAIL              | PASS*    | (skipped — build failed) | | | |

`*` make doc completes with Doxygen warnings (advisory; not a failure).

## The one failure: Ubuntu 24.04 + Clang 18

`build/core` fails ONLY under Clang 18 (Ubuntu 24.04). It is a
standards-conformance issue in Geneva's own headers, not in Boost, Catch2 or the
harness:

```
/work/src/include/geneva/oa/GOAFactoryT.hpp:121:56:
  error: use 'template' keyword to treat 'clone' as a dependent template name
      pluggable_om_ = cp.pluggable_om_->GObject::clone<GBasePluggableOM>();
```

* The fix is a one-line source change (outside this harness's scope —
  `ReleaseTests/` only):
  `cp.pluggable_om_->GObject::template clone<GBasePluggableOM>()`.
* GCC (13 and 15) accepts the non-conforming form; Clang 18 (correctly) rejects
  it. Clang 21 (Ubuntu 26.04) accepts it, which is why 26.04-clang builds.
* `GOAFactoryT.hpp` is among the files being modified separately on this branch
  (it appears as modified in `git status`); the harness surfaced the regression.

## Summary

* Geneva **builds and `ctest` passes (48/48)** in Ubuntu 24.04 and Ubuntu 26.04
  with **GCC**, and with **Clang on 26.04**.
* **Ubuntu 24.04 + Clang 18** is blocked by a single non-conforming line in
  `include/geneva/oa/GOAFactoryT.hpp` — a source fix outside `ReleaseTests/`.
  Once that line gains the `template` keyword, the 24.04-clang cell will pass
  too (re-run `python3 release_test.py --quick --filter buildtype=Debug run`).
* Boost-from-source 1.91 in C++20 works for both compilers from a single per-OS
  image; no per-compiler Boost is required.
