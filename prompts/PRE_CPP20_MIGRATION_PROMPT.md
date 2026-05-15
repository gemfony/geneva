# Implementation Prompt: Pre-C++20 / Boost Migration

## What you are doing and why

You are working on the Geneva library (Grid-Enabled Evolutionary Algorithms), a C++20
optimization library. The repository root is `/home/rberlich/ClionProjects/geneva`.
The active branch is `catch2-migration`. The active build directory is `/home/rberlich/build`.

The goal is to remove unnecessary Boost dependencies by replacing Boost constructs with
their C++20 (or C++17) standard-library equivalents, in priority order. The plan and
survey results are in `PRE_CPP20_MIGRATION_PLAN.md`; this prompt is the executable
implementation specification.

**Work is organised into 16 priority-ordered items.** Items 1–11 form a milestone: when
they are done, `boost/cast.hpp`, `boost/lexical_cast.hpp`, and `boost/utility.hpp` can be
removed from `hap/GRandomFactory.hpp`, eliminating Boost entirely from the CUDA Zone 1
translation units. Items 12–16 follow independently.

**Implement one item at a time. Verify after each item. Commit after each item (or group
of closely related items) with a descriptive message. Do not batch multiple items into one
commit.**

---

## Critical constraints

### 1. CUDA Zone 1 — never introduce restricted features here

Zone 1 headers are transitively included by `src/hap/GCUDARng.cu` and must compile
cleanly through the nvcc device-compiler pass. The Zone 1 header set is:

```
hap/GCUDARng.hpp
hap/GRandomFactory.hpp        ← currently includes boost/cast.hpp, boost/lexical_cast.hpp,
                                  boost/utility.hpp
hap/GRandomDefines.hpp
common/GBoundedBufferT.hpp
common/GCommonHelperFunctionsT.hpp
common/GCommonHelperFunctions.hpp
common/GErrorStreamer.hpp
common/GExceptions.hpp
common/GSingletonT.hpp
common/GThreadGroup.hpp
common/GLogger.hpp
```

**Never add to any Zone 1 header:**
- `std::optional`, `std::variant`, `std::any` — no `__host__ __device__` constructors
- `requires` clauses or concept constraints — nvcc device-compiler rejects them
- Coroutines or `std::format`
- Any header that transitively pulls the above

Before adding a feature to *any* `hap/` or `common/` header, run:
```bash
grep -rn "YourHeader.hpp" src/hap/ examples/geneva/15_GCUDAWorker/ benchmarks/
```
If the file appears, it is in Zone 1 or Zone 2 — check the CUDA restriction table in
`PRE_CPP20_MIGRATION_PLAN.md` before proceeding.

### 2. Read every file before editing it

The Edit tool requires a prior Read of each file. Never skip this step.

### 3. Run verification grep after each item

Each item below includes a "Verify" command. Run it and confirm zero (or the expected
count of) remaining occurrences before committing.

### 4. Update `CHANGES` once per commit group

Add an entry for each committed item under the current version heading in `CHANGES`.

### 5. The four CMakeModules trees are kept in sync

If any CMake file is modified (unlikely for this migration, but possible for Items 2/10),
propagate the change to the three example copies under `examples/*/CMakeModules/`.

---

## Priority 1 — Item 7: `boost::noncopyable` → explicit `= delete`

**Count:** 7 usages. **Effort:** ~1 h. **CUDA risk:** NONE.

### Find all usages

```bash
grep -rn "boost::noncopyable\|boost/noncopyable\|boost/utility\.hpp" \
    include/ src/ --include="*.hpp" --include="*.cpp"
```

### Migration rule

Replace every class that inherits from `boost::noncopyable`:

```cpp
// Before:
#include <boost/utility.hpp>
class Foo : public boost::noncopyable { ... };

// After:
class Foo {
public:
    Foo(const Foo&) = delete;
    Foo& operator=(const Foo&) = delete;
    // rest of class unchanged
};
```

