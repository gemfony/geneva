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

#include <cfloat>
#include <climits>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Boost headers go here
#include <boost/asio.hpp>

// Geneva headers go here
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "courtier/GCourtierEnums.hpp"

namespace Gem::Courtier {

/******************************************************************************/
/**
 * @brief Assembles a query string from a given command, right-justified into a fixed-width field.
 * @param query The string to place into the fixed-width query string
 * @param sz The desired total width of the resulting query string
 * @return The query string
 */
std::string assembleQueryString(const std::string &query, const std::size_t &sz);

/**
 * @brief Extracts the size of ASIO's data section from a (hex-encoded) C string.
 * @param ds The data string holding the hex-encoded data size
 * @param sz The number of characters in @p ds to read
 * @return The size of the data section
 */
std::size_t extractDataSize(const char *ds, const std::size_t &sz);

/**
 * @brief Cleanly shuts down (bidirectionally) and closes a socket.
 * @param socket The socket on which the shutdown and close should be performed
 */
void disconnect(boost::asio::ip::tcp::socket &socket);

/**
 * @brief Creates a boolean mask with the half-open range [start, end) marked as unprocessed.
 * @param vec_size The total length of the mask vector
 * @param start The first index (inclusive) to mark as unprocessed
 * @param end The index one past the last entry (exclusive) to mark as unprocessed
 * @return A boolean mask with [start, end) set to GBC_UNPROCESSED and the rest GBC_PROCESSED
 */
std::vector<bool> getBooleanMask(std::size_t vec_size, std::size_t start, std::size_t end);

/**
 * @brief Translates the processingStatus into a clear-text string.
 * @param ps The processingStatus to be translated
 * @return A string representing the processing status (empty for an unrecognized value)
 */
std::string psToStr(const processingStatus &ps);

/**
 * @brief Translates the networked_consumer_payload_command into a clear-text string.
 * @param pc The networked_consumer_payload_command to be translated
 * @return A string representing the command (empty for an unrecognized value)
 */
std::string pcToStr(const networked_consumer_payload_command &pc);

/******************************************************************************/

} /* namespace Gem::Courtier */
