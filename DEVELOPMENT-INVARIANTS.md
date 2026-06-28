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
stores, completion latches, content-addressed/aging stores, and any lock-free structures** — come from
Geneva's shared concurrency facilities, consolidated in the distinct sub-module
`common/include/common/concurrency/` (namespace `Gem::Common::Concurrency`) — mirroring geneva's
`par`/`ind`/`oa`, so they stay able to use `common`'s facilities yet are clearly separated and impossible to
overlook. These include `GBlockingMPMCQueueT`, `GPreallocatedMPMCQueueT`, the `GMPMCQueueT` facade + the
`MPMCQueue` concept, the thread pool/group, `GContentAddressedStoreT`, `GThreadSafeKeyedStoreT`,
`GThreadSafeSetT`, `GCompletionLatchT`, `GSPSCStagingRingT` and `GAgingStoreT`. No part of Geneva rolls its
own. If a primitive does not meet a requirement, **improve it or add a variant there** — never a local
`deque`+`mutex`, bespoke ring, hand-rolled thread pool, or ad-hoc thread-safe map at the use site.

*Why:* one place to get memory ordering / ThreadSanitizer correctness right, one set of semantics, isolated
tests — and a distinct, named dependency makes Invariant 1 unforgettable. The original cleanups owed to this
rule are discharged (the consumer's `late_returns_` buffer + retained-original map → `GAgingStoreT`, the
in-flight borrow set → `GThreadSafeSetT`, Hap's `GRotatingPool` → `GSPSCStagingRingT`, the global option
stores → `GThreadSafeKeyedStoreT`). *Remaining (deferred, MPI-transport-local):* the MPI transport's raw
`receiverThread_`/`cleanUpThread_` → `GThreadGroup` and its `openSessions_` vector reaping → a library store.

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

## 9. A failing test is fixed before moving on — even if it looks unrelated

If any test errors during development, **fixing it has the highest priority**, even when the failure appears
unrelated to the change in hand. A red test is never stepped over, deferred, or written off as "pre-existing"
or "environmental noise" so that other work can proceed: it is diagnosed and made green first. (Invariant 7
still governs *how* — fix the code for a genuine failure, amend the test for a deliberate contract change — but
the failure is always addressed, never left red.) A green test suite is the precondition for continuing, and
for every commit.

## 10. Newly discovered compilation warnings must be investigated

A warning that appears during a build is a signal, not noise: **investigate every newly surfaced compiler
warning** rather than letting it scroll past. Determine its cause, and either fix it at the root or, if it is
genuinely benign and unavoidable, understand and record why. The same discipline as Invariant 9 applies to
warnings as to test failures — they are not stepped over because they look unrelated to the change in hand.

## 11. Usage and impact searches must cover the WHOLE tree

When determining whether something is used, or assessing the blast radius of a change, **search across all of
Geneva — every library, both headers (`.hpp`/`.h`/`.cuh`) and sources (`.cpp`/`.cu`), and including examples,
tests, benchmarks, and demos.** Do not scope the search to a single directory, to `src/` only, or to
`.cpp` only: a symbol declared and consumed entirely in a header (e.g. an inline OA helper) is invisible to a
`src/*.cpp` grep, and a "no callers" conclusion drawn from a partial search is worse than none — it licenses a
wrong deletion or migration. Confirm "unused" only after a tree-wide search; when in doubt, widen the net.

## 12. Substantial changes keep the Doxygen documentation in sync

When making a substantial change — altered signatures, moved or renamed members, new/removed classes, changed
ownership or lifecycle, a different invariant — **cross-check the Doxygen documentation (class/file/function
comments, `@param`/`@return`, `@par` notes, cross-references) and bring it up to date in the same change.**
Stale API documentation silently misleads; the docs are part of the contract, not an afterthought, and drift is
fixed where it is introduced rather than left for a later sweep.

## 13. Tests and demos adapt to a worthwhile change — they never veto it

A worthwhile architecture or code change is **not** abandoned because it would make existing tests, demos, or
examples fail, nor because the **only** remaining consumer of a feature or data member is a test, demo, or
example. In that case it is the test/demo/example that is updated (or the feature retired), **not** the change
that is dropped. A sole test/demo consumer does **not** make a feature "used" for the purpose of justifying its
retention. This composes with two neighbours: Invariant 11 (search the whole tree — *including* tests and demos —
to find a feature's real consumers) and Invariant 7 (how to triage the resulting red test: amend it when the
change is a deliberate new contract, fix the code only for a genuine regression). Tests guard behaviour that must
hold; they are never an excuse to forgo an improvement to that behaviour.

---

*Add new invariants below as the maintainer establishes them. Keep each rule short, mandatory, and
general — design decisions for a specific feature belong in that feature's design notes, not here.*
