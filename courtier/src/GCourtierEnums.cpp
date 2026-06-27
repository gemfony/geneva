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

#include "courtier/GCourtierEnums.hpp"
#include "common/GCommonEnums.hpp"
#include <istream>
#include <ostream>

namespace Gem::Courtier {

/******************************************************************************************/
/**
 * Puts a Gem::Courtier::run_state item into a stream
 *
 * @param o The ostream the item should be added to
 * @param rs the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Courtier::run_state &rs) {
    auto tmp = static_cast<Gem::Common::ENUMBASETYPE>(rs);
    o << tmp;
    return o;
}

/******************************************************************************************/
/**
 * Reads a Gem::Courtier::run_state item from a stream
 *
 * @param i The stream the item should be read from
 * @param rs The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Courtier::run_state &rs) {
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    rs = static_cast<Gem::Courtier::run_state>(tmp);
    return i;
}

/******************************************************************************************/
/**
 * Puts a Gem::Courtier::networked_consumer_payload_command item into a stream
 *
 * @param o The ostream the item should be added to
 * @param ps the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &
operator<<(std::ostream &o, const Gem::Courtier::networked_consumer_payload_command &ps) {
    auto tmp = static_cast<Gem::Common::ENUMBASETYPE>(ps);
    o << tmp;
    return o;
}

/******************************************************************************************/
/**
 * Reads a Gem::Courtier::networked_consumer_payload_command item from a stream
 *
 * @param i The stream the item should be read from
 * @param ps The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Courtier::networked_consumer_payload_command &ps) {
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    ps = static_cast<Gem::Courtier::networked_consumer_payload_command>(tmp);
    return i;
}

/******************************************************************************************/
/**
 * Puts a Gem::Courtier::beast_ping_state item into a stream
 *
 * @param o The ostream the item should be added to
 * @param ps the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Courtier::beast_ping_state &ps) {
    auto tmp = static_cast<Gem::Common::ENUMBASETYPE>(ps);
    o << tmp;
    return o;
}

/******************************************************************************************/
/**
 * Reads a Gem::Courtier::beast_ping_state item from a stream
 *
 * @param i The stream the item should be read from
 * @param ps The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Courtier::beast_ping_state &ps) {
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    ps = static_cast<Gem::Courtier::beast_ping_state>(tmp);
    return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Courtier::submissionReturnMode item into a stream
 *
 * @param o The ostream the item should be added to
 * @param srm the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Courtier::submissionReturnMode &srm) {
    auto tmp = static_cast<Gem::Common::ENUMBASETYPE>(srm);
    o << tmp;
    return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Courtier::submissionReturnMode item from a stream
 *
 * @param i The stream the item should be read from
 * @param srm The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Courtier::submissionReturnMode &srm) {
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    srm = static_cast<Gem::Courtier::submissionReturnMode>(tmp);
    return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Courtier::processingStatus item into a stream
 *
 * @param o The ostream the item should be added to
 * @param srm the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Courtier::processingStatus &srm) {
    switch(srm) {
        using enum Gem::Courtier::processingStatus;
    case UNPROCESSED:
        o << "processingStatus::UNPROCESSED";
        break;
    case DO_PROCESS:
        o << "processingStatus::DO_PROCESS";
        break;
    case PROCESSED:
        o << "processingStatus::PROCESSED";
        break;
    case EXCEPTION_CAUGHT:
        o << "processingStatus::EXCEPTION_CAUGHT";
        break;
    case ERROR_FLAGGED:
        o << "processingStatus::ERROR_FLAGGED";
        break;
    default:
        o << "UNKNOWN processing status";
        break;
    }

    return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Courtier::processingStatus item from a stream
 *
 * @param i The stream the item should be read from
 * @param srm The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Courtier::processingStatus &srm) {
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    srm = static_cast<Gem::Courtier::processingStatus>(tmp);
    return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Courtier::consumerType into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param o The ostream the item should be added to
 * @param bm The consumerType item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Courtier::consumerType &bm) {
    auto tmp = static_cast<Gem::Common::ENUMBASETYPE>(bm);
    o << tmp;
    return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Courtier::consumerType item from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param i The stream the item should be read from
 * @param bm The consumerType item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Courtier::consumerType &bm) {
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    bm = static_cast<Gem::Courtier::consumerType>(tmp);
    return i;
}

/******************************************************************************/

} /* namespace Gem::Courtier */
