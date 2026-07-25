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
#include <array>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <span>
#include <spanstream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// Boost headers go here

// Geneva headers go here
#include "common/GArchiveNamed.hpp"       // archive_named (boost-vs-GArchive member emitter)
#include "weft/GArchivePolymorphic.hpp" // GArchive codecs for the GEM_BINARY / GEM_JSON wire arm
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GProcessable.hpp"
#include "courtier/GWireSerializationContext.hpp" // GWireBlobId / GWirePeerId for the blob-fetch commands

namespace Gem::Courtier {

/******************************************************************************************/
////////////////////////////////////////////////////////////////////////////////////////////
/******************************************************************************************/
/**
 * This class encapsulates a processable item that may be transmitted to a remote site,
 * equipped with a command.
 *
 * @tparam processable_type The type of the processable work item carried as payload
 * @tparam command_type The enumeration of commands that may accompany the payload
 */
template <typename processable_type, typename command_type>
class GCommandContainerT {
    ///////////////////////////////////////////////////////////////
    friend struct Gem::Weft::access;

    /**
     * @brief Serialization hook that (de-)serializes the command and the payload pointer, plus the
     * optional blob-fetch fields (a peer id, a blob id and a serialized blob).
     *
     * The three extra fields are inert for the common COMPUTE / RESULT / GETDATA / NODATA / STOP traffic
     * (the peer id is 0 / unused, the blob id is all-zero and the blob is empty there): they carry data
     * only for the REQUEST_BLOB (which fills the blob id) and SEND_BLOB (which fills the blob id
     * and the blob) cache-miss-fetch commands, and for transports that announce a stable peer id on every
     * request (ASIO). They are written/read symmetrically and unconditionally, so this stays a single,
     * version-free format that round-trips for every command. The blob id's two 64-bit halves are
     * streamed individually so no std::array archive support is required.
     *
     * @tparam Archive The GArchive codec type
     * @param ar The archive to read from / write to
     * @param version The class version (unused)
     */
    template <class Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using Gem::Common::archive_named;
        archive_named(ar, "command_", command_);
        archive_named(ar, "payload_ptr_", payload_ptr_);
        archive_named(ar, "peer_id_", peer_id_);
        archive_named(ar, "blob_id_hi", blob_id_[0]);
        archive_named(ar, "blob_id_lo", blob_id_[1]);
        archive_named(ar, "blob_content_", blob_content_);
    }
    ///////////////////////////////////////////////////////////////

    // The payload must be a processable (status / process() lifecycle).
    static_assert(
        std::is_base_of_v<Gem::Courtier::GProcessable, processable_type>,
        "processable_type must derive from Gem::Courtier::GProcessable"
    );

public:
    //-------------------------------------------------------------------------
    /**
	  * @brief Initialization with a command only, in cases where no payload
	  * needs to be transported.
	  *
	  * @param command The command to be executed
	  */
    explicit GCommandContainerT(command_type command)
      : command_(command) { /* nothing */
    }

    //-------------------------------------------------------------------------
    /**
	  * @brief Initialization with command and payload (in cases where a payload needs
	  * to be transferred).
	  *
	  * @param command The command to be executed
	  * @param payload_ptr The payload transported by this object (sole ownership is taken by move)
	  */
    GCommandContainerT(command_type command, std::unique_ptr<processable_type> payload_ptr)
      : command_(command)
      , payload_ptr_(std::move(payload_ptr)) { /* nothing */
    }

    //-------------------------------------------------------------------------
    // Defaulted constructors, destructor and move assigment operator

    GCommandContainerT() = default;
    GCommandContainerT(GCommandContainerT &&cp) noexcept = default;
    ~GCommandContainerT() = default;

    GCommandContainerT &operator=(GCommandContainerT &&cp) noexcept = default;

    //-------------------------------------------------------------------------
    // Deleted copy-constructors and assignment operator -- the class is non-copyable

    GCommandContainerT(const GCommandContainerT &) = delete;
    GCommandContainerT &operator=(const GCommandContainerT &) = delete;