If move operations should also be suppressed (check original class intent), add:
```cpp
    Foo(Foo&&) = delete;
    Foo& operator=(Foo&&) = delete;
```

After replacing all usages in a file, remove the `#include <boost/utility.hpp>` line from
that file **only if no other `boost/utility.hpp` symbol is used in that file**. Check with:
```bash
grep -n "boost::" TheFile.hpp | grep -v "noncopyable"
```

Special case: `hap/GRandomFactory.hpp` includes `<boost/utility.hpp>`. After replacing
`boost::noncopyable` in `GRandomFactory`, the include can be removed from that header
**only after Items 1 and 2 are also complete** (they also remove Boost symbols from that
file). Do not remove it prematurely.

### Verify

```bash
grep -rn "boost::noncopyable" include/ src/
```
Expected: zero occurrences.

---

## Priority 2 — Item 3: `boost::optional` → `std::optional`

**Count:** 15 usages. **Effort:** ~1 h. **CUDA risk:** NONE (not in Zone 1 headers).

### Find all usages

```bash
grep -rn "boost::optional\|boost/optional\.hpp" \
    include/ src/ --include="*.hpp" --include="*.cpp"
```

### Migration rules

| Before | After |
|--------|-------|
| `#include <boost/optional.hpp>` | `#include <optional>` |
| `boost::optional<T>` | `std::optional<T>` |
| `boost::none` | `std::nullopt` |
| `val.get()` (boost form) | `val.value()` or `*val` |
| `val.get_ptr()` | `val ? &val.value() : nullptr` |
| `val.is_initialized()` | `val.has_value()` |

Do not introduce `std::optional` into Zone 1 headers.

### Verify

```bash
grep -rn "boost::optional\|boost/optional" include/ src/
```
Expected: zero occurrences.

---

## Priority 3 — Item 6: `boost::tuple` → `std::tuple`

**Count:** 13 usages. **Effort:** ~1 h. **CUDA risk:** NONE.

### Find all usages

```bash
grep -rn "boost::tuple\|boost::get\|boost/tuple" \
    include/ src/ --include="*.hpp" --include="*.cpp"
```

Be careful: `boost::get<N>(t)` for tuples must become `std::get<N>(t)`. However,
`boost::get<T>(v)` is also used for `boost::variant` (Item 5). Distinguish by context —
only convert the tuple `get` calls here; leave variant `get` calls for Item 5.

### Migration rules

| Before | After |
|--------|-------|
| `#include <boost/tuple/tuple.hpp>` | `#include <tuple>` |
| `boost::tuple<A, B, C>` | `std::tuple<A, B, C>` |
| `boost::make_tuple(a, b, c)` | `std::make_tuple(a, b, c)` |
| `boost::get<N>(t)` (tuple context) | `std::get<N>(t)` |
| `boost::tie(a, b)` | `std::tie(a, b)` |

### Verify

```bash
grep -rn "boost::tuple\|boost/tuple" include/ src/
```
Expected: zero occurrences.

---

## Priority 4 — Item 4: `boost::any` / `boost::any_cast` → `std::any`

**Count:** 30 usages. **Effort:** ~2 h. **CUDA risk:** NONE (not in Zone 1).

### Find all usages

```bash
grep -rn "boost::any\|boost/any\.hpp" \
    include/ src/ --include="*.hpp" --include="*.cpp"
```

### Migration rules

| Before | After |
|--------|-------|
| `#include <boost/any.hpp>` | `#include <any>` |
| `boost::any` | `std::any` |
| `boost::any_cast<T>(v)` | `std::any_cast<T>(v)` |
| `boost::bad_any_cast` | `std::bad_any_cast` |

`boost::any_cast` throws `boost::bad_any_cast` on failure; `std::any_cast` throws
`std::bad_any_cast`. Catch sites that catch `boost::bad_any_cast` must be updated.

### Verify

```bash
grep -rn "boost::any\|boost/any" include/ src/
```
Expected: zero occurrences.

