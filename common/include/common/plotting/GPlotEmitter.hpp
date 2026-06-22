/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

#include <string>

namespace Gem::Common {

/******************************************************************************/

class GPlotDesigner; // forward declaration

/******************************************************************************/
/**
 * The set of backends a GPlotDesigner can emit through. ROOT (the historical
 * default) generates a ROOT macro; GNUPLOT generates a gnuplot script for the
 * graph plotters.
 */
enum class plotBackend {
    ROOT,      ///< Emit a ROOT macro (the default; output is byte-identical to the historical generator)
    GNUPLOT,   ///< Emit a gnuplot script (graph plotters only)
    MATPLOTLIB ///< Emit a Python/matplotlib script (graph plotters and histograms)
};

/******************************************************************************/
/**
 * The strategy interface that separates backend-specific document emission from
 * the data collection performed by GPlotDesigner / the GBasePlotter hierarchy. A
 * concrete emitter turns a fully-populated GPlotDesigner into a backend-specific
 * script (a ROOT macro, a gnuplot script, ...).
 */
class IPlotEmitter {
public:
    /** @brief The (defaulted) virtual destructor */
    virtual ~IPlotEmitter() = default;

    /**
     * @brief Emits the complete backend document for a populated designer
     * @param gpd The designer holding the plotters and canvas configuration
     * @return The complete backend-specific document as a string
     */
    [[nodiscard]] virtual std::string emitDocument(const GPlotDesigner &gpd) const = 0;

    /**
     * @brief The conventional file-name extension for this backend's documents
     * @return The extension, including the leading dot (e.g. ".C" or ".gp")
     */
    [[nodiscard]] virtual std::string fileExtension() const = 0;
};

/******************************************************************************/
/**
 * The ROOT backend. emitDocument() reproduces the historical GPlotDesigner ROOT
 * macro byte-for-byte: a TCanvas, a TPaveLabel title, a Divide()'d TPad and the
 * per-plotter Header / Body / Footer sections.
 */
class GRootEmitter : public IPlotEmitter {
public:
    /**
     * @brief Emits the complete ROOT macro for a populated designer
     * @param gpd The designer holding the plotters and canvas configuration
     * @return The complete ROOT macro source as a string
     */
    [[nodiscard]] std::string emitDocument(const GPlotDesigner &gpd) const override;

    /**
     * @brief The ROOT-macro file extension
     * @return The string ".C"
     */
    [[nodiscard]] std::string fileExtension() const override;
};

/******************************************************************************/
/**
 * The gnuplot backend. emitDocument() supports ONLY the graph plotters
 * (GGraph2D / GGraph2ED / GGraph3D / GGraph4D); any other plotter type triggers a
 * clear geneva_exception directing the caller to the ROOT backend. The emitted
 * script is terminal-agnostic (a validity harness prepends its own `set terminal`
 * / `set output`), laid out as a `set multiplot` grid of per-pad plots.
 */
class GnuplotEmitter : public IPlotEmitter {
public:
    /**
     * @brief Emits the complete gnuplot script for a populated designer
     * @param gpd The designer holding the graph plotters and canvas configuration
     * @return The complete gnuplot script as a string
     */
    [[nodiscard]] std::string emitDocument(const GPlotDesigner &gpd) const override;

    /**
     * @brief The gnuplot-script file extension
     * @return The string ".gp"
     */
    [[nodiscard]] std::string fileExtension() const override;
};

/******************************************************************************/
/**
 * The matplotlib backend. emitDocument() supports the graph plotters
 * (GGraph2D / GGraph2ED / GGraph3D / GGraph4D) and the histogram plotters
 * (GHistogram1D / GHistogram2D); any other plotter type (e.g. a function plotter)
 * triggers a clear geneva_exception directing the caller to the ROOT backend. The
 * emitted script is a self-contained Python program that selects the headless Agg
 * backend, builds a `fig` with one Axes per pad and plots into it, but deliberately
 * ends WITHOUT `fig.savefig(...)` so it stays terminal-agnostic -- a validity
 * harness (or the caller) appends its own `fig.savefig('out.png')`.
 */
class MatplotlibEmitter : public IPlotEmitter {
public:
    /**
     * @brief Emits the complete matplotlib (Python) script for a populated designer
     * @param gpd The designer holding the plotters and canvas configuration
     * @return The complete matplotlib script as a string
     */
    [[nodiscard]] std::string emitDocument(const GPlotDesigner &gpd) const override;

    /**
     * @brief The matplotlib-script file extension
     * @return The string ".py"
     */
    [[nodiscard]] std::string fileExtension() const override;
};

/******************************************************************************/

} /* namespace Gem::Common */
