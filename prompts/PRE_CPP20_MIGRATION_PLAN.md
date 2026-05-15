# Pre-C++20 Construct Migration Plan

## Purpose

Survey and migration path for pre-C++20 constructs remaining in Geneva.
**No implementation is done here.** This document is the survey result and the ordered
migration plan. Every item is assessed against CUDA 13.1 compatibility constraints.

---

## CUDA Contamination Zones

CUDA `.cu` files compiled by nvcc include Geneva headers. Any C++20 feature added to those
headers must compile cleanly under nvcc's device-compiler pass, which supports a strict subset
of C++20. The contamination zones are:

### Zone 1 — `src/hap/GCUDARng.cu` (narrow)
Directly includes:
- `hap/GCUDARng.hpp` → stdlib only (clean)
- `hap/GRandomFactory.hpp` → `boost/cast.hpp`, `boost/lexical_cast.hpp`, `boost/utility.hpp`,
  and transitively `common/GBoundedBufferT.hpp`, `common/GCommonHelperFunctionsT.hpp`,
  `common/GErrorStreamer.hpp`, `common/GExceptions.hpp`, `common/GSingletonT.hpp`,
  `common/GThreadGroup.hpp`, `hap/GRandomDefines.hpp`
- `common/GLogger.hpp` → `boost/predef/version_number.h` + stdlib

Any `hap/` and `common/` header is potentially in Zone 1.

### Zone 2 — Benchmark CUDA files (broad)
`benchmarks/geneva/GCUDAOptBenchmark/GCUDAOptBenchmark.cu` and
`GBenchmarkBatchEvaluator.cu` include:
- `courtier/GBrokerT.hpp`, `geneva/GParameterSet.hpp`, `GBenchmarkBatchEvaluator.cuh`,
  `geneva-individuals/GBenchmarkFunctions.hpp`

Zone 2 spans most of the library. However, device kernels in this zone are small and
self-contained; the heavy Geneva headers are compiled as host code by the host compiler.
Practical risk is lower than Zone 1 for most C++20 features.

### Zone 3 — Example CUDA files
`examples/geneva/15_GCUDAWorker/GImageIndividualEvaluator.cu` includes
`GImageIndividualEvaluator.hpp` (Geneva-external eval header, no deep Geneva includes).

### CUDA 13.1 Feature Restrictions (device-code path)

| Feature | Host code | Device code | Notes |
|---|---|---|---|
| `std::optional` | OK | NO | No `__host__ __device__` constructors |
| `std::variant` / `std::visit` | OK | NO | RTTI-based, not available |
| `std::any` / `std::any_cast` | OK | NO | RTTI-based |
| Concepts / `requires` | OK | NO | cudafe++ rejects constraints |
| Coroutines | OK | NO | Not supported |
| `std::format` | OK | NO | Needs libfmt runtime |
| `std::jthread` | OK | N/A | Host-only |
| Three-way `<=>` | OK | OK | Supported since CUDA 11.1 |
| `consteval` / `constinit` | OK | OK | Supported |
| Designated initializers | OK | OK | Supported since CUDA 11.1 |
| `std::span` | OK | Limited | Trivial uses OK; avoid in device kernels |
| `std::make_shared` | OK | N/A | Host-only |
| `std::string` / `std::string_view` | OK | NO | Dynamic allocation |

**Golden rule for Zone 1 (`hap/` and `common/` headers):**
Do not add `std::optional`, `std::variant`, `std::any`, concepts, or coroutines to any
header that is (or might be) transitively included by a `.cu` file. Check with a grep for
the header name across all `.cu` / `.cuh` files before using these features.

---

## Survey Results

### Item 1 — `boost::lexical_cast` → `std::to_string` / `std::stoi` / `std::from_chars`

**Count:** 122 usages across `include/` and `src/`  
**Zone 1 exposure:** YES — `hap/GRandomFactory.hpp` includes `<boost/lexical_cast.hpp>`  
**C++20 replacement:** `std::to_string`, `std::stoi`, `std::stod`, `std::stol`,
`std::from_chars` (C++17, fully supported in all zones)

Migration path:
- `boost::lexical_cast<std::string>(x)` → `std::to_string(x)` (for numeric types) or
  `std::string(x)` (for single chars)
- `boost::lexical_cast<int>(s)` → `std::stoi(s)` (with `std::invalid_argument` / `std::out_of_range`)
- `boost::lexical_cast<double>(s)` → `std::stod(s)`
- `boost::lexical_cast<T>(s)` for general T: use `std::istringstream` or `std::from_chars`
- Remove `<boost/lexical_cast.hpp>` from `GRandomFactory.hpp` once all usages removed

CUDA risk: NONE — `std::to_string` / `std::stoi` are host-only stdlib, fine in Zone 1.

