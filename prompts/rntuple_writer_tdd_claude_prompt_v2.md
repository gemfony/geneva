# Prompt for Claude Code: ROOT-Independent C++20 RNTuple Writer — Geneva Integration

You are Claude Code, acting as a senior C++ systems engineer, binary-format implementer,
test architect, and Geneva library contributor.

## Mission

Build a **ROOT-independent C++20 library** that writes RNTuple files (format version
1.0.0.0, released with ROOT 6.34 in November 2024) readable by `ROOT::RNTupleReader`.
The library:

- depends on no ROOT sources, headers, or libraries at build time
- uses ROOT only as an external black-box validation tool in tests
- targets Linux, C++20, GCC 13+ or Clang 18+
- is **integrated into the Geneva source tree** and follows all Geneva coding conventions
  (clang-format, clang-tidy, class naming, CMake structure, exception types, logging)
- is developed incrementally with the realistic TDD philosophy described in §3

The final deliverable is a tested Geneva sub-library that writes ROOT-readable RNTuple
files, offering a simple single-value API, a batch API, a streaming adaptor API, and
optional automatic timestamp recording.

---

## 0. Knowledge Persistence Protocol

> **Claude: Read this section before doing anything else, and re-read it after every
> context compression event.**

### Purpose

During development, Claude discovers format facts, resolves spec ambiguities, makes
build-system decisions, and learns test-infrastructure quirks. These findings must
survive context compression. The **§20 Knowledge Log** at the end of this file is
Claude's persistent notepad — entries written there are "notes from Claude to itself"
that outlast any context window.

### When to write a §20 entry

**Immediately** — as soon as a finding is confirmed, before the next tool call.
Do not batch findings for later. Context compression can happen at any moment.
Write an entry for:

- Any format fact confirmed by reference-file inspection (byte layout, field sizes,
  encoding details that the spec leaves ambiguous)
- Any resolved specification ambiguity
- Any build-system or dependency decision discovered through trial
- Any ROOT validation quirk or CI infrastructure finding
- Any finding that, if forgotten, would require repeating non-trivial research

Do NOT write entries for things already documented in §9 or elsewhere in this file.
§9 is the authoritative format reference; §20 captures *new* discoveries made during
implementation.

### After every context compression event

**The first action after any compression must be:**

1. Re-read `prompts/rntuple_writer_tdd_claude_prompt_v2.md` starting from §20
   (the Knowledge Log) to recover all persisted findings
2. Briefly summarise which entries are now active context
3. Then continue implementation from where the work left off

Skipping step 1 means repeating research that has already been done.

### §20 entry format

```
### YYYY-MM-DD: <one-line summary>
**Type:** format-fact | spec-ambiguity | build-decision | test-finding | env-finding
**Finding:** <confirmed fact, specific enough to act on>
**Source:** <spec section / reference-file hash:offset / experiment / tool output>
**Impact:** <what changes in the implementation because of this>
```

---

## 1. Why this plan looks like it does

This project implements a **publicly specified binary format**. The specification (RNTuple
Binary Format 1.0.0.0) is well-written but has ambiguities and cross-references to other
ROOT format documents. Resolving ambiguities requires inspecting **files produced by ROOT**
— never ROOT source code. This is standard clean-room practice.

Pure top-down TDD breaks down when the test author must guess what the spec means in
places where it is silent. The plan therefore uses a hybrid approach:

- **Environment first** (Phase −2): verify that all required tools are present before any
  code is written
- **Spike first, then test** (Phase −1): a throwaway prototype proves feasibility and
  surfaces format details before the disciplined work begins
- **Bare-file before TFile** (Phase 6 vs 7): the bare-file container is far simpler;
  targeting it first removes the hardest risk from the critical path
- **Continuous external validation** (Phase 0 onwards): ROOT 6.36.12 is installed on
  the development machine at `/opt/root/`; validation scripts invoke it directly
- **Reference-file methodology**: every non-trivial format claim is cross-checked against
  a ROOT-generated reference file, with the hex dump committed to the repository
- **Explicit time-boxes and abort criteria**: high-risk phases have scheduled go/no-go
  reviews

---

## 2. Non-negotiable goals

The library must:

1. Write RNTuple data conforming to format specification 1.0.0.0
2. Produce files that `ROOT::RNTupleReader` (ROOT 6.34+) can open and read with all
   values matching what was written
3. Not include, copy, translate, or derive from ROOT source code
4. Not include ROOT headers in any public or private file
5. Not link against ROOT in the library target itself
6. Be developed test-first wherever the test can be written first
7. Follow all Geneva coding conventions:
   - Class names begin with uppercase `G` (e.g., `GChronicleWriter`, `GChronicleFile`)
   - All code is formatted with the Geneva `.clang-format` configuration
   - All code passes the Geneva `.clang-tidy` configuration
   - Exceptions use Geneva exception types from `common/GExceptions.hpp`
   - Logging uses `common/GLogger.hpp`
   - The CMake target is named `gemfony-chronicle` following the `gemfony-*` convention
8. Have a documented clean-room policy
9. Have a documented assumptions and ambiguities log
10. Have reproducible tests, golden files, and ROOT compatibility validation
11. Include usage examples alongside the tests

---

## 3. Realistic TDD philosophy

Use TDD where it works. Do not pretend it works where it does not.

**Pure TDD applies to (test before code):**
- Binary writer primitives (endianness, bit-packing, IEEE-754 layout)
- xxHash3-64 checksum integration (verifiable against canonical test vectors)
- Schema and column-buffer logic
- Page construction and metadata accounting
- Public API behaviour (RAII, error handling, idempotent close)
- All three API modes (single-value, batch, streaming adaptor)
- Timestamp recording correctness

**Format-discovery work uses "spec + reference file → test → code":**

For each non-trivial format element:

1. Read the relevant section of the public RNTuple format specification
2. If unambiguous, write a test encoding that requirement
3. If the spec has gaps, generate a reference `.root` file with ROOT in the validation
   environment, commit it under `tests/chronicle/fixtures/reference_files/`, hex-dump and
   annotate it, write a test asserting our writer produces semantically equivalent output
4. Implement the writer code
5. Run ROOT against our output to confirm

When a test is changed after discovering format reality, append to `docs/chronicle/assumptions.md`:

```markdown
## YYYY-MM-DD: Test refinement: <test name>

Previous assumption:
- ...

New evidence (reference-file hash and offset, if applicable):
- ...

Change:
- ...

Impact on other tests:
- ...
```

Tests must never be silently rewritten to match a broken implementation.

---

## 4. Effort estimate and abort criteria

| Phase | Description | Estimated effort | Abort criterion |
|---|---|---|---|
| −2 | Environment check | 1 day | Blocker found → fix before proceeding |
| −1 | Spike prototype | 1–2 weeks | Spike fails to produce ROOT-readable bare file → escalate |
| 0 | Skeleton + validation infra | 3–5 days | ROOT validation fails after fixing PATH → investigate before proceeding |
| 1–4 | Primitives, schema, buffers, pages | 1–2 weeks | None expected; pure TDD |
| 5 | RNTuple envelopes | 1–2 weeks | None expected |
| 6 | Bare-file embedding | 1 week | Not accepted by RNTupleReader after 2 weeks → reconsider |
| 7 | TFile embedding | 3–6 weeks | Anchor rejected after 4 weeks → fall back to bare-file |
| 8 | Robustness + edge cases | 1–2 weeks | None |
| 9 | Compression (optional) | 1 week | None |

**Total realistic envelope: 10–18 weeks** for the full deliverable. **Bare-file-only: 6–10 weeks.**

---

## 5. Clean-room policy

Create `docs/chronicle/clean-room-policy.md` before writing implementation code.

### Allowed sources

- The published RNTuple binary format specification:
  `https://github.com/root-project/root/blob/master/tree/ntuple/v7/doc/specifications.md`
  and the `BinaryFormatSpecification.md` on the v6-34-patches branch
- Public ROOT documentation (`https://root.cern/doc/`)
- CERN papers on RNTuple (e.g. arXiv:2204.04557)
- Public on-disk layout descriptions in academic publications
- The xxHash specification and reference test vectors
- Files generated by ROOT, used as black-box compatibility samples
- Hex-level analysis of ROOT-generated files
- Independent RNTuple implementations not derived from ROOT source (e.g. `uproot` writer —
  for format understanding only, not code reuse; after license review)

### Forbidden sources

- ROOT source code (`tree/ntuple/`, `tree/ntuple/v7/`, `io/io/`, etc.)
- ROOT private headers or class layouts
- Translations of ROOT C++ code into another form

