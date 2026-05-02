# Phase 1 Log — Bibliothek `common` auf Catch2 migriert

**Datum:** 2026-05-02  
**Branch:** `catch2-migration`  
**Commit:** `08250004`  
**Status:** ✅ Erfolgreich abgeschlossen

---

## Durchgeführte Schritte

### 1. `include/common/tests/GBoundedBufferT_tests.hpp` (288 Macros)

- `#include <boost/test/unit_test.hpp>` → `#include <catch2/catch_test_macros.hpp>`
- `BOOST_CHECK_NO_THROW` → `CHECK_NOTHROW` (43 Vorkommen)
- `BOOST_CHECK` → `CHECK` (245 Vorkommen)
- Keine `using namespace boost::unit_test_framework;`-Zeilen vorhanden (keine Aktion nötig)
- Template-Argumente mit Komma (z.B. `GBoundedBufferT<copy_only_struct, 0>`) waren bereits in extra Klammern — Catch2-kompatibel

### 2. `include/common/tests/GCommon_tests.hpp`

Keine Boost.Test-Includes in der Datei selbst — sie zog nur `GBoundedBufferT_tests.hpp` rein. Keine Änderungen nötig.

### 3. `tests/common/UnitTests/GCommonStandardTests.cpp`

Komplett neu geschrieben:
```cpp
#include <catch2/catch_test_macros.hpp>
#include "common/tests/GCommon_tests.hpp"

TEST_CASE_METHOD(Gem::Common::Tests::GBoundedBufferT_tests,
                 "GBoundedBuffer no_failure_expected", "[common][standard]") {
    no_failure_expected();
}
TEST_CASE_METHOD(Gem::Common::Tests::GBoundedBufferT_tests,
                 "GBoundedBuffer failures_expected", "[common][standard][failures-expected]") {
    failures_expected();
}
```
Removes: `BOOST_TEST_DYN_LINK`, `BOOST_TEST_MAIN`, `BOOST_TEST_ALTERNATIVE_INIT_API`, `BOOST_FIXTURE_TEST_SUITE`, `BOOST_AUTO_TEST_CASE`, `BOOST_AUTO_TEST_SUITE_END`.

### 4. `tests/common/UnitTests/CMakeLists.txt`

`Catch2::Catch2WithMain` zur Link-Liste hinzugefügt.

### 5. `tests/common/ManualTests/GFormulaParserTest/GFormulaParserTest.cpp`

Komplett neu geschrieben:
- Boost.Test-Defines und -Includes entfernt
- Catch2-Header: `<catch2/catch_test_macros.hpp>` und `<catch2/matchers/catch_matchers_floating_point.hpp>`
- Boost-Non-Test-Includes beibehalten: `boost/math/constants`, `boost/assign/list_of`
- `BOOST_AUTO_TEST_CASE(formula_parser_tests)` → `TEST_CASE("formula_parser_tests", "[common][manual]")`
- Makro `testFormula`: `BOOST_CHECK_CLOSE(parse_val, fp_val, 0.001)` → `CHECK_THAT(parse_val, Catch::Matchers::WithinRel(fp_val, 0.001/100.0))` (Boost-Eps ist Prozent, Catch2 ist Faktor)
- Makro `testFormulaFailure`: `BOOST_CHECK_THROW` → `CHECK_THROWS_AS`
- Alle 6 inline `BOOST_CHECK_CLOSE`-Aufrufe ebenfalls auf `CHECK_THAT(...WithinRel...)` umgestellt

### 6. `tests/common/ManualTests/GFormulaParserTest/CMakeLists.txt`

`Catch2::Catch2WithMain` zur Link-Liste hinzugefügt.

---

## Build-Ergebnis

```
make -j$(nproc) GCommonStandardTests GFormulaParserTest
→ [100%] Built target GCommonStandardTests
→ [100%] Built target GFormulaParserTest
```

Beide Targets ohne Fehler und Warnings.

---

## Test-Ergebnisse

| Executable | Assertions | Test Cases | Ergebnis |
|---|---|---|---|
| `GCommonStandardTests` | 1.390.118 | 2 | ✅ All tests passed |
| `GFormulaParserTest` | 67 | 1 | ✅ All tests passed |

---

## Akzeptanzkriterien

- [x] Keine `boost/test/...`-Includes mehr in den migrierten Dateien
- [x] `grep -r "BOOST_" tests/common/ include/common/tests/` → keine Treffer
- [x] `GCommonStandardTests` baut und alle Test-Cases grün
- [x] `GFormulaParserTest` baut und alle Test-Cases grün

---

## Geänderte Dateien

| Datei | Änderungsart |
|---|---|
| `include/common/tests/GBoundedBufferT_tests.hpp` | 288 BOOST_ → Catch2 |
| `tests/common/UnitTests/GCommonStandardTests.cpp` | Komplett neu geschrieben |
| `tests/common/UnitTests/CMakeLists.txt` | Catch2::Catch2WithMain hinzugefügt |
| `tests/common/ManualTests/GFormulaParserTest/GFormulaParserTest.cpp` | Komplett neu geschrieben |
| `tests/common/ManualTests/GFormulaParserTest/CMakeLists.txt` | Catch2::Catch2WithMain hinzugefügt |

---

## Nächster Schritt

**Phase 2:** Bibliothek `hap` migrieren — bereits abgeschlossen, siehe `phase2.md`.