### Item 2 — `boost::numeric_cast` → custom `narrow_cast<T>`

**Count:** 162 usages across `include/` and `src/`  
**Zone 1 exposure:** YES — `hap/GRandomFactory.hpp` includes `<boost/cast.hpp>`  
**C++20 replacement:** No std equivalent. Define a `Gem::narrow_cast<T>` in
`include/common/GCommonHelperFunctions.hpp`:

```cpp
template<typename To, typename From>
To narrow_cast(From v) {
    auto r = static_cast<To>(v);
    if (static_cast<From>(r) != v)
        throw std::runtime_error("narrow_cast: value changed during cast");
    return r;
}
```

CUDA risk: LOW — this is host-only code. The helper function itself must not be annotated
`__device__`. Keep it in a host-only header or guard with `#ifndef __CUDA_ARCH__`.

### Item 3 — `boost::optional` → `std::optional`

**Count:** 15 usages  
**Zone 1 exposure:** NO (not in `hap/` or `common/` headers)  
**C++20 replacement:** `std::optional<T>` (C++17, available in all host-code zones)

Migration path: mechanical `s/boost::optional/std::optional/g` + swap `<boost/optional.hpp>`
for `<optional>`. `boost::none` → `std::nullopt`. `val.get()` → `val.value()` or `*val`.

CUDA risk: NONE for Zone 1. **Do NOT use `std::optional` in device kernels or in Zone 1
headers if the optional value needs to be accessed from device code.**

### Item 4 — `boost::any` / `boost::any_cast` → `std::any`

**Count:** 30 usages  
**Zone 1 exposure:** NO  
**C++20 replacement:** `std::any` (C++17). `boost::any_cast<T>` → `std::any_cast<T>`.

Migration path: mechanical swap + replace header.

CUDA risk: NONE for Zone 1 (not in CUDA headers). Cannot be used in device code.

### Item 5 — `boost::variant` / `boost::apply_visitor` → `std::variant` / `std::visit`

**Count:** 18 usages (variant + apply_visitor + recursive_wrapper)  
**Zone 1 exposure:** NO  
**C++20 replacement:**
- `boost::variant<A,B,C>` → `std::variant<A,B,C>`
- `boost::apply_visitor(visitor, v)` → `std::visit(visitor, v)`
- `boost::get<T>(v)` → `std::get<T>(v)`
- `boost::recursive_wrapper<T>` → **No std equivalent.** Options:
  a. Replace the recursive type with a `std::unique_ptr` indirection.
  b. Replace the `variant` with a class hierarchy.

The `recursive_wrapper` cases require design decisions; audit each usage before migrating.

CUDA risk: NONE for Zone 1. Cannot be used in device code.

### Item 6 — `boost::tuple` → `std::tuple`

**Count:** 13 usages (tuple + get<>)  
**Zone 1 exposure:** NO — `boost::tuple` is not included by any CUDA-zone header  
**C++20 replacement:** `std::tuple<...>`. `boost::get<N>(t)` → `std::get<N>(t)`.

Migration path: mechanical.

CUDA risk: NONE.

### Item 7 — `boost::noncopyable` → explicit `= delete`

**Count:** 7 usages  
**Zone 1 exposure:** LOW — `hap/GRandomFactory.hpp` includes `<boost/utility.hpp>` which
pulls `boost::noncopyable`, but only `GRandomFactory` itself uses it.  
**C++20 replacement:** Explicit deleted members:

```cpp
// Replace: class Foo : public boost::noncopyable { ... };
// With:
class Foo {
public:
    Foo(const Foo&) = delete;
    Foo& operator=(const Foo&) = delete;
    // ... rest of class
};
```

Removing `<boost/utility.hpp>` from `GRandomFactory.hpp` eliminates one Boost header from
Zone 1.

CUDA risk: NONE — `= delete` is fully supported everywhere.

### Item 8 — `std::enable_if` / SFINAE → C++20 `requires`

**Count:** 123 usages  
**Zone 1 exposure:** MEDIUM — `common/GCommonHelperFunctionsT.hpp` is in Zone 1 and
contains `std::enable_if` patterns.

**CUDA restriction:** Concepts and `requires` are **NOT supported in CUDA device code**.
The `common/` headers that use `enable_if` are transitively included by `GCUDARng.cu`.

Migration path:
1. Headers **not** transitively included by `.cu` files: migrate freely to C++20 concepts.
2. Headers in Zone 1 (e.g., `GCommonHelperFunctionsT.hpp`): **keep `std::enable_if`** or
   guard concept-based overloads with `#ifndef __CUDA_ARCH__`. Concepts can be added for
   the host compiler using `if constexpr` / `static_assert` alternatives for device paths.

