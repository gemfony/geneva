# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## ⚠️ Development Invariants (MANDATORY — read first)

**[`DEVELOPMENT-INVARIANTS.md`](DEVELOPMENT-INVARIANTS.md) holds the non-negotiable rules for all work in this
repository, and they OVERRIDE convenience and default behaviour.** Read it before making changes; if a change
would violate one, stop and find another approach. In particular:

1. **Prefer existing Geneva capabilities over re-inventing them** — search `common/`/`hap/`/`courtier/`/`geneva/`
   for an existing utility, container, or scaffold and use (or improve) it before writing a new one.
2. **Concurrency primitives come only from the shared concurrency facilities** — the `common/concurrency/`
   sub-module (namespace `Gem::Common::Concurrency`): thread pools, thread groups, thread-safe queues
   (`GBlockingMPMCQueueT` / `GPreallocatedMPMCQueueT` / `GMPMCQueueT`), thread-safe keyed/set stores
   (`GThreadSafeKeyedStoreT` / `GThreadSafeSetT`), content-addressed (`GContentAddressedStoreT`) and aging
   (`GAgingStoreT`) stores, a completion latch (`GCompletionLatchT`), a lock-free SPSC staging ring
   (`GSPSCStagingRingT`). No roll-your-own `deque`+`mutex`, bespoke ring, hand-rolled pool, or ad-hoc
   thread-safe map; improve/extend the shared ones (creating a new one only after consultation, and in the owning library).
3. Build out-of-source only · 4. Serialization complete and single-sourced (`localMembers_`) · 5. One consumer
   per process (`GConsumerRegistry`) · 6. The genome is pure data; mutation lives on the OA · 7. On a test
   failure during a refactor/major change, triage first — fix the code only for a genuine failure, else amend
   the test to the new valid contract (never silence a real failure, never bend correct code to a stale test) ·
   8. Full builds go through `cd $HOME/build && $HOME/ClionProjects/geneva/scripts/prepareBuild.sh --clean -y
   --build genevaConfig.gcfg` (the `.gcfg` centralises compiler/CUDA/build-type choices; use `$HOME`) · 9. A
   failing test is fixed before moving on, even if it looks unrelated — never stepped over as "pre-existing"
   or "environmental"; a green suite is the precondition for continuing and for every commit · 10. Newly
   discovered compilation warnings must be investigated (root-fix or understand-and-record why benign).

See the file for the full, authoritative list.

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
| `courtier` | Consumer-based parallelization framework. A process uses ONE consumer, held in a process-global `GConsumerRegistry`: `GSerialConsumerT` (`sc`), `GStdThreadConsumerT` (`stc`), `GAsioConsumerT` (`asio`), `GWebsocketConsumerT` (`beast`), `GMPIConsumerT` (`mpi`), or the optional `GGPUConsumerT` (`gpu`, folded into `gemfony-courtier` when built with `GENEVA_BUILD_WITH_GPU_CONSUMER` — opt-in like the MPI consumer; there is no separate GPU library). An algorithm submits a batch straight to that consumer (`GBaseConsumerT::processBatch`), which evaluates and reconciles it against the submission policy |
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

#### Normalized coordinate model

Internally, a floating-point parameter is stored in a **normalized internal coordinate** (magnitude ≈ 1, confined to the centered unit interval `[-0.5, 0.5)` for a bounded parameter); the user-visible **external** value is its affine image, composed in `long double` for a well-conditioned round-trip. This is invisible above the genome read/write layer — `fitnessCalculation()`, the GPU marshaller and user inspection always see external (user-coordinate) values via `streamline<T>()` / `streamlineFP()`; the optimization algorithms are **boundary-agnostic** and work in the internal coordinate via the `*Internal` accessors.

- A parameter is **bounded** (`addDouble(init, lo, hi)`, `addDoubleGroup`, …) or **unbounded** (`addDoublePlainGroup`, `addDouble(init)`, …) — a single `fold` bit. A bounded value folds into its `[lo, hi)` interval on every write; an out-of-range **external assignment throws** (including exactly the open `upper`). An unbounded value roams ℝ; its `[min, max]` is only the init perimeter / mutation scale, not a constraint.
- **Mutation parameters are dimensionless fractions of a parameter's range.** σ, σ-bounds, the GD/CGD finite step and the PSO velocity fraction are interpreted as a fraction of the parameter's range (`1.0` = the full range; `×100` = percent) — the same default works for any range. Rates/probabilities (`sigma_sigma`, `ad_prob`, ACO `xi`) are dimensionless rates, unchanged.
- Stored representation changed, so **older binary checkpoints do not load** (a clean break); JSON configs need no migration (the values were already fractional).

