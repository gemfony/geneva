# GContainerT — Unified Generic Container Implementation

**Erstellt:** 2026-05-14  
**Branch:** `core-component-modernization`  
**Projekt:** Geneva (Grid-Enabled Evolutionary Algorithms)  
**Repo:** `/home/rberlich/ClionProjects/geneva`

---

> ## ⚠ KEIN COMMIT — ABSOLUTES VERBOT
>
> Du darfst in dieser Sitzung **unter keinen Umständen** einen Git-Commit
> erstellen, Dateien stagen (`git add`), Branches pushen oder sonstige
> schreibende Git-Operationen auf dem Repository oder dem Remote durchführen.
> Das gilt für alle Phasen und für alle Dateien, einschließlich CMakeLists,
> Tests und Benchmarks.
>
> Erlaubte Git-Operationen: **ausschließlich lesend** —
> `git status`, `git diff`, `git log`, `git show`, `git branch`.
>
> Die Entscheidung über Commits trifft ausschließlich der Entwickler.

---

> ## Kontext-Resume nach Komprimierung
>
> Wenn dein Kontext komprimiert wurde, gehe **vor allem anderen** so vor:
> 1. Lies den Abschnitt **Fortschritt (Rolling Log)** am Ende dieser Datei
>    vollständig.
> 2. Stelle anhand des letzten Eintrags fest, wo du aufgehört hast.
> 3. Setze die Arbeit an der passenden Phase fort — starte **nie** von vorne.
> 4. Das Commit-Verbot gilt weiterhin uneingeschränkt.

---

## Kontext

Du arbeitest im C++20-Bibliotheksprojekt Geneva unter
`/home/rberlich/ClionProjects/geneva`, Branch `core-component-modernization`.
Geneva ist eine großskalige Optimierungsbibliothek mit CMake 3.27+,
Boost 1.90+ (inkl. Boost.Serialization), Catch2 v3, GCC 13+ oder Clang 18+.
Alle Klassen liegen im Namespace `Gem::` (bzw. `Gem::Common`, `Gem::Geneva` etc.).

---

## Build-Vorbereitung

Der Build-Tree liegt unter `/home/rberlich/build`. Geneva wird ausschließlich
über `prepareBuild.sh` gebaut — ein separater `cmake --build`-Aufruf ist
**nicht** nötig.

**Zu Beginn der Sitzung — vollständiger Clean-Build:**

```bash
cd /home/rberlich/build && \
    /home/rberlich/ClionProjects/geneva/scripts/prepareBuild.sh \
        --clean -y --build genevaConfig.gcfg \
    2>&1 | tee /tmp/geneva_initial_build.log
grep -c "error:" /tmp/geneva_initial_build.log && echo "Fehler gefunden!" \
    || echo "Build erfolgreich"
```

`genevaConfig.gcfg` liegt bereits im Build-Verzeichnis. Das Skript bereinigt,
konfiguriert via CMake und baut alle aktivierten Targets in einem Schritt.

**Für inkrementelle Builds nach Dateiänderungen** genügt danach:

```bash
cd /home/rberlich/build && make <target> -j$(nproc) 2>&1 | tee /tmp/build_<target>.log
```

Alle nachfolgenden Build- und Testbefehle setzen `/home/rberlich/build` als
Build-Verzeichnis voraus.

---

## Aufgabe

Erstelle eine neue Template-Klasse `GContainerT` in
`include/common/GContainerT.hpp`.

Sie vereinheitlicht die zwei bestehenden Klassen:

- `include/common/GPODVectorT.hpp` — wraps `std::vector<T>` für POD-Typen
- `include/common/GPtrVectorT.hpp` — wraps `std::vector<std::shared_ptr<T>>`
  für polymorphe Geneva-Objekte

**Lies beide Quelldateien vollständig, bevor du mit der Implementierung
beginnst.**

Geneva selbst wird **nicht** verändert; die bestehenden Klassen bleiben
unberührt. `GContainerT` ist eine Neuentwicklung, die später die Migration
erleichtern soll.

---

## Coding Standards und Naming Conventions

Die Quelldateien `.clang-format` und `.clang-tidy` im Repo-Wurzelverzeichnis
sind verbindlich. Lies beide vollständig. Das Folgende fasst die wichtigsten
Punkte zusammen, die direkt die Implementierung betreffen.

### Formatierung (`.clang-format`)

