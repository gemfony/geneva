/**
 * @file
 */

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

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <cfloat>
#include <climits>

// Boost headers go here
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GErrorStreamer.hpp"
#include "courtier/GCourtierEnums.hpp"

namespace Gem {
namespace Courtier {

/******************************************************************************/
/** @brief Assembles a query string from a given command */
G_API_COURTIER std::string assembleQueryString(const std::string &, const std::size_t &);

/** @brief Extracts the size of ASIO's data section from a C string. */
G_API_COURTIER std::size_t extractDataSize(const char *, const std::size_t &);

/** @brief Cleanly shuts down a socket */
G_API_COURTIER void disconnect(boost::asio::ip::tcp::socket &);

/** @brief Create a boolean mask */
G_API_COURTIER std::vector<bool> getBooleanMask(
	std::size_t vecSize
	, std::size_t start
	, std::size_t end
);

/** @brief Translate the processingStatus into a clear-text string */
G_API_COURTIER std::string psToStr(const processingStatus&);

/** @brief Translate the networked_consumer_payload_command into a clear-text string */
G_API_COURTIER std::string pcToStr(const networked_consumer_payload_command&);

/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */
