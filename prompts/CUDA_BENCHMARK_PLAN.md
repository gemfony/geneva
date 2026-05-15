# Plan: GPU-beschleunigter Algorithmen-Vergleichsbenchmark

Zieldatum: offen
Branch: catch2-migration (oder separater Branch)
Zuständig: Ruediger Berlich

---

## Ziel

Ein konfigurierbares Benchmark-Executable, das

- eine wählbare Zahl an Runs mehrerer Optimierungsalgorithmen (EA, SA, Swarm, GD)
  über eine wählbare Benchmark-Funktion durchführt,
- die Runs anhand definierter Abbruchkriterien terminiert (Iterationen, Stalls, Ziel-Fitness),
- die Ergebnisse statistisch aggregiert (Mittelwert ± Standardabweichung via `GStandardDeviation`)
  und in einer maschinenlesbaren Form abspeichert, die den Vergleich verschiedener
  Algorithmen — oder des gleichen Algorithmus unter verschiedenen Bedingungen — erlaubt,
- und dazu eine CUDA-fähige GPU nutzt, indem ganze Populationen als Batch auf die GPU
  geschickt werden (Speedup durch Amortisierung des GPU-Launch-Overheads).

---

## Verzeichnis

**Alles** liegt in einem einzigen Verzeichnis:

```
benchmarks/geneva/GCUDAOptBenchmark/
```

Das bisherige Verzeichnis `benchmarks/geneva/GBenchmarkCUDA/` wird per `git mv` umbenannt
und erweitert. **Es werden keine Dateien außerhalb dieses Verzeichnisses geändert.**

---

## Vorhandene Infrastruktur (bleibt erhalten, wird erweitert)

| Datei | Status | Inhalt |
|---|---|---|
| `GBenchmarkBatchEvaluator.cuh/.cu` | bleibt, ggf. Erweiterung | CUDA-Kernel, `GBenchmarkCUDAContext` (persistente Device-Buffer, lazy realloc), `batchEvalBenchmarkGPU()` |
| `GBenchmarkCUDAConsumer.hpp` | Umbau | wird zur echten Consumer-Klasse `GCUDABatchConsumer` |
| `GBenchmarkCUDATest.cpp` | bleibt | Validierungstest CPU vs. GPU, 15 Funktionen |
| `CMakeLists.txt` | erweitert | neues Target für Benchmark-Executable |

Genutzte Geneva-Infrastruktur (nur lesend, keine Änderungen):

| Was | Wo |
|---|---|
| `GStandardDeviation<double>(vec)` → `tuple<mean, sigma>` | `include/common/GCommonHelperFunctionsT.hpp` |
| Abbruchkriterien: `setMaxIteration`, `setStallCounterThreshold`, `setQualityThreshold` | `G_OptimizationAlgorithm_Base` |
| Abfrage nach Terminierung: `getBestFitness`, `getIteration`, `stallCounterThresholdExceeded`, `qualityThresholdMet` | ebd. |
| `GFitnessMonitor` (ROOT-Plot der Konvergenz, optional) | `include/geneva/GPluggableOptimizationMonitors.hpp` |
| `GOptimizationBenchmark` (strukturelles Vorbild) | `benchmarks/geneva/GOptimizationBenchmark/` |
| `GGraph2ED` (Plots mit Fehlerbalken) | `include/common/GPlotDesigner.hpp` |
| `parameterset_processing_result(double)` | `include/geneva/GParameterSet.hpp` |
| `GBaseConsumerT`, `GSerialConsumerT` (Vorbild für Consumer-Struktur) | `include/courtier/` |

**Orientierung an Beispiel 15 (`examples/geneva/15_GCUDAWorker/`):**

`GImageIndividualEvaluator::evaluate()` (Zeile 654) zeigt die Lösung für das
`process()`-Problem: statt `process()` ohne Argumente (würde intern `fitnessCalculation()`
aufrufen) wird das Overload mit vorberechneten Ergebnissen genutzt:

```cpp
individual->process(
    std::vector<parameterset_processing_result>(1,
        parameterset_processing_result(gpuComputedFitness))
);
```

`GFunctionIndividual` wird dadurch **nicht verändert** und bleibt auf CPU-losen Rechnern
voll lauffähig.

---

## Neue Dateien in `GCUDAOptBenchmark/`