If a ROOT source file is opened to confirm a spec ambiguity, the exact question must first
be written in `docs/chronicle/assumptions.md`; only the factual answer (e.g. "the field uses
little-endian uint32_t") may be transferred. No code, no algorithm, no class layout.

### Permitted validation

- Generate a file with this library, run ROOT externally as a separate process
- Generate reference files with ROOT in a separate environment
- Compare semantic behaviour, metadata content, and per-entry values
- Compare binary layouts to understand the public format

Byte-identical output is **not required**.

---

## 6. Geneva integration: repository structure

The library lives inside the Geneva source tree under a new `chronicle` component, following
the same `include/<lib>/` + `src/<lib>/` pattern used by `common`, `hap`, `courtier`,
`geneva`, and `geneva-individuals`.

```text
<geneva-root>/
├── include/
│   └── chronicle/
│       ├── GChronicleFile.hpp            # top-level file handle
│       ├── GChronicleWriter.hpp          # writer for one ntuple
│       ├── GChronicleField.hpp           # typed field handle
│       ├── GChronicleOptions.hpp         # configuration (page size, compression, embedding)
│       ├── GChronicleAdaptor.hpp         # base concept/interface for streaming adaptors
│       ├── GChronicleStream.hpp          # streaming interface template
│       └── GChronicleVersion.hpp         # library version
├── src/
│   └── chronicle/
│       ├── CMakeLists.txt
│       ├── binary/
│       │   ├── GEndianWriter.cpp
│       │   ├── GXXHash3.cpp            # wraps libxxhash or vendored
│       │   └── GByteBuffer.cpp
│       ├── format/
│       │   ├── GEnvelopeWriter.cpp
│       │   ├── GPageWriter.cpp
│       │   ├── GLocator.cpp
│       │   ├── GSchema.cpp
│       │   └── GMetadataWriter.cpp
│       ├── embedding/
│       │   ├── GBareFileWriter.cpp
│       │   ├── GTFileWriter.cpp
│       │   ├── GTKeyWriter.cpp
│       │   └── GAnchorWriter.cpp
│       └── writer/
│           ├── GChronicleFile.cpp
│           ├── GChronicleWriter.cpp
│           └── GColumnBuffer.cpp
├── tests/
│   └── chronicle/
│       ├── CMakeLists.txt
│       ├── unit/
│       ├── golden/
│       ├── property/
│       ├── integration/
│       ├── root_validation/
│       └── fixtures/
│           ├── reference_files/        # ROOT-generated committed binaries
│           └── reference_dumps/        # annotated hex dumps
├── examples/
│   └── chronicle/
│       ├── CMakeLists.txt
│       ├── GSimpleWrite.cpp            # minimal: write a few scalars
│       ├── GBatchWrite.cpp             # batch/container API
│       ├── GStreamWrite.cpp            # adaptor streaming API
│       ├── GTimeSeriesWrite.cpp        # timestamp recording
│       └── GComplexObjectWrite.cpp     # struct decomposition via adaptor
└── docs/
    └── chronicle/
        ├── clean-room-policy.md
        ├── design.md
        ├── assumptions.md
        ├── format-notes.md             # distilled spec knowledge; see §9
        ├── testing-strategy.md
        ├── compatibility.md
        └── spike-findings.md
```

The CMake target name follows Geneva's `gemfony-*` convention:

```cmake
ADD_LIBRARY( gemfony-chronicle SHARED ${CHRONICLE_SOURCES} )
```

The library links against `gemfony-common` (for `GLogger`, `GExceptions`). It does **not**
link against `gemfony-hap`, `gemfony-courtier`, `gemfony-geneva`, or any optimization
component. It is a standalone I/O utility library within the Geneva ecosystem.

### 6.1 Geneva integration constraints

- **Other Geneva libraries** (`hap`, `courtier`, `geneva`, `geneva-individuals`) must
  **not** be modified as part of this project. `chronicle` is a new standalone addition
  to the Geneva tree; it must not require changes to existing Geneva code.

- **`gemfony-common`** may be modified to add general utility that is genuinely useful
  beyond `chronicle` (e.g., a new endian helper, a new exception category). Every such
  addition must:
  - Not change or break any existing API, class, or behaviour in `common`
  - Not break any existing Geneva library, test, or example
  - Pass the full Geneva build and test suite before the `chronicle` code that relies
    on the addition is written (build-test gate)
  - Be committed as a separate commit on `chronicle-draft`, before and independent of
    the `chronicle` code that depends on it

- **Build-system changes** (any `CMakeLists.txt` outside `chronicle/`, any file under
  `CMakeModules/`) are subject to the same build-test gate.

- `gemfony-chronicle` must link against `gemfony-common` only. It must **not**
  transitively pull in `gemfony-hap`, `gemfony-courtier`, `gemfony-geneva`, or any
  optimization component.

---

## 7. Build system

Integrated into Geneva's existing CMake infrastructure (CMake 3.27+, C++20). New CMake
options in `genevaConfig.gcfg` and `prepareBuild.sh`:

```cmake
option(GENEVA_BUILD_CHRONICLE        "Build the RNTuple writer library"         OFF)
option(GCHR_ENABLE_ROOT_VALIDATION "Run external ROOT validation tests"        OFF)
option(GCHR_WITH_ZSTD              "Enable ZSTD compression"                   OFF)
option(GCHR_WITH_LZ4               "Enable LZ4 compression"                    OFF)
option(GCHR_WITH_ZLIB              "Enable ZLIB compression"                   OFF)
option(GCHR_USE_SYSTEM_XXHASH      "Link libxxhash instead of vendoring"        ON)
option(GCHR_ENABLE_SANITIZERS      "Enable ASan/UBSan in chronicle tests"         OFF)
```

When `GENEVA_BUILD_CHRONICLE=ON`, the `src/chronicle/CMakeLists.txt` is included. The library
target must not transitively pull in any ROOT dependency. The ROOT-validation tests are a
separate CTest target that `exec()`s the `root` binary at runtime.

xxHash3-64 is mandatory for the format. Default to system xxhash (`libxxhash-dev` on
Ubuntu); allow vendoring for portability.

clang-format and clang-tidy must pass. Run:
```bash
clang-format --dry-run --Werror $(find include/chronicle src/chronicle -name "*.hpp" -o -name "*.cpp")
clang-tidy $(find src/chronicle -name "*.cpp") -- -I include -std=c++20
```

### 7.1 Development branch

All development happens on branch **`chronicle-draft`**, which is cloned from
`catch2-migration`:

```bash
git checkout catch2-migration
git checkout -b chronicle-draft
```

The main integration branch of the Geneva repository is `develop`. When the bare-file
deliverable is complete and all acceptance criteria in §16 are met, open a PR from
`chronicle-draft` into `develop`.

Do not push `chronicle-draft` to `origin` until the Phase 0 skeleton is in place and
the CI matrix in §14 passes on at least the `ubuntu-24.04 gcc-13 Debug` configuration.

---

## 8. Public API

All public classes begin with `G` per Geneva convention. The API has three usage modes
that share the same underlying engine, plus optional timestamp recording.

### 8.1 Simple single-value mode

The canonical, minimal usage pattern:

```cpp
#include "chronicle/GChronicleFile.hpp"

int main() {
    Gem::Chronicle::GChronicleOptions opts;
    opts.setPageSize(64 * 1024);
    opts.setCompression(Gem::Chronicle::GCompression::None);
    opts.setEmbedding(Gem::Chronicle::GEmbedding::TFile);   // or BareFile

    Gem::Chronicle::GChronicleFile file("output.root", opts);
    auto writer = file.makeWriter("Events");

    auto px = writer.makeField<float>("px");
    auto py = writer.makeField<float>("py");
    auto id = writer.makeField<std::int32_t>("id");

    for (std::int32_t i = 0; i < 1000; ++i) {
        px = static_cast<float>(i);
        py = static_cast<float>(i * 2);
        id = i;
        writer.fill();
    }
    // RAII: writer.close() and file.close() called on destruction
}
```

### 8.2 Batch / container mode

Pass an entire container of values for one field at once. All containers for a given
`fillBatch()` call must have the same size; a `GChronicleException` is thrown otherwise.

```cpp
std::vector<float> px_vals = {1.0f, 2.0f, 3.0f};
std::vector<float> py_vals = {4.0f, 5.0f, 6.0f};
std::vector<std::int32_t> id_vals = {0, 1, 2};

writer.fillBatch("px", px_vals);
writer.fillBatch("py", py_vals);
writer.fillBatch("id", id_vals);
// The three calls above collectively commit three entries.
// Field registration is implicit on first fillBatch call.
```

Any contiguous container satisfying `std::ranges::contiguous_range<C>` with a compatible
`value_type` is accepted. This includes `std::vector<T>`, `std::array<T,N>`, `std::span<T>`,
and raw pointer + size pairs (via `std::span`).

### 8.3 Streaming adaptor mode

User-defined adaptors decompose complex objects (structs, class instances, tuples) into
the individual fields stored in the RNTuple. This enables a `<<`-style streaming interface
and decouples the object model from the storage model.

#### Defining an adaptor

```cpp
struct GMyEventAdaptor {
    // Called once to register the fields. writer is not yet frozen.
    void declareFields(Gem::Chronicle::GChronicleWriter& w) {
        w.declareField<float>("px");
        w.declareField<float>("py");
        w.declareField<std::int32_t>("id");
    }

    // Called once per object to decompose and fill all fields.
    void fill(Gem::Chronicle::GChronicleWriter& w, const MyEvent& e) {
        w.setField("px", e.px);
        w.setField("py", e.py);
        w.setField("id", e.id);
        w.fill();
    }
};
```

Adaptors are plain structs; no base class or virtual functions are required. The library
uses a concept to verify that a type satisfies the adaptor requirements:

```cpp
template<typename A, typename T>
concept GChronicleAdaptor =
    requires(A& a, Gem::Chronicle::GChronicleWriter& w, const T& obj) {
        a.declareFields(w);
        a.fill(w, obj);
    };
```

#### Using the stream interface

```cpp
Gem::Chronicle::GChronicleFile file("output.root");
auto writer = file.makeWriter("Events");
auto stream = writer.makeStream<MyEvent>(GMyEventAdaptor{});

MyEvent e{1.0f, 2.0f, 42};
stream << e;                           // writes one entry
stream << MyEvent{3.0f, 4.0f, 43};    // writes another

// Batch streaming from a container:
std::vector<MyEvent> events = { ... };
stream << events;                      // writes all entries in order
```

The stream object holds a reference to the writer; it must not outlive the writer.

#### Composing adaptors

Sub-objects can be decomposed by delegating to nested adaptors:

```cpp
struct GTrackAdaptor {
    void declareFields(Gem::Chronicle::GChronicleWriter& w) {
        w.declareField<float>("track_pt");
        w.declareField<float>("track_eta");
    }
    void fill(Gem::Chronicle::GChronicleWriter& w, const Track& t) {
        w.setField("track_pt",  t.pt);
        w.setField("track_eta", t.eta);
        w.fill();
    }
};
```

### 8.4 Timestamp recording (time series)

When enabled, the writer automatically records the wall-clock time at which each entry
was committed. The timestamp is stored as a separate `std::int64_t` field (nanoseconds
since Unix epoch) alongside the user's fields.

```cpp
Gem::Chronicle::GChronicleOptions opts;
opts.enableTimestamps(true);                       // adds field "chronicle_timestamp_ns"
opts.setTimestampFieldName("submission_time_ns");  // optional: rename the field

auto writer = file.makeWriter("TimeSeries");
auto value  = writer.makeField<double>("temperature");

value = 36.5;
writer.fill();   // timestamp recorded automatically at this point

value = 37.1;
writer.fill();   // second timestamp recorded
```

The timestamp field is always the last field in the schema. Its name is configurable but
defaults to `"chronicle_timestamp_ns"`. Timestamps use `std::chrono::system_clock` and are
stored as `int64_t` nanoseconds since the Unix epoch.

### 8.5 API invariants

- Field names are unique within an RNTuple; duplicates throw `GChronicleFieldException`
- Schema is frozen on first `fill()`; subsequent `makeField()` / `declareField()` calls
  throw `GChronicleSchemeFrozenException`
- Every field must have been set before `fill()`, or have a registered default value
  (documented in `docs/chronicle/design.md`); missing fields throw `GChronicleFieldNotSetException`
- `close()` is idempotent
- Destructors do not throw; they call `close()` and log errors via `GLogger`
- I/O errors are surfaced as exceptions derived from `Gem::Common::GException`
- The stream `operator<<` returns the stream reference (chainable)
- Batch mode and streaming mode cannot be mixed with single-value mode within the same
  writer instance; attempting to do so throws `GChronicleModeConflictException`

---

## 9. Known format facts (knowledge reference)

This section captures confirmed knowledge about the RNTuple Binary Format 1.0.0.0.
It is the distilled result of specification study and reference-file inspection.
**It is not derived from ROOT source code.** Each fact cites its source. This section
is the persistent memory for format details that must survive context compression.

### 9.1 Overall structure

An RNTuple file contains:
- A container (bare file or TFile)
- One or more RNTuple blobs, each consisting of:
  - A **header envelope** (schema description: field descriptors, column descriptors,
    alias column descriptors, extra type information)
  - Zero or more **page list envelopes**, one per cluster
  - Zero or more **page blobs** (raw column data, organized by cluster)
  - A **footer envelope** (cluster summaries, cluster group records, and a reference
    back to the header and page list envelopes)

Entries are grouped into **clusters**. A cluster is a contiguous range of entries. All
pages for a given column within a cluster are stored together. The footer envelope
carries the cluster summaries (first entry index + entry count) and page list envelope
locators.

### 9.2 Envelope format

Every envelope (header, footer, page list) has:
- An 8-byte **type-and-size prefix**: a little-endian `uint64_t` where the upper 16 bits
  encode the envelope type and the lower 48 bits encode the payload size in bytes
- The **payload** (variable length, format depends on type)
- An 8-byte **xxHash3-64 checksum** of the preceding bytes (type-size prefix + payload),
  stored as a little-endian `uint64_t`

Envelope type codes:
- `0x0000` — reserved (never used)
- `0x0001` — Header envelope
- `0x0002` — Footer envelope
- `0x0003` — Page list envelope

Source: RNTuple format specification §Envelopes.

### 9.3 Record frames and list frames

All variable-length structures inside envelopes use **record frames** or **list frames**:

- **Record frame**: a little-endian `int32_t` giving the total byte size of the record
  (including the 4-byte size field itself). Negative size indicates a future-extension
  record that readers must skip.
- **List frame**: a little-endian `int32_t` giving the size (negative = future extension),
  followed by a little-endian `uint32_t` giving the number of items, followed by the items.

Record and list frames enable forward compatibility: readers must skip unknown record
frames by seeking past them using the size field.

Source: RNTuple format specification §Frames.

### 9.4 Column type codes

The column type is encoded as a little-endian `uint16_t` in the column descriptor.
Known type codes for primitive scalar fields:

| C++ type | RNTuple type name | Type code |
|---|---|---|
| `float` | `kReal32` | 5 |
| `double` | `kReal64` | 6 |
| `bool` / `uint8_t` | `kBit` / `kByte` | 1 / 2 |
| `int16_t` | `kInt16` | 17 |
| `uint16_t` | `kUInt16` | 18 |
| `int32_t` | `kInt32` | 9 |
| `uint32_t` | `kUInt32` | 10 |
| `int64_t` | `kInt64` | 11 |
| `uint64_t` | `kUInt64` | 12 |

Note: type codes above 16 were added in format version 1; earlier draft files may differ.
Verify exact codes against a reference file generated by ROOT 6.34. Commit the
verification to `docs/chronicle/format-notes.md` with the reference file hash.

Source: RNTuple format specification §Column Types and `docs/chronicle/format-notes.md`
(to be populated during Phase 5).

### 9.5 Locators

A **locator** identifies the byte position and size of a blob in the file:

- **Standard locator**: 4-byte little-endian `int32_t` (size, must be > 0 for on-disk
  data; negative values have special meaning defined in the spec) + 8-byte little-endian
  `uint64_t` (absolute file offset)
- **Extended locator**: indicated by a negative size field; used for remote storage URLs.
  Not needed for local file output.

For all local file writing in v1, use standard locators only.

Source: RNTuple format specification §Locators.

### 9.6 Page structure

Each page contains:
- The raw element data: `N` contiguous little-endian values of the column's type
- No per-page header in the page blob itself; all page metadata (offset, size, element
  count, checksum) lives in the page list envelope

Pages are checksummed: the 8-byte xxHash3-64 of the page payload (the element data only,
not counting the checksum itself) is appended to the page blob on disk.
Total on-disk page size = element data size + 8 bytes.

Source: RNTuple format specification §Pages.

### 9.7 Cluster summary

The footer envelope contains one cluster summary record per cluster:
- First entry index: little-endian `uint64_t`
- Number of entries: little-endian `uint64_t`
- Flags: little-endian `uint64_t` (currently 0 for uncompressed clusters)

Source: RNTuple format specification §Cluster Summaries.

### 9.8 Bare-file embedding

The bare-file container is the simplest possible RNTuple container:

- An 8-byte magic identifier (exact bytes to be confirmed against reference file;
  the spec states this is a specific sequence identifying the format version)
- The RNTuple **anchor** structure (fixed-size, describes offsets to the header, footer,
  and pages within the file)
- The envelope and page data following the anchor

The anchor structure contains:
- Format version: little-endian `uint16_t` (1 for format 1.0.0.0)
- Feature flags: little-endian `uint16_t`
- File UUID: 16 bytes
- Header locator (offset + size)
- Footer locator (offset + size)
- An xxHash3-64 checksum of the preceding anchor fields

The exact byte layout of the anchor, including field sizes and padding, must be confirmed
against a ROOT-generated reference bare file. Document the confirmed layout in
`docs/chronicle/format-notes.md` during Phase 6.

Source: RNTuple format specification §Bare File.

### 9.9 TFile embedding

A TFile is a container format for ROOT objects. For RNTuple, the relevant TFile structures
are:

- **TFile header**: starts with the ASCII magic `root` (bytes 0x72 0x6f 0x6f 0x74),
  followed by the format version, file header size, end-of-data offset, free list offset,
  number of free segments, number of objects, compression level, seek-free offset,
  seek-info offset (pointing to TStreamerInfo), UUID, and similar bookkeeping fields.
  Most fields are 4-byte big-endian `uint32_t` (TFile uses big-endian, unlike RNTuple
  which uses little-endian internally).

- **TKey**: a key-value record. Each TKey has a fixed header (key length, version, object
  size, date/time, key length again, cycle number, seek-key offset, seek-parent-dir
  offset, class name, object name, object title) followed by the serialized object data.
  String fields in TKey use ROOT's TString encoding (1-byte length for short strings, or
  0xFF + 4-byte length for long strings ≥ 255 bytes).