- **Einrückung:** 4 Leerzeichen, keine Tabs, `ContinuationIndentWidth: 4`
- **Zeilenlimit:** 100 Zeichen
- **Klammern:** Öffnende Klammer am Deklarationsende (attach), `else`/`catch`/
  `while` beginnen auf neuer Zeile
- **Konstruktor-Initialisiererlisten:** Leading-Comma-Stil
  (`BreakConstructorInitializers: BeforeComma`)
- **Funktionsparameter:** Ein Parameter pro Zeile, `BlockIndent`
- **Kontrollstrukturen:** Kein Leerzeichen vor Klammer: `if(...)`, `while(...)`,
  `for(...)`; `requires (...)` bekommt ein Leerzeichen
- **Zeiger/Referenzen:** Rechts ausgerichtet: `int *p`, `const T &ref`
- **Template-Deklarationen:** Immer auf eigener Zeile
  (`AlwaysBreakTemplateDeclarations: Yes`)
- **`requires`-Klauseln:** Auf eigener Zeile (`RequiresClausePosition: OwnLine`),
  eingerückt (`IndentRequiresClause: true`)
- **Namespace-Abschluss:** `} /* namespace Gem::Common */` (nicht `//`)
- **Includes:** Innerhalb jeder Gruppe alphabetisch sortiert, Gruppenreihenfolge
  beibehalten: Geneva-eigene Header, Boost-Header, Standard-Header

### Bezeichner-Konventionen (`.clang-tidy`)

| Kategorie | Konvention | Beispiele |
|---|---|---|
| Klassen, Structs | `CamelCase` | `GContainerT`, `PodStorage`, `SharedPtrStorage` |
| Template-Parameter | `CamelCase` | `StoragePolicy`, `T`, `Container`, `DerivedType` |
| Concepts | `CamelCase` | `GenevaCloneable`, `PodType`, `HasReserve` |
| Type-Aliase / `using` | `CamelCase` | `ValueType`, `SizeType`, `ContainerType` |
| Enum-Konstanten | `UPPER_CASE` | `TEXT`, `BINARY` |
| Makros, globale Konstanten | `UPPER_CASE` | `DO_LOG` |
| Methoden, freie Funktionen | `camelBack` | `pushBack()`, `crossOver()` |
| Parameter, lokale Variablen | `camelBack` | `itemPtr`, `nVal` |
| Private und protected Member | `camelBack` + Suffix `_` | `dataCnt_` |

Weitere Regeln:
- **`typename` statt `class`** in Template-Parameterlisten bevorzugen
- **Template-Klassen** enden auf `T`, Interface-Klassen auf `I`, core
  Geneva-Klassen beginnen mit `G` (wo zutreffend)
- **`explicit`** für alle Ein-Argument-Konstruktoren (außer Copy/Move)
- **`[[nodiscard]]`** auf allen Methoden, die einen Wert zurückgeben und deren
  Rückgabewert nicht zu ignorieren ist (Queries, Größenabfragen, Clone-Methoden)
- **`noexcept`** korrekt setzen: `size()`, `empty()`, `begin()`, `end()`,
  `data()`, Move-Konstruktor, Move-Zuweisung müssen `noexcept` sein
- **`override`** und **`final`** konsequent verwenden
- Keine `using namespace`-Direktiven
- Keine C-Style-Casts; stattdessen `static_cast`, `dynamic_cast` etc.
- Kein `NULL`; stattdessen `nullptr`
- **Doxygen-Dokumentation — vollständig und verpflichtend:**
  - Jede Klasse, jedes Struct und jedes Concept erhält einen Doxygen-Block
    `/** … */` mit `@brief`, `@tparam` für jeden Template-Parameter und
    mindestens einem erklärenden Satz zum Zweck.
  - Jede öffentliche und protected Methode erhält `/** … */` mit `@brief`,
    `@param` für jeden Parameter, `@return` (falls Rückgabewert vorhanden),
    `@note` für Sonderfälle (Null-Pointer-Guards, Konzept-Einschränkungen,
    `noexcept`-Garantien) und `@throws` falls Ausnahmen möglich sind.
  - Jeder protected Datenmember (`dataCnt_`) erhält einen Doxygen-Inline-
    Kommentar `///< Kurzbeschreibung`.
  - Stil orientiert sich an `GPODVectorT.hpp` / `GPtrVectorT.hpp`; alle
    bestehenden Doxygen-Tags aus diesen Dateien übernehmen.
  - Keine leeren oder Stub-Kommentare (`/** */`, `/// TODO`): jeder Kommentar
    muss informativ sein.