    //-------------------------------------------------------------------------
    /**
	  * @brief Reset to a new command and payload, or clear the object.
	  *
	  * @param command The new command (defaults to command_type(0), i.e. cleared)
	  * @param payload_ptr The new payload (defaults to an empty pointer; ownership is taken by move)
	  * @return A reference to this object, so we can serialize it in one go
	  */
    const GCommandContainerT &reset(
        command_type command = command_type(0),
        std::unique_ptr<processable_type> payload_ptr = std::unique_ptr<processable_type>()
    ) {
        command_ = command;
        payload_ptr_ = std::move(payload_ptr);
        // Also clear the optional blob-fetch fields, so a reused container never carries stale
        // id / blob / peer data into the next message. A caller that needs them (a SEND_BLOB reply)
        // sets them explicitly AFTER reset().
        peer_id_ = 0;
        blob_id_ = GWireBlobId{0, 0};
        blob_content_.clear();
        return *this;
    }

    //-------------------------------------------------------------------------
    /**
	  * @brief Setting of the command to be executed on the payload (possibly on the remote side).
	  * @param command The command to be executed on the payload
	  */
    void set_command(command_type command) {
        command_ = command;
    }

    //-------------------------------------------------------------------------
    /**
	  * @brief Retrieval of the command to be executed on the payload.
	  * @return The command to be executed on the payload
	  */
    [[nodiscard]] command_type get_command() const noexcept {
        return command_;
    }

    //-------------------------------------------------------------------------
    /**
	  * @brief Retrieves the payload by const reference (a non-destructive borrow). Use
	  * release_payload() to take ownership of it.
	  *
	  * @return A const reference to the owned payload pointer (may be empty if no payload is present)
	  */
    [[nodiscard]] const std::unique_ptr<processable_type> &get_payload() const {
        return payload_ptr_;
    }

    //-------------------------------------------------------------------------
    /**
	  * @brief Extracts (moves out) the payload, transferring sole ownership to the caller. The
	  * container's payload is empty afterwards. Used by the transports to hand a received result on to
	  * the OA.
	  *
	  * @return The payload pointer; the container retains no ownership afterwards (may be empty)
	  */
    std::unique_ptr<processable_type> release_payload() {
        return std::move(payload_ptr_);
    }

    //-------------------------------------------------------------------------
    // blob send-once: optional fields carried alongside the command/payload. They are unused
    // (peer 0, zero id, empty blob) for ordinary COMPUTE/RESULT/GETDATA/NODATA/STOP traffic and only
    // populated for the REQUEST_BLOB / SEND_BLOB cache-miss-fetch commands and for transports that
    // announce a stable peer id on each request (ASIO).

    /** @brief Sets the announcing peer's stable id (used by ASIO, whose one-shot connections have no
     *  persistent per-connection identity, to tell the server which peer a request belongs to).
     *  @param peer The stable peer id of the announcing client. */
    void set_peer_id(GWirePeerId peer) noexcept { peer_id_ = peer; }

    /** @brief @return The stable peer id announced on this request (0 if none). */
    [[nodiscard]] GWirePeerId get_peer_id() const noexcept { return peer_id_; }

    /** @brief Sets the blob id carried by a REQUEST_BLOB / SEND_BLOB command.
     *  @param id The 128-bit content id of the blob being requested / returned. */
    void set_blob_id(const GWireBlobId &id) noexcept { blob_id_ = id; }

    /** @brief @return The blob id carried by this command (all-zero if none). */
    [[nodiscard]] const GWireBlobId &get_blob_id() const noexcept { return blob_id_; }

    /** @brief Sets the serialized blob carried by a SEND_BLOB reply.
     *  @param blob The serialized blob (moved in). */
    void set_blob(std::string blob) { blob_content_ = std::move(blob); }

    /** @brief @return The serialized blob carried by this command (empty if none). */
    [[nodiscard]] const std::string &get_blob() const noexcept { return blob_content_; }

    //-------------------------------------------------------------------------
    /**
	  * @brief Processing of the payload. Delegates to the payload's process() method.
	  *
	  * Errors during processing are handled one layer down: GProcessingContainerT::process()
	  * wraps the user's process_() in a try/catch, records the failure on the item
	  * (processing_status_ = EXCEPTION_CAUGHT plus stored error descriptions) and rethrows a
	  * typed g_processing_exception. This method intentionally lets that exception propagate to
	  * the caller (the worker compute loop), which catches it and returns the item carrying its
	  * error state -- so a faulty work item never crashes the worker.
	  *
	  * @throws geneva_exception if the container holds no payload
	  * @throws g_processing_exception (from the payload) if processing flagged or threw an error
	  */
    void process() {
        if(payload_ptr_) {
            payload_ptr_->process();
        }
        else {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GCommandContainerT<processable_type, command_type>::process():" << '\n'
                << "Tried to process a work item while payload_ptr_ is empty" << '\n'
            );
        }
    }

