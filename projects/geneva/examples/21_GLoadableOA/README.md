# Example 21 — a runtime-loadable optimization algorithm (OA module)

This example is the reference for authoring an **optimization algorithm** that Geneva loads at runtime, so
a new algorithm can be shipped and used **without recompiling Geneva or the optimizer**. It is the algorithm
analogue of example 19 (a loadable problem) and has two artifacts:

- **`libGRandomSearch.so`** — the algorithm (a random search), built as a shared object.
- **`GLoadableOAOptimizer`** — a generic optimizer with **no algorithm compiled in**; it loads one at
  runtime via `--module` and then selects it by its mnemonic.

## Run it

```bash
# from this example's build directory (so ./config/Go2.json is found):
./GLoadableOAOptimizer --module ./libGRandomSearch.so --optimizationAlgorithms rsearch
```

`--module` is repeatable, and a module's algorithms are loaded *before* the option surface is built, so
`rsearch` appears in `--help` and contributes its own command-line options exactly as a built-in does. Use
`--module` for algorithms and `--individual` for the optimization problem: naming an algorithm module with
`--individual` is refused by name.

## How to make your OWN loadable algorithm

1. **Write the algorithm and its personality traits** (`GRandomSearch.hpp/.cpp`,
   `GRandomSearch_PersonalityTraits.hpp/.cpp`). Nothing about either is module-specific — the same classes
   could be compiled into Geneva. The traits' `nickname` is the mnemonic users will type.

2. **Name its factory.** For almost every algorithm this is one line — the scaffold generates the config
   path, `getMnemonic()`, `getAlgorithmName()`, `getObject_()` and the constructors:

   ```cpp
   using GMyAlgorithmFactory =
       Gem::Geneva::OptimizationAlgorithms::GOptimizationAlgorithmFactoryT<GMyAlgorithm, GMyAlgorithm_PersonalityTraits>;
   ```

   Derive from that instantiation instead if your algorithm needs more of its factory — extra command-line
   options or a `postProcess_()` step (`GParameterScanFactory` in the Geneva library is the in-tree example).
   Either way the factory **is** the provider Geneva's algorithm store holds, so nothing wraps it.

3. **Add one translation unit** (`GRandomSearchPlugin.cpp`) with the serialization registrations and the
   manifest entry point:

   ```cpp
   #include "weft/GArchivePolymorphic.hpp"
   #include "geneva/oa/GOAPlugin.hpp"
   #include "GMyAlgorithm.hpp"
   #include "GMyAlgorithmFactory.hpp"
   #include "GMyAlgorithm_PersonalityTraits.hpp"

   GEM_REGISTER_ARCHIVABLE(GMyAlgorithm)                    // checkpoint tags: the algorithm ...
   GEM_REGISTER_ARCHIVABLE(GMyAlgorithm_PersonalityTraits)  // ... and its personality traits

   extern "C" BOOST_SYMBOL_EXPORT const GenevaModuleManifest *geneva_module_manifest() {
       return Gem::Geneva::oaManifest<GMyAlgorithmFactory, "GMyAlgorithm">();
   }
   ```

   The `extern "C"` `geneva_module_manifest()` is the only irreducible boilerplate (the loader resolves this
   fixed, unmangled symbol via `dlsym`); everything else is the typed helper
   `Gem::Geneva::oaManifest<Factory, Name>()`. It builds the manifest: the **toolchain-compatibility
   fingerprint** (`GenevaCompat`) and the **module-ABI stamp** (`GENEVA_MODULE_ABI_VERSION`), both validated
   by the loader before anything else is touched, plus one **OA contribution** whose thunk hands the loader
   your factory. The registrations are the same ones a compiled-in algorithm needs, so a checkpoint written
   with the module loaded resumes with it loaded.

4. **Package it** with `GENEVA_ADD_INDIVIDUAL_MODULE` (see `CMakeLists.txt` — the helper is kind-agnostic).
   The module links **none** of the Geneva libraries: it needs only their headers and resolves their symbols
   from the host process at load time.

That is the entire author-facing surface. The registration path is the same one Geneva's own algorithms
take — `Gem::Geneva::registerOptimizationAlgorithm()` — reached from the module loader instead of from
`GInitializerT`; there is no separate mechanism for a loaded algorithm.

## What the mechanism guarantees

- **No shadowing.** A module whose algorithm claims a mnemonic a built-in or another module already holds
  is refused, naming the mnemonic. Pick a distinct one.
- **Load-time safety.** A module built with a different compiler / standard library / Boost / build mode /
  Geneva version, or against a different module ABI, is rejected before any of its C++ is touched, with a
  message naming the offending axis. So are a shared object that is not a Geneva module, a manifest that
  advertises nothing, a contribution without its factory entry point, and a contribution of a kind this
  Geneva cannot serve. **A failed load is never silent.**
- **One process, one symbol namespace.** The module is loaded `RTLD_GLOBAL | RTLD_NOW`, so it shares the
  host's singletons and its serialization registry — the loaded algorithm submits through the same process
  consumer as everything else — and every symbol it needs is resolved eagerly, at load, rather than
  surfacing as a crash mid-run.