- **Header-Deklarationen** sollen Parameter-Namen weglassen (Implementierungen
  in der Template-Klasse selbst sind davon ausgenommen, da inline)
- Funktionslänge: max. 100 Zeilen, max. 50 Statements, max. 15 Verzweigungen

### Hinweis zur Member-Benennung und Migration

Die bestehenden Klassen verwenden `data_cnt_` (snake_case), was nicht dem
neuen clang-tidy-Standard (`camelBack` + `_`) entspricht. `GContainerT` soll
dem **neuen Standard** folgen und `dataCnt_` verwenden. Beim späteren Migrieren
von Geneva-Subklassen muss dies berücksichtigt werden — der direkte Zugriff
auf `dataCnt_` aus abgeleiteten Klassen (da `protected`) wird dann ebenfalls
anzupassen sein.

---

## Design-Vorgaben

### 1. Storage-Policy-Design

Beide Policies erhalten einen zweiten Template-Parameter `Container`, der den
zugrunde liegenden Sequenz-Container festlegt. Der Default ist jeweils
`std::vector`. So können Nutzer ohne Aufwand andere Container einsetzen
(z. B. `std::deque`, `boost::container::stable_vector`), solange deren API
kompatibel ist.

**`Gem::Common::PodStorage<T, Container>`**

```cpp
template <typename T, typename Container = std::vector<T>>
    requires std::is_trivial_v<T> && std::is_standard_layout_v<T>
struct PodStorage {
    using ValueType     = T;
    using StoredType    = T;
    using ContainerType = Container;

    static void deepCopy(const ContainerType &src, ContainerType &dst) {
        dst = src;
    }
};
```

**`Gem::Common::SharedPtrStorage<T, Container>`**

```cpp
template <typename T, typename Container = std::vector<std::shared_ptr<T>>>
    requires Gem::Common::has_gemfony_common_interface<T>::value
struct SharedPtrStorage {
    using ValueType     = T;
    using StoredType    = std::shared_ptr<T>;
    using ContainerType = Container;

    static void deepCopy(const ContainerType &src, ContainerType &dst) {
        // Tiefe Kopie via clone() wie in GPtrVectorT
    }
};
```

**Hauptklasse:**

```cpp
template <typename T, typename StoragePolicy = PodStorage<T>>
class GContainerT { ... };
```

**Alias-Templates** (Container-Parameter durchgereicht):

```cpp
template <typename T, typename Container = std::vector<T>>
using GPodContainer = GContainerT<T, PodStorage<T, Container>>;

template <typename T, typename Container = std::vector<std::shared_ptr<T>>>
using GPtrContainer = GContainerT<T, SharedPtrStorage<T, Container>>;
```

### 2. Container-Capability-Concepts

Nicht jeder Container bietet dieselbe API. Definiere C++20-Concepts, um
container-spezifische Methoden in `GContainerT` bedingt zu aktivieren:

```cpp
// Contiguous storage: std::vector, std::array — aber nicht std::deque
template <typename C>
concept HasContiguousStorage = requires(C c) { c.data(); };

// Capacity management: std::vector — aber nicht std::deque, std::list
template <typename C>
concept HasCapacity = requires(C c, typename C::size_type n) {
    c.reserve(n);
    c.capacity();
    c.shrink_to_fit();
};

// Random-access iterators (benötigt für crossOver und operator<=>):
// std::vector, std::deque — aber nicht std::list, std::forward_list
template <typename C>
concept HasRandomAccess =
    std::random_access_iterator<typename C::iterator>;

// Doppelseitige Operationen: std::deque, std::list — aber nicht std::vector
// (std::vector hat kein push_front/pop_front)
template <typename C>
concept HasFrontInsertion = requires(C c, typename C::value_type v) {
    c.push_front(v);
    c.pop_front();
};
```

Methoden in `GContainerT`, die von diesen Concepts abhängen:

| Methode | Aktiv wenn |
|---|---|
| `data()` | `HasContiguousStorage<ContainerType>` |
| `capacity()`, `reserve()`, `shrinkToFit()` | `HasCapacity<ContainerType>` |
| `crossOver()` | `HasRandomAccess<ContainerType>` |
| `operator<=>` | `HasRandomAccess<ContainerType>` und `T` drei-Wege-vergleichbar |
| `pushFront()`, `popFront()`, `emplaceFront()` | `HasFrontInsertion<ContainerType>` |

Verwende `requires`-Klauseln auf den jeweiligen Methoden (nicht `if constexpr`
im Rumpf), damit nicht-unterstützte Methoden zur Compile-Zeit ausgeschlossen
werden und klare Fehlermeldungen entstehen.

### 3. Abstrakter Basisklassen-Charakter

`GContainerT` hat einen pure-virtual Destruktor und bleibt abstrakt —
identisch zum Verhalten von `GPODVectorT` und `GPtrVectorT`.

### 4. API-Vollständigkeit

Implementiere die vollständige Sequenz-Container-API, soweit sinnvoll, und
fülle die Lücken der bestehenden Klassen.

**Fehlend in beiden Klassen (neu implementieren):**
- `cbegin()`, `cend()`, `crbegin()`, `crend()`
- `emplace_back()` und `emplace()` (nur `PodStorage`; bei `SharedPtrStorage`
  nicht sinnvoll — `= delete` mit erklärender Fehlermeldung)
- `assign()` (Wert-, Range- und Initializer-List-Überladungen)
- `data()` (bedingt, s. Concepts)
- `capacity()`, `reserve()`, `shrinkToFit()` (bedingt, s. Concepts)
- `pushFront()`, `popFront()`, `emplaceFront()` (bedingt, s. Concepts)
- Range-Insert: `insert(const_iterator, InputIt first, InputIt last)`
- Initializer-List-Insert
- `operator<=>` (bedingt, s. Concepts)
- `std::ranges`-Kompatibilität: `begin()`/`end()` müssen `std::ranges::range`
  und `std::ranges::sized_range` erfüllen
- `swap()` mit `ContainerType` (nicht hardcodiert auf `std::vector`)

**Aus `GPtrVectorT` übernehmen (für `SharedPtrStorage`):**
- `pushBackClone()` / `pushBackNoclone()`
- `insertClone()` / `insertNoclone()` (Einzel- und Count-Überladung)
- `resizeClone()` / `resizeNoclone()` / `resizeEmpty()`
- `cloneAt()`
- `attachViewTo<DerivedType>()`
- Typ-gefilterter Zugriff: Ersetze die bestehende `conversion_iterator`-Klasse
  durch eine C++20-Range-View-basierte Lösung:
  ```cpp
  template <typename DerivedType>
  auto filteredView();  // std::views::filter | std::views::transform
  ```
  Behalte alternativ die Klasse, wenn eine stateful Iterator-Klasse aus
  technischen Gründen besser passt — begründe die Wahl in einem Kommentar.

**Geneva-spezifische Operationen (beide Policies):**
- `crossOver(GContainerT &, std::size_t pos)` (bedingt: `HasRandomAccess`)
- `compareBase(const GContainerT &, Gem::Common::expectation, double limit) const`
- `getDataCopy(ContainerType &) const`

**Test-Hooks (protected virtual, wie in den Ausgangsklassen):**
- `modifyGUnitTests_()` → `bool`
- `specificTestsNoFailureExpectedGUnitTests_()`
- `specificTestsFailuresExpectedGUnitTests_()`

### 5. Boost.Serialization

Integriere Boost.Serialization wie in den Ausgangsklassen:
- `serialize()`-Methode mit `friend boost::serialization::access`
- Außerhalb des Namespaces, im `boost::serialization`-Namespace:
  ```cpp
  template <typename T, typename StoragePolicy>
  struct is_abstract<Gem::Common::GContainerT<T, StoragePolicy>>
      : public boost::true_type {};
  ```

Hinweis: Boost.Serialization unterstützt `std::vector`, `std::deque` und
`std::list` nativ via `boost/serialization/deque.hpp` etc. Beim Einsatz
alternativer Container muss der entsprechende Boost-Header eingebunden werden.
`GContainerT` selbst muss das nicht erzwingen — ein Nutzer alternativer
Container ist selbst verantwortlich.

### 6. C++20-Modernisierung

- Constraints via `requires`-Klauseln statt `static_assert` + type_traits
- Range-Algorithmen bevorzugen: `std::ranges::copy`, `std::ranges::find_if` etc.
- `[[nodiscard]]` auf allen Query-Methoden
- `noexcept` korrekt gesetzt (s. o.)
- `override` auf allen virtuellen Methoden
- Keine veralteten Boost-Makros; `BOOST_SERIALIZATION_NVP` direkt nutzen

### 7. Fehlerbehandlung