---

## Priority 5 — Item 9: `std::shared_ptr<T>(new T(...))` → `std::make_shared<T>(...)`

**Count:** ~50 usages. **Effort:** ~2 h. **CUDA risk:** NONE.

### Find all usages

```bash
grep -rn "shared_ptr<.*>(new " include/ src/ --include="*.hpp" --include="*.cpp"
grep -rn "shared_ptr<.*>( *new " include/ src/ --include="*.hpp" --include="*.cpp"
```

Also catch:
```bash
grep -rn "std::shared_ptr<[A-Za-z_:]*>(new " include/ src/ --include="*.hpp" --include="*.cpp"
```

### Migration rules

Straightforward mechanical replacement:
```cpp
// Before:
std::shared_ptr<Foo>(new Foo(a, b))
std::shared_ptr<Foo>(new Foo())

// After:
std::make_shared<Foo>(a, b)
std::make_shared<Foo>()
```

**Exceptions — do NOT convert these patterns:**

1. Classes with **private constructors** that use a friend factory. `make_shared` cannot
   invoke private constructors. Leave as `shared_ptr<T>(new T(...))` or apply the PassKey
   idiom only if explicitly asked.

2. The `return new X(*this)` pattern inside `clone_()` virtual methods. These return raw
   pointers and are part of the clone API. Do not touch them.

3. Custom deleters: `std::shared_ptr<T>(ptr, deleter)`. `make_shared` does not accept a
   custom deleter. Leave these unchanged.

For each candidate, verify the constructor is accessible before converting.

### Verify

```bash
grep -rn "shared_ptr<[A-Za-z_: ]*>(new " include/ src/
```
Remaining occurrences should only be the exception cases documented above.

---

## Priority 6 — Item 12: `boost::xpressive` → `std::regex`

**Count:** ~6 usages, all in `include/common/GFormulaParserT.hpp`. **Effort:** ~1 h.
**CUDA risk:** NONE (not in Zone 1).

### Find all usages

```bash
grep -rn "boost::xpressive\|boost/xpressive" \
    include/ src/ --include="*.hpp" --include="*.cpp"
```

### Migration rules

The usage in `GFormulaParserT.hpp` is pattern substitution of `{{key}}` → value.

| Before | After |
|--------|-------|
| `#include <boost/xpressive/xpressive.hpp>` | `#include <regex>` |
| `boost::xpressive::sregex` | `std::regex` |
| `boost::xpressive::smatch` | `std::smatch` |
| `boost::xpressive::regex_replace(s, rx, fmt)` | `std::regex_replace(s, rx, fmt)` |
| `boost::xpressive::regex_search(s, m, rx)` | `std::regex_search(s, m, rx)` |

For the `{{key}}` pattern: `boost::xpressive::as_xpr("{{") >> *~boost::xpressive::as_xpr('}')`
translates to the `std::regex` literal `"\\{\\{[^}]*\\}\\}"` (or similar). Read the
actual regex pattern in the file before translating; do not guess.

### Verify

```bash
grep -rn "boost::xpressive\|boost/xpressive" include/ src/
```
Expected: zero occurrences.

---

## Priority 7 — Item 14: `boost::accumulators` → member variable with `std::max`

**Count:** ~4 usages in `include/courtier/GExecutorT.hpp`. **Effort:** ~1 h.
**CUDA risk:** NONE.

### Find all usages

```bash
grep -rn "boost::accumulators\|boost/accumulators" \
    include/ src/ --include="*.hpp" --include="*.cpp"
```

### Migration rules

The only statistic used is `max`. Replace the accumulator with a plain member variable:

```cpp
// Before:
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/max.hpp>

namespace ba = boost::accumulators;
ba::accumulator_set<double, ba::stats<ba::tag::max>> acc_;

// usage:
acc_(new_value);
double current_max = ba::max(acc_);

// After:
#include <algorithm>   // std::max (likely already included)

double max_seen_ = std::numeric_limits<double>::lowest();  // or 0.0 depending on context

// usage:
max_seen_ = std::max(max_seen_, new_value);
double current_max = max_seen_;
```

