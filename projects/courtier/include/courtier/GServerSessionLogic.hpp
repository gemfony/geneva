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
#include <memory>
#include <string>
#include <utility>

// Geneva headers go here
#include "common/GCommonEnums.hpp" // Gem::Common::serializationMode
#include "common/GErrorStreamer.hpp"
#include "common/GLogger.hpp"
#include "courtier/GCommandContainerT.hpp"        // GCommandContainerT
#include "courtier/GCourtierEnums.hpp"            // networked_consumer_payload_command
#include "courtier/GWireCodec.hpp"                // wireEncode / buildLayoutReply
#include "courtier/GWireSerializationContext.hpp" // GWireSerializationContext / registry

namespace Gem::Courtier {

/******************************************************************************/
/**
 * Transport-agnostic server-side session logic for the networked consumer protocol. A server session,
 * on receiving a request from a worker, decides what to do with it: serve a work item (GETDATA), sink a
 * returned result and serve the next (RESULT), or answer a blob cache-miss fetch (REQUEST_BLOB). The
 * three transports all make the same decisions; only how they pull/sink items (a callable wrapping the
 * consumer's checkout/checkin) and how they drive their I/O differs. The decision pieces live here once.
 *
 * Two helpers, matching the two server styles:
 *
 *  - serveWorkItem(): the pure container decision -- check a work item out and set the container to
 *    COMPUTE (with the item) or NODATA (queue empty). No serialization, no I/O. Used by every transport
 *    (the synchronous ones via handleServerRequest() below; MPI directly, as its decide and send phases
 *    are split across its asynchronous receive/send).
 *
 *  - handleServerRequest(): the full synchronous request -> response-bytes dispatch used by the
 *    request/response transports (Asio, websocket). It takes an already-deserialized inbound container
 *    (the transport owns the receive + its wire scope) and returns the serialized response. MPI does not
 *    use it (its server is structured as a split deserialize-phase + later send-phase, not a synchronous
 *    call) -- it composes serveWorkItem() + the GWireCodec helpers itself.
 *
 * The item source / sink are passed as callables so this header stays free of any consumer type:
 *   getItem : () -> std::unique_ptr<processable_type>   (checkout; null when the queue is empty)
 *   putItem : (std::unique_ptr<processable_type>) -> void (checkin a returned result)
 */

/******************************************************************************/
/**
 * @brief Checks a work item out and stores it in the container as COMPUTE, or stores NODATA when the
 * queue is empty. Pure container mutation -- no serialization, no I/O.
 *
 * @tparam processable_type The payload type of the command container
 * @tparam GetItemF A callable () -> std::unique_ptr<processable_type> (the checkout)
 * @param container The command container to fill (reset to COMPUTE+item or NODATA)
 * @param getItem The checkout callable
 */
template <typename processable_type, typename GetItemF>
void serveWorkItem(
    GCommandContainerT<processable_type, networked_consumer_payload_command> &container,
    GetItemF &&getItem
) {
    auto payload_ptr = getItem();
    if(payload_ptr) {
        container.reset(networked_consumer_payload_command::COMPUTE, std::move(payload_ptr));
    }
    else {
        container.reset(networked_consumer_payload_command::NODATA);
    }
}

/******************************************************************************/
/**
 * @brief Synchronous server dispatch: acts on an already-deserialized inbound request and returns the
 * serialized response. Used by the request/response transports (Asio, websocket).
 *
 * GETDATA serves a work item; RESULT sinks the returned payload (checkin) and serves the next; both
 * responses are serialized under @p respCtx so a COMPUTE item's blob is sent send-once. REQUEST_BLOB
 * is answered from the shared registry (under a null scope, via the codec). An unknown command logs a
 * warning and yields no response.
 *
 * @tparam processable_type The payload type of the command container
 * @tparam GetItemF A callable () -> std::unique_ptr<processable_type> (the checkout)
 * @tparam PutItemF A callable (std::unique_ptr<processable_type>) -> void (the checkin)
 * @param container The already-deserialized inbound container (reused to build the response)
 * @param getItem The checkout callable
 * @param putItem The checkin callable
 * @param registry The shared blob registry (for REQUEST_BLOB; may be nullptr)
 * @param respCtx The wire context to serialize a COMPUTE/NODATA response under (may be nullptr)
 * @param serMode The serialization format to use
 * @return The serialized response, or an empty string on an unknown/invalid command
 */
template <typename processable_type, typename GetItemF, typename PutItemF>
std::string handleServerRequest(
    GCommandContainerT<processable_type, networked_consumer_payload_command> &container,
    GetItemF &&getItem,
    PutItemF &&putItem,
    GWireBlobRegistry *registry,
    const GWireSerializationContext *respCtx,
    Gem::Common::serializationMode serMode
) {
    switch(container.get_command()) {
        using enum networked_consumer_payload_command;
    case GETDATA: {
        serveWorkItem(container, std::forward<GetItemF>(getItem));
        return wireEncode(container, respCtx, serMode);
    }
    case RESULT: {
        // Sink the returned payload (checkin), then serve the next item.
        auto payload_ptr = container.release_payload();
        if(payload_ptr) {
            putItem(std::move(payload_ptr));
        }
        else {
            glogger << "In Gem::Courtier::handleServerRequest():" << '\n'
                    << "payload is empty even though a RESULT was expected" << '\n'
                    << GWARNING;
        }
        serveWorkItem(container, std::forward<GetItemF>(getItem));
        return wireEncode(container, respCtx, serMode);
    }
    case REQUEST_BLOB: {
        // Blob cache-miss fetch: answer with the serialized blob from the shared registry (empty
        // blob -> the worker treats the fetch as failed).
        return buildLayoutReply<processable_type>(container.get_blob_id(), registry, serMode);
    }
    default: {
        glogger << "In Gem::Courtier::handleServerRequest():" << '\n'
                << "Got unknown or invalid command " << container.get_command() << '\n'
                << GWARNING;
        return {};
    }
    }
}

/******************************************************************************/

} /* namespace Gem::Courtier */