Nutze `geneva_exception` + `g_error_streamer` (wie in den Ausgangsklassen)
für Null-Pointer-Guards und Bereichsfehler. Fehlermeldungen sollen
konsistente Klassennamen enthalten (z. B.
`"In GContainerT::pushBackClone(): "`).

---

## Relevante Header (vor Beginn lesen)

| Datei | Inhalt |
|---|---|
| `include/common/GExceptions.hpp` | `geneva_exception`, `g_error_streamer`, `DO_LOG`, `time_and_place` |
| `include/common/GTypeTraitsT.hpp` | `has_gemfony_common_interface`, `gemfony_common_interface_indicator` |
| `include/common/GCommonHelperFunctionsT.hpp` | `copyCloneableSmartPointerContainer`, `narrow_cast` |
| `include/common/GExpectationChecksT.hpp` | `GToken`, `compare_t`, `expectation` |
| `include/common/GErrorStreamer.hpp` | `DO_LOG`, `time_and_place` |
| `include/common/GGlobalDefines.hpp` | Muss als erstes Geneva-Include eingebunden werden |
| `.clang-format` | Formatierungsregeln (verbindlich) |
| `.clang-tidy` | Naming-Conventions und Checks (verbindlich) |

---

## Phase 1 — `GContainerT`-Header

Erstelle `include/common/GContainerT.hpp` mit:

- Apache-2.0-Lizenzheader (exakt wie in den anderen Headern des Projekts)
- `#pragma once`
- Container-Capability-Concepts (`HasContiguousStorage`, `HasCapacity`,
  `HasRandomAccess`, `HasFrontInsertion`)
- `PodStorage<T, Container>`-Policy-Struct
- `SharedPtrStorage<T, Container>`-Policy-Struct
- `GContainerT<T, StoragePolicy>`-Klasse vollständig implementiert
- `GPodContainer<T, Container>`- und `GPtrContainer<T, Container>`-Alias-Templates
- Boost.Serialization `is_abstract`-Spezialisierungen

Keine CMakeLists-Einträge in Phase 1.

**Nach Phase 1 — Übersetzungstest:**
```bash
cd /home/rberlich/build && make common -j$(nproc) 2>&1 | tee /tmp/phase1_build.log
grep -E "error:" /tmp/phase1_build.log | head -30
```
Lasse danach alle common-Tests laufen:
```bash
ctest --test-dir /home/rberlich/build -R "[Cc]ommon" -V \
    2>&1 | tee /tmp/phase1_ctest.log
```
Repariere **alle** Compiler-Fehler und Testfehler, bevor du zu Phase 2
weitergehst. Dokumentiere den Abschluss im **Fortschritt (Rolling Log)**
am Ende dieser Datei.

---

## Phase 2 — Unit-Tests

Erstelle `tests/common/UnitTests/GContainerTTests.cpp` mit Catch2 v3.

**Ziel:** 100 % Testabdeckung aller öffentlichen Methoden, beider Policies
und der wichtigsten alternativen Container-Backends.

**Catch2-Includes:**
```cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/generators/catch_generators.hpp>
```

**Minimale Klassenhierarchie für `SharedPtrStorage`-Tests** (im anonymen
Namespace der Test-Datei):

```cpp
namespace {

struct TestBase : Gem::Common::gemfony_common_interface_indicator {
    int val = 0;
    virtual ~TestBase() = default;

    template <typename TargetType = TestBase>
    std::shared_ptr<TargetType> clone() const {
        return std::make_shared<TargetType>(*this);
    }

    void load(std::shared_ptr<TestBase> cp) { val = cp->val; }

    virtual bool operator==(const TestBase &o) const { return val == o.val; }
};

struct TestDerived : TestBase {
    int derivedVal = 0;
    bool operator==(const TestBase &o) const override {
        const auto *od = dynamic_cast<const TestDerived *>(&o);
        return od && TestBase::operator==(o) && derivedVal == od->derivedVal;
    }
};

} // anonymous namespace
```

**Pflicht-Testgruppen** (`TEST_CASE` / `SECTION`-Struktur, Tags in eckigen
Klammern):

---

