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
#include "courtier/GCourtierEnums.hpp"
#include <string>


namespace Gem::Courtier {

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
