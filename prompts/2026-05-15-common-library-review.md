# Geneva — Code Review of the Common Library

**Date**: 2026-05-15
**Branch**: `core-component-modernization`
**Scope**: `include/common/` and `src/common/` (Geneva's foundation library — utilities, logging, threading, parsing, plotting, serialization helpers). All other libraries (`hap`, `courtier`, `geneva`, `geneva-individuals`) are out of scope.
**Method**: Four parallel review passes (containers/interfaces · logger/errors · threading/concurrency · parser/helpers/math/plotter), then key findings spot-verified by direct file reads. **No code was changed.**

**Severity scale** (consistent with the 2026-05-15 validation review):
- **HIGH** — real bug; wrong behaviour, UB, leak, or race that can affect users.
- **MEDIUM** — bug only under uncommon inputs or in specific maintenance/runtime scenarios.
- **LOW** — clear code-quality improvement, no correctness impact.
- **TRIVIAL** — cosmetic / micro-modernisation.

---

## Executive summary

| Bucket | Count | Notes |
|---|---|---|
| HIGH-severity bugs | **4** | All in shared singletons / mutators: GLogger, GSingletonT, GGlobalOptionsT iterator state. |
| MEDIUM-severity bugs | ~12 | Mostly thread-safety, error handling, numeric edge cases. |
| LOW / TRIVIAL bugs | ~12 | Misleading defaults, comment mismatches, missing `[[nodiscard]]`. |
| C++20 modernisation wins | ~25 | Mostly clean-ups; a few worth doing *because they fix a HIGH bug* (e.g. `std::atomic<std::shared_ptr<T>>` in GSingletonT, `std::jthread` in GThreadGroup). |

The most attention-worthy areas are:
1. **GSingletonT** — broken double-checked locking on a non-atomic `std::shared_ptr` is observable UB. Modernising to `std::atomic<std::shared_ptr<T>>` (or a Meyers singleton) both fixes the race *and* reduces line count.
2. **GLogger** — public log-target mutators are unsynchronised while readers take a mutex; reconfiguring log targets at runtime is racy.
3. **GExpectationChecksT::identity\<T\>** — defaulted assignment operators are misleading on a class with reference and `const` members, and the structure stores raw references that outlive their right to do so if held.
4. **GGlobalOptionsT** — stateful iterator (`pos_`) is invalidated by `remove()` and not safe across `set/setOnce` calls; the API itself is 90s-vintage.
5. **GPlotDesigner::gLineStyle** — `shortdashdot` and `longdashdot` both equal `4` — round-tripping loses information.

The Common library is otherwise in good shape: container/interface code is solid (the recent `GContainerT` insert-fix is clean), helpers are mostly straightforward, and the formula/parser code, while large, has no obvious functional bugs aside from the items below.

---

## HIGH-severity findings

### H1. Data race on `GLogger`'s log-target list — `include/common/GLogger.hpp:268, 285, 302, 310`

`setDefaultLogTarget`, `addLogTarget`, `hasLogTargets`, `resetLogTargets` mutate or read `log_cnt_` / `default_logger_` **without acquiring `logger_mutex_`**, while `log()` / `logWithSource()` / `throwException()` / `terminateApplication()` (all at the same scope) iterate `log_cnt_` and use `default_logger_` **under** the lock. Concurrent calls to `addLogTarget()` and `log()` from different threads are a textbook `std::vector` data race; the singleton's nature ensures multiple users hit this.

**Impact if not fixed**: UB anytime a Geneva program reconfigures logging at runtime (e.g. attaching a file logger after worker threads have started). Today most callers configure the logger up front, which masks the bug, but the API offers reconfiguration and the docstring on `log()` advertises thread-safety.

**Fix**: take `std::scoped_lock(logger_mutex_)` at the top of each of the four public mutators/observers. While there: drop the `unique_lock<std::mutex>` in the readers in favour of `std::scoped_lock` (no need for the extra state bit `unique_lock` carries).

### H2. Broken double-checked locking in `GSingletonT::Instance` — `include/common/GSingletonT.hpp:104-128`

```cpp
static std::shared_ptr<T> p;
static std::mutex creation_mutex;
switch(mode) {
case 0:
    if(not p) {                                  // ← read of non-atomic shared_ptr
        std::unique_lock<std::mutex> lk(creation_mutex);
        if(not p) p = Gem::Common::TFactory_GSingletonT<T>();
    }
    return p;
case 1:
    p.reset();                                   // ← write under NO lock
    break;
}
```

The first `if(not p)` reads `p` without any synchronisation, while `case 1` writes `p` without any synchronisation — concurrent `Instance(0)` / `Instance(1)` is a data race on `p`'s control-block pointer, which is UB. The classic broken DCLP. Compounded by the API quirk that `mode == 1` (`reset`) and `mode == 0` (`get`) share a single function with an integer "mode".

**Impact if not fixed**: UB on any concurrent use that mixes get/reset. Even pure-getter races are technically UB (concurrent reads + writes of a non-atomic object), though current optimisers often produce "right-looking" code.

**Fix**: simplest — use a Meyers singleton (`static auto p = TFactory_GSingletonT<T>(); return p;`); guaranteed thread-safe since C++11, no manual locking. If `reset` semantics must remain, expose `release()` separately and protect both with `std::atomic<std::shared_ptr<T>>` (C++20).

### H3. `identity<T>` defaults assignment operators on a class with references/`const` members — `include/common/GExpectationChecksT.hpp:178-202`

```cpp
identity(identity const&) = default;
identity(identity&&) = default;
identity& operator=(identity const&) = default;        // ← misleading
identity& operator=(identity&&) = default;             // ← misleading
…
const T &x;
const T &y;
const std::string x_name;
const std::string y_name;
const double limit;
```

Defaulted copy/move-assignment on a class with reference members or `const` members is **implicitly deleted** per `[class.copy.assign]/7`. So `= default` is misleading — it says "user-provided" when it isn't. A caller that tries `id1 = id2` will get a confusing template-y diagnostic.

Worse, `identity` stores raw references; the `IDENTITY(a, b)` macro and `getIdentity(...)` produce these objects, which only outlive `a`/`b` for the duration of the surrounding expression. Any pattern that captures the result into a longer-lived variable (e.g. `auto id = IDENTITY(local, local2); … some_function(id);` outside the locals' scope) is dangling-reference UB.

**Impact if not fixed**: misleading code that compiles, plus a quiet lifetime trap. Not currently exercised because all internal call sites use `IDENTITY()` as an rvalue, but the trap is one stray `auto` away.

**Fix**: either (a) explicitly `= delete` both assignment operators to surface the actual semantics, or (b) redesign `identity` to own the names by value and take `x`/`y` either by value or via `std::reference_wrapper`. The macro idiom can stay.

### H4. `GGlobalOptionsT` stateful iterator is invalidated by `remove()` — `include/common/GGlobalOptionsT.hpp:62, 99, 201, 227-276, 284`

Class exposes an internal iterator (`pos_`, `rewind`, `goToNextPosition`, `getCurrentItem`, `getNextItem`) that points into `kvp_` (`std::map`). `remove(key)` calls `kvp_.erase(it)`, which invalidates iterators to the erased element. If `pos_` happens to point at the erased element (or one between two erases), any subsequent `getCurrentItem()`/`getNextItem()` dereferences a dangling iterator. The class also uses `mutex_` to protect map mutations but **leaks the iterator state across calls**, so any interleaving of `rewind()` → `set(other_key, …)` → `getNextItem()` from different threads (or even the same thread but with intermediate `remove`) is unsafe.

Additionally, `T get(std::string const&)` uses `kvp_[key]` (line 99), which **silently inserts a default-constructed `T`** if the key is missing, contradicting read-only semantics.

**Impact if not fixed**: latent UB / silent map growth as soon as anyone uses the stateful-iterator API alongside `remove`/`set`. The traversal API is rarely used in current Geneva code, which is why this hasn't surfaced.

**Fix**: drop the stateful iterator entirely; return a snapshot vector (`getKeyVector()` / `getContentVector()` already exist) and let callers iterate that. Also replace `kvp_[key]` with `kvp_.find(key)` + explicit throw on miss.

---

## MEDIUM-severity findings

### M1. `GUnitTestFrameworkT.hpp:64-67` — exception sliced on re-throw

```cpp
catch (const geneva_exception &g) { throw g; }
```

`throw g;` copy-constructs from `g` and re-throws the *static* type `geneva_exception`. Any subclass is silently sliced. Use a bare `throw;` (or remove the catch entirely if the only purpose was propagation).

### M2. `GLogger::throwException` / `terminateApplication` — terminate-with-lock-held

`include/common/GLogger.hpp:382, 393`: both functions hold `logger_mutex_` while calling `std::terminate()` / `throw`. The unwinding throw is OK (`unique_lock` releases on destruction), but terminating with the mutex still locked leaves global logger state inconsistent for any terminate handler.

### M3. `GFileLogger::log` — `std::terminate()` on file-open failure

`src/common/GLogger.cpp:88-94, 117-123`: transient I/O failure (disk full, EACCES) brings down the entire optimisation run. Fall back to `std::cerr` or queue-for-retry; only terminate on truly unrecoverable conditions.

### M4. `GErrorStreamer::operator std::string() const` — re-entrant logging in unwinding paths

`include/common/GErrorStreamer.hpp:144-164`: implicit conversion to `std::string` does file I/O and calls back into `GLogger`. If invoked during stack unwinding (`throw geneva_exception(g_error_streamer(...) << …)`), a logger-side throw on top of an in-flight throw calls `std::terminate`. Make the conversion `noexcept(false)` *and* document it, or split out a side-effect-free `to_string()` member.

### M5. `narrow_cast` — silently bypassed for enum targets — `include/common/GCommonHelperFunctionsT.hpp:75-92`

`if constexpr (std::is_integral_v<To> && std::is_integral_v<From>)` and the floating-point branch are the only check paths. For `To = scoped enum` (e.g. the `gColor`/`gMarker`/`gLineStyle` round-trips in `GPlotDesigner.cpp:79-198`), `std::is_integral_v<scoped enum>` is `false` (it's `is_enum_v`). Both branches are skipped — `narrow_cast<gColor>(tmp)` becomes a plain unchecked `static_cast`. Add a branch using `std::underlying_type_t<To>` (or `std::to_underlying` after a check).

### M6. `narrow_cast` — float→int range check incorrect at extremes — `include/common/GCommonHelperFunctionsT.hpp:85-89`

`static_cast<From>(std::numeric_limits<To>::max())` for `To = int64_t, From = double` produces `2^63` (exact in double), but `int64_t::max == 2^63 - 1`. The `value > 9.22e18` test passes a value equal to `9.22e18`, and `static_cast<int64_t>(9.22e18)` is UB. Same on the negative side for `int64_t::min` ≈ `-2^63` (representable, but `≤ min` should also reject). Use the canonical form `value < From(min)` (strict on min) and `value >= From(max) + 1.0` or the `std::nextafter` idiom.

### M7. `gLineStyle::shortdashdot` and `longdashdot` share value `4` — `include/common/GPlotDesigner.hpp:136-137`

Both enumerators are explicitly `= 4`. Round-tripping through `operator<<`/`operator>>` collapses one into the other, and equality comparison treats them as the same. Almost certainly a typo (ROOT's TLine styles are 1..10 with distinct meanings).

### M8. `GFormulaParserT::replacePlaceHolders` — `std::regex` recompiled in hot loop — `include/common/GFormulaParserT.hpp:743-776`

Building `std::regex("\\{\\{" + key + "\\}\\}")` per variable per `evaluate()` call. `std::regex` compilation is expensive (allocates, builds an NFA). For a formula evaluated millions of times during optimisation, this is a significant hot-path cost. Worse: the pattern is just a literal `{{name}}` / `{{name[i]}}` — no regex needed at all (`std::string::find` works).

### M9. `GFormulaParserT::execute()` — fixed 4096-slot stack, no bounds check — `include/common/GFormulaParserT.hpp:465, 793-967`

`stack_ptr_` is incremented via `*stack_ptr_++` without a `stack_ptr_ < stack_.end()` check. Deeply nested parsed formulas (or corrupt byte-code) overflow and write past the std::vector storage. Add a checked-push helper or use a dynamically growing stack.

### M10. `GFactoryT::globalInit()` — not thread-safe — `include/common/GFactoryT.hpp:334-340`

`bool initialized_` is a plain `bool`, and `init_()` is called without any mutex. Concurrent `get()` on the same factory can run `init_()` twice or observe a half-initialised factory. Replace with `std::once_flag` + `std::call_once`.

### M11. `GFixedSizePriorityQueueT::isBetter` — asymmetric strictness across sort orders — `include/common/GFixedSizePriorityQueueT.hpp:547-549`

`<=` for LOWERISBETTER vs `>` for HIGHERISBETTER: equal values are considered "better" in one direction only. Affects tie-breaking and incumbent-replacement.

### M12. `runExternalCommand` — naïve shell-string concatenation — `src/common/GCommonHelperFunctions.cpp:214-256`

Arguments are joined into a single string passed to `system()` without quoting. Spaces, `;`, `&`, `|`, backticks, `$`, newlines in any argument alter or break the command. Geneva typically passes developer-controlled arguments, which keeps the risk low — but using `posix_spawn` / `execvp` with an `argv` vector eliminates the class of issue and was already in scope when this function was written.

### M13. `GCommonInterfaceT::~GCommonInterfaceT() = default;` — non-virtual destructor on a polymorphic base — `include/common/GCommonInterfaceT.hpp:475`

Class has virtual functions (`load_`, `compare_`, `name_`, `clone_`); destructor is non-virtual and `protected`. The `protected` keyword does prevent `delete pBase;` from outside, but the contract is implicit. Either mark `virtual` or assert it in a comment that no derived class delete-through-base is permitted.

---

## LOW-severity findings (correctness / robustness, no current bug)

- **`GContainerT.hpp:149-150, 184-185`** — `PodStorage<T, Container>` / `SharedPtrStorage<T, Container>` do not constrain `Container::value_type == T` (resp. `shared_ptr<T>`); `PodStorage<int, std::vector<double>>` would compile.
- **`GContainerT.hpp:308`** — copy ctor double-initialises `data_cnt_` (default-construct then `deepCopy` assigns over). Avoidable with a policy-aware member initialiser.
- **`GContainerT.hpp:1339-1351`** — `resize_empty` reinvents `std::vector::resize`; default-constructed `shared_ptr<T>` is already null.
- **`GSerializableFunctionObjectT.hpp:104-110, 137-141`** — `p_load` computed but unused; the function is invoked for its side-effect (throw on type mismatch). Drop the variable or `[[maybe_unused]]`.
- **`GBoundedBufferT.hpp:136-140`** — destructor catch logs `"In GRandomFactory::producer()"`. Copy-paste error; mis-attributes the source.
- **`GThreadGroup.cpp:71-77`** — `join_all()` holds `mutex_` while joining workers; a worker calling back into the group deadlocks.
- **`GThreadGroup.hpp:115`** — bare `new std::thread(f)` followed by `shared_ptr` wrap. Use `std::make_shared`.
- **`GGlobalOptionsT.hpp:99`** — `get(std::string const&)` uses `kvp_[key]`, silently inserting a default `T` on miss.
- **`GGlobalOptionsT.hpp:201-208`** — `getKeyVector` clears the caller's vector *before* taking the lock.
- **`src/common/GCommonEnums.cpp:118-125`** — `operator<<(std::ostream&, tribool)` has no `default` arm; corrupted deserialised value writes nothing to the stream.
- **`src/common/GFormulaParserT.cpp:55, 65`** — exception ctors declared `noexcept` while base ctor can allocate (message string). Throw → `std::terminate`.
- **`include/common/GCommonHelperFunctionsT.hpp:242-281`** — `g_ptr_conversion` comment says "upcasts only", the `requires`-clause enforces downcasts. Documentation mismatch.
- **`include/common/GCommonHelperFunctionsT.hpp:540`** — `copySmartPointerArrays` performs `reset` + `delete[]` + reallocate; if the new allocation throws after delete, the holder is left with a stale pointer. Replace the whole API with `std::vector<std::shared_ptr<T>>`.
- **`include/common/GErrorStreamer.hpp:49-50`** — `const bool DO_LOG = true; const bool NO_LOG = false;` at namespace scope. Make `inline constexpr` to avoid ODR risk.
- **`include/common/GExpectationChecksT.hpp:189-194`** — `operator identity<base_type>()` should be `explicit`.
- **`src/common/GCommonHelperFunctions.cpp:472-485`** — `currentTimeAsString` uses `localtime_r`/`localtime_s` `#ifdef`. C++20 `std::chrono::current_zone()` + `std::format("{:%c}", ...)` eliminates the platform fork.
- **`include/common/GThreadPool.cpp:54`** — `GConsoleLogger::log` writes `std::clog` without flush; on TERMINATION path buffered output is lost.
- **`include/common/GThreadGroup.hpp:113-117`** + **`GThreadPool.cpp` whole** — pool is hand-rolled on Boost.ASIO; a `std::jthread` + `std::move_only_function<void()>` task queue could replace it.
- **`include/common/GExpectationChecksT.hpp:558-902, 1103-1238`** — five near-identical container-compare templates differing only in `c_type`/`s_type`. Collapse via `std::ranges::input_range` constraint.
- **`include/common/GExpectationChecksT.hpp:323-540`** — four near-identical `compare(...)` overloads with identical switch skeletons. Extract a helper.
- **`include/common/GSerializeTupleT.hpp:72-161`** — six 1..6-element tuple-serialisation specialisations. Collapse with `std::index_sequence` + fold expression.
- **`include/common/GTupleIO.hpp:65-145`** — manual `tuple_output_seq<N>` recursion; replace with `std::apply` + fold.
- **`src/common/GCommonHelperFunctions.cpp:265-280`** — `serializationModeToString` returns `std::string` for fixed labels; `constexpr std::string_view` lookup is cleaner and allocation-free.
- **`include/common/GParserBuilder.hpp` (~16 sites)** — `std::shared_ptr<X>(new X(...))` should be `std::make_shared<X>(...)`.

---

## C++20 modernisation opportunities (TRIVIAL unless noted)

These are improvements that don't fix a bug but materially simplify or modernise the code:

- **GSingletonT.hpp** — Meyers singleton OR `std::atomic<std::shared_ptr<T>>` (also fixes H2; severity **HIGH** to do).
- **GLogger.hpp + GExceptions.hpp** — replace `LOCATIONSTRING` / `time_and_place` macros with `std::source_location` default arguments on `raiseException`, `g_error_streamer` ctor, and `GManipulator` ctor. Eliminates `GEXCEPTION`/`GTERMINATION`/`GWARNING`/`GSTDERR` macros; gives function names for free. **MEDIUM** modernisation, high readability win.
- **GThreadGroup → std::jthread** — auto-join + `stop_token`; eliminates `join_all()`/`clearThreads()`/`stop`-flag plumbing and the "destruct before join" hazard. **MEDIUM**.
- **GThreadPool → std::move_only_function + std::counting_semaphore** — drops the Boost.ASIO dependency from this header. Also `if constexpr (std::is_void_v<R>)` collapses the two near-identical `async_schedule` overloads (`GThreadPool.hpp:100-208 / 220-327`). **MEDIUM**.
- **GBoundedBufferT.hpp** — every push/pop has two `requires`-gated variants for `t_capacity == 0` vs `> 0`. Single function body with `if constexpr` halves the code.
- **GFactoryT::globalInit** — `std::once_flag` + `std::call_once` (also fixes M10).
- **GLogger thread-safety** — drop `std::unique_lock<std::mutex>` in favour of `std::scoped_lock` everywhere (purely TRIVIAL once the H1 fix is in).
- **GFormulaParserT.hpp:476-479** — `static_cast<fp_type>(2.71828...L)` etc. → `std::numbers::e_v<fp_type>`, `std::numbers::pi_v<fp_type>`.
- **GFormulaParserT.hpp:482-485, 749, 761** — iterator-pair `for`-loops → range-for with structured bindings.
- **GCommonHelperFunctionsT.hpp:99-116** — `generate_uuid_v4` ostringstream-pipeline → `std::format("{:08x}-{:04x}-…")`.
- **GCommonHelperFunctionsT.hpp:121-126** — `from_string` for arithmetic `T` → `std::from_chars` (locale-independent, faster).
- **GCommonHelperFunctionsT.hpp:451-547** — `copyArrays`/`copySmartPointerArrays`/`g_delete`/`g_array_delete` — entire raw-array-ownership API can be deleted once callers migrate to `std::vector`.
- **GCommonMathHelperFunctionsT.hpp:69-162** — `enforceRangeConstraint` / `checkRangeCompliance` — `std::clamp` + a single concept-constrained template.
- **GCommonMathHelperFunctionsT.hpp:380-535** — 2/3/4-D `getMinMax` overloads can be folded via `std::index_sequence`. ~150 lines.
- **GCommonMathHelperFunctionsT.hpp:599-606** — `PowSmallPosInt` → a `constexpr` C++20 function.
- **GContainerT, GCommonInterfaceT, GFormulaParserT, GFixedSizePriorityQueueT** — pervasive missing `[[nodiscard]]` on value-returning accessors (`size()`, `empty()`, `front()`, `back()`, `evaluate()`, `getFormula()`, `getKeyVector()`, `getMaxSize()`, etc.).
- **GCommonInterfaceT.hpp:103-106** — `virtual void log(std::string const&) const = 0;` → `std::string_view` to avoid forced copies.
- **GCommonEnums.hpp** — multiple `operator<<`/`operator>>` per enum; unify behind a single template, or add `std::formatter` specialisations.
- **GPlotDesigner.cpp:69-198** — six enum-streamer overloads → one templated `template <ScopedEnum E> std::ostream& operator<<(...)`.
- **GExpectationChecksT.hpp** — manual loop iterator pairs everywhere → `std::ranges::mismatch`, `std::ranges::equal`, `std::span`.
- **GParserBuilder.hpp** (~16 sites) — `make_shared` instead of raw `new`.

---

## Items NOT to action

- The recent `insert_clone`/`insert_noclone` refactor in `GContainerT.hpp` (range-insert + `count == 0` guard) was spot-checked and is clean; no further work required there.
- `std::filesystem::path` → `.string()` mismatch claim from the 2026-05-14 review remains incorrect on this branch (confirmed earlier).
- `boost::variant` / `boost::spirit::qi` in `GFormulaParserT.hpp` could in principle migrate to `std::variant` + a hand-written parser, but that is a multi-day project with no defect being fixed; treat as future modernisation, not a current action item.

---

## Recommended action order

| # | Action | Severity if not done | Effort |
|---|---|---|---|
| 1 | Replace `GSingletonT::Instance` DCLP with a Meyers singleton (or `std::atomic<std::shared_ptr<T>>` if `reset` must stay). | HIGH | small |
| 2 | Take `std::scoped_lock(logger_mutex_)` in `GLogger::setDefaultLogTarget`, `addLogTarget`, `hasLogTargets`, `resetLogTargets`. | HIGH | small |
| 3 | Either `= delete` `identity`'s assignment operators (least change) or rework it to own its members by value. | HIGH | small–medium |
| 4 | Drop the stateful iterator from `GGlobalOptionsT`; route callers through `getKeyVector` / `getContentVector`. Make `get()` non-mutating. | HIGH | medium |
| 5 | Fix `narrow_cast` for enum targets and for the float→int boundary; add unit tests for `int64_t::max` / enum round-trip. | MEDIUM | small |
| 6 | Fix `gLineStyle::longdashdot` value (most likely should be `5` or `6` per ROOT TLine convention; check with the author). | MEDIUM | trivial |
| 7 | Bare `throw;` in `GUnitTestFrameworkT.hpp` re-throw site. | MEDIUM | trivial |
| 8 | Add `std::once_flag` to `GFactoryT::globalInit()`. | MEDIUM | trivial |
| 9 | Bounds-check `GFormulaParserT::stack_ptr_` increment / cache pre-compiled patterns (or drop regex for literal-string replacement). | MEDIUM | small |
| 10 | Fall back to `std::cerr` instead of `std::terminate()` in `GFileLogger` open failures. | MEDIUM | trivial |
| 11 | Remove `noexcept` from `math_logic_error` / `division_by_0` ctors. | MEDIUM | trivial |
| 12 | Modernisations (LOW/TRIVIAL): `std::source_location`, `std::jthread`, `std::numbers`, `std::format`, `std::scoped_lock`, `[[nodiscard]]`, `make_shared` audit. | LOW | spread across a refactor PR |

The HIGH items are concentrated in **four files** (`GSingletonT.hpp`, `GLogger.hpp`, `GExpectationChecksT.hpp`, `GGlobalOptionsT.hpp`) and together are perhaps a half-day of focused work plus testing.

---

## Bottom line

The Common library is structurally healthy. Its weak spots are concentrated in the **thread-safety of singleton/global services** (logger, factory, global options, the singleton template itself) and in a handful of **API-design papercuts** that have been latent because real callers use the affected APIs in restricted ways. The modernisation backlog is sizeable but mostly stylistic — except where it overlaps with the HIGH bugs (atomic shared_ptr in GSingletonT, source_location in GLogger), where modernising is the cleanest way to fix the bug.