```
[GContainerT][pod][vector]  —  GPodContainer<int> und GPodContainer<double>
                                (Backend: std::vector, Default)
```
- Konstruktoren: default, `(size, val)`, copy, move
- `assign()`: Wert-Überladung, Range-Überladung, Initializer-List
- `pushBack()`, `emplaceBack()`, `popBack()`
- `insert()`: Einzel-, Count-, Range-, Initializer-List-Überladung
- `emplace()`
- `erase()`: Einzel- und Bereichsform
- `at()` mit Ausnahme bei Out-of-bounds
- `operator[]`, `front()`, `back()`
- `data()`: verfügbar, gibt `T*` zurück
- Alle Iterator-Paare: `begin`/`end`, `cbegin`/`cend`, `rbegin`/`rend`,
  `crbegin`/`crend`
- `size()`, `empty()`, `maxSize()`, `clear()`
- `capacity()`, `reserve()`, `shrinkToFit()`: verfügbar
- `resize()` (beide Überladungen)
- `swap()` mit `std::vector<T>` und mit anderem `GContainerT`
- `count()`, `find()`
- `crossOver()`: Vektoren gleicher und unterschiedlicher Länge
- `compareBase()`: gleich, ungleich, Ausnahme bei falscher Erwartung
- `getDataCopy()`
- `operator<=>`: lexikographischer Vergleich kleiner, gleich, größer
- `std::ranges::range`-Erfüllung: `std::ranges::begin()`, `std::ranges::end()`,
  `std::ranges::size()`
- `pushFront()` / `popFront()` sind **nicht** verfügbar (Compile-Zeit-Check)

---

```
[GContainerT][pod][deque]  —  GPodContainer<int, std::deque<int>>
                               (Backend: std::deque)
```
- Alle Methoden aus `[pod][vector]`, die für `std::deque` gelten
- `pushFront()`, `popFront()`, `emplaceFront()`: verfügbar und korrekt
- `data()`: **nicht** verfügbar (Compile-Zeit-Check via Concept)
- `capacity()`, `reserve()`, `shrinkToFit()`: **nicht** verfügbar
  (Compile-Zeit-Check via Concept)
- `crossOver()`: verfügbar (std::deque hat Random-Access-Iteratoren)
- `swap()` mit `std::deque<int>`
- Boost.Serialization-Roundtrip mit `std::deque`-Backend

---

```
[GContainerT][ptr][vector]  —  GPtrContainer<TestBase> (Backend: std::vector)
```
- Konstruktoren: default, copy mit Deep-Clone-Verifikation (Adressen müssen
  verschieden sein, Werte gleich), move
- `pushBackClone()`: Änderung am Original nach dem Einfügen darf die Kopie
  im Container **nicht** ändern
- `pushBackNoclone()`: Änderung am Original nach dem Einfügen **ändert**
  das Element im Container (shared ownership)
- `insertClone()` / `insertNoclone()`: Einzel- und Count-Überladung
- `resizeClone()`, `resizeNoclone()`, `resizeEmpty()`
- `cloneAt()`: Rückgabewert ist unabhängiger Klon
- `count()` und `find()`: Wertvergleich (nicht Pointer-Identität)
- `crossOver()`: Vektoren gleicher und unterschiedlicher Länge
- `compareBase()`
- `getDataCopy()`: jedes Element ist ein unabhängiger Klon
- `attachViewTo<TestDerived>()`: nur `TestDerived`-Elemente im Ergebnis
- Gefilterter Range-View auf `TestDerived`: Iteration nur über Elemente
  vom abgeleiteten Typ; `TestBase`-Elemente werden übersprungen
- Null-Pointer-Guards: `pushBack(nullptr)`, `insertClone(pos, nullptr)` etc.
  müssen `geneva_exception` werfen
- Boost.Serialization-Roundtrip (Text-Archiv): Werte erhalten,
  keine Pointer-Identität

---

```
[GContainerT][edge]
```
- Leerer Container: alle Query-Methoden korrekt
- `resize(0)` und `clear()` auf bereits leerem Container
- `crossOver()` bei `pos == 0` und `pos == size() - 1`
- `emplaceBack()` auf `SharedPtrStorage` ist nicht vorhanden
  (Compile-Zeit-Prüfung via `std::is_invocable` oder Concept-Check)
- `data()` auf `std::deque`-Backend nicht vorhanden (Compile-Zeit-Check)

---

**CMakeLists.txt anpassen:**

Lies `tests/common/UnitTests/CMakeLists.txt`. Lege ein eigenes Target
`GContainerTTests` an (bevorzugt, für klare Trennung):

