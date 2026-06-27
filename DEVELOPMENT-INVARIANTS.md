# Development Invariants

Mandatory rules for **all** work in this repository — human or AI. They **override** convenience and
default behaviour. If a change would violate one, stop and find another approach. An invariant is changed
only by an explicit maintainer decision, recorded here.

Every rule below is, at bottom, an instance of **Invariant 1**.

---

## 1. Prefer existing Geneva capabilities over re-inventing them

Before writing a new utility, container, algorithm scaffold, parser, or helper, look for an existing one in
`common/`, `hap/`, `courtier/`, or `geneva/` and **use it** — improving or extending it if it falls short.
Reinvention fragments the codebase, duplicates bugs, and bypasses hardening and tests that already exist.

Creating a **new** general component (rather than extending an existing one) is allowed **only after
consultation with the maintainer, and only when extending an existing facility genuinely does not fit**. When
you do create one, it **must** be added to a library as a reusable building block, **never dropped at the
call site out of convenience**. Placement follows generality: **if the component is applicable to more than
one library, it goes to `common/`** (or at least to the more general of the libraries that may use it); only a
component specific to a single library belongs in that library. Utilities, containers, and concurrency
primitives are general → `common/`. Queues and the other concurrency primitives (Invariant 2) are one concrete
example of this rule; apply the same reflex everywhere.

## 2. Concurrency primitives come only from the shared concurrency facilities

All concurrency building blocks — **thread pools, thread groups, thread-safe queues, thread-safe keyed
stores, and any lock-free structures** — come from Geneva's shared concurrency facilities in `common/`
(`GBlockingMPMCQueueT`, `GPreallocatedMPMCQueueT`, the `GMPMCQueueT` facade + the `MPMCQueue` concept, the
thread pool/group; **being consolidated into a distinct concurrency sub-module of `common/`** — mirroring
geneva's `par`/`ind`/`oa` — so they stay able to use `common`'s facilities yet are clearly separated and
impossible to overlook). No part of Geneva rolls its own. If a primitive does not meet a requirement,
**improve it or add a variant there** — never a local `deque`+`mutex`, bespoke ring, hand-rolled thread pool,
or ad-hoc thread-safe map at the use site.

*Why:* one place to get memory ordering / ThreadSanitizer correctness right, one set of semantics, isolated
tests — and a distinct, named dependency makes Invariant 1 unforgettable. *Known cleanups owed to this rule:*
the consumer's roll-your-own `late_returns_` buffer → a library queue, its `{uuid,iteration}`→clone retention
map → a (new) library thread-safe keyed store, and Hap's lock-free `GRotatingPool` → a lock-free SPSC ring
lifted into the library (the existing lock-based queues would regress the RNG hot path).

## 3. Build out-of-source only

Never configure or build inside the source tree. Use an external build directory (e.g. `~/build`) via
`scripts/prepareBuild.sh`. In-source builds pollute the repository and are rejected by `prepareBuild.sh`.

## 4. Serialization is complete and single-sourced

Every class that adds data members implements `serialize()` and `load_()` / `save_()` (required for
checkpointing and network transport). Keep the compared/serialized member list **single-sourced** (the
`localMembers_` pattern) so that `serialize()`, `load_()`, and `compare_()` can never drift apart — a member
forgotten in one list is silent data loss.

## 5. One consumer per process

A process uses exactly one consumer, held in `GConsumerRegistry`. Optimization algorithms are
transport-agnostic: they submit to that single process consumer and **never** build, select, or inject a
consumer / broker / executor. The transport is chosen externally (via `Go2` or the command line).

## 6. The genome is pure data; mutation strategy lives on the optimization algorithm

The genome (`GFlatGenome`) carries the problem's parameter values and structure — **not** adaptors or
optimization-algorithm scratch. Adaption is configured on the OA-owned `GAdaptionConfig`; per-individual OA
scratch (e.g. self-adaptive σ) lives on OA-owned storage, never on the genome (the "config-strip" model).

## 7. When a test fails during a refactor or major change, triage it before touching code

A failing test during a refactoring or large-scale change is **not** automatically a signal to change the
production code. First decide which kind of failure it is:

- **Genuine failure** — the test encodes behaviour that must still hold, and the change broke it. **Fix the
  code.**
- **Expected consequence** — the change *legitimately and intentionally* altered the behaviour the test pinned
  (a new, valid contract). **Amend the test** to reflect the new reality.

Adapt the code to the test **only** for genuine errors. Never silence a genuine failure by loosening a test,
and never bend correct new code back to an obsolete expectation. State which case applies, and why, when you
make the change. Rule of thumb: in a **behaviour-preserving** refactor (e.g. the `common` concurrency
extraction) almost every red is genuine — a characterization test going red means the refactor changed
something it must not have. In a **behaviour-changing** redesign (e.g. the genome SoC work) a red may simply be
the test catching up to a deliberate new contract, and amending it is correct.

## 8. Full builds go through `prepareBuild.sh` + a `genevaConfig.gcfg`

For a full (clean) build of Geneva, use the project's build driver rather than a hand-rolled `cmake`
invocation — e.g.

```bash
cd $HOME/build && $HOME/ClionProjects/geneva/scripts/prepareBuild.sh --clean -y --build genevaConfig.gcfg
```

(use `$HOME`, never a hard-coded home path). The `genevaConfig.gcfg` in the build directory centralises the
build settings — compiler choice (clang vs g++), CUDA on/off, build type, MPI/GPU consumers, Boost location,
and more — so that one file, not scattered command-line flags, determines how Geneva is built. This keeps full
builds reproducible and consistent with Invariant 3 (out-of-source). Fast iterative rebuilds of a single target
may still use `cmake --build <dir> --target <t>`, but a full verifying build uses `prepareBuild.sh`.

---

*Add new invariants below as the maintainer establishes them. Keep each rule short, mandatory, and
general — design decisions for a specific feature belong in that feature's design notes, not here.*