- **RNTuple anchor TKey**: the RNTuple anchor is stored as a TKey whose class name is
  `"ROOT::RNTuple"`. The anchor is serialized using ROOT's object serialization (the exact
  streamed form must be confirmed by reference-file inspection during Phase 7b).

- **TStreamerInfo**: ROOT classes that appear as TKey objects require a corresponding
  `TStreamerInfo` record in the file's StreamerInfo blob (a TKey with class name
  `"TList"` named `"StreamerInfo"`). The `TStreamerInfo` for `ROOT::RNTuple` describes
  the member fields of the anchor and their types.

The TFile embedding phase (Phase 7) is the highest-risk component because the TKey and
TStreamerInfo formats are not fully specified in the public RNTuple spec and must be
confirmed by reference-file inspection. Do not attempt to write TFile embedding without
first committing annotated reference files.

Source: ROOT TFile format documentation (public), RNTuple format specification §TFile
Embedding, reference-file inspection.

### 9.10 xxHash3-64

The format mandates xxHash3-64 checksums on:
- Every page blob (checksum of the element data, appended to the page)
- Every envelope (checksum of the type-size prefix + payload)
- The bare-file anchor (checksum of the anchor fields)

Use the canonical xxHash3 implementation via system `libxxhash` (`libxxhash-dev` on
Ubuntu). The 64-bit result is stored as a little-endian `uint64_t`.