private:
    //-------------------------------------------------------------------------
    // Data

    command_type command_{command_type(0)};         ///< The command to be exeecuted
    std::unique_ptr<processable_type> payload_ptr_; ///< The actual payload, if any (sole ownership)

    // blob send-once: optional fields (see the accessors above). Inert/zero for normal traffic.
    GWirePeerId peer_id_{0};        ///< stable announcing-peer id (ASIO); 0 == none
    GWireBlobId blob_id_{0, 0}; ///< blob id for REQUEST_BLOB / SEND_BLOB (all-zero == none)
    std::string blob_content_;       ///< serialized blob for a SEND_BLOB reply (empty == none)

    //-------------------------------------------------------------------------
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief Conversion of a GCommandContainerT to a serialized string.
 *
 * @tparam processable_type The payload type of the command container
 * @tparam command_type The command enumeration of the command container
 * @param container The command container to be serialized
 * @param serMode The serialization format to use (text, XML or binary)
 * @return The serialized representation of the container (empty string only on the unreachable fall-through)
 * @throws geneva_exception if serialization fails
 */
template <typename processable_type, typename command_type>
std::string container_to_string(
    const GCommandContainerT<processable_type, command_type> &container,
    Gem::Common::serializationMode serMode
) {
    try {
        switch(serMode) {
            using enum Gem::Common::serializationMode;
        case GEM_BINARY: {
            // GArchive flat-binary codec. The blob send-once interning is orthogonal: it lives
            // inside GGenome::save (an ambient GWireSerializationScope, if any, is honoured there,
            // and the interned blob travels as an opaque blob), so the outer codec choice is free.
            Gem::Weft::GBinaryOArchive oa;
            oa &Gem::Weft::make_nvp("command_container", container);
            return oa.str();
        }

        case GEM_JSON: {
            Gem::Weft::GJsonOArchive oa;
            oa &Gem::Weft::make_nvp("command_container", container);
            return oa.str();
        }
        }
    }
    catch(const std::exception &e) { // boost::system::system_error derives from std::exception
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In container_to_string(GCommandContainerT<>):" << '\n'
            << "Caught std::exception exception with messages:" << '\n'
            << e.what() << '\n'
            << "with serializationMode == " << Gem::Common::serModeToString(serMode) << '\n'
        );
    }
    catch(...) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In container_to_string(GCommandContainerT<>):" << '\n'
            << "Caught unknown exception" << '\n'
            << "with serializationMode == " << Gem::Common::serModeToString(serMode) << '\n'
        );
    }

    // Make the compiler happy
    return {};
}

/******************************************************************************/
/**
 * @brief Loading of a GCommandContainerT from a serialized string. The container is reset before
 * loading.
 *
 * @tparam processable_type The payload type of the command container
 * @tparam command_type The command enumeration of the command container
 * @param descr The serialized representation to load from
 * @param container The command container to be filled (output parameter; reset before loading)
 * @param serMode The serialization format the string was produced with (GArchive binary or JSON)
 * @throws geneva_exception if de-serialization fails
 */
template <typename processable_type, typename command_type>
void container_from_string(
    const std::string &descr,
    GCommandContainerT<processable_type, command_type> &container,
    Gem::Common::serializationMode serMode
) {
    container.reset();

    try {
        switch(serMode) {
            using enum Gem::Common::serializationMode;
        case GEM_BINARY: {
            // descr is a live named parameter, so GBinaryIArchive's string_view over it stays valid.
            Gem::Weft::GBinaryIArchive ia(descr);
            ia &Gem::Weft::make_nvp("command_container", container);
        } break;

        case GEM_JSON: {
            Gem::Weft::GJsonIArchive ia(descr);
            ia &Gem::Weft::make_nvp("command_container", container);
        } break;
        }
    }
    catch(const std::exception &e) { // boost::system::system_error derives from std::exception
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In container_from_string(GCommandContainerT<>):" << '\n'
            << "Caught std::exception exception with messages:" << '\n'
            << e.what() << '\n'
            << "with serializationMode == " << Gem::Common::serModeToString(serMode) << '\n'
        );
    }
    catch(...) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In container_from_string(GCommandContainerT<>):" << '\n'
            << "Caught unknown exception" << '\n'
            << "with serializationMode == " << Gem::Common::serModeToString(serMode) << '\n'
        );
    }
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Courtier */

/******************************************************************************/
