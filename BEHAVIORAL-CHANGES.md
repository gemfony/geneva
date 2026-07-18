# Behavioral changes in Geneva 1.12 (migrating from 1.11 "Hendaye")

Version 1.12 "Puente de Santiago" is a ground-up redesign, so it is a single large
breaking change relative to 1.11. This file collects the **behavioral, source- and
serialization-breaking** changes you need in order to port existing code,
configuration files or checkpoints. The high-level rationale and build prerequisites
are in `CHANGES` and `INSTALL`; this file is the practical upgrade guide.

> **Checkpoints and binary archives from 1.11 (or earlier) no longer load.** The
> individual model, the serialization layout and the stored value representation all
> changed. This is a clean break: re-run optimizations from scratch. JSON
> configuration files are *not* binary and are covered case-by-case below.

---

## 1. Toolchain and build

- **C++23 is now required** (was C++20). Minimum compilers: **GCC ≥ 14** or
  **Clang ≥ 18**. The build pins `CMAKE_CXX_STANDARD=23` and rejects anything lower,
  and the exported CMake package advertises `cxx_std_23`, so **downstream consumers of
  the `Geneva::` targets are compiled as C++23 too**. A C++20-only toolchain can no
  longer build or link against Geneva.
- **Boost ≥ 1.91**, built in C++23 mode; **CMake ≥ 3.27**; **Catch2 v3** for the tests.
- **CUDA (optional) raised to ≥ 13.3, and now gated on the device too.** CUDA code (the
  CUDA examples/benchmark, the CUDA RNG backend, the GPU consumer) is enabled only when
  *both* the toolkit *and* the installed driver support CUDA ≥ 13.3; otherwise CUDA is
  auto-disabled with a warning and the rest of Geneva still builds. Escape hatches:
  `SKIPALLCUDA=1` / `-DGENEVA_SKIP_CUDA=ON` (no CUDA); `FORCECUDA=1` /
  `-DGENEVA_FORCE_CUDA=ON` (bypass the *driver* check for build farms/CI, still
  requiring the 13.3 toolkit).
- **Processors:** 64-bit **x86-64** or **AArch64** (both first-class). SIMD (AVX2/NEON)
  and a CUDA GPU are used when present but never required.

## 2. Defining a problem (the individual model)

- **The tree of parameter objects is gone.** `GParameterSet` and the whole
  `GParameterBase` / `GParameterT` / `GConstrainedDoubleObject` / `GDoubleCollection` /
  `GParameterObjectCollection` hierarchy (each node carrying its own adaptor) has been
  removed and replaced by a single **flat genome**.
- **Author a problem by subclassing `GGenome`** (typically via the CRTP helper
  `GGenomeT<Derived>`). Build a flat, fixed-structure array of double / float / int32 /
  bool values once in the constructor with `GGenomeBuilder` (`addDoubleGroup`,
  `addDouble`, `addInt32Group`, `addBoolArray`, …) and install it with `setGenome()`.
  Read values by position via `streamline<T>()` / `streamlineFP()`.
- **Fitness hook changed: override `std::vector<double> evaluate()`** — a virtual member
  returning the raw fitness values (size 1 for a single-criterion problem, one entry per
  criterion otherwise) — **in place of `double fitnessCalculation()`**, which has been
  removed from the base and every individual. `evaluate()` *returns* the results (it
  sets no fitness itself); the framework applies the feasibility / policy / PROCESSED
  pass. A multi-criterion problem populates every criterion from the one returned vector.
  *Migration:* replace `double fitnessCalculation()` with
  `std::vector<double> evaluate()` returning the raw vector.
- **The genome is pure data; mutation lives on the algorithm.** Adaptors are no longer
  attached to parameters. Author them on an **OA-owned `GAdaptionConfig`** (via
  `oa::makeAdaptionConfig<...>(genome)` and the fluent API) and distribute it
  (`setAdaptionConfig`, `Go2::registerAdaptionConfig`, or `oa::StandaloneAdapter`). An
  adapting algorithm with **no** config is a hard error — there is no auto-derivation.
- **OA state removed from the individual.** The per-individual best-known-fitness and
  stall counters are gone (that bookkeeping lives only on the algorithm). The
  adaption-retry limits `max_unsuccessful_adaptions` / `max_retries_until_valid` **moved
  onto the OA-owned adaption configuration** — the individual's config file no longer
  accepts those keys; set them on the adaption config instead.
- **Population-uniform rules are shared.** Optimization direction, evaluation policy,
  sigmoid parameters and the constraint object were hoisted out of every individual into
  a single reference-counted `GProblemPolicy`, so they travel/checkpoint once per
  population.
