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
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// Boost headers go here
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/rfc6455.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/serialization/vector.hpp>

// Geneva headers go here
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GProcessingContainerT.hpp"
#include "courtier/GWireSerializationContext.hpp" // Phase 9: GWireLayoutId / GWirePeerId for the layout-fetch commands

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
    friend class boost::serialization::access;

    /**
     * @brief Boost.Serialization hook that (de-)serializes the command and the payload pointer, plus the
     * optional Phase 9 layout-fetch fields (a peer id, a layout id and a serialized layout blob).
     *
     * The three extra fields are inert for the common COMPUTE / RESULT / GETDATA / NODATA / STOP traffic
     * (the peer id is 0 / unused, the layout id is all-zero and the blob is empty there): they carry data
     * only for the REQUEST_LAYOUT (which fills the layout id) and SEND_LAYOUT (which fills the layout id
     * and the blob) cache-miss-fetch commands, and for transports that announce a stable peer id on every
     * request (ASIO). They are written/read symmetrically and unconditionally, so this stays a single,
     * version-free format that round-trips for every command. The layout id's two 64-bit halves are
     * streamed individually so no std::array archive support is required.
     *
     * @tparam Archive The Boost.Serialization archive type
     * @param ar The archive to read from / write to
     * @param version The class version (unused)
     */
    template <class Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        ar &BOOST_SERIALIZATION_NVP(command_) & BOOST_SERIALIZATION_NVP(payload_ptr_);
        ar &BOOST_SERIALIZATION_NVP(peer_id_);
        ar &boost::serialization::make_nvp("layout_id_hi", layout_id_[0]);
        ar &boost::serialization::make_nvp("layout_id_lo", layout_id_[1]);
        ar &BOOST_SERIALIZATION_NVP(layout_blob_);
    }
    ///////////////////////////////////////////////////////////////

    // Make sure processable_type adheres to the GProcessingContainerT interface
    static_assert(
        std::is_base_of_v<
            Gem::Courtier::
                GProcessingContainerT<processable_type, typename processable_type::result_type>,
            processable_type>,
        "processable_type does not adhere to the GProcessingContainerT interface"
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
        // Also clear the optional Phase 9 layout-fetch fields, so a reused container never carries stale
        // id / blob / peer data into the next message. A caller that needs them (a SEND_LAYOUT reply)
        // sets them explicitly AFTER reset().
        peer_id_ = 0;
        layout_id_ = GWireLayoutId{0, 0};
        layout_blob_.clear();
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
    command_type get_command() const noexcept {
        return command_;
    }

    //-------------------------------------------------------------------------
    /**
	  * @brief Retrieves the payload by const reference (a non-destructive borrow). Use
	  * release_payload() to take ownership of it.
	  *
	  * @return A const reference to the owned payload pointer (may be empty if no payload is present)
	  */
    const std::unique_ptr<processable_type> &get_payload() const {
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
    // Phase 9 layout send-once: optional fields carried alongside the command/payload. They are unused
    // (peer 0, zero id, empty blob) for ordinary COMPUTE/RESULT/GETDATA/NODATA/STOP traffic and only
    // populated for the REQUEST_LAYOUT / SEND_LAYOUT cache-miss-fetch commands and for transports that
    // announce a stable peer id on each request (ASIO).

    /** @brief Sets the announcing peer's stable id (used by ASIO, whose one-shot connections have no
     *  persistent per-connection identity, to tell the server which peer a request belongs to).
     *  @param peer The stable peer id of the announcing client. */
    void set_peer_id(GWirePeerId peer) noexcept { peer_id_ = peer; }

    /** @brief @return The stable peer id announced on this request (0 if none). */
    [[nodiscard]] GWirePeerId get_peer_id() const noexcept { return peer_id_; }

    /** @brief Sets the layout id carried by a REQUEST_LAYOUT / SEND_LAYOUT command.
     *  @param id The 128-bit content id of the layout being requested / returned. */
    void set_layout_id(const GWireLayoutId &id) noexcept { layout_id_ = id; }

    /** @brief @return The layout id carried by this command (all-zero if none). */
    [[nodiscard]] const GWireLayoutId &get_layout_id() const noexcept { return layout_id_; }

    /** @brief Sets the serialized layout blob carried by a SEND_LAYOUT reply.
     *  @param blob The serialized layout (moved in). */
    void set_layout_blob(std::string blob) { layout_blob_ = std::move(blob); }

    /** @brief @return The serialized layout blob carried by this command (empty if none). */
    [[nodiscard]] const std::string &get_layout_blob() const noexcept { return layout_blob_; }

    //-------------------------------------------------------------------------
    /**
	  * @brief Processing of the payload. Delegates to the payload's process() method.
	  *
	  * @throws geneva_exception if the container holds no payload
	  *
	  * // TODO: Check for errors during processing
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

    // Phase 9 layout send-once: optional fields (see the accessors above). Inert/zero for normal traffic.
    GWirePeerId peer_id_{0};        ///< stable announcing-peer id (ASIO); 0 == none
    GWireLayoutId layout_id_{0, 0}; ///< layout id for REQUEST_LAYOUT / SEND_LAYOUT (all-zero == none)
    std::string layout_blob_;       ///< serialized layout blob for a SEND_LAYOUT reply (empty == none)

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
        case TEXT: {
            std::ostringstream oss; // NOLINT(cppcoreguidelines-init-variables)
            boost::archive::text_oarchive oa(oss);
            oa << boost::serialization::make_nvp("command_container", container);
            return oss.str();
        } break; // archive and stream closed at end of scope

        case XML: {
            std::ostringstream oss; // NOLINT(cppcoreguidelines-init-variables)
            boost::archive::xml_oarchive oa(oss);
            oa << boost::serialization::make_nvp("command_container", container);
            return oss.str();
        } break;

        case BINARY: {
            std::ostringstream oss(std::ios_base::binary);
            boost::archive::binary_oarchive oa(oss);
            oa << boost::serialization::make_nvp("command_container", container);
            return oss.str();
        } break;
        }
    }
    catch(const boost::system::system_error &e) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In container_to_string(GCommandContainerT<>):" << '\n'
            << "Caught boost::system::system_error exception with messages:" << '\n'
            << e.what() << '\n'
            << "with serializationMode == " << Gem::Common::serModeToString(serMode) << '\n'
        );
    }
    catch(const std::exception &e) {
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
 * @param serMode The serialization format the string was produced with (text, XML or binary)
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
        case TEXT: {
            std::istringstream iss(descr);
            boost::archive::text_iarchive ia(iss);
            ia >> boost::serialization::make_nvp("command_container", container);
        } break; // archive and stream closed at end of scope

        case XML: {
            std::istringstream iss(descr);
            boost::archive::xml_iarchive ia(iss);
            ia >> boost::serialization::make_nvp("command_container", container);
        } break;

        case BINARY: {
            std::istringstream iss(descr, std::ios_base::binary);
            boost::archive::binary_iarchive ia(iss);
            ia >> boost::serialization::make_nvp("command_container", container);
        } break;
        }
    }
    catch(const boost::system::system_error &e) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In container_from_string(GCommandContainerT<>):" << '\n'
            << "Caught boost::system::system_error exception with messages:" << '\n'
            << e.what() << '\n'
            << "with serializationMode == " << Gem::Common::serModeToString(serMode) << '\n'
        );
    }
    catch(const std::exception &e) {
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