Read `GExecutorT.hpp` carefully to understand the reset semantics of the accumulator
(e.g., is it reset per optimization cycle?) and replicate that reset logic on `max_seen_`.

### Verify

```bash
grep -rn "boost::accumulators\|boost/accumulators" include/ src/
```
Expected: zero occurrences.

---

## Priority 8 — Item 10: `boost::logic::tribool` → custom `Gem::tribool` enum

**Count:** 10+ usages. **Effort:** ~3 h. **CUDA risk:** NONE with a custom enum.

### Find all usages

```bash
grep -rn "boost::logic::tribool\|boost::logic\|boost/logic\|tribool\|indeterminate" \
    include/ src/ --include="*.hpp" --include="*.cpp"
```

Note also `BOOST_SERIALIZATION_SPLIT_FREE` specializations tied to `boost::logic::tribool`
in the serialization helpers.

### Migration rules

**Step 1:** Define the replacement enum in `include/common/GGlobalDefines.hpp`:

```cpp
namespace Gem::Common {
    enum class tribool : std::int8_t {
        tribool_false        = 0,
        tribool_indeterminate = 1,
        tribool_true         = 2
    };
} // namespace Gem::Common
```

**Step 2:** Replace all usages:

| Before | After |
|--------|-------|
| `#include <boost/logic/tribool.hpp>` | *(remove — type is now in GGlobalDefines.hpp)* |
| `boost::logic::tribool` | `Gem::Common::tribool` |
| `boost::logic::indeterminate` | `Gem::Common::tribool::tribool_indeterminate` |
| `t == true` | `t == Gem::Common::tribool::tribool_true` |
| `t == false` | `t == Gem::Common::tribool::tribool_false` |
| `boost::logic::indeterminate(t)` | `t == Gem::Common::tribool::tribool_indeterminate` |

**Step 3:** Replace the `BOOST_SERIALIZATION_SPLIT_FREE` specialization for the Boost type
with a `serialize` member or free function for `Gem::Common::tribool`. The enum serializes
as its underlying `std::int8_t` value — use `Gem::Common::tribool` directly in the
`serialize` overload rather than `boost::logic::tribool`.

### Verify

```bash
grep -rn "boost::logic\|boost/logic\|boost::logic::tribool" include/ src/
```
Expected: zero occurrences. Confirm the serialization specialization compiles.

---

## Priority 9 — Item 1: `boost::lexical_cast` → `std::to_string` / `std::stoi` / `std::stod`

**Count:** 122 usages. **Effort:** ~4 h. **CUDA risk:** NONE (stdlib replacements are
host-only and Zone 1-safe once the include is removed from `GRandomFactory.hpp`).

### Find all usages

```bash
grep -rn "boost::lexical_cast\|boost/lexical_cast" \
    include/ src/ --include="*.hpp" --include="*.cpp"
```

### Migration rules

Work file-by-file. For each occurrence decide which replacement applies:

| Pattern | Replacement |
|---------|-------------|
| `boost::lexical_cast<std::string>(x)` where `x` is numeric | `std::to_string(x)` |
| `boost::lexical_cast<std::string>(c)` where `c` is `char` | `std::string(1, c)` |
| `boost::lexical_cast<int>(s)` | `std::stoi(s)` |
| `boost::lexical_cast<long>(s)` | `std::stol(s)` |
| `boost::lexical_cast<long long>(s)` | `std::stoll(s)` |
| `boost::lexical_cast<unsigned long>(s)` | `std::stoul(s)` |
| `boost::lexical_cast<float>(s)` | `std::stof(s)` |
| `boost::lexical_cast<double>(s)` | `std::stod(s)` |
| `boost::lexical_cast<long double>(s)` | `std::stold(s)` |
| `boost::lexical_cast<bool>(s)` | `(s == "true" || s == "1")` or a helper |
| `boost::lexical_cast<T>(s)` for general T | `std::istringstream(s) >> result` |

Exception handling: `boost::lexical_cast` throws `boost::bad_lexical_cast` on failure.
The `std::sto*` functions throw `std::invalid_argument` and `std::out_of_range`. Update
any catch blocks that catch `boost::bad_lexical_cast`:

```cpp
// Before:
catch (const boost::bad_lexical_cast& e) { ... }

// After:
catch (const std::invalid_argument& e) { ... }
catch (const std::out_of_range& e) { ... }
```

After all usages are removed from a file, remove `#include <boost/lexical_cast.hpp>` from
that file. The include in `hap/GRandomFactory.hpp` must be removed last, after this item
is complete across all files, as part of the Zone 1 Boost removal milestone (after Item 2).

### Verify

```bash
grep -rn "boost::lexical_cast\|boost/lexical_cast" include/ src/
```
Expected: zero occurrences.

---

## Priority 10 — Item 2: `boost::numeric_cast` → `Gem::narrow_cast<T>`

**Count:** 162 usages. **Effort:** ~4 h. **CUDA risk:** LOW (keep helper host-only).

### Step 1: Define `Gem::narrow_cast` in `include/common/GCommonHelperFunctions.hpp`

Add the following in the `Gem::Common` namespace (or at file scope if the file uses `using
namespace Gem::Common`). Place it near the top of the helper functions section:

```cpp
/**
 * Checked narrowing cast. Throws std::runtime_error if the value changes during conversion.
 * Host-only — do not call from CUDA device code.
 */
template<typename To, typename From>
[[nodiscard]] To narrow_cast(From v) {
    auto r = static_cast<To>(v);
    if (static_cast<From>(r) != v) {
        throw std::runtime_error(
            std::string("narrow_cast: value changed during cast to ")
            + typeid(To).name());
    }
    return r;
}
```

Do **not** annotate this with `__host__ __device__`. It must not appear in device code.
This header (`GCommonHelperFunctions.hpp`) is in Zone 1; the function itself is safe there
because it is never called from device kernels.

### Step 2: Find all usages

```bash
grep -rn "boost::numeric_cast\|boost/cast\.hpp\|boost/numeric/conversion/cast" \
    include/ src/ --include="*.hpp" --include="*.cpp"
```

### Step 3: Replace each occurrence

```cpp
// Before:
#include <boost/cast.hpp>
boost::numeric_cast<int>(some_double)
boost::numeric_cast<uint32_t>(some_int64)

// After:
// (no new include needed — GCommonHelperFunctions.hpp is already transitively included)
Gem::Common::narrow_cast<int>(some_double)
Gem::Common::narrow_cast<uint32_t>(some_int64)
```

Exception handling: `boost::numeric_cast` throws `boost::numeric::bad_numeric_cast`.
Replace catch blocks:
```cpp
// Before:
catch (const boost::numeric::bad_numeric_cast& e) { ... }

// After:
catch (const std::runtime_error& e) { ... }
```

After all usages in a file are replaced, remove `#include <boost/cast.hpp>`. The include
in `hap/GRandomFactory.hpp` is removed last, as part of the Zone 1 milestone below.

### Verify

```bash
grep -rn "boost::numeric_cast\|boost/cast\.hpp" include/ src/
```
Expected: zero occurrences.

---

## Milestone after Priorities 1–10: Remove Boost from Zone 1

After Items 7, 1, and 2 are fully complete (i.e., `boost::noncopyable`, `boost::lexical_cast`,
and `boost::numeric_cast` are all gone), remove the three Boost includes from
`hap/GRandomFactory.hpp`:

```cpp
// Remove these three lines:
#include <boost/cast.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/utility.hpp>
```

After removing them, verify Zone 1 is clean:
```bash
grep -rn "boost/cast\|boost/lexical_cast\|boost/utility\|boost/noncopyable" \
    include/hap/ include/common/ src/hap/ src/common/
```
Expected: zero occurrences.

Then confirm the CUDA build still succeeds:
```bash
cd /home/rberlich/build && make hap -j$(nproc) 2>&1 | tail -20
```

Commit this milestone removal as a separate commit with message:
`hap: remove last Boost headers from CUDA Zone 1 (GRandomFactory.hpp)`

---

## Priority 11 — Item 15: `boost::iterator_facade` → C++20 iterator

**Count:** 1 usage in `include/common/GPtrVectorT.hpp`. **Effort:** ~2 h.
**CUDA risk:** LOW (iterator type is host-only; the `requires` constraint approach must
be used only if this header is confirmed not in Zone 1 — check first).

### Check Zone 1 exposure

```bash
grep -rn "GPtrVectorT\.hpp" src/hap/ benchmarks/ examples/geneva/15_GCUDAWorker/
```

If the header appears in CUDA translation units, do not add `requires` constraints to the
iterator or its containing class methods.

### Find the usage

```bash
grep -rn "iterator_facade\|boost/iterator" include/ src/ --include="*.hpp" --include="*.cpp"
```

### Migration rule

Read the existing iterator class in `GPtrVectorT.hpp`. Replace the `boost::iterator_facade`
base with a hand-written iterator that satisfies the C++20 `std::random_access_iterator`
concept (or whichever category the facade was configured for):

Required members for a random-access iterator:
```cpp
using value_type        = T;
using difference_type   = std::ptrdiff_t;
using pointer           = T*;
using reference         = T&;
using iterator_category = std::random_access_iterator_tag;

// Operators: ++, --, +, -, +=, -=, ==, !=, <, >, <=, >=, *, ->, []
```

Do not add `requires std::random_access_iterator<MyIter>` in the class definition if it
is in Zone 1.

### Verify

```bash
grep -rn "iterator_facade\|boost/iterator" include/ src/
```
Expected: zero occurrences.

---

## Priority 12 — Item 5a: `boost::variant` → `std::variant` (non-recursive cases)

**Count:** ~15 non-recursive usages. **Effort:** ~3 h. **CUDA risk:** NONE (not in Zone 1).

### Find all usages

```bash
grep -rn "boost::variant\|boost::apply_visitor\|boost::get\|boost/variant\|boost::static_visitor" \
    include/ src/ --include="*.hpp" --include="*.cpp"
```

Separate the `boost::recursive_wrapper` cases first (see Priority 13) — do not convert
those here.

### Migration rules

| Before | After |
|--------|-------|
| `#include <boost/variant.hpp>` | `#include <variant>` |
| `boost::variant<A, B, C>` | `std::variant<A, B, C>` |
| `boost::apply_visitor(vis, v)` | `std::visit(vis, v)` |
| `boost::get<T>(v)` (variant context) | `std::get<T>(v)` |
| `boost::static_visitor<R>` base class | remove base class; `std::visit` works with any callable |
| `boost::bad_get` | `std::bad_variant_access` |

`boost::apply_visitor` with a visitor struct that inherits `boost::static_visitor<R>`:
remove the base class and the result_type typedef; `std::visit` deduces the return type
automatically (or use `std::visit<R>(vis, v)` if an explicit return type is needed).

### Verify

```bash
grep -rn "boost::variant\|boost::apply_visitor\|boost::static_visitor" include/ src/ \
    | grep -v "recursive_wrapper"
```
Expected: zero occurrences (recursive_wrapper cases handled in Priority 13).

---

## Priority 13 — Item 5b: `boost::recursive_wrapper` cases

**Count:** ~3 usages. **Effort:** design-dependent. **CUDA risk:** NONE.

### Find all usages

```bash
grep -rn "recursive_wrapper\|boost::recursive_wrapper" include/ src/ --include="*.hpp" --include="*.cpp"
```

### Migration approach

`std::variant` does not support recursive types directly because the size of the variant
must be known at definition time. For each `boost::recursive_wrapper<T>` usage, choose
one of these approaches:

