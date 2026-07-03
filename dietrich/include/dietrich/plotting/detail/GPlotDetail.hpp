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
 * NOTE THAT THE LICENSE MENTIONED ABOVE APPLIES TO THIS FILE ONLY, and not to
 * any other files in this repository, unless explicitly stated otherwise.
 *
 * For further information on Gemfony scientific, see http://www.gemfony.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020, with copyright by Gemfony scientific.
 *
 ********************************************************************************/

#pragma once

/**
 * @file GPlotDetail.hpp
 * @brief Library-private helpers shared by the plotting implementation files (GPlotDesigner.cpp, the
 * plot-type implementations GGraphPlots.cpp / GHistogramPlots.cpp, and the per-backend emitters
 * GRootEmitter.cpp / GGnuplotEmitter.cpp / ...). These are implementation details of the dietrich
 * plotting library: the header lives under a `detail/` subdirectory and is deliberately NOT part of
 * the installed public API (it is not in the INSTALL list) -- do not include it from outside the
 * dietrich sources. It was extracted from the former monolithic GPlotDesigner.cpp when that was split
 * into several translation units, so every plotting TU shares one definition.
 */

#include "dietrich/plotting/GBasePlotter.hpp" // GPlotColumn

#include <cstddef>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <string>
#include <variant>

namespace Gem::Dietrich {

/******************************************************************************/
/**
 * A locale-independent, full-round-trip-precision string builder for emitted ROOT macro text.
 * Every coordinate value must be emitted through such a stream: a default std::ostringstream prints
 * only 6 significant digits (lossy for optimization traces) and honours the global locale (a
 * comma-decimal locale would emit "12,5", which ROOT misparses as two arguments). Imbuing the
 * classic ("C") locale and setting max_digits10 precision once, in the constructor, fixes both for
 * all subsequent insertions.
 */
class EmitStream : public std::ostringstream {
public:
    EmitStream() {
        this->imbue(std::locale::classic());
        *this << std::setprecision(std::numeric_limits<double>::max_digits10);
    }
};

/******************************************************************************/
/**
 * Escape a user-supplied string for safe inclusion inside a ROOT C-string literal (axis/plot/canvas
 * labels, TF1/TF2 formulae) or a // comment. Without this, a label containing a double quote,
 * backslash or newline would produce a non-compiling macro (or, inside a comment, inject code past
 * the comment). Only the C-string metacharacters are touched, so ROOT TLatex markup (#frac, #sqrt,
 * braces) passes through unchanged.
 */
inline std::string rootEscape(const std::string &in) {
    std::string out;
    out.reserve(in.size());
    for(char c : in) {
        switch(c) {
            case '\\': out += "\\\\"; break;
            case '"':  out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            case '\r':                break; // drop bare CRs
            default:   out += c;      break;
        }
    }
    return out;
}

/******************************************************************************/
/**
 * Escape a user-supplied string for safe inclusion inside a gnuplot DOUBLE-quoted
 * string literal (labels, titles). gnuplot double-quoted strings interpret C-style
 * backslash escapes (\" \\ \n \t), so the metacharacters must be backslash-escaped
 * exactly as for those literals -- this is deliberately NOT rootEscape (which targets
 * a ROOT C-string literal and additionally drops bare CRs / has its own contract).
 */
inline std::string gnuplotEscape(const std::string &in) {
    std::string out;
    out.reserve(in.size());
    for(char c : in) {
        switch(c) {
            case '\\': out += "\\\\"; break;
            case '"':  out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            case '\r': out += "\\r"; break;
            default:   out += c;      break;
        }
    }
    return out;
}

/******************************************************************************/
/**
 * Escape a user-supplied string for safe inclusion inside a Python DOUBLE-quoted
 * string literal (labels, titles). Python double-quoted literals interpret C-style
 * backslash escapes (\" \\ \n \t \r), so the metacharacters are backslash-escaped
 * exactly as for those literals -- this is deliberately NEITHER rootEscape (ROOT
 * C-string contract, drops bare CRs) NOR gnuplotEscape (which targets a gnuplot
 * double-quoted literal). The result is always inserted between two `"` delimiters.
 */
inline std::string pythonEscape(const std::string &in) {
    std::string out;
    out.reserve(in.size());
    for(char c : in) {
        switch(c) {
            case '\\': out += "\\\\"; break;
            case '"':  out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            case '\r': out += "\\r"; break;
            default:   out += c;      break;
        }
    }
    return out;
}

/******************************************************************************/
/**
 * @brief The number of elements in a column (visits whichever value vector it holds). Shared by the
 * gnuplot and DATA backends, so it lives here rather than in one backend's translation unit.
 */
inline std::size_t columnSize(const GPlotColumn &c) {
    return std::visit([](const auto *v) { return v->size(); }, c);
}

/******************************************************************************/

} // namespace Gem::Dietrich