Preferred concept form where safe:
```cpp
// Before (SFINAE):
template<typename T,
    typename std::enable_if<std::is_base_of<Base, T>::value>::type* = nullptr>
void foo(T& t) { ... }

// After (C++20, not in CUDA headers):
template<typename T> requires std::derived_from<T, Base>
void foo(T& t) { ... }
```

CUDA risk: HIGH for Zone 1 headers. Do not migrate `enable_if` in Zone 1 to `requires`.

### Item 9 — `std::shared_ptr<T>(new T(...))` → `std::make_shared<T>(...)`

**Count:** 50 usages  
**Zone 1 exposure:** LOW — `common/GSingletonT.hpp` (Zone 1) has one such pattern.  
**C++20 replacement:** `std::make_shared<T>(args...)` — C++11 best practice.

Migration path: mechanical replacement. Exception: classes with private constructors that
declare `shared_ptr` factories as friends cannot use `make_shared` without the `PassKey`
idiom. Audit each case.

The `return new X(*this)` pattern in `clone_()` virtual methods is a separate concern:
these should return `std::unique_ptr<X>` or remain as raw pointer until the clone API is
redesigned.

CUDA risk: NONE — `make_shared` is host-only, fine everywhere.

### Item 10 — `boost::logic::tribool` → `std::optional<bool>` or custom enum

**Count:** 10+ usages (primarily in `common/GSerializationHelperFunctionsT.hpp`,
`GCommonHelperFunctionsT.hpp`, `GPtrVectorT.hpp`, `GPODVectorT.hpp`)  
**Zone 1 exposure:** MEDIUM — `GCommonHelperFunctionsT.hpp` is Zone 1.  
**C++20 replacement:** Options:
- `std::optional<bool>` (C++17): `true` = true, `false` = false, `std::nullopt` = indeterminate.
  Clean and idiomatic but changes the call interface.
- Custom `enum class tribool { false_v, indeterminate, true_v }` in `GGlobalDefines.hpp`:
  preserves the existing three-state semantics, fully CUDA-safe.

Given the serialization specializations (`BOOST_SERIALIZATION_SPLIT_FREE`) tied to the
Boost type, a custom enum approach avoids touching the serialization layer.

CUDA risk: NONE for a custom enum. `std::optional<bool>` must not be used in device code,
but since these usages are host-only, either choice is safe in practice.

### Item 11 — `boost::uuids::random_generator` → custom or `std::random_device`-based UUID

**Count:** 3 usages (2 active, 1 commented out)  
**Zone 1 exposure:** NO  
**C++20 replacement:** No `<uuid>` in C++23 yet (proposed for C++26). Options:
- Implement a minimal UUID-v4 generator using `std::random_device` + `std::mt19937_64`
  and format with `std::format`.
- Keep `boost::uuids` until C++26 `<uuid>` lands.

CUDA risk: NONE.

### Item 12 — `boost::xpressive` (regex) → `std::regex`

**Count:** ~6 usages in `include/common/GFormulaParserT.hpp`  
**Zone 1 exposure:** NO — formula parser is not in the CUDA include chain.  
**C++20 replacement:** `std::regex` + `std::regex_replace`. The usage is simple
pattern substitution (`{{key}}` → value), which translates directly.

CUDA risk: NONE.

### Item 13 — `boost::spirit::qi` → keep or replace with hand-written parser

**Count:** ~10 includes in `include/geneva/GParameterPropertyParser.hpp`  
**Zone 1 exposure:** NO  
**C++20 replacement:** No std equivalent for Spirit. Options:
- Keep Boost.Spirit (it is already a required Boost component).
- Replace with a hand-written recursive-descent parser (moderate effort; the grammar
  handled is well-defined and relatively small).

This is the lowest-priority item: Spirit works, is tested, and removing it gives no
CUDA benefit since the parser is never in the CUDA translation unit.

CUDA risk: NONE.

### Item 14 — `boost::accumulators` → manual running-max tracking

**Count:** ~4 usages in `include/courtier/GExecutorT.hpp`  
**Zone 1 exposure:** NO  
**C++20 replacement:** The sole statistic used is `max`. Replace with a plain member
variable updated via `std::max(current_max_, new_value)`. No external library needed.

CUDA risk: NONE.

### Item 15 — `boost::iterator_facade` → C++20 iterator concept

**Count:** 1 usage in `include/common/GPtrVectorT.hpp`  
**Zone 1 exposure:** LOW — `GPtrVectorT.hpp` may be pulled into Zone 1 transitively.  
**C++20 replacement:** Implement the iterator requirements directly:
- Define `value_type`, `difference_type`, `iterator_category` (or `iterator_concept`).
- Provide `operator++`, `operator*`, `operator->`, `operator==`.
C++20 iterator concepts (`std::input_iterator`, `std::forward_iterator`) can be used
as constraints on the containing class methods, but not on the iterator type definition
itself if it lands in Zone 1.

