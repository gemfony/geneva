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

// Standard headers go here
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <typeinfo>
#include <vector>

// Boost headers go here
#include <boost/property_tree/ptree_fwd.hpp>

// Geneva headers go here
#include "common/GCommonEnums.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"

namespace Gem::Common {

/******************************************************************************/
/** @brief Creates a file in a given path, optionally with content */

std::filesystem::file_time_type touch_time(
    std::filesystem::path const &path,
    std::string const &content = "",
    bool remove_if_not_present = false
);

/******************************************************************************/
/** @brief Reads a json-document from a std::filesystem::path. This is a helper-function */

void read_json(std::filesystem::path const &path, boost::property_tree::ptree &pt);

/******************************************************************************/
/** @brief Determines a suitable number of threads for the current architecture */

unsigned int getNHardwareThreads();

/******************************************************************************/
/** @brief This function loads textual (ASCII) data from an external file */

std::string loadTextDataFromFile(std::filesystem::path const &);

/******************************************************************************/
/** @brief This function loads textual (ASCII) data from an external file, putting it line by line into a std::vector */

std::vector<std::string> loadTextLinesFromFile(std::filesystem::path const &);

/******************************************************************************/
/** @brief This function executes an external command on the operating system */

int runExternalCommand(
    std::filesystem::path const &,
    std::vector<std::string> const &,
    std::filesystem::path const &,
    std::string &
);

/******************************************************************************/
/**
 * @brief Returns a textual label for a given serialization mode.
 *
 * constexpr / string_view: the labels are fixed strings, so a constexpr
 * lookup avoids per-call std::string allocation. Returning string_view
 * lets call sites stream the result directly (it implicitly converts to
 * std::string where one is required).
 */
constexpr std::string_view serializationModeToString(serializationMode s) noexcept {
    switch(s) {
    case serializationMode::TEXT:   return "text mode";
    case serializationMode::XML:    return "XML mode";
    case serializationMode::BINARY: return "binary mode";
    }
    return {}; // unreachable for valid enumerator inputs
}

/******************************************************************************/
/** @brief Splits a string into a vector of strings, according to a seperator character */

std::vector<std::string> splitString(std::string const &, const char *);

/******************************************************************************/
/** @brief Splits a string into a vector of std::uint16_t values, if possible */

std::vector<unsigned int> stringToUIntVec(std::string const &, char = ',');

/******************************************************************************/
/** @brief Splits a string into a vector of double values, if possible */

std::vector<double> stringToDoubleVec(std::string const &);

/******************************************************************************/
/** @brief Splits a string into a vector of unsigned int-tuples, if possible */

std::vector<std::tuple<unsigned int, unsigned int>> stringToUIntTupleVec(std::string const &);

/******************************************************************************/
/** @brief Translates a string of the type "00:10:30" into a std::chrono::duration<double> */

std::chrono::duration<double> duration_from_string(std::string const &);

/******************************************************************************/
/** @brief Converts the current time to a string */

std::string currentTimeAsString();

/******************************************************************************/
/** @brief Returns the number of milliseconds since 1.1.1970 */

std::string getMSSince1970();

/******************************************************************************/
/** @brief Converts a std::chrono::high_resolution_clock::time_point into an arithmetic number */

std::chrono::milliseconds::rep
time_point_to_milliseconds(std::chrono::high_resolution_clock::time_point const &);

/******************************************************************************/
/** @brief Converts an arithmetic number into  a std::chrono::high_resolution_clock::time_point */

std::chrono::high_resolution_clock::time_point
milliseconds_to_time_point(std::chrono::milliseconds::rep const &);

/******************************************************************************/
/** @brief Raise an exception if a given define wasn't set */

void condnotset(std::string const &, std::string const &);

/******************************************************************************/

} /* namespace Gem::Common */