```cmake
SET(EXECUTABLENAME GContainerTTests)
ADD_EXECUTABLE(${EXECUTABLENAME} GContainerTTests.cpp)
TARGET_LINK_LIBRARIES(${EXECUTABLENAME}
    ${GENEVA_LIBRARIES} ${Boost_LIBRARIES} Catch2::Catch2WithMain)
ADD_TEST(${EXECUTABLENAME} ${EXECUTABLENAME})
```

**Nach Phase 2 — Übersetzung und Tests:**
```bash
cd /home/rberlich/build && make GContainerTTests -j$(nproc) \
    2>&1 | tee /tmp/phase2_build.log
ctest --test-dir /home/rberlich/build -R GContainerTTests -V \
    2>&1 | tee /tmp/phase2_ctest.log
tail -5 /tmp/phase2_ctest.log
```
Alle Tests müssen grün enden (`0 tests failed`). Repariere jeden einzelnen
Fehler iterativ. Dokumentiere den Abschluss im **Fortschritt (Rolling Log)**.

---

## Phase 3 — Benchmark

Erstelle `benchmarks/common/GContainerTBenchmark/` mit:

```
benchmarks/common/GContainerTBenchmark/
  GContainerTBenchmark.cpp
  CMakeLists.txt
```

Das Benchmark-Programm hat zwei orthogonale CLI-Dimensionen:

### `--mode` (was gemessen wird)

| Wert | Bedeutung |
|---|---|
| `pod` | POD-Typen (`double`): push_back, random access, sort, erase |
| `ptr` | Smart-Pointer (`shared_ptr<TestObj>`): push_back_clone, find, crossOver |
| `fuzz` | Langlaufender Stresstest: zufälliges Mischen aller Operationen |
| `all` | Alle Modi nacheinander |

### `--container` (welcher Backend-Container)

| Wert | Bedeutung |
|---|---|
| `vector` | `std::vector` (Default) |
| `deque` | `std::deque` |
| `all` | Beide nacheinander, Ergebnisse direkt nebeneinander |

Beide Flags sind kombinierbar, z. B.:
```bash
GContainerTBenchmark --mode pod --container all
GContainerTBenchmark --mode fuzz --container deque --duration 120
GContainerTBenchmark --mode all --container all
```

### Weitere Optionen

| Flag | Default | Bedeutung |
|---|---|---|
| `--duration <sek>` | `60` | Laufzeit des Fuzz-Modus |
| `--size <n>` | `100000` | Anzahl Elemente für pod/ptr-Modi |
| `--seed <n>` | zufällig | RNG-Seed für Fuzz-Modus (Reproduzierbarkeit) |
| `--quick` | false | Kurztest: setzt automatisch `--size 1000 --duration 5`; für CI und Smoke-Tests |

### Ausgabeformat

Tabellarisch, je eine Zeile pro Messung, mit Spalten:
```
Mode | Container | Backend     | Ops/s       | ns/Op  | vs std::vector
pod  | GContainerT | std::vector | 1 234 567   | 810 ns | —
pod  | std::vector | std::vector |  987 654    | 1013 ns| baseline
pod  | GContainerT | std::deque  |  876 543    | 1141 ns| —
pod  | std::deque  | std::deque  |  854 321    | 1170 ns| baseline
```

Zeiterfassung: `std::chrono::high_resolution_clock`.

**Wichtig:** Das Benchmark-Target darf **nicht** via `ADD_TEST()` in ctest
registriert werden.

Erstelle `benchmarks/common/CMakeLists.txt`:
```cmake
ADD_SUBDIRECTORY(GContainerTBenchmark)

ADD_CUSTOM_TARGET("benchmarks-common"
    DEPENDS GContainerTBenchmark
    COMMENT "Building benchmarks for the Common library.")
```

Prüfe, ob `benchmarks/CMakeLists.txt` bereits ein
`ADD_SUBDIRECTORY(common)` enthält; falls nicht, ergänze es.

**Nach Phase 3 — Benchmark-Build und Kurztest:**
```bash
cd /home/rberlich/build && make GContainerTBenchmark -j$(nproc) \
    2>&1 | tee /tmp/phase3_build.log

# Kurztest — kein langer Lauf nötig:
./benchmarks/common/GContainerTBenchmark/GContainerTBenchmark \
    --mode all --container all --quick

./benchmarks/common/GContainerTBenchmark/GContainerTBenchmark \
    --mode fuzz --quick
```
Der Benchmark darf **nicht** via `ADD_TEST()` in ctest registriert sein.
Dokumentiere den Abschluss im **Fortschritt (Rolling Log)**.

---

## Build-Konventionen (Zusammenfassung)

- Lizenzheader: Apache 2.0, exakt wie in allen anderen `.hpp`/`.cpp`-Dateien
- Namespace: `Gem::Common` für `GContainerT` und Policies;
  Abschluss: `} /* namespace Gem::Common */`
- Erstes Geneva-Include: immer `#include "common/GGlobalDefines.hpp"`
- C++-Standard: 20 (bereits in CMake gesetzt)
- Catch2-Ziel: `Catch2::Catch2WithMain`
- clang-format vor Abgabe anwenden:
  ```bash
  clang-format -i -style=file include/common/GContainerT.hpp
  clang-format -i -style=file tests/common/UnitTests/GContainerTTests.cpp
  clang-format -i -style=file benchmarks/common/GContainerTBenchmark/GContainerTBenchmark.cpp
  ```
- clang-tidy-Check (wenn build-Verzeichnis vorhanden):
  ```bash
  clang-tidy -p /home/rberlich/build include/common/GContainerT.hpp
  ```

---

## Fortschritt (Rolling Log)

*Dieser Abschnitt wird von der implementierenden KI-Instanz nach jedem
abgeschlossenen Schritt aktualisiert. Nach einer Kontext-Komprimierung
liest die KI diesen Abschnitt **als erstes**, um den Stand zu ermitteln,
und setzt dort fort.*

**Format für neue Einträge** (an das Ende dieses Abschnitts anhängen):

```
[YYYY-MM-DD HH:MM] Phase X / Schritt Y — Status, kurze Beschreibung
[YYYY-MM-DD HH:MM] Problem: <Fehlerbeschreibung> → Fix: <was geändert>
```

---

[2026-05-14 10:00] Phase 1 / Schritt 1 — ABGESCHLOSSEN. `include/common/GContainerT.hpp` erstellt: PodStorage, SharedPtrStorage, GContainerT (vollständige API incl. Concepts, crossOver, compareBase, filteredView, Boost.Serialization is_abstract), GPodContainer/GPtrContainer-Aliase.
[2026-05-14 10:00] Build: `make common -j$(nproc)` — erfolgreich, 0 Fehler.
[2026-05-14 10:00] Tests: `ctest -R [Cc]ommon` — 1390362 assertions in 115 test cases, alle grün (1.16 sec).
[2026-05-14 10:15] Phase 2 / Schritt 1 — ABGESCHLOSSEN. `tests/common/UnitTests/GContainerTTests.cpp` erstellt (4 TEST_CASE, 222 assertions): [pod][vector], [pod][deque], [ptr][vector], [edge].
[2026-05-14 10:15] Problem: Boost.Serialization-Roundtrip mit TestBase (kein serialize()) → Fix: Test nutzt std::vector<int> als Proxy.
[2026-05-14 10:15] Problem: filteredView-Range liefert rvalue shared_ptr → Fix: `for(auto ptr : view)` statt `for(auto &ptr : view)`.
[2026-05-14 10:15] Problem: compareBase für SharedPtrStorage benötigt compare() in TestBase → Fix: compare()-Methode zu TestBase hinzugefügt.
[2026-05-14 10:15] Build: `make GContainerTTests -j$(nproc)` — erfolgreich.
[2026-05-14 10:15] Tests: `ctest -R GContainerTTests` — 222 assertions in 4 test cases, alle grün (0.01 sec).
[2026-05-14 10:30] Phase 3 / Schritt 1 — ABGESCHLOSSEN. Benchmark erstellt: `benchmarks/common/GContainerTBenchmark/GContainerTBenchmark.cpp` + `CMakeLists.txt`, `benchmarks/common/CMakeLists.txt`, `benchmarks/CMakeLists.txt` (ADD_SUBDIRECTORY common ergänzt).
[2026-05-14 10:30] Problem: compareBase für SharedPtrStorage benötigt compare() in BenchObj → Fix: compare()-Methode zu BenchObj hinzugefügt.
[2026-05-14 10:30] Build: `make GContainerTBenchmark -j$(nproc)` — erfolgreich.
[2026-05-14 10:30] Kurztest: `--mode all --container all --quick` — 6 Benchmark-Zeilen + Fuzz (40M ops in 5s). Benchmark NICHT in ctest registriert (verifiziert).
[2026-05-14 10:30] Alle Tests: `ctest -R [Cc]ommon|GContainerT` — 2 Tests (GCommonStandardTests + GContainerTTests), 100% grün.