CUDA risk: LOW — the iterator type is host-only.

### Item 16 — `boost::assign::list_of` — already removed

**Count:** 0 (no usages found)  
No action needed.

### Item 17 — `typedef` declarations — already removed

**Count:** 0 top-level `typedef` declarations found in `include/` or `src/`  
(One comment referencing "typedef" exists but is a comment, not a declaration.)  
No action needed.

### Item 18 — `NULL` — already removed

**Count:** 0 usages found  
No action needed.

---

## Migration Order (Recommended)

Ordered by: safety (CUDA risk first), count (most impactful), and dependency between items.

| Priority | Item | Count | CUDA Risk | Effort |
|---|---|---|---|---|
| 1 | Item 7: `boost::noncopyable` → `= delete` | 7 | LOW | 1 h |
| 2 | Item 3: `boost::optional` → `std::optional` | 15 | NONE | 1 h |
| 3 | Item 6: `boost::tuple` → `std::tuple` | 13 | NONE | 1 h |
| 4 | Item 4: `boost::any` → `std::any` | 30 | NONE | 2 h |
| 5 | Item 9: `shared_ptr(new T)` → `make_shared` | 50 | NONE | 2 h |
| 6 | Item 12: `boost::xpressive` → `std::regex` | 6 | NONE | 1 h |
| 7 | Item 14: `boost::accumulators` → `std::max` field | 4 | NONE | 1 h |
| 8 | Item 10: `boost::logic::tribool` → custom enum | 10 | NONE | 3 h |
| 9 | Item 1: `boost::lexical_cast` → `std::to_string` / `std::stoi` | 122 | NONE | 4 h |
| 10 | Item 2: `boost::numeric_cast` → `Gem::narrow_cast` | 162 | LOW | 4 h |
| 11 | Item 15: `boost::iterator_facade` → C++20 iterator | 1 | LOW | 2 h |
| 12 | Item 5: `boost::variant` → `std::variant` (non-recursive cases) | ~15 | NONE | 3 h |
| 13 | Item 5: `boost::recursive_wrapper` cases | ~3 | NONE | design needed |
| 14 | Item 8: `std::enable_if` → concepts (non-Zone-1 headers only) | ~100 | HIGH (Zone 1) | 8 h |
| 15 | Item 11: `boost::uuids` → custom UUID | 3 | NONE | 2 h |
| 16 | Item 13: `boost::spirit::qi` → keep or hand-write | N/A | NONE | optional |

### After completing Priority 1–11

At that point `boost/cast.hpp`, `boost/lexical_cast.hpp`, and `boost/utility.hpp` can be
removed from `hap/GRandomFactory.hpp`, fully eliminating Boost from Zone 1. This is a
meaningful milestone: CUDA translation units will no longer include any Boost headers.

### Remaining required Boost components after full migration

Even after completing all items above, these Boost components remain in use and have no
adequate std replacement yet:

| Component | Why kept |
|---|---|
| Boost.Serialization | Core of Geneva's network transport; no std equivalent |
| Boost.Asio / Boost.Beast | Websocket and TCP consumers; no std equivalent |
| Boost.Program_options | Command-line parsing; `std::getopt` is inadequate |
| Boost.Property_tree | JSON/XML config reading; replaceable with a lighter library later |
| Boost.Math | Float neighbor functions (`float_prior`, `float_next`); no std equivalent |
| Boost.Spirit.Qi | Parameter property parser; keep until replaced with hand-written parser |
| Boost.UUID | Until C++26 `<uuid>` |

---

## CUDA 13.1 Compatibility Checklist

Before introducing any new C++20 feature in a header, answer these questions:

1. Is the header transitively included by any `.cu` or `.cuh` file?
   `grep -rn "YourHeader.hpp" src/hap/ examples/geneva/15_GCUDAWorker/ benchmarks/`
2. If yes, is the feature used only in host-code paths (never in `__device__` functions)?
3. Does the feature require RTTI? (`std::any`, `std::variant`, `dynamic_cast`) — **NO in device code**
4. Does the feature use `requires` or concept constraints? — **NO in device code or Zone 1 headers**
5. Does the feature throw exceptions? — **NO in device code** (exceptions are disabled in nvcc device pass)

If any answer is "yes" and the feature is restricted, either:
- Move the feature to a `.cpp` translation unit (not included by `.cu`), or
- Guard with `#ifndef __CUDA_ARCH__` (only evaluated by host compiler path), or
- Keep the pre-C++20 form in that specific header.
