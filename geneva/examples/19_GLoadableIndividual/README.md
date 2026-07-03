# Example 19 — a runtime-loadable optimization problem (individual plugin)

This example shows how to express an optimization problem as a **runtime-loadable plugin**, so Geneva can
run a new problem **without being recompiled**. It has two artifacts:

- **`libGLoadableParaboloid.so`** — the problem (an *n*-dimensional paraboloid), built as a shared object.
- **`GGenericOptimizer`** — a generic optimizer with **no problem compiled in**; it loads a problem at
  runtime via `--individual`.

## Run it

```bash
# from this example's build directory (so ./config/Go2.json is found):
./GGenericOptimizer --individual ./libGLoadableParaboloid.so
```

You can also point at the plugin from the config file instead of the command line — set
`individual_plugin_path` in `config/Go2.json` (the `--individual` option overrides it).

Because a Geneva server and client are the **same binary** launched with different options, a networked run
just needs the same `.so` present on every node (already there on a shared HPC filesystem; staged next to
the binary in the cloud):

```bash
./GGenericOptimizer --individual ./libGLoadableParaboloid.so --consumer beast          # server
./GGenericOptimizer --individual ./libGLoadableParaboloid.so --consumer beast --client  # worker
```

## How to make your OWN loadable individual

1. **Write an ordinary Geneva flat individual** (`GLoadableParaboloid.hpp` here). Nothing about it is
   plugin-specific — the same class could be compiled in. It supplies the usual config-driven hooks
   (`Config`, `describeConfig`, `buildGenome`, `buildAdaptionConfig`, `fitnessCalculation`, `serialize`).

2. **Add one translation unit** (`GLoadableParaboloidPlugin.cpp`) with exactly two registrations:

   ```cpp
   #include "geneva/ind/GFlatIndividualFactory.hpp"
   #include "geneva/ind/GIndividualPlugin.hpp"
   #include "MyProblem.hpp"

   BOOST_CLASS_EXPORT(MyProblem)                                     // wire / checkpoint GUID
   GENEVA_INDIVIDUAL_PLUGIN(
       Gem::Geneva::Genome::GFlatIndividualFactory<MyProblem>,
       "./config/MyProblem.json")                                   // the factory's config file
   ```

   `GENEVA_INDIVIDUAL_PLUGIN` emits the two C entry points the loader resolves: an **ABI-version marker**
   (baked in from `GENEVA_VERSION` at build time) and a **factory** (built with the given config file,
   auto-created with the individual's defaults if absent). `BOOST_CLASS_EXPORT` registers the
   serialization GUID so the individual can cross the wire and a checkpoint.

3. **Build it as a shared object** linking Geneva (see `CMakeLists.txt`).

That is the entire author-facing surface.

## What the mechanism guarantees

- **Exactly one problem per process.** Compiling an individual in *and* passing `--individual` (or passing
  two plugins) is a hard error naming both sources. Providing none is a clear error listing the three ways
  to supply a problem.
- **Version safety.** A plugin built against a different Geneva version is **rejected at load** with a
  precise message (its ABI marker is checked before any individual is constructed) — no silent
  mis-load. (Boost.DLL itself performs no compatibility check; this gate is Geneva's.)
- **Checkpoints.** A checkpoint stores the concrete individuals, so resuming one requires the **same**
  individual provided the same way (compiled in, or the same `--individual` plugin) that wrote it. Resuming
  without it fails with actionable guidance instead of an opaque deserialization error.
