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
#include <cstdint>

// Geneva headers go here
#include "common/GLogger.hpp"
#include "courtier/GWireProtocolT.hpp"
#include "geneva/genome/GGenome.hpp"

namespace Gem::Geneva::Genome {

/******************************************************************************/
/**
 * What a message MEANS in Geneva's use of the courtier transport.
 *
 * Courtier owns the frame vocabulary (GFrameKind) and never learns these: it carries the tag as an
 * opaque number and hands it back to the seam below. Geneva alone decides what each one implies.
 *
 * The values are part of the wire format and are therefore stable.
 */
enum class geneva_command : std::uint32_t {
    evaluate = 0,               ///< server -> worker: evaluate this individual
    evaluation = 1,             ///< worker -> server: the computed results of an unmodified individual
    evaluated_and_modified = 2, ///< worker -> server: the whole individual, whose genome the worker changed
    terminate = 3               ///< server -> worker: stop working
};

/******************************************************************************/

} /* namespace Gem::Geneva::Genome */

namespace Gem::Courtier {

/******************************************************************************/
/**
 * Geneva's answers to the three questions courtier cannot answer about a message.
 *
 * This specialization is defined in a header that GGenome.hpp itself includes, so no translation unit
 * can instantiate a courtier transport for a Geneva individual without it -- the primary template's
 * degenerate protocol can never be picked up by accident.
 */
template <>
struct GWireProtocolT<Gem::Geneva::Genome::GGenome> {
    /** @brief Geneva ships whole individuals in both directions for now; the results-only return
     *  becomes a payload of its own in a following change. */
    using payload_type = GNoLibraryPayload;

    /** @brief @return The tag on a dispatched work item: evaluate it. */
    static std::uint32_t workTag() {
        return static_cast<std::uint32_t>(Gem::Geneva::Genome::geneva_command::evaluate);
    }

    /** @brief @return The tag on a shutdown frame: stop working. */
    static std::uint32_t shutdownTag() {
        return static_cast<std::uint32_t>(Gem::Geneva::Genome::geneva_command::terminate);
    }

    /**
     * @brief Worker side: chooses the message a processed individual comes back as.
     * @param item The processed individual
     * @param tag Receives the geneva_command describing the return
     * @param payload Unused while every return ships the whole individual
     * @return true -- the individual itself travels back
     */
    static bool buildReturn(
        [[maybe_unused]] const Gem::Geneva::Genome::GGenome &item,
        std::uint32_t &tag,
        [[maybe_unused]] payload_type &payload
    ) {
        tag = static_cast<std::uint32_t>(Gem::Geneva::Genome::geneva_command::evaluated_and_modified);
        return true;
    }

    /**
     * @brief Server side: writes a returned message onto the live population element it belongs to.
     *
     * The element is reconciled IN PLACE -- its heap address is load-bearing (a concurrent per-individual
     * prefetch may hold a snapshot of population addresses across the submission), so nothing is swapped
     * out. The OA-owned scratch is never re-copied: it is stripped on the wire and the element's own copy
     * is the live one.
     *
     * @param slot The live population element the return belongs to
     * @param tag The geneva_command the worker chose
     * @param payload Unused while every return ships the whole individual
     * @param item The returned individual, or nullptr if none travelled
     */
    static void applyReturn(
        Gem::Geneva::Genome::GGenome &slot,
        std::uint32_t tag,
        [[maybe_unused]] const payload_type *payload,
        const Gem::Geneva::Genome::GGenome *item
    ) {
        using Gem::Geneva::Genome::geneva_command;
        switch(static_cast<geneva_command>(tag)) {
        case geneva_command::evaluated_and_modified:
            if(item != nullptr) {
                slot.absorbGenomeFrom(*item); // the worker changed the genome -> take it
                slot.absorbResultsFrom(*item);
            }
            break;

        default:
            glogger << "In GWireProtocolT<GGenome>::applyReturn():" << '\n'
                    << "Got a return carrying the unexpected geneva command " << tag << '\n'
                    << "The return is ignored; the slot keeps its previous state." << '\n'
                    << GWARNING;
            break;
        }
    }
};

/******************************************************************************/

} /* namespace Gem::Courtier */