```
GCUDAOptBenchmark/
  CMakeLists.txt                      ← erweitert
  GBenchmarkBatchEvaluator.cuh        ← bleibt
  GBenchmarkBatchEvaluator.cu         ← bleibt / ggf. erweitert
  GBenchmarkCUDAConsumer.hpp          ← Umbau zu GCUDABatchConsumer
  GBenchmarkCUDATest.cpp              ← bleibt (Validierungstest)
  GBenchmarkRunResult.hpp             ← NEU: Datenmodell
  GAlgorithmBenchmarkRunner.hpp       ← NEU: Benchmark-Engine
  GAlgorithmBenchmarkRunner.cpp       ← NEU
  GBenchmarkResultWriter.hpp          ← NEU: CSV/JSON-Ausgabe
  GBenchmarkResultWriter.cpp          ← NEU
  GCUDAOptBenchmark.cpp               ← NEU: Haupt-Executable
  config/
    GCUDAOptBenchmark.json            ← NEU: Benchmark-Hauptconfig
    Go2.json                          ← NEU (oder kopiert)
    GFunctionIndividual.json          ← NEU (oder kopiert)
    GEvolutionaryAlgorithm.json       ← NEU (oder kopiert)
    GSimulatedAnnealing.json          ← NEU (oder kopiert)
    GSwarmAlgorithm.json              ← NEU (oder kopiert)
    GGradientDescent.json             ← NEU (oder kopiert)
```

---

## Phase 1 — Umbenennung und GPU Batch Consumer

**Schritt 1:** `git mv benchmarks/geneva/GBenchmarkCUDA benchmarks/geneva/GCUDAOptBenchmark`

**Schritt 2:** `GBenchmarkCUDAConsumer.hpp` zum echten Broker-Consumer umbauen.

Neue Klasse `GCUDABatchConsumer : public GBaseConsumerT<GFunctionIndividual>`,
orientiert an `GSerialConsumerT` als strukturellem Vorbild:

- Interne Worker-Loop (ein Thread, gestartet in `async_startProcessing_()`):
  1. Zieht `shared_ptr<GFunctionIndividual>` aus dem Broker-Get-Buffer (mit kurzem Timeout)
  2. Akkumuliert bis `batchSize_` Stück erreicht **oder** Flush-Timeout abläuft
  3. Ruft `GBenchmarkCUDAContext::eval()` für den kompletten Batch auf
  4. Für jedes Individual i: `ind->process({parameterset_processing_result(fitness[i])})`
  5. Gibt alle Individuen via Broker-Put-Buffer zurück

- Konfigurierbar: `batchSize_` (0 = beim ersten Flush aus Populationsgröße ermitteln),
  GPU Device ID, Flush-Timeout (Default: 50 ms)

**Kein Fallback:** `GCUDAOptBenchmark` wird nur übersetzt wenn CUDA verfügbar ist.
Das wird in `CMakeLists.txt` analog zu Beispiel 15 mit `if(USECUDARNG)` (oder dem
entsprechenden CUDA-Check) abgesichert. Auf Rechnern ohne CUDA-fähige GPU entfällt
das Target vollständig — `GFunctionIndividual` selbst bleibt davon unberührt.

---

## Phase 2 — Datenmodell

**Neue Datei:** `GBenchmarkRunResult.hpp`

```cpp
enum class TerminationReason { MaxIterations, StallThreshold, QualityThreshold, Unknown };

struct GBenchmarkRunResult {
    std::string algorithmTag;      // "ea", "sa", "swarm", "gd"
    std::string functionName;      // z.B. "PARABOLA"
    std::uint32_t nDimensions;
    std::uint32_t runIndex;
    double finalFitness;
    std::uint32_t iterationsConsumed;
    double wallTimeSeconds;
    TerminationReason terminationReason;
    bool targetReached;
};

struct GAlgorithmBenchmarkResult {
    std::string algorithmTag;
    std::string functionName;
    std::uint32_t nDimensions;
    std::size_t nRuns;
    double meanFinalFitness;  double sigmaFinalFitness;
    double meanIterations;    double sigmaIterations;
    double meanWallTime;      double sigmaWallTime;
    double successRate;       // Anteil Runs mit targetReached == true
};
```

---

## Phase 3 — Multi-Algorithmus Benchmark-Engine

**Neue Dateien:** `GAlgorithmBenchmarkRunner.hpp/.cpp`

**Designentscheidungen:**
- Abbruchkriterien stehen in den Algorithmus-spezifischen JSON-Dateien und werden von Geneva
  selbst gelesen. Algorithmus-spezifische Einstellungen (Populationsgröße, Mutationsrate usw.)
  bleiben dort ebenfalls konfigurierbar.
- Pro Algorithmus eine eigene Ergebnis-CSV in vergleichbarem Format.
- Nur eine Benchmark-Funktion pro Lauf.
- Alle Runs — auch Vergleiche zwischen mehreren Algorithmen oder zwischen verschiedenen
  Settings desselben Algorithmus — laufen **sequenziell**. Das gilt auch für den Vergleich
  z.B. verschiedener Populationsgrößen desselben EA.
- Es muss möglich sein, **einen einzelnen Algorithmus** zu selektieren und ohne Vergleich
  zu laufen.

```
GAlgorithmBenchmarkRunner::run(config) → vector<GAlgorithmBenchmarkResult>
```

Für jeden Algorithmus-Eintrag in `config.algorithmConfigs` (sequenziell):
```
for each algorithmConfig in config.algorithmConfigs:   // kann auch nur ein Eintrag sein
    rawResults = []
    for run in 0..nRuns:
        go = new Go2(argc, argv, algorithmConfig.configFile)
        if cudaAvailable:
            enrol GCUDABatchConsumer
        else:
            enrol GStdThreadConsumerT
        go.push_back(individualFromFactory)
        startTime = chrono::now()
        go.optimize()
        endTime = chrono::now()
        rawResults.push_back(GBenchmarkRunResult {
            algorithmTag  = algorithmConfig.tag,
            finalFitness  = go.getBestFitness(),
            iterations    = go.getIteration(),
            wallTime      = duration(endTime - startTime),
            termination   = aus stallCounterThresholdExceeded() / qualityThresholdMet()
        })

    aggregiere rawResults mit GStandardDeviation<double>
    → GAlgorithmBenchmarkResult für diesen Algorithmus
    → schreibe sofort Einzel-CSV (Phase 4)

→ return vector<GAlgorithmBenchmarkResult>
```

`algorithmConfig.tag` ist ein frei wählbarer Name, z.B. `"ea_pop100"` oder `"sa_default"`,
der im Dateinamen der Ergebnis-CSV erscheint. So lassen sich auch Runs desselben Algorithmus
mit unterschiedlichen Settings unterscheiden.

Optionaler `GFitnessMonitor` pro Run für ROOT-Konvergenzplots.

---

## Phase 4 — Datenspeicherung & Vergleich

**Neue Dateien:** `GBenchmarkResultWriter.hpp/.cpp`

Pro Algorithmus-Eintrag eine eigene CSV, alle im gleichen Format — dadurch direkt
vergleichbar, unabhängig ob verschiedene Algorithmen oder derselbe Algorithmus mit
verschiedenen Settings verglichen wird:

| Datei | Inhalt |
|---|---|
| `raw_<Tag>_<Funktion>_<Datum>.csv` | eine Zeile pro Run (Rohdaten, wird während des Laufs geschrieben) |
| `convergence_<Tag>_<Funktion>_<Datum>.C` (optional) | ROOT-Plot via `GGraph2ED` |

**CSV-Spalten (identisch für alle Tags):**
```
run_index, algorithm_tag, function, nDims, final_fitness,
iterations, wall_time_s, termination_reason, target_reached
```

Die Aggregation (Mittelwert ± σ) wird nach Abschluss aller Runs aus den Rohdaten berechnet
und auf stdout ausgegeben sowie optional in eine `summary_<Datum>.csv` geschrieben, die
alle Tags nebeneinander enthält — das ist die Vergleichstabelle.

---

## Phase 5 — Konfiguration, CMake, Hauptprogramm

**`GCUDAOptBenchmark.json` (Entwurf):**
```json
{
  "benchmarkFunction": "PARABOLA",
  "nRuns":             30,
  "gpu": {
    "enabled":         true,
    "deviceId":        0,
    "batchSize":       0,
    "flushTimeoutMs":  50
  },
  "output": {
    "writeRawCSV":     true,
    "writeSummaryCSV": true,
    "writeRootPlots":  false
  },
  "algorithmConfigs": [
    { "tag": "ea_default",   "configFile": "config/GEvolutionaryAlgorithm.json" },
    { "tag": "sa_default",   "configFile": "config/GSimulatedAnnealing.json"    },
    { "tag": "swarm_default","configFile": "config/GSwarmAlgorithm.json"        }
  ]
}
```

Ein einzelner Algorithmus läuft, indem `algorithmConfigs` nur einen Eintrag enthält.
Derselbe Algorithmus mit verschiedenen Settings erscheint als zwei Einträge mit
verschiedenen Tags und verschiedenen Config-Dateien, z.B.:
```json
"algorithmConfigs": [
  { "tag": "ea_pop50",  "configFile": "config/GEvolutionaryAlgorithm_pop50.json"  },
  { "tag": "ea_pop200", "configFile": "config/GEvolutionaryAlgorithm_pop200.json" }
]
```

`GFunctionIndividual.json` und `Go2.json` sind gemeinsame Configs für alle Algorithmen.

**CMake:** Das Target wird nur erzeugt wenn CUDA verfügbar ist, analog zu Beispiel 15.
Auf Rechnern ohne CUDA wird `GCUDAOptBenchmark` stillschweigend übersprungen.

---

## Phasenabhängigkeiten

```
Phase 1 (git mv + GPU Consumer)     Phase 2 (Datenmodell)
              \                             /
               \                           /
                Phase 3 (Benchmark Engine)
                          |
                Phase 4 (Result Writer)
                          |
                Phase 5 (CMake + Driver)
```

Phase 1 und Phase 2 sind voneinander unabhängig.

---

## Entscheidungen (festgelegt)

1. **Abbruchkriterien** in den Algorithmus-JSON-Dateien, von Geneva selbst gelesen.
2. **Pro Algorithmus-Tag eine eigene CSV**, einheitliches Format für alle Tags.
3. **Eine Benchmark-Funktion pro Lauf.**
4. **Sequenzielle Runs** — auch beim Vergleich mehrerer Algorithmen oder Einstellungen.
