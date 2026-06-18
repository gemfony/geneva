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
#include <string>

// Geneva headers go here
#include "common/GCommonEnums.hpp" // Gem::Common::serializationMode
#include "common/GErrorStreamer.hpp"
#include "common/GLogger.hpp"
#include "courtier/GCommandContainerT.hpp"        // GCommandContainerT, container_to/from_string
#include "courtier/GCourtierEnums.hpp"            // networked_consumer_payload_command
#include "courtier/GWireSerializationContext.hpp" // GWireSerializationScope / registry / ids

namespace Gem::Courtier {

/******************************************************************************/
/**
 * Transport-agnostic codec for the networked consumer/worker session protocol. The three networked
 * transports (Asio, websocket, MPI) all (de)serialize a GCommandContainerT and all implement the
 * layout send-once cache-miss fetch (REQUEST_LAYOUT / SEND_LAYOUT). Only the raw byte I/O differs
 * between them; the (de)serialization itself -- and in particular the subtle wire-scope discipline
 * around it -- is identical, so it lives here once instead of being copy-pasted into each transport.
 *
 * Two rules this codec encapsulates so no transport has to remember them:
 *
 *  1. A WORK ITEM is (de)serialized inside a GWireSerializationScope bound to the transport's
 *     GWireSerializationContext, so the layout send-once machinery engages (wireEncode / wireDecode).
 *
 *  2. A LAYOUT FETCH message (REQUEST_LAYOUT from the worker, SEND_LAYOUT from the server) is
 *     (de)serialized under a NULL scope: it carries no genome, and -- crucially on the worker side,
 *     where the fetch happens mid-decode of a work item -- it must NOT recurse into the send-once
 *     logic of the work item being decoded. The build/parse helpers below install that null scope
 *     themselves.
 *
 * Every function is pure with respect to I/O: it turns containers/ids into strings or vice versa.
 * The transport owns the actual send/recv.
 */

/******************************************************************************/
/**
 * @brief Serializes a work-item command container under the given wire context (layout send-once).
 * @tparam processable_type The payload type of the command container
 * @tparam command_type The command enumeration of the command container
 * @param container The command container to serialize
 * @param ctx The wire context to engage for the duration (may be nullptr for "no context")
 * @param serMode The serialization format to use
 * @return The serialized representation of the container
 */
template <typename processable_type, typename command_type>
std::string wireEncode(
    const GCommandContainerT<processable_type, command_type> &container,
    const GWireSerializationContext *ctx,
    Gem::Common::serializationMode serMode
) {
    GWireSerializationScope scope(ctx);
    return container_to_string(container, serMode);
}

/******************************************************************************/
/**
 * @brief De-serializes a work-item command container under the given wire context (layout send-once).
 * @tparam processable_type The payload type of the command container
 * @tparam command_type The command enumeration of the command container
 * @param descr The serialized representation to load from
 * @param container The command container to fill (reset before loading)
 * @param ctx The wire context to engage for the duration (may be nullptr for "no context")
 * @param serMode The serialization format the string was produced with
 */
template <typename processable_type, typename command_type>
void wireDecode(
    const std::string &descr,
    GCommandContainerT<processable_type, command_type> &container,
    const GWireSerializationContext *ctx,
    Gem::Common::serializationMode serMode
) {
    GWireSerializationScope scope(ctx);
    container_from_string(descr, container, serMode);
}

/******************************************************************************/
/**
 * @brief Worker side: builds a serialized REQUEST_LAYOUT message for a layout cache-miss fetch.
 *
 * Serialized under a NULL wire scope (rule 2 above): a REQUEST_LAYOUT carries no genome, and this
 * runs mid-decode of a work item, so it must not recurse into the send-once context.
 *
 * @tparam processable_type The payload type of the command container
 * @param id The content id of the layout the worker needs
 * @param peer The worker's peer id (echoed so the server can answer the right session)
 * @param serMode The serialization format to use
 * @return The serialized REQUEST_LAYOUT message
 */
template <typename processable_type>
std::string buildLayoutRequest(
    const GWireLayoutId &id,
    GWirePeerId peer,
    Gem::Common::serializationMode serMode
) {
    GWireSerializationScope no_scope(nullptr);
    GCommandContainerT<processable_type, networked_consumer_payload_command> request{
        networked_consumer_payload_command::REQUEST_LAYOUT
    };
    request.set_layout_id(id);
    request.set_peer_id(peer);
    return container_to_string(request, serMode);
}

/******************************************************************************/
/**
 * @brief Worker side: parses a SEND_LAYOUT reply and returns the carried layout blob.
 *
 * De-serialized under a NULL wire scope (rule 2 above). Returns an empty string if the reply could
 * not be parsed or did not carry the expected SEND_LAYOUT command, which the caller treats as a
 * failed fetch.
 *
 * @tparam processable_type The payload type of the command container
 * @param reply_str The serialized SEND_LAYOUT reply received from the server
 * @param serMode The serialization format the reply was produced with
 * @return The serialized layout blob, or an empty string on failure / command mismatch
 */
template <typename processable_type>
std::string parseLayoutReply(
    const std::string &reply_str,
    Gem::Common::serializationMode serMode
) {
    GWireSerializationScope no_scope(nullptr);
    GCommandContainerT<processable_type, networked_consumer_payload_command> reply{
        networked_consumer_payload_command::NONE
    };
    container_from_string(reply_str, reply, serMode);
    if(reply.get_command() != networked_consumer_payload_command::SEND_LAYOUT) {
        glogger << "In Gem::Courtier::parseLayoutReply():" << '\n'
                << "expected SEND_LAYOUT but got command " << reply.get_command() << '\n'
                << GWARNING;
        return {};
    }
    return reply.get_layout_blob();
}

/******************************************************************************/
/**
 * @brief Server side: builds a serialized SEND_LAYOUT reply to a worker's REQUEST_LAYOUT.
 *
 * The blob is copied out of the consumer's shared registry; on a miss (or a null registry) it is left
 * empty and the worker treats the fetch as failed. Serialized under a NULL wire scope: the reply
 * carries only the raw blob, never a genome, so it must not engage the send-once logic.
 *
 * @tparam processable_type The payload type of the command container
 * @param id The content id of the requested layout
 * @param registry The shared layout registry to resolve the blob from (may be nullptr)
 * @param serMode The serialization format to use
 * @return The serialized SEND_LAYOUT reply (with the blob if the id was cached, empty otherwise)
 */
template <typename processable_type>
std::string buildLayoutReply(
    const GWireLayoutId &id,
    GWireLayoutRegistry *registry,
    Gem::Common::serializationMode serMode
) {
    GWireSerializationScope no_scope(nullptr);
    std::string blob;
    if(registry != nullptr) {
        registry->tryGet(id, blob); // leaves blob empty on a miss
    }
    GCommandContainerT<processable_type, networked_consumer_payload_command> reply{
        networked_consumer_payload_command::SEND_LAYOUT
    };
    reply.set_layout_id(id);
    reply.set_layout_blob(std::move(blob));
    return container_to_string(reply, serMode);
}

/******************************************************************************/

} /* namespace Gem::Courtier */