- **Candidates hold no random-number engine.** The per-individual RNG (`gr_` /
  `getRandomEngine()`) was removed; a candidate is pure data. Code that drew randomness
  inside `evaluate()` via `getRandomEngine()` should now lease a proxy:
  `auto l = Gem::Hap::randomLeasePool().acquire(); /* draw from *l */;`.
- **Large problem constants** (a training set, a target image) live in the individual
  module's load-once store (`GProblemStoreT`, filled from the factory's `init_()` hook),
  not on each individual.

## 3. Normalized coordinates and dimensionless mutation

- A floating-point parameter is stored in a **normalized internal coordinate**; the
  user-visible value is its affine image. This is invisible above the genome
  read/write layer — `evaluate()` and inspection always see external (user) values.
- A parameter is **bounded** (`addDouble(init, lo, hi)`, `addDoubleGroup`, …) or
  **unbounded** (`addDoublePlainGroup`, `addDouble(init)`, …). A bounded value folds
  into `[lo, hi)` on every write; an out-of-range **external assignment throws**.
- **Mutation parameters are dimensionless fractions of a parameter's range** (σ, σ
  bounds, the GD/CGD step, the PSO velocity fraction): `1.0` = the full range. The same
  default works for any range. Rates/probabilities (`sigma_sigma`, `ad_prob`, …) are
  unchanged.

## 4. Configuration files

- **Configuration files are now strict, standard JSON** (previously Boost.PropertyTree /
  XML). Boost.PropertyTree has been removed from Geneva entirely; the individual→tree
  dump moved to JSON as well. Configs use native JSON scalars (numbers, booleans).
- **The external-evaluation protocol was rewritten** (a breaking change for external
  evaluators; the bundled `evaluator.py` was rewritten to match).
- A generated **configuration reference** lives in `docs/config-reference/`. Programs can
  materialize/refresh their config files with `--update-configs` (non-Go2 programs honor
  it too).

## 5. Parallelization and consumers

- **One consumer per process.** A process uses a single consumer held in a process-global
  `GConsumerRegistry`; an algorithm submits to it and gets back a fully-evaluated
  population, transport-agnostic. The former per-algorithm broker/executor injection —
  `setBroker` / `setLocalConsumer` and the `GBrokerT` / `GExecutorT` wrappers — has been
  **removed**. Serial execution is just the local thread consumer (`stc`) with one worker.
