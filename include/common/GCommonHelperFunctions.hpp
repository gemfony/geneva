/**
 * @file GCommonHelperFunctions.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <vector>
#include <sstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <fstream>
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
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/tokenizer.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/tuple.hpp>
#include <boost/fusion/include/boost_tuple.hpp>
#include <boost/fusion/adapted/std_tuple.hpp> // Compare http://stackoverflow.com/questions/18158376/getting-boostspiritqi-to-use-stl-containers
#include <boost/predef.h>

#ifndef GCOMMONHELPERFUNCTIONS_HPP_
#define GCOMMONHELPERFUNCTIONS_HPP_

// Geneva headers go here
#include "common/GCommonEnums.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GErrorStreamer.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
/** @brief This function tries to determine a suitable number of threads for the current architecture */
G_API_COMMON
unsigned int getNHardwareThreads(
	const unsigned int& defaultNThreads = Gem::Common::DEFAULTNHARDWARETHREADS
	, const unsigned int& maxNThreads = 0
);

/******************************************************************************/
/** @brief This function loads textual (ASCII) data from an external file */
G_API_COMMON
std::string loadTextDataFromFile(const boost::filesystem::path &);

/******************************************************************************/
/** @brief This function executes an external command on the operating system */
G_API_COMMON
int runExternalCommand(
	const boost::filesystem::path &
	, const std::vector<std::string> &
	, const boost::filesystem::path &
	, std::string &
);

/******************************************************************************/
/** @brief Returns a string for a given serialization mode */
G_API_COMMON
std::string serializationModeToString(const serializationMode &);

/******************************************************************************/
/** @brief Splits a string into a vector of strings, according to a seperator character */
G_API_COMMON
std::vector<std::string> splitString(const std::string &, const char *);

/******************************************************************************/
/** @brief Splits a string into a vector of std::uint16_t values, if possible */
G_API_COMMON
std::vector<unsigned int> stringToUIntVec(const std::string &, char = ',');

/******************************************************************************/
/** @brief Splits a string into a vector of double values, if possible */
G_API_COMMON
std::vector<double> stringToDoubleVec(const std::string &);

/******************************************************************************/
/** @brief Splits a string into a vector of unsigned int-tuples, if possible */
G_API_COMMON
std::vector<std::tuple<unsigned int, unsigned int>> stringToUIntTupleVec(const std::string &);

/******************************************************************************/
/** @brief Translates a string of the type "00:10:30" into a std::chrono::duration<double> */
G_API_COMMON
std::chrono::duration<double> duration_from_string(const std::string&);

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
std::chrono::milliseconds::rep time_point_to_milliseconds(const std::chrono::high_resolution_clock::time_point&);

/******************************************************************************/
/** @brief Converts an arithmetic number into  a std::chrono::high_resolution_clock::time_point */
G_API_COMMON
std::chrono::high_resolution_clock::time_point milliseconds_to_time_point(const std::chrono::milliseconds::rep&);

/******************************************************************************/
/** @brief Raise an exception if a given define wasn't set */
G_API_COMMON
void condnotset(const std::string&, const std::string&);

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GCOMMONHELPERFUNCTIONS_HPP_ */
