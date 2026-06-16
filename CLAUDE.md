# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Geneva (Grid-Enabled Evolutionary Algorithms) is a C++20 library for large-scale parametric optimization. It supports evolutionary algorithms, simulated annealing, swarm algorithms, gradient descent, and parameter scans — all running transparently in serial, multi-threaded, MPI, or websocket-distributed modes.

Current version: **1.11 "Hendaye"**. Apache License 2.0.

## Build System

Geneva uses CMake 3.27+ and requires an **out-of-source build** (do not build inside the source tree).

### Quick build using the config script

```bash
mkdir ~/build && cd ~/build
cp /path/to/geneva/scripts/genevaConfig.gcfg ./myConfig.gcfg
# Edit myConfig.gcfg to set BOOSTROOT, BUILDMODE, etc.
/path/to/geneva/scripts/prepareBuild.sh ./myConfig.gcfg
make -j$(nproc)
sudo make install
```

### Direct CMake build

```bash
mkdir ~/build && cd ~/build
cmake /path/to/geneva \
  -DGENEVA_BUILD_TYPE=Debug \         # Debug (default), Release, RelWithDebInfo, MinSizeRel, Sanitize
  -DGENEVA_BUILD_TESTS=TRUE \
  -DGENEVA_BUILD_EXAMPLES=TRUE \
  -DGENEVA_BUILD_BENCHMARKS=TRUE \
  -DBOOST_ROOT=/opt/boost \           # Only if Boost is in a non-standard location
  -DGENEVA_BUILD_WITH_MPI_CONSUMER=TRUE  # Optional: enable MPI parallelization
make -j$(nproc)
```

### Building individual targets

```bash
make common             # Build only the Common library
make hap                # Build only the Hap library
make courtier           # Build only the Courtier library
make geneva             # Build only the Geneva library
make core               # Build all libraries (no examples/tests)
make gemfony-build-all  # Build everything
make doc                # Generate Doxygen docs (if Doxygen installed)
```

### Running tests

```bash
# Run all tests via CTest (from build directory)
ctest

# Run the Geneva unit test executable directly
./tests/geneva/UnitTests/GenevaStandardTests

# Run a single CTest test by name
ctest -R GenevaStandardTests -V
```

## Library Architecture

The codebase is split into five libraries with a strict dependency order:

```
common  <--  hap  <--  courtier  <--  geneva  <--  geneva-individuals
```

| Library | Purpose |
|---|---|
| `common` | Utilities: logging (`GLogger`), thread pool (`GThreadPool`), Boost.Serialization helpers, formula parser, plot designer, bounded buffers, exception types |
| `hap` | Random number generation (`GRandomT`, `GRandomFactory`). Optional CUDA GPU-based RNG via `GCUDARng` |
| `courtier` | Broker/consumer parallelization framework. Consumers: `GSerialConsumerT`, `GStdThreadConsumerT`, `GAsioConsumerT`, `GWebsocketConsumerT`, `GMPIConsumerT`. The broker (`GBrokerT`) dispatches work items to registered consumers |
| `geneva` | Core optimization: per-category CRTP roots (each deriving from `Gem::Common::GCommonInterfaceT<Root>`), `GFlatIndividualT<Derived>` (user subclass this to define a problem; flat genome built via `GGenomeBuilder`), `G_OptimizationAlgorithm_*` (EA, SA, Swarm, GD, ParameterScan), `Go2` (top-level orchestrator) |
| `geneva-individuals` | Reusable problem definitions (individuals) for examples and tests |

Headers are in `include/<library>/`, sources in `src/<library>/`. All code is in the `Gem::` namespace (e.g., `Gem::Geneva`, `Gem::Courtier`, `Gem::Common`, `Gem::Hap`).

## Key Design Concepts

### Writing an optimization problem

The genome is a **flat** list of parameters with a fixed structure; adaptors (mutation strategy) are
**owned by the optimization algorithm**, not the genome (the "config-strip" model).

1. Subclass `GFlatIndividualT<YourProblem>` (CRTP, in `include/geneva/ind/GFlatIndividualT.hpp`) and override `fitnessCalculation()`.
2. Build the genome **structure** in the constructor with `GGenomeBuilder` (`addDoubleGroup`, `addDouble`, `addInt32Group`, `addBoolArray`, …) and `setGenome(b.build())` — structure only, no adaptors. Read values in `fitnessCalculation()` via `streamline<T>()` / `streamlineFP()`.
3. Author adaptors on an **OA-owned `GAdaptionConfig`** via `oa::makeAdaptionConfig<...>(genome)` + the fluent API (`cfg->groupDouble(i).gauss(...)`, `cfg->forLabel(...)`), and distribute it with `oa_ptr->setAdaptionConfig(cfg)`, `Go2::registerAdaptionConfig(personality, cfg)`, or `oa::StandaloneAdapter(genome, cfg)`. An adapting algorithm with **no** config is a hard error (no auto-derivation).
4. Use `Go2` (in `include/geneva/Go2.hpp`) as the top-level driver — it reads configuration from a JSON file and handles client/server mode automatically.

See `docs/writing-optimization-problems.md` for the full guide, and `examples/geneva/10_GStarter/` (minimal) / `examples/geneva/03_GParameterObjectUsagePatterns/` for canonical examples. (The former `GParameterSet` tree-of-parameter-objects model has been removed.)

### Parallelization

Parallelization is configured externally (via `Go2` JSON config or command-line), not in the problem definition. The same individual code runs serially, multi-threaded, over MPI, or via websockets without modification.

### Serialization

The common interface (`Gem::Common::GCommonInterfaceT<T>`) and all subclasses use Boost.Serialization. Every class that adds data members must implement `serialize()` and `load_()` / `save_()` (or the combined `serialize` template). This is required for network transport of individuals.

### Adaptors

Parameter mutation is handled by adaptors (e.g., `GDoubleGaussAdaptor`, `GInt32FlipAdaptor`). Attach adaptors to parameter objects to control how they evolve.

### Pluggable monitors

`GPluggableOptimizationMonitors.hpp` provides logging/monitoring hooks that can be registered with `Go2::registerPluggableOM()` without modifying the optimization loop.

## Dependencies

- **Boost 1.91+** (filesystem, program_options, regex, serialization, test, atomic): must be compiled with C++20
- **CMake 3.27+**
- **GCC 13+ or Clang 18+** (C++20 required)
- **CUDA** (optional, for GPU-based RNG and CUDA example 15)
- **MPI** (optional, for `GMPIConsumerT` and examples 16/17)

## Notable CMake Flags

| Flag | Default | Description |
|---|---|---|
| `GENEVA_BUILD_TYPE` | `Debug` | Build type |
| `GENEVA_BUILD_TESTS` | `TRUE` | Build unit/manual tests |
| `GENEVA_BUILD_EXAMPLES` | (off) | Build examples |
| `GENEVA_BUILD_BENCHMARKS` | (off) | Build benchmarks |
| `GENEVA_BUILD_WITH_MPI_CONSUMER` | (off) | Enable MPI consumer |
| `USECUDARNG` | (off) | Use CUDA for random numbers in Hap |
| `BOOSTROOT` / `BOOSTINCL` / `BOOSTLIBS` | — | Non-standard Boost location |
| `MPIROOT` | — | Non-standard MPI root |

## Post-Install

After installing, the shared libraries must be visible to the linker:

```bash
echo "/opt/geneva/lib" | sudo tee /etc/ld.so.conf.d/geneva.conf
sudo ldconfig -v
# Or at runtime:
export LD_LIBRARY_PATH="/opt/geneva/lib:$LD_LIBRARY_PATH"
```