Test vectors are available in the xxHash specification and repository. All test vectors
must pass before any format work begins (Phase 2).

### 9.11 Compression chunk header (for Phase 9)

When compression is enabled, each compressed page is preceded by a 9-byte chunk header:
- 1 byte: algorithm identifier (0 = none, 1 = zlib, 2 = lz4, 4 = zstd)
- 3 bytes: compressed size (little-endian `uint24_t`, i.e. 3 bytes)
- 3 bytes: uncompressed size (little-endian `uint24_t`)
- 2 bytes: reserved / context (currently 0)

The uncompressed case (algorithm = 0) means the chunk header is present but the payload
is passed through unchanged, with compressed size == uncompressed size.

Source: RNTuple format specification §Compression.

### 9.12 Endianness

All RNTuple-internal multi-byte fields are **little-endian**. The TFile outer container
uses **big-endian** for its own header fields. This mixed endianness is the most common
source of implementation errors in Phase 7.

### 9.13 Append / reopen is not supported

RNTuple 1.0.0.0 is a **write-once-read-many** format. This is an intentional architectural
decision, not a missing feature. Once an RNTupleWriter is closed (released), the resulting
RNTuple is immutable. It cannot be reopened and extended with further entries.

**Why appending is not possible with the current format:**

1. The footer envelope is written at close time and contains all cluster summaries and
   page-list envelope locators. To append new entries, the footer would need to be
   completely rewritten — the old footer invalidated and a new one written that
   references both the original and new clusters. The format has no "append record" or
   "delta footer" mechanism.

2. Rewriting the footer requires knowing its on-disk location, parsing the existing
   file, writing new pages + a new page-list envelope, writing a new footer, and
   patching the footer locator reference. This is substantially more than a writer —
   it requires a reader as well. Out of scope for v1.

3. ROOT's own `RNTupleWriter::Append()` does NOT reopen an existing RNTuple. It adds a
   **new, separately named RNTuple** to an existing TFile container. The two RNTuples
   are independent.

4. Uproot's `WritableNTuple.extend()` works only within a single active writing session;
   it does not reopen a previously saved file.

5. No ROOT GitHub issue or roadmap item plans true append-to-existing-RNTuple support.

Sources: ROOT architecture documentation, ROOT GitHub issue #19168, ROOT 6.36 release
notes, uproot WritableNTuple documentation, Fermilab CHEP 2024 paper on RNTuple I/O.

**Consequence for the API design:** `GChronicleWriter` must document clearly that all
entries must be written in a single session before `close()` is called. The destructor
calls `close()` to ensure the footer is always finalized. There is no `reopen()` method.

**Workarounds for the caller who needs incremental writes:**
- Keep the writer alive for the full duration of data collection (the writer holds the
  file open); use `flush()` (if implemented) to commit clusters to disk without closing.
- Write multiple separate RNTuples into a single TFile with `GEmbedding::TFile` and
  multiple `makeWriter()` calls with different names.
- Write separate files and merge them with ROOT's `hadd` tool or `RNTupleImporter`
  after the fact.

**Future possibility (out of scope for v1):** A future version of the library could
support append by implementing a minimal footer reader, rewriting the footer after
appending new clusters, and patching the footer locator. This would require the library
to also expose a partial reader. Document as future work in `docs/chronicle/design.md`.

---

## 10. The validation oracle

ROOT 6.36.12 is installed on the development machine at `/opt/root/`. Activate it with:

```bash
source /opt/root/bin/thisroot.sh
```

The validation harness is a CMake-driven CTest that:

1. Builds the example `GSimpleWrite`
2. Runs it to produce `candidate.root` (or `candidate.bare`)
3. Invokes ROOT directly: `root -l -b -q tools/chronicle/validate_with_root.C(...)`
4. Collects exit code and stdout
5. Fails if the script reports any mismatch

The ROOT path is passed to CMake via `-DROOT_EXECUTABLE=/opt/root/bin/root` or detected
automatically when `GCHR_ENABLE_ROOT_VALIDATION=ON` and `root` is on `PATH`. In
`CMakeLists.txt` use `find_program(ROOT_EXECUTABLE root HINTS /opt/root/bin)`.

This harness must work end-to-end with a trivial passing case (a ROOT-generated reference
file validated against itself) before any RNTuple-writing code is written.

---

## 11. Implementation phases

### Phase −2: Environment check (1 day)

**This is the first phase. Do not proceed to Phase −1 until all checks pass.**

The goal of this phase is to verify that the development environment has all required
tools and to guide the user through installing anything that is missing.

**Check each of the following in order. For each missing tool, print a clear error
message and the appropriate installation command(s) for Ubuntu/Debian.**

#### Required tools (non-optional)

1. **C++ compiler**: GCC 13+ or Clang 18+
   ```bash
   g++ --version    # need 13.x or higher
   clang++ --version  # need 18.x or higher (either one is sufficient)
   ```
   If missing (Ubuntu 24.04):
   ```bash
   sudo apt install gcc-13 g++-13
   # or
   sudo apt install clang-18
   ```

2. **CMake 3.27+**
   ```bash
   cmake --version  # need 3.27 or higher
   ```
   If too old:
   ```bash
   # Ubuntu 24.04 ships CMake 3.28 — usually fine
   # For older Ubuntu, use pip or download from cmake.org:
   pip install cmake --upgrade
   ```

3. **xxHash library** (for the mandatory xxHash3-64 checksums)
   ```bash
   pkg-config --modversion libxxhash   # need 0.8.0 or higher
   ```
   If missing:
   ```bash
   sudo apt install libxxhash-dev
   ```