**Option A — `std::unique_ptr` indirection (preferred):**
Replace `boost::recursive_wrapper<Node>` with `std::unique_ptr<Node>`. The variant holds
an owning pointer rather than the node value directly. Access changes from `boost::get<Node>(v)`
to `*std::get<std::unique_ptr<Node>>(v)`.

**Option B — Class hierarchy:**
If the variant is effectively a discriminated union of a small, fixed set of types, replace
the variant + visitor pattern with a base class + virtual dispatch. This is more invasive
but eliminates the variant entirely.

Audit each recursive_wrapper usage individually and choose the option that requires fewer
changes to call sites. Document the choice in a comment above the definition.

### Verify

```bash
grep -rn "recursive_wrapper\|boost/variant" include/ src/
```
Expected: zero occurrences.

---

## Priority 14 — Item 8: `std::enable_if` / SFINAE → C++20 `requires` (non-Zone-1 headers only)

**Count:** ~100 usages total; ~80 safe to migrate, ~20 must stay as `enable_if`.
**Effort:** ~8 h. **CUDA risk:** HIGH in Zone 1 — never migrate Zone 1 headers.

### Step 1: Identify which headers are safe to migrate

```bash
# Find all headers containing enable_if:
grep -rln "std::enable_if\|enable_if_t" include/ src/ --include="*.hpp"

# For each found header, check if it is in Zone 1:
grep -rn "THAT_HEADER.hpp" src/hap/ benchmarks/ examples/geneva/15_GCUDAWorker/
```

Headers that appear in the grep output above are in Zone 1 or Zone 2 — **do not migrate
`enable_if` in those headers to `requires`**. Leave them as-is.

The safe headers are those in `include/geneva/`, `include/courtier/`, and any `common/`
header confirmed to not be in the CUDA include chain.

### Step 2: Migration rule for safe headers

```cpp
// Before (SFINAE return-type form):
template<typename T>
typename std::enable_if<std::is_arithmetic<T>::value, T>::type
clamp(T val, T lo, T hi);

// After (C++20 requires):
template<typename T> requires std::is_arithmetic_v<T>
T clamp(T val, T lo, T hi);

// Before (SFINAE default-argument form):
template<typename T,
    typename std::enable_if<std::is_base_of<Base, T>::value>::type* = nullptr>
void register(T& obj);

// After:
template<typename T> requires std::derived_from<T, Base>
void register(T& obj);
```

Useful C++20 concept equivalents:

| `enable_if` condition | C++20 concept |
|----------------------|---------------|
| `std::is_arithmetic<T>` | `std::is_arithmetic_v<T>` (use inline, no named concept) |
| `std::is_base_of<B, T>` | `std::derived_from<T, B>` |
| `std::is_same<T, U>` | `std::same_as<T, U>` |
| `std::is_integral<T>` | `std::integral<T>` |
| `std::is_floating_point<T>` | `std::floating_point<T>` |
| `std::is_convertible<F, T>` | `std::convertible_to<F, T>` |
| `std::is_default_constructible<T>` | `std::default_initializable<T>` |

### Verify

After migrating each file:
```bash
grep -n "std::enable_if" THAT_FILE.hpp
```
The remaining `enable_if` occurrences should only be in Zone 1 headers. Do a final audit:
```bash
grep -rln "std::enable_if" include/ src/
```
And confirm each remaining file is legitimately in Zone 1 or Zone 2.

---

## Priority 15 — Item 11: `boost::uuids` → custom UUID-v4 generator

**Count:** 3 usages (2 active, 1 commented out). **Effort:** ~2 h. **CUDA risk:** NONE.

### Find all usages

```bash
grep -rn "boost::uuids\|boost/uuid\|random_generator\|uuid_" \
    include/ src/ --include="*.hpp" --include="*.cpp"
```

### Migration rule

Implement a minimal UUID-v4 generator in a new helper function in
`include/common/GCommonHelperFunctions.hpp`:

