# Geneva Release Test Plan

## Build system

- Build with GCC (>= 13) and Clang (>= 18), both Debug and Release
- Build with all optional components enabled: `GENEVA_BUILD_TESTS`, `GENEVA_BUILD_EXAMPLES`, `GENEVA_BUILD_BENCHMARKS`, `GENEVA_BUILD_WITH_MPI_CONSUMER`, `USECUDARNG`
- Build with all optional components disabled (minimal build)
- Build each combination: with/without MPI, with/without CUDA, on a machine that supports both
- Build types: Debug, Release, RelWithDebInfo, Sanitize
- Verify `make core`, `make gemfony-build-all`, and `make doc` all succeed
- Verify `prepareBuild.sh` with a config file produces an out-of-source build correctly
- Verify `make install` places headers, libraries, and cmake config files under the install prefix

## Unit and integration tests

- Run the full CTest suite (`ctest`) and confirm zero failures
- Run `GenevaStandardTests` directly and check output for unexpected skips
- Run tests with `GENEVA_BUILD_TYPE=Sanitize` (thread sanitizer) and check for data races

## Parallelization consumers

- Serial consumer: run an optimization to completion
- Multithreaded consumer (`GStdThreadConsumerT`): run with 2 and N threads
- Websocket consumer (`GWebsocketConsumerT`): run a client/server split locally
- MPI consumer (`GMPIConsumerT`): run with at least 2 ranks (if MPI available)

## Optimization algorithms

- Verify each algorithm runs to completion on a simple test individual: EA, SA, Swarm, GD, ParameterScan
- Verify `Go2` JSON config round-trip (write default config, reload, run)
- Verify checkpoint save and restore produces identical results

## Examples

- `10_GStarter`: builds and runs to completion
- At least one networked example (client + server on localhost)
- CUDA example (`15_GCUDAWorker`) on a machine with a supported GPU

## Out-of-tree application build

- Install Geneva, then build an example application against the installed tree using `FindGeneva`
- Confirm `GENEVA_BUILD_TESTS` mismatch between library and application is caught with a useful error

## Installation and runtime linking

- Confirm `ldconfig` / `LD_LIBRARY_PATH` setup allows executables to find the shared libraries
- Confirm no unresolved symbol errors at startup (`ldd` check on the main executables)

## Documentation and release metadata

- `CHANGES` entry present for the new version with incompatibilities and noteworthy changes
- Version string updated in `CMakeLists.txt` and `CHANGES`
- `make doc` (Doxygen) completes without errors or undocumented-symbol warnings on public API
