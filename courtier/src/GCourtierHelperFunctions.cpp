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

#include "courtier/GCourtierHelperFunctions.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "courtier/GCourtierEnums.hpp"
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/error_code.hpp>
#include <cstddef>
#include <iomanip>
#include <ios>
#include <sstream>
#include <string>
#include <vector>


namespace Gem::Courtier {

/******************************************************************************/
/**
 * @brief Assembles a query string from a given command, emitting a string of a given size.
 *
 * This function is used in conjunction with Boost::Asio. The result is @p query right-justified
 * into a field of width @p sz.
 *
 * @param query The string to be placed into the fixed-width query string
 * @param sz The desired total width of the resulting query string
 * @return The query string
 */
std::string assembleQueryString(const std::string &query, const std::size_t &sz) {
    std::ostringstream query_stream; // NOLINT(cppcoreguidelines-init-variables)
    query_stream << std::setw(Gem::Common::narrow<int>(sz)) << query;
    return query_stream.str();
}

/******************************************************************************/
/**
 * @brief Extracts the size of ASIO's data section from a C string.
 *
 * Used in conjunction with Boost::Asio. See e.g. GAsioTCPClient. The header is parsed as a
 * hexadecimal number; an invalid header triggers a geneva_exception.
 *
 * @param ds The data string holding the (hex-encoded) data size
 * @param sz The number of characters in @p ds to read
 * @return The size of the data section
 */
std::size_t extractDataSize(const char *ds, const std::size_t &sz) {
    std::istringstream is(std::string(ds, sz));
    std::size_t inbound_data_size = 0;
    if(!(is >> std::hex >> inbound_data_size)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In extractDataSize: Got invalid header!" << '\n'
        );
    }

    return inbound_data_size;
}

/******************************************************************************/
/**
 * @brief Cleanly shuts down a socket
 *
 * Performs a bidirectional shutdown (ignoring any error) and then closes the socket.
 *
 * @param socket The socket on which the shutdown and close should be performed
 */
void disconnect(boost::asio::ip::tcp::socket &socket) {
    boost::system::error_code ignore;
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignore);
    socket.close();
}

/******************************************************************************/
/**
 * @brief Create a boolean mask marking a contiguous half-open range as unprocessed.
 *
 * Builds a vector of @p vec_size flags, all initialized to GBC_PROCESSED, then sets the
 * entries in the half-open index range [start, end) to GBC_UNPROCESSED.
 *
 * @param vec_size The total length of the mask vector
 * @param start The first index (inclusive) to mark as unprocessed
 * @param end The index one past the last entry (exclusive) to mark as unprocessed
 * @return A boolean mask with [start, end) set to GBC_UNPROCESSED and the rest GBC_PROCESSED
 */
std::vector<bool> getBooleanMask(std::size_t vec_size, std::size_t start, std::size_t end) {
    std::vector<bool> work_item_pos(vec_size, Gem::Courtier::GBC_PROCESSED);
    for(auto p_it = work_item_pos.begin() + start; p_it != work_item_pos.begin() + end; ++p_it) {
        *p_it = Gem::Courtier::GBC_UNPROCESSED;
    }
    return work_item_pos;
}

/******************************************************************************/
/**
 * @brief Translate the processingStatus into a clear-text string
 *
 * @param ps The processingStatus to be translated into a std::string
 * @return A string representing the processing status (empty string for an unrecognized value)
 */
std::string psToStr(const processingStatus &ps) {
    switch(ps) {
        using enum Gem::Courtier::processingStatus;
    case UNPROCESSED:
        return "UNPROCESSED";

    case DO_PROCESS:
        return "DO_PROCESS";

    case PROCESSED:
        return "PROCESSED";

    case EXCEPTION_CAUGHT:
        return "EXCEPTION_CAUGHT";

    case ERROR_FLAGGED:
        return "ERROR_FLAGGED";
    }

    // Make the compiler happy
    return {};
}

/******************************************************************************/
/**
 * @brief Translates a networked_consumer_payload_command into a clear-text string
 *
 * @param pc The networked_consumer_payload_command to be translated into a std::string
 * @return A string representing the command (empty string for an unrecognized value)
 */
std::string pcToStr(const networked_consumer_payload_command &pc) {
    switch(pc) {
        using enum Gem::Courtier::networked_consumer_payload_command;
    case NONE:
        return "NONE";

    case GETDATA:
        return "GETDATA";

    case NODATA:
        return "NODATA";

    case COMPUTE:
        return "COMPUTE";

    case RESULT:
        return "RESULT";

    case STOP:
        return "STOP";

    case REQUEST_LAYOUT:
        return "REQUEST_LAYOUT";

    case SEND_LAYOUT:
        return "SEND_LAYOUT";
    }

    // Make the compiler happy
    return {};
}

/******************************************************************************/

} /* namespace Gem::Courtier */
