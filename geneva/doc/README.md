# geneva/doc

Per-library documentation directory for the **Geneva** library
(the largest of the four; core optimisation: `Go2` driver, `GParameterSet`
workflow, the seven optimisation algorithms -- EA, SA, Swarm, GD, CGD,
Nelder-Mead, ParameterScan -- per-category `GCommonInterfaceT<Root>` CRTP
roots, individuals, pluggable monitors; absorbed the former
geneva-individuals library in 1.12.0).

## Building the docs

```sh
cd /your/build && make doc-geneva      # this library only
cd /your/build && make doc             # all four libraries (aggregator)
```

`doc-geneva` automatically depends on `doc-common`, `doc-hap`, and
`doc-courtier` so all three upstream tag files exist for cross-references.

## Output location

Generated HTML / XML / tag files land in the **build tree**:

```
<build>/geneva/doc/
    Doxyfile         configured from Doxyfile.in
    geneva.tag       Doxygen tag file (no downstream consumer in-tree;
                     handy for external projects that build against the
                     installed Geneva and want cross-refs into the docs)
    html/            browsable HTML reference
    xml/             XML for downstream tooling
```

## What lives here

- `Doxyfile.in`     -- per-library Doxygen configuration template
- `CMakeLists.txt`  -- registers the `doc-geneva` target and its
                       `doc-common` + `doc-hap` + `doc-courtier`
                       dependencies
- `README.md`       -- this file

Reserved for future Geneva-library manuals: the `Go2` workflow tutorial,
the algorithm-chaining example walkthrough, the per-category CRTP-roots
design note, the personality-traits dispatch model, the pluggable-monitor
extension guide, JSON config-file reference, etc.

## Settings notes

The Doxyfile is intentionally minimal: only keys that genuinely differ
from Doxygen's defaults are set. `WARN_AS_ERROR=NO` and
`WARN_IF_UNDOCUMENTED=NO` because Geneva is not fully Doxygenised yet --
focused doc-error warnings still fire and should be addressed.