```cpp
#include <random>
#include <sstream>
#include <iomanip>

inline std::string generate_uuid_v4() {
    static thread_local std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<uint64_t> dist;
    uint64_t hi = dist(rng);
    uint64_t lo = dist(rng);
    // Set version 4 bits (bits 12-15 of time_hi = 0100)
    hi = (hi & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
    // Set variant bits (bits 6-7 of clock_seq_hi = 10)
    lo = (lo & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;
    std::ostringstream oss;
    oss << std::hex << std::setfill('0')
        << std::setw(8)  << ((hi >> 32) & 0xFFFFFFFF) << '-'
        << std::setw(4)  << ((hi >> 16) & 0xFFFF)     << '-'
        << std::setw(4)  << ( hi        & 0xFFFF)      << '-'
        << std::setw(4)  << ((lo >> 48) & 0xFFFF)      << '-'
        << std::setw(12) << ( lo        & 0xFFFFFFFFFFFFULL);
    return oss.str();
}
```

Replace `boost::uuids::random_generator gen; std::string id = boost::uuids::to_string(gen())`
with `std::string id = Gem::Common::generate_uuid_v4()`.

### Verify

```bash
grep -rn "boost::uuids\|boost/uuid" include/ src/
```
Expected: zero occurrences.

---

## Priority 16 — Item 13: `boost::spirit::qi` — defer or replace

**Count:** ~10 includes in `include/geneva/GParameterPropertyParser.hpp`.
**Effort:** high if replaced; zero if deferred. **CUDA risk:** NONE.

This item has the lowest priority because Spirit works correctly, is fully tested, and its
removal provides no CUDA benefit (the parser is never in a CUDA translation unit).

### Option A — Defer (recommended for now)

Leave `boost::spirit::qi` in place. Record the deferral in `PRE_CPP20_MIGRATION_PLAN.md`
under Item 13 with a note: "Deferred — Spirit is not a blocker; revisit when C++ standard
library gains a constexpr parser framework, or when a lightweight hand-written parser is
designed."

No code changes for this option.

### Option B — Replace with hand-written recursive-descent parser

If chosen: read `GParameterPropertyParser.hpp` fully, understand the grammar (it parses
parameter property strings of the form `[name, lower, upper, step]` or similar), and
implement a `std::string`-based recursive-descent parser in its place. This is safe to do
at any time since the parser is host-only and has no CUDA exposure.

---

## Final verification checklist

After all priorities are complete, run this checklist:

```bash
# 1. No remaining boost:: calls that are targeted by this migration:
grep -rn "boost::lexical_cast\|boost::numeric_cast\|boost::noncopyable" include/ src/
grep -rn "boost::optional\|boost::any\|boost::tuple\|boost::variant" include/ src/
grep -rn "boost::accumulators\|boost::xpressive\|boost::logic::tribool" include/ src/
grep -rn "boost::uuids\|boost::iterator_facade" include/ src/
# Each of the above: expected zero occurrences (or only in comments)

# 2. Zone 1 is Boost-free:
grep -rn "#include.*boost" include/hap/ include/common/ src/hap/ src/common/
# Remaining includes should only be Boost.Serialization and boost/predef (GLogger.hpp)

# 3. Build succeeds with CUDA enabled:
cd /home/rberlich/build && make -j$(nproc) 2>&1 | grep -E "error:|warning:" | head -40

# 4. All tests pass:
cd /home/rberlich/build && ctest --output-on-failure

# 5. Confirm Boost component list in CommonGenevaBuild.cmake is accurate:
grep "GENEVA_BOOST_LIBS" CMakeModules/CommonGenevaBuild.cmake
# Should not list components that are no longer needed
```

Update `CHANGES` with a summary entry listing all Boost headers removed and all std
replacements introduced. Update the "Remaining required Boost components" table in
`PRE_CPP20_MIGRATION_PLAN.md` to mark completed items.
