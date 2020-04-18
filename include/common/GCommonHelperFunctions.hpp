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
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <vector>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <typeinfo>
#include <tuple>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <algorithm>

// Boost headers go here
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/spirit/include/qi_operator.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_string.hpp>
#include <boost/spirit/include/qi_numeric.hpp>
#include <boost/spirit/include/qi_auxiliary.hpp>
#include <boost/spirit/include/qi_nonterminal.hpp>
#include <boost/spirit/include/qi_action.hpp>
#include <boost/fusion/include/tuple.hpp>
#include <boost/fusion/include/boost_tuple.hpp>
#include <boost/fusion/adapted/std_tuple.hpp> // Compare http://stackoverflow.com/questions/18158376/getting-boostspiritqi-to-use-stl-containers
#include <boost/predef.h>

// Geneva headers go here
#include "common/GCommonEnums.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GErrorStreamer.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
/** @brief Creates a file in a given path, optionally with content */
G_API_COMMON
std::filesystem::file_time_type touch_time(
    std::filesystem::path const& path
    , std::string const& content = ""
    , bool remove_if_not_present = false
);

/******************************************************************************/
/** @brief Reads a json-document from a std::filesystem::path. This is a helper-function */
G_API_COMMON
void read_json(
	std::filesystem::path const& path
	, boost::property_tree::ptree& pt
);

/******************************************************************************/
/** @brief Determines a suitable number of threads for the current architecture */
G_API_COMMON
unsigned int getNHardwareThreads();

/******************************************************************************/
/** @brief This function loads textual (ASCII) data from an external file */
G_API_COMMON
std::string loadTextDataFromFile(std::filesystem::path const &);

/******************************************************************************/
/** @brief This function executes an external command on the operating system */
G_API_COMMON
int runExternalCommand(
	std::filesystem::path const &
	, std::vector<std::string> const &
	, std::filesystem::path const &
	, std::string &
);

/******************************************************************************/
/** @brief Returns a string for a given serialization mode */
G_API_COMMON
std::string serializationModeToString(serializationMode);

/******************************************************************************/
/** @brief Splits a string into a vector of strings, according to a seperator character */
G_API_COMMON
std::vector<std::string> splitString(std::string const &, const char *);

/******************************************************************************/
/** @brief Splits a string into a vector of std::uint16_t values, if possible */
G_API_COMMON
std::vector<unsigned int> stringToUIntVec(std::string const &, char = ',');

/******************************************************************************/
/** @brief Splits a string into a vector of double values, if possible */
G_API_COMMON
std::vector<double> stringToDoubleVec(std::string const &);

/******************************************************************************/
/** @brief Splits a string into a vector of unsigned int-tuples, if possible */
G_API_COMMON
std::vector<std::tuple<unsigned int, unsigned int>> stringToUIntTupleVec(std::string const &);

/******************************************************************************/
/** @brief Translates a string of the type "00:10:30" into a std::chrono::duration<double> */
G_API_COMMON
std::chrono::duration<double> duration_from_string(std::string const&);

/******************************************************************************/
/** @brief Converts the current time to a string */
G_API_COMMON
std::string currentTimeAsString();

/******************************************************************************/
/** @brief Returns the number of milliseconds since 1.1.1970 */
G_API_COMMON
std::string getMSSince1970();

/******************************************************************************/
/** @brief Converts a std::chrono::high_resolution_clock::time_point into an arithmetic number */
G_API_COMMON
std::chrono::milliseconds::rep time_point_to_milliseconds(std::chrono::high_resolution_clock::time_point const&);

/******************************************************************************/
/** @brief Converts an arithmetic number into  a std::chrono::high_resolution_clock::time_point */
G_API_COMMON
std::chrono::high_resolution_clock::time_point milliseconds_to_time_point(std::chrono::milliseconds::rep const&);

/******************************************************************************/
/** @brief Raise an exception if a given define wasn't set */
G_API_COMMON
void condnotset(std::string const&, std::string const&);

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */
