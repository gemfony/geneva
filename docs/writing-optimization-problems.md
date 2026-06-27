# Writing an optimization problem in Geneva (flat genome + OA-owned adaption config)

Geneva separates **what** is optimized (the problem's genome — a flat list of parameters with a
fixed structure) from **how** it is mutated (the adaptors — owned by the optimization algorithm, not
the genome). This guide covers the current API after the "config-strip": the genome builder is
**structure-only**, and adaptors live on an **OA-owned `GAdaptionConfig`**.

Canonical examples: `examples/10_GStarter` (minimal), `examples/03_GParameterObjectUsagePatterns`
(parameter patterns), `examples/09_GNeuralNetwork` (an architecture-decoded flat genome).

## 1. Subclass `GFlatIndividualT` and override `fitnessCalculation()`

```cpp
#include "geneva/ind/GFlatIndividualT.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"

namespace gen = Gem::Geneva::Genome;

class MyProblem : public gen::GFlatIndividualT<MyProblem> {       // CRTP: pass yourself
public:
    MyProblem() { buildGenome(); }                                 // build the STRUCTURE in the ctor
    MyProblem(const MyProblem &) = default;

protected:
    double fitnessCalculation() override {
        std::vector<double> x;
        this->streamline<double>(x);                               // read the (range-folded) values
        double sum = 0.;
        for(double v : x) { sum += v * v; }
        return sum;                                                // minimized by default
    }

private:
    void buildGenome() {
        gen::GGenomeBuilder b;
        b.addDoubleGroup(5, -10., 10.).init(1.0);                  // STRUCTURE only -- no adaptor here
        this->setGenome(b.build());
    }
};
```

`GFlatIndividualT<Derived>` supplies the clone/load/compare/serialize machinery. Your genome is a
flat set of value channels (double / float / int32 / bool); read it with `streamline<T>()`,
`streamlineFP()`, or — for a structured genome — a `GGenomeArchitecture` decoder (see ex09).

## 2. Build the genome STRUCTURE with `GGenomeBuilder`

The builder defines parameters and their **structure only** (count, init value, bounds/range,
optional label). It does **not** attach adaptors.

```cpp
gen::GGenomeBuilder b;
b.addDouble(0., -1., 1.);              // one constrained double in [-1, 1)
b.addDoubleGroup(4, -2., 2.);          // ONE group of 4 constrained doubles (shared adaptor later)
b.addDoubleArray(3, -3., 3.);          // 3 independent groups of 1
b.addDoublePlainGroup(8, -1., 1.);     // UNBOUNDED doubles; the range is just the init perimeter
b.addInt32Group(3, 0, 9);              // constrained int32s
b.addBoolArray(2);                     // booleans
b.addDoubleGroup(2, -1., 1.).label("position"); // a label groups params for config-by-label
this->setGenome(b.build());
```

`ParamHandle` modifiers (chainable): `.init(v)`, `.perimeter(lo, hi)`, `.label(name)`,
`.adaptionMode(mode)`. A handle from an `add*Array`/`add*Group` call applies to **every** group it
created.

### Bounded vs unbounded, and the normalized coordinate model

Geneva stores a floating-point parameter in a **normalized internal coordinate** (confined to the
centered unit interval `[-0.5, 0.5)` for a bounded parameter) and presents the user-visible **external**
value — the one your `fitnessCalculation()` sees via `streamline<T>()` — as its affine image. You never
deal with the internal coordinate directly; the model only changes how you think about two things:

- **Bounded vs unbounded is a single choice at build time.** A bounded parameter (`addDouble(init, lo,
  hi)`, `addDoubleGroup`, …) has a hard `[lo, hi)` wall: a mutation that overshoots is folded back into
  range, and assigning an out-of-range external value (including exactly `upper`) **throws**. An
  unbounded parameter (`addDoublePlainGroup`, `addDouble(init)`, …) has no wall — it may roam ℝ; its
  `[min, max]` is only the init perimeter / mutation scale (e.g. a neural-net weight seeded in `[-10,10]`
  may reach 200). `.perimeter(lo, hi)` narrows the random-init region (it may be tighter than a bounded
  parameter's wall).
- **Mutation magnitudes are fractions of the parameter's range.** The Gauss σ (and σ-bounds), the
  GD/CGD finite step and the PSO velocity fraction are dimensionless fractions of the range: `σ = 0.1`
  means "10% of this parameter's range" for *every* parameter and *every* problem, so re-ranging a
  parameter never requires re-tuning its σ. Rates and probabilities (`sigma_sigma`, `ad_prob`, ACO `xi`)
  are dimensionless and unaffected.

## 3. Author adaptors on the OA-owned `GAdaptionConfig`

Adaptors are configured **per optimization algorithm**, on a `GAdaptionConfig` built from the
genome's structure. Use the fluent API to attach a kernel to a group (by index or label):

```cpp
namespace oa = Gem::Geneva::OptimizationAlgorithms;

MyProblem proto;                                                   // a prototype to read the structure
auto cfg = oa::makeAdaptionConfig<oa::GEAAdaptionConfig>(proto);   // GEA / GSA / base variant
cfg->groupDouble(0).gauss(0.5, 0.8, 1e-3, 2., 1.);                 // sigma, sigma_sigma, min, max, ad_prob
cfg->groupInt32(0).intGauss(0.5, 0.8, 1e-3, 2., 1.);              // integer Gaussian
cfg->groupBool(0).flip(1.);                                        // bit-flip
cfg->forLabel("position").gauss(0.3, 0.8, 1e-3, 1., 1.);          // configure by structural label
```

`makeAdaptionConfig<ConfigT>(genome)` returns a `shared_ptr<ConfigT>` sized to the genome's groups.
The config is validated against the genome (`checkConsistency`) when it is adopted.

## 4. Distribute the config to the optimization algorithm

There are three ways to give an algorithm its adaption config:

- **Directly on the algorithm** — `oa_ptr->setAdaptionConfig(cfg);`
- **Through Go2 (by OA personality type)** — when you let Go2 build the algorithm chain:
  ```cpp
  go.registerAdaptionConfig("PERSONALITY_EA", cfg);   // every EA in the chain adopts this cfg
  ```
- **Self-driven adaption (no OA)** — to adapt a genome outside an algorithm:
  ```cpp
  oa::StandaloneAdapter adapter(genome, cfg);
  adapter.adapt(genome);                              // mutates in place
  ```

## 5. Adapting without a config is a hard error

An algorithm that needs to adapt but has **no** adaption config is a **fatal error** at setup — there
is no silent auto-derivation of adaptors from the genome. Always provide a config via one of the
mechanisms above (or, for a quick start, build a default from the genome's structure with
`makeAdaptionConfig` + a Gaussian on every FP group, as the generic post-optimizer does).

## Why this split?

The genome is pure data (structure + values + fitness); the same problem can be optimized by EA, SA,
swarm, gradient descent or a parameter scan without changing the individual, because the mutation
strategy is the algorithm's concern, authored on its own config. This is what makes a Geneva
individual transport-cheap (a flat genome with a shared, immutable layout) and lets one problem
definition run serially, multi-threaded, over MPI or via websockets unchanged.