- **Meta-optimization** (tuning an algorithm's own parameters) is now the single facility
  `GMetaEvolutionaryAlgorithm` (example 11), which evaluates its umbrella individuals on
  its own orchestration pool while their sub-optimizations submit to the one process
  consumer — so the consumer is never re-entered.
- **The GPU consumer is a first-class mnemonic** (`--consumer gpu`, when built with
  `GENEVA_BUILD_WITH_GPU_CONSUMER`). A GPU problem contributes only its device marshaller
  via `Gem::Geneva::registerGPUMarshaller<...>(...)`; Go2 builds/selects the consumer
  like any other.
- **Wire transport optimizations.** The shared genome layout is sent **once per client**
  and referenced by a content id thereafter; on the websocket/Asio consumers a processed
  item returns **results-only** by default (the server grafts the original parameters
  back on). Checkpoint/file serialization stays self-contained (full layout by value).
- **Networked consumers' timeout / death-detection is now user-configurable** through a
  config file.
- **The EA post-optimizer always refines inline.** `GEvolutionaryAlgorithmPostOptimizer`
  is constructed from the inner-EA config file alone; the former `execMode` constructor
  parameter and `setExecMode` / `getExecMode` have been **removed** (the nested
  refinement never submits to the process consumer, so there was no mode to select).

## 6. Packaging: runtime-loadable modules

- **The Geneva library ships no concrete optimization individual.** The reusable sample
  problems were folded into `gemfony-geneva` (`Gem::Geneva::Individuals`); there is no
  separate `geneva-individuals` library.
- **Individuals, optimization algorithms and GPU marshallers can be shipped as
  runtime-loadable `.so` modules** (`GENEVA_DECLARE_INDIVIDUAL` /
  `GENEVA_ADD_INDIVIDUAL_MODULE`, `individualManifest<>` / `oaManifest<>` /
  `marshallerManifest<>`), loaded with `--module <path>.so` (repeatable). Modules carry a
  toolchain-compatibility fingerprint; an incompatible module is rejected at load with a
  clear diagnostic.

## 7. Renames and small API removals

- **The vestigial "Flat" qualifier was dropped from the genome layer** (source-breaking;
  update any references to the old `*Flat*` names).
- **The standalone gradient descent ("gd") is gone.** Plain gradient descent survives as
  the steepest-descent (β = 0) mode of the conjugate gradient descent (`cgd`);
  `GGradientDescent_PersonalityTraits` was removed with it.
- **`GRandomFactory::init()` was removed.** It had been an empty formality for a long
  time: constructing the factory singleton (`Gem::Hap::randomFactory()`) is all that is
  needed, and the producer threads start lazily on the first container request. Drop the
  call; `finalize()` is unchanged.
- **The `execMode` enum is gone.** It was the selector of the removed per-algorithm
  broker parallelization model; under the one-consumer model the choice is simply which
  consumer the process registers. The direct-mode examples/benchmark keep their numeric
  `--parallelizationMode 0|1|2` command-line option (now a plain integer).
- **A custom optimization algorithm no longer has to override `actOnStalls_()`.** The
  hook now has an empty default on `GOptimizationAlgorithmBase`; override it only when
  the algorithm actually reacts to a stall (as the parent-child EA base does).
- **Geneva's enums stream through one shared, opt-in operator template**
  (`Gem::Common::numeric_enum_io_v` in `GCommonEnums.hpp`) instead of per-enum
  hand-written `operator<<`/`operator>>` pairs. The textual form (the underlying
  number) is unchanged, so configuration files and archives are unaffected; only code
  that took the address of one of the old operator functions needs adjusting. A
  user-defined enum can opt into the same machinery by specializing the marker.
- **dietrich: `project<I>()`/`projectX..W()` on a non-all-double collector is now a
  compile-time error** (it used to compile and throw at run time), and the projections
  are available for every axis of every all-double collector arity.
- **The optimization algorithms' population-sweep hook `runFitnessCalculation_()` was
  renamed to `evaluatePopulation_()`** (a protected virtual on
  `GOptimizationAlgorithmBase`; source-breaking for an out-of-tree algorithm that
  overrides it). The old name referenced the removed `fitnessCalculation()` user hook.
- **courtier: `executor_status_t` was renamed to `submission_status_t`** (and its header
  `GExecutorStatusT.hpp` to `GSubmissionStatusT.hpp`) — the type is the return of the
  submission entry point `workOn` and was named for the removed `GExecutorT` wrapper.
- **Two benchmark directories were renamed** for the same reason: `GBrokerOverhead` →
  `GConsumerOverhead` and `GBrokerSanityChecks` → `GConsumerSanityChecks` (the
  executable names change accordingly).
- **common: `GLogger` is no longer a class template.** The single streamer type it ever
  produced (`GLogStreamer`) is now fixed; code naming `GLogger<GLogStreamer>` drops the
  template arguments. The `glogger` singleton and all streaming behavior are unchanged.
- **common: the parser's reference-parameter proxy classes were removed**
  (`GFileReferenceParsableParameterT` and its vector/array siblings). The
  `registerFileParameter(name, reference, default, ...)` overloads are **unchanged** —
  they now assign through the callback proxy internally; the on-disk config format is
  identical. The test-only helpers `configureFromFile()` and `GParserBuilder::cl_at()`
  were removed as well.
- **dietrich: the public headers no longer re-export all of `Gem::Common`** (the former
  `using namespace Gem::Common;` in every header). The specific common names dietrich's
  own API uses are still imported per-name; a consumer that relied on the blanket
  re-export for its *own* unqualified use of common names must now qualify or import
  them itself.
- **courtier: the socket consumers/clients gained shared bases** — `GAsioConsumerT` /
  `GWebsocketConsumerT` now derive from `GTcpAcceptingConsumerT` (the common TCP server
  shell) and `GAsioConsumerClientT` / `GWebsocketClientT` from `GPrefetchingClientT`
  (the common prefetch/compute pipeline). Public construction, options and the wire
  protocol are unchanged; only code naming the old direct base classes is affected.

## 8. Checkpointing

- **The checkpoint directory is created lazily**, at the first actual checkpoint write;
  merely configuring an algorithm (`setCheckpointBaseName`) no longer touches the
  filesystem.
- **A final checkpoint is written when a run halts** (file name tagged `final`) whenever
  checkpointing is enabled (`cp_interval != 0`); with checkpointing disabled a run
  performs no checkpoint I/O at all.
- **The personality-traits archive layout changed within the 1.99 development series**:
  the position-only traits of CGD / Nelder-Mead / parameter scan / GSA / ACO / PSO2011 and
  the parent-child traits now serialize their population position through the shared
  `GPositionPersonalityTraits` base (one added nesting level; the GSA/ACO
  `population_position_` and PSO `particle_` tags became `pop_pos_`). Checkpoints written
  by earlier 1.99 development builds do not load; 1.11 checkpoints never loaded in 1.12+
  anyway (see the top of this document).

---

*For the day-to-day authoring model with worked examples, see
`docs/writing-optimization-problems.md`, `examples/geneva/10_GStarter/` (minimal) and
`examples/geneva/03_GParameterObjectUsagePatterns/`.*