4. **Catch2 v3** (Geneva's test framework)
   ```bash
   # Check if Catch2 is findable by CMake:
   cmake -S /tmp/catch2_test -B /tmp/catch2_test_build \
     -DCMAKE_PREFIX_PATH=/opt/catch2   # or wherever installed
   ```
   If missing (build from source, Geneva requires Catch2 v3):
   ```bash
   git clone --depth 1 -b v3.x https://github.com/catchorg/Catch2.git /tmp/Catch2
   cmake -S /tmp/Catch2 -B /tmp/Catch2/build -DCMAKE_INSTALL_PREFIX=/opt/catch2
   cmake --build /tmp/Catch2/build --parallel
   sudo cmake --install /tmp/Catch2/build
   ```

5. **ROOT 6.34+** (external validation oracle; not a build dependency)

   ROOT 6.36.12 is installed at `/opt/root/`. Check it is reachable:
   ```bash
   source /opt/root/bin/thisroot.sh
   root --version   # must print 6.36.xx
   root -l -b -q -e 'std::cout << "ROOT OK, version " << gROOT->GetVersion() << std::endl; gApplication->Terminate();'
   ```
   If `source /opt/root/bin/thisroot.sh` is not automatic in your shell, add it to
   `~/.bashrc` or pass the ROOT path explicitly via `-DROOT_EXECUTABLE=/opt/root/bin/root`
   when configuring CMake with `GCHR_ENABLE_ROOT_VALIDATION=ON`.

   If for some reason ROOT needs to be reinstalled, the recommended approach is a
   pre-built binary from `https://root.cern/install/` compiled with C++20 support.

#### Optional but recommended tools

6. **clang-format 18+** (for Geneva code style)
   ```bash
   clang-format --version
   sudo apt install clang-format-18
   ```

8. **clang-tidy 18+** (for Geneva static analysis)
   ```bash
   clang-tidy --version
   sudo apt install clang-tidy-18
   ```

9. **xxd or hexyl** (for reference-file hex analysis)
   ```bash
   xxd --version        # usually pre-installed
   # hexyl is nicer:
   sudo apt install hexyl
   ```

10. **Python 3 + numpy** (for the reference-file dump scripts in `tools/chronicle/`)
    ```bash
    python3 --version
    python3 -c "import numpy"
    # If missing:
    sudo apt install python3-numpy
    ```

#### Environment check script

Create `tools/chronicle/check_env.sh` that runs all checks above and prints a summary
table:

```
[OK ] GCC 13.2
[OK ] CMake 3.28.1
[OK ] xxhash 0.8.2
[OK ] ROOT 6.36.12 (/opt/root/bin/root)
[OK ] Catch2 3.4.0
...
```

Exit 0 if all required tools pass (warnings for optional tools are non-fatal).
Exit 1 if any required tool fails.

**Acceptance criterion for Phase −2:** `check_env.sh` exits 0.

---

### Phase −1: Spike prototype (1–2 weeks)

Goal: produce one ROOT-readable bare-file RNTuple with a single `float` field and a
handful of entries, using no ROOT headers, to surface format ambiguities early.

The spike lives in `spike/` (deleted or archived before Phase 0). Its value is the
knowledge captured in `docs/chronicle/spike-findings.md`.

Allowed during spike: everything listed in §5 Allowed sources.
Forbidden during spike: everything in §5 Forbidden sources.

Deliverables:
- Working invocation: `spike/write && root -l -b -q spike/validate.C`
- `docs/chronicle/spike-findings.md` listing every byte-level question that arose,
  every answer found (with reference), and all unresolved ambiguities

Success criterion: ROOT prints "OK" for a file generated by hand-rolled code using no
ROOT headers. If this cannot be achieved in 2 weeks: stop, escalate.

---

### Phase 0: Skeleton, validation infra, clean-room policy (3–5 days)

Deliverables:
- Repository skeleton from §6 (directories, empty files, CMakeLists.txt hierarchy)
- `gemfony-chronicle` CMake target building an empty library
- CTest plumbing with Geneva's test infrastructure
- A trivial validation test: `validate_with_root.C` opens a ROOT-generated reference
  RNTuple file and reports OK
- `docs/chronicle/clean-room-policy.md` complete
- `docs/chronicle/assumptions.md` initialized
- `docs/chronicle/spike-findings.md` from Phase −1 committed
- `tools/chronicle/check_env.sh` from Phase −2 committed

Test gates:
- `test_project_builds`
- `test_no_root_headers_in_public_api` (compile test or grep)
- `test_validation_oracle_works`

---

### Phase 1: Binary writer primitives (3–5 days)

Pure TDD. All in Geneva style (class `GEndianWriter`, `GByteBuffer`).

```text
test_write_u16_le
test_write_u32_le
test_write_u64_le
test_write_i32_negative_values
test_write_float_exact_bytes
test_write_double_exact_bytes
test_patch_u32_at_offset
test_offset_after_writes
test_append_bytes
test_reject_patch_out_of_range
```

---

### Phase 2: xxHash3-64 integration (2–3 days)

Tests against the canonical xxHash3 test vectors (not from ROOT):

```text
test_xxh3_64_empty_input
test_xxh3_64_single_byte
test_xxh3_64_short_inputs
test_xxh3_64_aligned_block
test_xxh3_64_unaligned_block
test_xxh3_64_long_input
test_xxh3_64_matches_libxxhash
```

Acceptance: all canonical vectors pass; system and vendored backends produce identical
output.

---

### Phase 3: Schema model (3–5 days)

Pure TDD. `GSchema`, `GFieldDescriptor`, `GColumnDescriptor`.

```text
test_create_float_field
test_create_double_field
test_create_int32_field
test_create_uint64_field
test_duplicate_field_name_rejected
test_field_order_preserved
test_schema_freezes_after_first_fill
test_unsupported_type_rejected
test_timestamp_field_added_when_enabled
test_timestamp_field_is_last
test_timestamp_field_name_configurable
```

MVP types: `float`, `double`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`, and `int64_t`
for timestamps. Document the column-type-code mapping in `docs/chronicle/format-notes.md`
(§9.4 above gives provisional codes; confirm with reference files).

---

### Phase 4: Column buffers, row filling, all API modes (3–5 days)

Tests for single-value mode, batch mode, streaming adaptor mode, and timestamps:

```text
# Single-value mode
test_fill_one_entry_three_columns
test_fill_1000_entries
test_missing_field_assignment_throws
test_field_value_reset_policy
test_column_entry_counts_match
test_cannot_add_field_after_fill

# Batch mode
test_fillBatch_vector_int32
test_fillBatch_vector_float
test_fillBatch_span_double
test_fillBatch_mismatched_sizes_throws
test_fillBatch_empty_container_is_noop
test_fillBatch_multiple_calls_accumulate

# Streaming adaptor mode
test_adaptor_concept_satisfied
test_adaptor_concept_not_satisfied_missing_fill
test_stream_operator_single_object
test_stream_operator_container
test_stream_operator_chain
test_adaptor_declareFields_called_once
test_adaptor_nested_composition

# Timestamp mode
test_timestamp_field_not_present_when_disabled
test_timestamp_field_present_when_enabled
test_timestamp_is_int64
test_timestamp_increases_monotonically
test_timestamp_is_nanoseconds
test_timestamp_field_name_default
test_timestamp_field_name_custom
```

Acceptance: all three API modes produce consistent column-buffer state; entry counts
are consistent; no file I/O yet.

---

### Phase 5: Pages and envelopes (1–2 weeks)

First phase touching the RNTuple format proper. See §9.2–9.6.

For each envelope test, generate a reference file with ROOT, commit it under
`tests/chronicle/fixtures/reference_files/`, and annotate the hex dump.

Page tests:
```text
test_float_column_single_page
test_float_column_multiple_pages
test_page_payload_uncompressed_size
test_page_element_count
test_page_xxh3_checksum
test_page_boundary_exact_fit
test_page_boundary_one_over
test_empty_column_policy
```

Envelope tests:
```text
test_header_envelope_type_id_and_length
test_footer_envelope_type_id_and_length
test_envelope_xxh3_trailer
test_metadata_contains_ntuple_name
test_metadata_contains_field_descriptors
test_metadata_contains_column_descriptors
test_page_list_envelope_references_pages
test_cluster_summary_entry_range
test_locator_serialisation
```

---

### Phase 6: Bare-file embedding (1 week)

See §9.8.

```text
test_bare_file_magic
test_bare_file_anchor_layout
test_bare_file_anchor_xxh3_checksum
test_bare_file_full_writer_internal_invariants
test_bare_file_root_validation_simple_scalars
test_bare_file_root_validation_batch_write
test_bare_file_root_validation_timestamp_field
```

Acceptance: ROOT reads a bare file produced by our writer with all entries round-tripping
for all three API modes. **First major milestone.**

---

### Phase 7: TFile embedding (3–6 weeks)

See §9.9. Highest-risk phase; time-boxed at 4 weeks for Phase 7b.

**7a: TFile header and trivial keys (1 week)**

```text
test_tfile_header_layout
test_tfile_header_magic
test_tkey_layout_for_blob
test_root_can_open_empty_tfile
```

Go/no-go: ROOT must open our empty TFile after 1 week, or stop.

**7b: TStreamerInfo and anchor (2–3 weeks)**

```text
test_streamerinfo_class_name
test_streamerinfo_class_version
test_streamerinfo_member_list_layout
test_anchor_struct_serialisation
test_anchor_xxh3_checksum_position
test_root_can_list_keys_in_our_file
test_root_can_get_rntuple_object
test_root_can_open_rntuple_for_reading
```

**7c: Full integration (1 week)**

```text
test_tfile_full_writer_root_validation
test_tfile_root_validation_batch_write
test_tfile_root_validation_timestamp_field
```

---

### Phase 8: Robustness and edge cases (1–2 weeks)

All three API modes tested against edge cases:

```text
test_zero_entries
test_one_entry
test_page_exact_fit
test_page_one_over_fit
test_many_pages
test_many_clusters
test_negative_int32_values
test_int32_min_max
test_uint64_max
test_float_nan_inf
test_double_nan_inf
test_close_without_fill
test_double_close
test_file_open_error
test_directory_missing_error
test_write_permission_error
test_file_writer_is_noncopyable
test_ntuple_writer_is_noncopyable
test_destructor_does_not_throw
test_close_flushes_metadata
test_exception_on_invalid_state
test_error_message_contains_filename
test_stream_outlives_writer_is_ub_documented  # documented, not testable
```

---

### Phase 9: Compression (optional, 1 week)

See §9.11.

```text
test_zstd_compressed_page_roundtrip_with_root
test_lz4_compressed_page_roundtrip_with_root
test_zlib_compressed_page_roundtrip_with_root
test_compression_none_is_default
test_unknown_compression_rejected
test_compressed_size_recorded_correctly
test_uncompressed_size_recorded_correctly
test_compression_block_header_format
```

---

## 12. Examples (required alongside tests)

Each example in `examples/chronicle/` must compile and run successfully. Examples are not
validation tests (they don't use Catch2) but must be built as part of the Geneva
`GENEVA_BUILD_EXAMPLES` target.

| File | Demonstrates |
|---|---|
| `GSimpleWrite.cpp` | Single-value API; minimal viable usage |
| `GBatchWrite.cpp` | `fillBatch()` with `std::vector` and `std::span` |
| `GStreamWrite.cpp` | `GChronicleAdaptor` definition and `<<` operator |
| `GTimeSeriesWrite.cpp` | `enableTimestamps()`, reading back timestamps with ROOT |
| `GComplexObjectWrite.cpp` | Composing adaptors for nested structs |
| `GBareFileWrite.cpp` | Explicitly writing a bare-file RNTuple |

Each example file must include a comment block at the top explaining what it demonstrates
and the expected output (field names, entry count, a few sample values).

---

## 13. Test categories

### Unit tests
Small, fast, deterministic. Never invoke ROOT or external processes.

### Golden tests
Compare generated bytes against committed expected bytes.
```cpp
REQUIRE_THAT(buffer.bytes(), Catch::Matchers::RangeEquals(std::vector<uint8_t>{
    0x72, 0x6e, 0x74, 0x75, 0x01, 0x00, 0x00, 0x00
}));
```
Note: Geneva uses Catch2 v3 (not Google Test). Use Catch2 matchers throughout.

### Property tests
Deterministic loops with documented seeds (or a C++ property-testing framework).

### Internal integration tests
Exercise the full writer through the public API; parse output with a test-only reader.

### ROOT-validation tests
Invoke ROOT directly at `/opt/root/bin/root`. Enabled by `-DGCHR_ENABLE_ROOT_VALIDATION=ON`.
CMake locates the ROOT binary via `find_program(ROOT_EXECUTABLE root HINTS /opt/root/bin)`.

### Reference-file tests
Assert that our writer's output is semantically equivalent to committed ROOT-generated
reference files in `tests/chronicle/fixtures/reference_files/`.

---

## 14. CI matrix

```yaml
ci.yml:                         # No ROOT, fast feedback
  ubuntu-22.04: gcc-12, clang-15  # minimum supported
  ubuntu-24.04: gcc-13, clang-18  # preferred
  modes: Debug, Release, ASan+UBSan, vendored xxhash, system xxhash

ci-root.yml:                    # ROOT validation, slower
  # ROOT 6.36.12 at /opt/root/ on the dev machine; for CI runners, set ROOT_EXECUTABLE
  modes: full validation suite for all phases reached
```

CI commands:
```bash
cmake -S . -B build \
  -DGENEVA_BUILD_CHRONICLE=ON \
  -DGENEVA_BUILD_TESTS=ON \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure -L chronicle
```

---

## 15. Documentation requirements

- `docs/chronicle/clean-room-policy.md` — Phase 0; never modified except by versioned amendment
- `docs/chronicle/spike-findings.md` — Phase −1 output; frozen
- `docs/chronicle/design.md` — Phase 0 stub; completed after Phase 5
- `docs/chronicle/assumptions.md` — append-only; started Phase 0
- `docs/chronicle/format-notes.md` — Phase 3 onwards; cite §9 and extend it with confirmed facts
- `docs/chronicle/testing-strategy.md` — Phase 0 stub; finalized after Phase 8
- `docs/chronicle/compatibility.md` — Phase 7c onwards; ROOT versions tested

---

## 16. Definition of done

### Bare-file deliverable (minimum viable release)

1. Library builds without ROOT dependency
2. Public headers contain no ROOT includes
3. Library does not link ROOT
4. All Phase 1–6 unit, golden, property, and integration tests pass
5. Phase 6 ROOT-validation tests pass for all three API modes
6. Phase 8 edge cases pass for bare-file embedding
7. All examples build and run
8. clang-format and clang-tidy pass on all library files
9. Documentation complete
10. Clean-room policy followed and audited

### Full deliverable (target)

Everything above, plus:

11. Phase 7 ROOT-validation tests pass for TFile-embedded RNTuples
12. `GSimpleWrite.cpp` produces a `candidate.root` readable by ROOT
13. Compatibility table documents tested ROOT versions

---

## 17. Working style for Claude Code

1. Work in small, reviewable increments. Each commit corresponds to one test added/passing
   or one documented refinement.
2. Before implementation, state which test you are adding and why.
3. Run tests after every change. State the result.
4. When a test fails, classify the failure: expected (we are red), bug, spec ambiguity,
   ROOT-version issue, or environment problem.
5. Never skip failing tests silently. Mark pending tests in code and document why.
6. Never introduce ROOT as a library dependency.
7. Never copy code from ROOT sources. Document any factual answer extracted from ROOT
   source (if it was consulted for ambiguity resolution) in `docs/chronicle/assumptions.md`.
8. Apply the Geneva `.clang-format` to every new file before committing.
9. Ensure clang-tidy passes on every new `.cpp` file before committing.
10. Prefer correctness over performance until correctness is proven.
11. Update documentation in the same commit as the change it describes.
12. Respect time-boxes. If a phase exceeds its envelope, stop and escalate.
13. All new class names begin with `G`. All public types live in the
    `Gem::Chronicle` namespace.
14. Use Geneva exception types (`Gem::Common::GException` and subclasses).
15. Use `GLogger` for all diagnostic output.
16. Do not add Boost dependencies. The RNTuple library depends only on `gemfony-common`
    and the C++20 standard library.
17. **Parallel agents**: Where work can be decomposed into independent sub-tasks, spawn
    multiple Claude agents concurrently. Examples:
    - Phase −2 environment checks and Phase −1 reference-file collection can overlap
    - Writing test stubs for Phase 2 (xxHash) while Phase 1 (primitives) is passing CI
    - Hex-analysing reference files while skeleton code is being written
    - Inspecting `docs/chronicle/spike-findings.md` while running validation
    Each parallel agent must receive a self-contained prompt with all needed context
    (phase number, acceptance criteria, relevant §9 facts, path conventions). After
    agents complete, merge their findings into the main context before proceeding.
18. **Persist findings immediately**: whenever a format fact, build decision, or
    spec-ambiguity resolution is confirmed, append a §20 entry before the next tool
    call. See §0 for the required format and the re-read protocol after context
    compression.

---

## 18. First concrete tasks

Execute in order. Do not advance to the next task until the current one is complete.

### Task 1 — Phase −2: Environment check

Create `tools/chronicle/check_env.sh` and run it. For every missing required tool, provide
the installation command and wait for the user to install and re-run the check.

ROOT 6.36.12 is installed at `/opt/root/`. Verify it is reachable:
```bash
source /opt/root/bin/thisroot.sh
root -l -b -q -e 'std::cout << "ROOT OK, version " << gROOT->GetVersion() << std::endl; gApplication->Terminate();'
```

Do not proceed to Task 2 until `check_env.sh` exits 0.

### Task 2 — Phase −1: Spike

Set up `spike/` with the simplest possible code that produces a bare-file RNTuple
readable by ROOT. No Geneva conventions, no tests, no production style. Output:
working spike + `docs/chronicle/spike-findings.md`. Populate the confirmed facts back
into §9 of this document.

If the spike does not succeed within 2 weeks: stop and report.

### Task 3 — Phase 0: Skeleton

Create the repository structure from §6 and the trivial oracle test. Confirm the
validation harness runs end-to-end (using system ROOT at `/opt/root/bin/root`) on a
known-good ROOT-generated reference file before any RNTuple-writing code is added.

### Task 4 — Phases 1–9: Phase by phase

Follow §11 strictly. At each phase boundary:
- Confirm acceptance criteria
- Update `docs/chronicle/design.md`
- Verify clang-format and clang-tidy pass
- Commit and tag
- Report

---

## 19. Important reminders

- The goal is files conforming to the public RNTuple specification 1.0.0.0, readable
  by ROOT 6.34+. Not an imitation of ROOT internals.
- Compatibility is proven by ROOT-validation tests, not by confidence.
- The environment check (Phase −2) is not optional. Broken tooling wastes weeks.
- The spike is not a hack — it is a deliberate de-risking instrument.
- Bare-file embedding is the first real milestone.
- TFile embedding is the highest-risk component. Time-box it. Have a fallback.
- Tests are living specifications.
- Format ambiguity, when discovered, is documented before resolved; the §9 format
  knowledge section is the persistent record — update it whenever a format fact is
  confirmed.
- The final product must be boring, deterministic, heavily tested, Geneva-compliant,
  and documented.
- Mixed endianness (little-endian RNTuple data inside big-endian TFile headers) is the
  most common implementation error in Phase 7. Read §9.12 carefully.
- **After every context compression**: re-read §20 (Knowledge Log) before doing
  anything else. The entries there are your own notes from earlier in this session.
  Without them, you will repeat work already done.
- **Write §20 entries immediately** when a finding is confirmed — one entry per
  finding, before the next tool call. Do not batch. Do not defer.
- Development happens on branch `chronicle-draft` (cloned from `catch2-migration`).
  Do not modify `develop` directly.

Begin with Task 1 (Phase −2 environment check). Report the result of `check_env.sh`
before continuing.

---

## 20. Knowledge Log (Claude's persistent notes)

> **Claude: Read this section first after any context compression.**
> **Write a new entry here immediately whenever a finding is confirmed.**
> These are notes from Claude to itself. Without them, context compression erases
> research that cannot be cheaply repeated.

### Entry format

```
### YYYY-MM-DD: <one-line summary>
**Type:** format-fact | spec-ambiguity | build-decision | test-finding | env-finding
**Finding:** <confirmed fact, specific enough to act on>
**Source:** <spec section / reference-file SHA256:offset / experiment / tool output>
**Impact:** <what changes in the implementation because of this>
```

---

*No entries yet. First entry will be written during Phase −2 environment check.*
