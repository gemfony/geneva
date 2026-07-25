# Coding style — identifier naming

House conventions for identifiers in the Geneva library collection. These record the **existing majority**
convention so new code is written consistently and the one genuinely split axis (function/method names) has a
single ruling. This complements the architectural rules in [`DEVELOPMENT-INVARIANTS.md`](DEVELOPMENT-INVARIANTS.md)
(in particular Inv 23 — a complete public API is the contract) and the memory rule *no identifier renames of
config keys*.

## The conventions

| Identifier kind | Convention | Example |
|---|---|---|
| **Types** (class / struct / enum / concept) | `PascalCase`, `G`-prefixed | `GRandomFactory`, `GGenomeT`, `GMPMCQueueT` |
| **Member variables** | `snake_case` with a **trailing underscore** | `n_producer_threads_`, `seeding_mutex_` |
| **Local variables / parameters** | `snake_case` | `n_producer_threads`, `seed_seq` |
| **Constants / enum values / macros** | `ALL_CAPS` (SNAKE) | `DEFAULTARRAYSIZE`, `GEM_BINARY`, `ACTIVEONLY` |
| **Functions / methods** | **`camelCase`** (see ruling below) | `fitnessCalculation`, `getNewRandomContainer`, `setNProducerThreads` |
| **Namespaces** | `PascalCase` under `Gem::` | `Gem::Common::Concurrency` |

Four of these five axes are already uniform across the whole tree; they are recorded here so they stay that
way. The fifth — function/method names — was historically a mix (classic `geneva/` core is camelCase; the
newer `courtier/` transport and `common/concurrency/` layers picked up snake_case mirroring STL/Boost.Asio).
The ruling resolves that mix.

## Ruling: methods are camelCase, with a pragmatic carve-out

**New and modified methods use `camelCase`.** This is the existing majority (the entire public `geneva/` API is
camelCase) and is therefore the project standard.

**Carve-out — mirror the concept you implement.** A function that deliberately models a standard-library or
Boost concept keeps that concept's `snake_case` spelling, because the name *is* the contract the idiom expects:

- Container / range surface a caller (or a range-based `for`, or an algorithm) resolves by name:
  `begin` / `end` / `cbegin` / `size` / `empty` / `push_back` / `emplace_back` / `try_push` / `pop_wait`.
- Customisation points found by ADL or specialisation: `swap`, `hash_value`, `operator==`.
- Asio/Beast-style asynchronous composition: `async_start_run`, `async_read`, and the like.

The test is intent, not layer: a method named to satisfy an external concept keeps that concept's spelling;
every other method is camelCase. When in doubt, prefer camelCase.

## What is frozen — never renamed for style

Style convergence must never break a caller or a config. The following are **frozen** and are *not* renamed to
satisfy this convention:

- **Public API names** — any method reachable by an out-of-tree consumer (Inv 23: the public API is the
  contract; churning its names breaks downstream code for no functional gain). A public snake_case method that
  is *not* a concept mirror is grandfathered, not renamed.
- **Configuration-option keys** — every string key read from a JSON config or `program_options`
  (`add_options()` keys, JSON member names, checkpoint field names). Renaming a config key silently
  incapacitates existing configuration files (the option is no longer recognised). Config keys are a wire/format
  contract, not code style: they keep their spelling regardless of this convention. If a method that *backs* a
  config key is renamed, the **key string stays put** — adapt the binding, never the key.

## How convergence happens — opportunistic, never a mass sweep

The remaining internal/private snake_case methods (non-public, non-config, non-concept — e.g. private helpers
in the transport layer) converge to camelCase **opportunistically**: when a file is already being edited for
another reason, its private helpers may be brought into line in the same change. There is deliberately **no
standalone rename sweep** — a mass rename would churn the public API against Inv 23, risk a config-key slip, and
produce a large review with no behavioural value. The convention governs what is written going forward; the
existing tree converges as it is touched.
