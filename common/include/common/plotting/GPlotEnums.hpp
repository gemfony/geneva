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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

// Boost headers go here
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

// Geneva headers go here
#include "common/GCommonEnums.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "common/GContainerT.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "common/GSerializeTupleT.hpp"
#include "common/GTupleIO.hpp"
#include "common/GTypeTraitsT.hpp"

namespace Gem::Common {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * An enum for some basic colors (to be extended over time)
 */
enum class gColor : ENUMBASETYPE {
    white = 0,
    black = 1,
    red = 2,
    green = 3,
    blue = 4,
    grey = 14 // note the id of this color, compared to preceding values
};

/**
 * @brief Puts a gColor into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param os The output stream the color is written to
 * @param c The gColor item to be streamed out
 * @return A reference to the output stream
 */
std::ostream &operator<<(std::ostream &o, const gColor &x);

/**
 * @brief Reads a gColor item from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param is The input stream the color is read from
 * @param c The gColor item that receives the value read from the stream
 * @return A reference to the input stream
 */
std::istream &operator>>(std::istream &i, gColor &x);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * An enum for basic marker types (to be extended over time)
 */
enum class gMarker : ENUMBASETYPE {
    none = 0,
    openCircle = 4,
    closedCircle = 20,
    closedTriangle = 22,
    openTriangle = 26,
    closedStar = 29,
    openStar = 30
};

/**
 * @brief Puts a gMarker into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param os The output stream the marker is written to
 * @param m The gMarker item to be streamed out
 * @return A reference to the output stream
 */
std::ostream &operator<<(std::ostream &o, const gMarker &x);

/**
 * @brief Reads a gMarker item from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param is The input stream the marker is read from
 * @param m The gMarker item that receives the value read from the stream
 * @return A reference to the input stream
 */
std::istream &operator>>(std::istream &i, gMarker &x);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/*******************************************************************	***********/
/**
 * An enum for basic line styles (to be extended over time)
 */
enum class gLineStyle : ENUMBASETYPE {
    straight = 1,
    shortdashed = 2,
    dotted = 3,
    shortdashdot = 4,
    // Previously also `= 4` (collision with shortdashdot — `>>`/`<<`
    // round-trips would collapse the two names onto a single line style).
    // Moved to the next free slot below shortdashdot/longdashed. No call
    // sites reference `longdashdot` directly, so changing the value is
    // observably a fix rather than a breaking change.
    longdashdot = 5,
    longdashdotdot = 6, ///< maps to ROOT TAttLine style 6
    longdashed = 7
};

/**
 * @brief Puts a gLineStyle into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param os The output stream the line style is written to
 * @param ls The gLineStyle item to be streamed out
 * @return A reference to the output stream
 */
std::ostream &operator<<(std::ostream &o, gLineStyle const &x);

/**
 * @brief Reads a gLineStyle item from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param is The input stream the line style is read from
 * @param ls The gLineStyle item that receives the value read from the stream
 * @return A reference to the input stream
 */
std::istream &operator>>(std::istream &i, gLineStyle &x);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Determines whether a scatter plot or a curve should be recorded
 */
enum class graphPlotMode : ENUMBASETYPE {
    SCATTER = 0,
    CURVE = 1
};

/**
 * @brief Puts a graphPlotMode into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param os The output stream the plot mode is written to
 * @param gpm The graphPlotMode item to be streamed out
 * @return A reference to the output stream
 */
std::ostream &operator<<(std::ostream &o, graphPlotMode const &x);

/**
 * @brief Reads a graphPlotMode item from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param is The input stream the plot mode is read from
 * @param gpm The graphPlotMode item that receives the value read from the stream
 * @return A reference to the input stream
 */
std::istream &operator>>(std::istream &i, graphPlotMode &x);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * An enum for 2D-drawing options
 */
enum class tddropt : ENUMBASETYPE {
    TDEMPTY = 0,
    SURFONE = 1,
    SURFTWOZ = 2,
    SURFTHREE = 3,
    SURFFOUR = 4,
    CONTZ = 5,
    CONTONE = 6,
    CONTTWO = 7,
    CONTTHREE = 8,
    TEXT = 9,
    SCAT = 10,
    BOX = 11,
    ARR = 12,
    COLZ = 13,
    LEGO = 14,
    LEGOONE = 15,
    SURFONEPOL = 16,
    SURFONECYL = 17
};

/**
 * @brief Puts a tddropt into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param os The output stream the 2D-drawing option is written to
 * @param tdo The tddropt item to be streamed out
 * @return A reference to the output stream
 */
std::ostream &operator<<(std::ostream &o, tddropt const &x);

/**
 * @brief Reads a tddropt item from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param is The input stream the 2D-drawing option is read from
 * @param tdo The tddropt item that receives the value read from the stream
 * @return A reference to the input stream
 */
std::istream &operator>>(std::istream &i, tddropt &x);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

//Some default values

constexpr std::uint32_t DEFCXDIM = 1024;
constexpr std::uint32_t DEFCYDIM = 768;

constexpr std::uint32_t DEFCXDIV = 1;
constexpr std::uint32_t DEFCYDIV = 1;

constexpr std::size_t DEFNINDENTIONSPACES = 3;

constexpr std::size_t DEFNSAMPLES = 100;

const graphPlotMode DEFPLOTMODE = graphPlotMode::CURVE;

constexpr double DEFMINMARKERSIZE = 0.001;
constexpr double DEFMAXMARKERSIZE = 1.;

// Easier access to the header-, body- and footer-data
using plotData = std::tuple<std::string, std::string, std::string>;

// Easier acces to lines
using pointData = std::tuple<double, double, double>;
using line = std::tuple<pointData, pointData>;

// Forward declaration in order to allow a friend statement in GBasePlotter
class GPlotDesigner;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////

} /* namespace Gem::Common */