### Parallelization

Parallelization is configured externally (via `Go2` JSON config or command-line), not in the problem definition. The same individual code runs serially, multi-threaded, over MPI, or via websockets without modification. On the networked transports the shared genome layout is sent once per client and the individual's parameters are not echoed back with each result — see "Wire transport" under Serialization below.

**One consumer per process.** An algorithm is transport-agnostic: it never builds or selects a consumer, it just submits to the single process consumer (held in `Gem::Courtier::GConsumerRegistry`) and gets back a fully-evaluated population — never knowing whether evaluation happened locally, on a GPU, or on a remote client. The consumer is established once: by `Go2` (from the chosen mnemonic / command-line default), or manually via `Go2::registerConsumer()` for a fully custom consumer. The GPU consumer is a first-class mnemonic (`--consumer gpu`, available when Geneva is built with `GENEVA_BUILD_WITH_GPU_CONSUMER` / gcfg `BUILDGPUCONSUMER`, which folds the device backends into `gemfony-courtier`): a GPU problem contributes only its device marshaller through `Go2::registerGPUConsumerBuilder(...)` and Go2 builds/selects the consumer like any other (see example 15). A standalone algorithm run without `Go2` lazily builds a default local thread-pool consumer on first use. There is no per-algorithm broker/executor injection (the former `setBroker` / `setLocalConsumer` and the `GBrokerT` / `GExecutorT` wrappers have been removed); `Go2`'s destructor releases the consumer it established so a networked server is torn down by RAII at the end of the run (or call `GConsumerRegistry::clear()` to shut it down explicitly).

**Meta-optimization** (tuning an algorithm's own parameters) uses `GMetaEvolutionaryAlgorithm` — a standard EA whose population members are "umbrella" individuals (`GMetaOptimizerIndividualT`), each running a sub-optimization. It evaluates those umbrella individuals on its OWN orchestration thread pool, while their sub-optimizations submit to the one process consumer. Because the orchestration pool and the work consumer are distinct, the consumer is never re-entered (no nested-pool deadlock). It is the single facility for meta-optimization (example 11). A non-distributing nested refiner (the post-optimizer) instead evaluates inline via `GEvolutionaryAlgorithm::setInlineEvaluation()`.

### Serialization

The common interface (`Gem::Common::GCommonInterfaceT<T>`) and all subclasses use Boost.Serialization. Every class that adds data members must implement `serialize()` and `load_()` / `save_()` (or the combined `serialize` template). This is required for network transport of individuals.

#### Wire transport: layout send-once + results-only returns

A flat genome's shared structural layout (bounds / grouping / labels — `GGenomeLayout`) is identical across an entire population, so re-sending it with every work item dominates the wire size for a large structured genome. On the network path the layout is therefore **sent once and referenced by a content id** thereafter:

- `GGenomeLayout::layoutId()` is a 128-bit content hash; `GFlatGenome::save()` emits the full layout to a given client only the first time that client sees the id, and the id alone after. Because the id is content-derived, an evolving structure (a different layout) simply hashes to a new id and is sent once more — no "layout changed" signalling.
- The machinery is transport-agnostic and lives in the **courtier** consumer/session layer (`courtier/include/courtier/GWireSerializationContext.hpp`): a thread-local `GWireSerializationScope` engaged around a work-item (de)serialization, plus a `GWireLayoutRegistry` (a content-addressed, opaque-blob store with per-peer ack tracking and LRU eviction). It knows nothing about the genome — geneva computes the id and (de)serializes the layout. All three networked transports (websocket, Asio, MPI) opt in; **checkpoint / file serialization deliberately stays self-contained** (no scope active → the full layout travels by value, so an archive always loads on its own).
- A client that receives an id for a layout it does not hold fetches it on demand (`REQUEST_LAYOUT` / `SEND_LAYOUT`); reconnecting / late-joining clients are handled transparently.
- **Returns** (worker → server): on the websocket and Asio consumers a processed item is returned **results-only** by default — only the computed results (all evaluations of a multi-criterion individual) travel; the server still holds the originally-submitted item and grafts its parameters back on (`GProcessingContainerT::graftInputDataFrom`, applied in `GNetworkedConsumerT::checkin`). A client that has *modified* the individual (e.g. a network-tiered client doing its own nested optimization) opts into a full return via `GOptimizableEntity::setReturnFullIndividual(true)`. The MPI consumer keeps no per-item original to graft onto, so it returns the full individual; only its submit direction is deduplicated.

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
