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
#include "common/GCommonHelperFunctionsT.hpp"


namespace Gem::Courtier {

/******************************************************************************/
/**
 * Assembles a query string from a given command, emitting a string of a given size.
 * This function is used in conjunction with Boost::Asio .
 *
 * @param str The string from which the size should be extracted
 * @param sz Resulting size of the query string
 * @return The query string
 */
std::string assembleQueryString(const std::string &query, const std::size_t &sz) {
    std::ostringstream query_stream; // NOLINT(cppcoreguidelines-init-variables)
    query_stream << std::setw(Gem::Common::narrow_cast<int>(sz)) << query;
    return query_stream.str();
}

/******************************************************************************/
/**
 * Extracts the size of ASIO's data section from a C string.
 * Used in conjunction with Boost::Asio. See e.g. GAsioTCPClient.
 *
 * @param ds The data string holding the data size
 * @param sz The size of the data string
 * @return The size of the data
 */
std::size_t extractDataSize(const char *ds, const std::size_t &sz) {
    std::istringstream is(std::string(ds, sz));
    std::size_t inbound_data_size = 0;
    if(!(is >> std::hex >> inbound_data_size)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In extractDataSize: Got invalid header!" << '\n'
        );
    }

    return inbound_data_size;
}

/******************************************************************************/
/**
 * Cleanly shuts down a socket
 *
 * @param socket The socket on which the shutdown should be performed
 */
void disconnect(boost::asio::ip::tcp::socket &socket) {
    boost::system::error_code ignore;
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignore);
    socket.close();
}

/******************************************************************************/
/**
 * Create a boolean mask
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
 * Translate the processingStatus into a clear-text string
 *
 * @param ps The processingStatus to be translated into a std::string
 * @return A string representing the processing status
 */
std::string psToStr(const processingStatus &ps) {
    switch(ps) {
    case processingStatus::DO_IGNORE:
        return "DO_IGNORE";

    case processingStatus::DO_PROCESS:
        return "DO_PROCESS";

    case processingStatus::PROCESSED:
        return "PROCESSED";

    case processingStatus::EXCEPTION_CAUGHT:
        return "EXCEPTION_CAUGHT";

    case processingStatus::ERROR_FLAGGED:
        return "ERROR_FLAGGED";
    }

    // Make the compiler happy
    return {};
}

/******************************************************************************/
/**
 * Translates anetworked_consumer_payload_command into a clear-text string
 */
std::string pcToStr(const networked_consumer_payload_command &pc) {
    switch(pc) {
    case networked_consumer_payload_command::NONE:
        return "NONE";

    case networked_consumer_payload_command::GETDATA:
        return "GETDATA";

    case networked_consumer_payload_command::NODATA:
        return "NODATA";

    case networked_consumer_payload_command::COMPUTE:
        return "COMPUTE";

    case networked_consumer_payload_command::RESULT:
        return "RESULT";

    case networked_consumer_payload_command::STOP:
        return "STOP";
    }

    // Make the compiler happy
    return {};
}

/******************************************************************************/

} /* namespace Gem::Courtier */
