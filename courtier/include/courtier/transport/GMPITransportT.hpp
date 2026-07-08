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

// utility functions for working with MPI
#include "courtier/GMPIHelperFunctions.hpp"

// Standard headers go here
#include <array>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

// Boost headers go here
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/vector.hpp>

// MPI headers go here

#include <mpi.h>

// Geneva headers go here
#include "common/GCommonEnums.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "common/concurrency/GThreadPool.hpp"
#include "courtier/GCommandContainerT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GCourtierHelperFunctions.hpp"
#include "courtier/GServerSessionLogic.hpp"       // shared server-side work-item decision (serveWorkItem)
#include "courtier/GWireCodec.hpp"                // shared scope-wrapped (de)serialization + layout fetch
#include "courtier/GWireSerializationContext.hpp" // layout send-once: registry + wire scope

// TODO: extract double buffering to GBaseConsumerClientT

namespace Gem::Courtier::Consumers {
// constants that are used by the master and the worker nodes
constexpr int TAG_REQUEST_WORK_ITEM = 42;
constexpr int TAG_SEND_WORK_ITEM = 43;
/// layout send-once cache-miss fetch (worker <-> master). A worker that receives an id-only
/// work item whose layout it does not hold sends a REQUEST_LAYOUT message on TAG_REQUEST_LAYOUT; the
/// master's receiver loop picks it up (it matches any tag) and answers with the serialized layout on
/// TAG_SEND_LAYOUT. A distinct send tag keeps the reply from being mistaken for an ordinary work-item
/// response by the worker's (double-buffered) MPI_ANY_TAG receive.
constexpr int TAG_REQUEST_LAYOUT = 44;
constexpr int TAG_SEND_LAYOUT = 45;
constexpr int RANK_MASTER_NODE = 0;
/// Once the master has been asked to stop, how long it keeps waiting for stragglers (a live worker's
/// final double-buffered request, or an open session to finish) before abandoning them. Bounds
/// shutdown so the loss of a worker cannot wedge the master at teardown.
constexpr std::chrono::seconds GMPICONSUMERSHUTDOWNGRACE{10};
/// How long a worker waits for the master to complete a send/receive before giving up. The master
/// always answers a request promptly (work or NODATA), so this only trips when the master has died or
/// gone silent -- it is generous to avoid ever false-killing a worker while a live master is busy.
constexpr std::chrono::seconds GMPICONSUMERWORKERMPITIMEOUT{120};
static MPI_Comm MPI_COMMUNICATOR =
    MPI_COMM_WORLD; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

/******************************************************************************/
/**
 * Initializes MPI with MPI_THREAD_MULTIPLE if it has not been initialized yet. Returns true iff this
 * call performed the initialization (so the caller knows whether it owns the matching MPI_Finalize).
 * Relocated from the former GMPIConsumerT consumer class so it survives that class's removal; used by
 * the courtier MPI consumer and the MPI sub-client optimizer.
 *
 * @param argc Optional pointer to the program's argc, forwarded to MPI_Init_thread (may be nullptr)
 * @param argv Optional pointer to the program's argv, forwarded to MPI_Init_thread (may be nullptr)
 * @return true if this call performed the MPI initialization, false if MPI was already initialized
 */
inline bool initializeMPI(int *argc = nullptr, char ***argv = nullptr) {
    int isAlreadyInitialized{0};
    MPI_Initialized(&isAlreadyInitialized);

    if(!isAlreadyInitialized) {
        int providedThreadingLevel = 0;
        MPI_Init_thread(argc, argv, MPI_THREAD_MULTIPLE, &providedThreadingLevel);

        if(providedThreadingLevel != MPI_THREAD_MULTIPLE) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "Gem::Courtier::Consumers::initializeMPI():" << '\n'
                << "Geneva requires an MPI implementation with level MPI_THREAD_MULTIPLE (a.k.a. "
                << MPI_THREAD_MULTIPLE
                << ") but the runtime environment only supports level " << providedThreadingLevel
                << '\n'
            );
        }
    }

    return !isAlreadyInitialized;
}

/******************************************************************************/
/**
 * Sets the base communicator used for all master<->worker communication, letting user code use MPI
 * alongside Geneva by splitting communicators. Relocated from the former GMPIConsumerT consumer class.
 *
 * @param communicator The MPI communicator to use for all subsequent master<->worker communication
 */
inline void setMPICommunicator(MPI_Comm communicator) {
    MPI_COMMUNICATOR = communicator;
}

/**
     * Stores configuration options which are used by master node and worker nodes
     */
struct MPIConsumerConfig {
    /**
     * @brief Default constructor. Seeds nHandlerThreads with the hardware recommendation.
     *
     * The value may later be overwritten by user-defined command line options.
     */
    MPIConsumerConfig() : nHandlerThreads(this->nHandlerThreadsRecommendation()) {
        // Set the handler threads to the hardware recommendation as a default value
        // This can later be overwritten by user-defined command line options
        
    }

    /**
         * Whether the clients should issue a request for the next work item while the current work item is still being processed
         */
    bool useAsyncReq{true};
    /**
         * Serialization mode to use when serializing messages before transmitting over network between master node and worker nodes
         */
    Gem::Common::serializationMode serializationMode{Gem::Common::serializationMode::BINARY};
    /**
         * The number of threads in a thread pool which is used to handle incoming requests.
         */
    std::uint32_t nHandlerThreads{0};
    /**
         * The time in ms between each check of completion of the send operation of a new work item to a worker node.
         */
    std::uint32_t masterCleanSessIntervalMSec{1'000};

    /**
     * @brief Registers this config's command-line options into the given option descriptions.
     *
     * The current member values are used as the option defaults (a default-constructed instance
     * already holds the intended defaults), avoiding duplicating the default values here.
     *
     * @param visible The options_description receiving user-facing options
     * @param hidden The options_description receiving advanced/hidden options
     */
    void addCLOptions_(
        boost::program_options::options_description &visible,
        boost::program_options::options_description &hidden
    ) {
        // Note that we use the current values of the members as default values, because in a default constructed
        // instance the defaults are already set. This allows to reduce duplication of those values
        namespace po = boost::program_options;

        visible.add_options()(
            "mpi_asyncReq",
            po::value<bool>(&useAsyncReq)->default_value(useAsyncReq),
            "\t[mpi] Whether the clients should issue a request for the next work item while"
            "the current work item is still being processed"
        );

        visible.add_options()(
            "mpi_nHandlerThreads",
            po::value<std::uint32_t>(&nHandlerThreads)->default_value(nHandlerThreads),
            "\t[mpi] The number of threads in a thread pool which is used to handle incoming "
            "requests."
            "Defaults to the number of CPU cores in the system, if able to determine by C++ runtime"
        );

        hidden.add_options()(
            "mpi_cleanSessInterval",
            po::value<std::uint32_t>(&masterCleanSessIntervalMSec)
                ->default_value(masterCleanSessIntervalMSec),
            "\t[mpi] The time interval in ms between each check for the completion "
            "of active sessions on the master node."
        );

        hidden.add_options()(
            "mpi_serializationMode",
            po::value<Gem::Common::serializationMode>(&serializationMode)
                ->default_value(serializationMode),
            "\t[mpi] Specifies whether serialization shall be done in TEXTMODE (0), XMLMODE (1) or "
            "BINARYMODE (2)"
        );
    }

    /**
     * @brief Recommends a default number of handler threads based on available hardware concurrency.
     * @return The number of hardware threads, or 8 if the C++ runtime cannot determine it
     */
    [[nodiscard]] static std::uint32_t nHandlerThreadsRecommendation() {
        // query hint that indicates how many hardware threads are available (might return 0 if unknown)
        const unsigned int hwThreads{std::thread::hardware_concurrency()};

        // if hint has returned 0, default to 8
        return hwThreads != 0 ? hwThreads : 8;
    }
};

/**
     * This class is responsible for the client side of network communication using MPI.
     * It will repeatedly request work items from the GMPIConsumerMasterNodeT, process those,
     * send them back to the worker and request more work items.
     *
     * The simplified workflow of the GMPIConsumerWorkerNodeT can be described as follows:
     *
     * (1) Send an asynchronous GET request (ask for the first work item) \n
     * (2) Asynchronously receive a message containing either the work item (COMPUTE) or the information that currently
     *      no items are available (NODATA)\n
     * (3) Deserialize the received message\n
     * (4.1) If the message contains DATA (a raw work item): process the received work item\n
     * (4.2) If the message contains no data: poll later again i.e. back to step (1)\n
     * (4.3) If the message contains a stop request, prepare to shutdown the worker \n
     * (5) Asynchronously send the processed work item to the master node, this message also implicitly requests a
     *      new work item\n
     * (6) Wait for the response i.e. go back to step (2) again\n
     *
     * All communication between GMPIConsumerWorkerNodeT and GMPIConsumerMasterNodeT works with asynchronous communication
     * using MPI.
     *
     * @tparam processable_type the type of work item exchanged with the master node
     */
template <typename processable_type>
class GMPIConsumerWorkerNodeT final
  : public std::enable_shared_from_this<GMPIConsumerWorkerNodeT<processable_type>> {
public:
    /**
         * @brief Constructor with arguments for all configuration options for this class.
         *
         * @param commRank this node's rank within this cluster
         * @param halt function that returns true if halt criterion has been reached
         * @param incrementProcessingCounter function that increments the counter for already processed items on this node
         * @param config const reference to configuration struct instance storing user-defined configuration
         */
    explicit GMPIConsumerWorkerNodeT(
        std::int32_t commRank,
        std::move_only_function<bool()> halt,
        std::move_only_function<void()> incrementProcessingCounter,
        const MPIConsumerConfig &config
    )
      : commRank_{commRank}
      , halt_{std::move(halt)}
      , incrementProcessingCounter_{std::move(incrementProcessingCounter)}
      , config_{config} {
        glogger << "GMPIConsumerWorkerNodeT with rank " << commRank_ << " started up" << '\n'
                << GLOGGING;

        // Engage the worker side of the layout send-once wire form. The worker caches every
        // layout it receives (keyed by content id) so an id-only work item resolves locally; on a miss
        // (a late-joining / restarted rank that never saw the full layout, or master-side eviction) the
        // fetch_blob asks the master for it via a blocking REQUEST_LAYOUT / SEND_LAYOUT round trip. The
        // fetch runs from inside the work-item deserialise, which on this worker is sequenced strictly
        // before that iteration's outgoing send/receive is launched, so it never overlaps other MPI
        // traffic on this rank.
        wireCtx_.enabled = true;
        wireCtx_.peer = 0; // worker side: the single upstream master
        wireCtx_.registry = &wireRegistry_;
        wireCtx_.mode = config_.serializationMode;
        // Return processed items in the lightweight results-only form by default. Like the websocket /
        // ASIO consumers, the MPI master uses the GNetworkedConsumerT slot model (its getPayloadItem /
        // putPayloadItem are wired to checkout / checkin), so the input parameters are grafted back from
        // the still-held original in checkin(). A late results-only return -- one whose batch has already
        // been reconciled, so no original remains to graft from -- is dropped rather than buffered (see
        // GNetworkedConsumerT::bufferLateReturn_locked). A work item can force a full return per item via
        // setReturnFullIndividual().
        wireCtx_.returning = true;
        wireCtx_.fetch_blob = [this](const Gem::Courtier::GWireLayoutId &id) -> std::string {
            return this->fetchLayoutBlob_(id);
        };
    }

    /**
         * @brief The destructor.
         */
    ~GMPIConsumerWorkerNodeT() = default;

    //-------------------------------------------------------------------------
    // Deleted functions

    // Deleted default-constructor -- enforce usage of a particular constructor
    GMPIConsumerWorkerNodeT() = delete;

    // Deleted copy-constructors and assignment operators -- the client is non-copyable
    GMPIConsumerWorkerNodeT(const GMPIConsumerWorkerNodeT<processable_type> &) = delete;

    GMPIConsumerWorkerNodeT(GMPIConsumerWorkerNodeT<processable_type> &&) = delete;

    GMPIConsumerWorkerNodeT<processable_type> &
    operator=(const GMPIConsumerWorkerNodeT<processable_type> &) = delete;

    GMPIConsumerWorkerNodeT<processable_type> &
    operator=(GMPIConsumerWorkerNodeT<processable_type> &&) = delete;

    /**
         * @brief Advises the worker to start requesting and processing work items.
         *
         * This call completes when a stop request has been received from the master node, indicating the end of the
         * optimization, or if a fatal error has been encountered.
         */
    void run() {
        // set message for initial GETDATA request (carries no genome, but keep it under the scope for
        // uniformity -- the scope is harmless for a payload-free command)
        outgoingMessage_ =
            Gem::Courtier::wireEncode(commandContainer_, &wireCtx_, config_.serializationMode);

        // send initial GETDATA request to receive first work item
        if(!sendResultAndRequestNewWork()) {
            return; // return if unrecoverable error in networking occurred
        }

        // stop if server tells this worker to stop or if the optimization stop criteria is fulfilled
        while(!stopRequestReceived_ && !halt_()) {
            // swap messages: serialize the (processed) container into outgoingMessage_, then deserialize
            // incomingMessage_ into the container. Both run under the layout send-once wire scope
            // the outgoing RESULT ships its layout to the master in full only the first time
            // and by id thereafter; the incoming COMPUTE resolves an id-only layout from the local cache
            // or, on a miss, via the fetch round trip. This deserialise is sequenced before the async
            // send/receive below, so a fetch here never overlaps this rank's other MPI traffic.
            outgoingMessage_ =
                Gem::Courtier::wireEncode(commandContainer_, &wireCtx_, config_.serializationMode);
            Gem::Courtier::wireDecode(
                incomingMessage_,
                commandContainer_,
                &wireCtx_,
                config_.serializationMode
            );

            if(config_.useAsyncReq) {
                std::future<bool> networkingSuccessful =
                    std::async(std::launch::async, [this]() -> bool {
                        return this->sendResultAndRequestNewWork();
                    });

                // process the currently available work item (might be a stop request)
                processWorkItem();

                if(!networkingSuccessful.get()) {
                    return; // return if unrecoverable error in networking occurred
                }
            }
            else {
                // process the currently available work item (might be a stop request)
                processWorkItem();

                if(!sendResultAndRequestNewWork()) {
                    return; // return if unrecoverable error in networking occurred
                }
            }
        }

        // await last server response for the due to double buffering unnecessarily send our request
        if(this->stopRequestReceived_) {
            processLastResponse();
        }
    }

private:
    /**
         * @brief Sends the pending outgoing message and requests/receives the next work item.
         *
         * Sends the current contents of outgoingMessage_ to the master node and requests a new work item which will
         * be assigned to incomingMessage_. Therefore both members outgoingMessage_ and incomingMessage_ are mutated
         * inside of this function and shall not be mutated from other threads at the same time.
         *
         * @return true if successful, otherwise false.\n
         * If unsuccessful, the members incomingMessage_ and outgoingMessage_ are undefined.
         * Once this call returns the asynchronous mpi communication requests associated with the handles, which are
         * stored as member variables of this class, are guaranteed to be completed successfully or a network error has occurred.
         *
         */
    [[nodiscard]] bool sendResultAndRequestNewWork() {
        // start asynchronous send call to send result of last computation (or GETDATA command if no result available)
        MPI_Isend(
            outgoingMessage_.data(),
            outgoingMessage_.size(),
            MPI_CHAR,
            RANK_MASTER_NODE,
            TAG_REQUEST_WORK_ITEM,
            MPI_COMMUNICATOR,
            &sendHandle_
        );

        MPI_Status status{};

        // Wait until sending completed -- bounded, so a dead/unresponsive master cannot hang the
        // worker here forever.
        if(not waitForRequestOrTimeout(sendHandle_, status)) {
            glogger
                << "In GMPIConsumerWorkerNodeT<processable_type>::sendResultAndRequestNewWork() "
                   "with rank="
                << commRank_ << ":" << '\n'
                << "Timed out (or was halted) while sending to GMPIConsumerMasterNodeT." << '\n'
                << "The master appears to be gone; worker node will shut down." << '\n'
                << GWARNING;

            return false;
        }

        if(status.MPI_ERROR != MPI_SUCCESS) {
            glogger
                << "In GMPIConsumerWorkerNodeT<processable_type>::sendResultAndRequestNewWork() "
                   "with rank="
                << commRank_ << ":" << '\n'
                << "Received an error sending a message to GMPIConsumerMasterNodeT:" << '\n'
                << mpiErrorString(status.MPI_ERROR) << '\n'
                << "Worker node will shut down." << '\n'
                << GWARNING;

            return false;
        }

        // Wait until the master's response is available -- bounded for the same reason (this is where a
        // dead master would otherwise hang the worker indefinitely). Probe first so the response can be
        // of any size (the master may inline a large full-layout work item); receiveProbedMessage() then
        // sizes the receive to the message exactly, removing the old fixed-size cap.
        if(not probeWithTimeout(RANK_MASTER_NODE, MPI_ANY_TAG, status)) {
            glogger
                << "In GMPIConsumerWorkerNodeT<processable_type>::sendResultAndRequestNewWork() "
                   "with rank="
                << commRank_ << ":" << '\n'
                << "Timed out (or was halted) while waiting for a response from "
                   "GMPIConsumerMasterNodeT."
                << '\n'
                << "The master appears to be gone; worker node will shut down." << '\n'
                << GWARNING;

            return false;
        }

        if(status.MPI_ERROR != MPI_SUCCESS) {
            glogger
                << "In GMPIConsumerWorkerNodeT<processable_type>::sendResultAndRequestNewWork() "
                   "with rank="
                << commRank_ << ":" << '\n'
                << "Received an error receiving a message from GMPIConsumerMasterNodeT:"
                << '\n'
                << mpiErrorString(status.MPI_ERROR) << '\n'
                << "Worker node will shut down." << '\n'
                << GWARNING;

            return false;
        }

        // Receive the response, sized exactly to its content.
        incomingMessage_ = receiveProbedMessage(status);

        return true;
    }

    /**
         * Waits for an outstanding MPI request to complete, polling so the worker can give up if the
         * master goes silent (GMPICONSUMERWORKERMPITIMEOUT) or a halt was requested, instead of
         * blocking forever in MPI_Wait. On timeout/halt the request is cancelled and reclaimed so it
         * cannot outlive into MPI_Finalize.
         *
         * @param handle The outstanding MPI request to wait on (cancelled and reclaimed on timeout/halt)
         * @param status The MPI_Status filled in if the request completes
         * @return true if the request completed (status filled); false on timeout / halt.
         */
    [[nodiscard]] bool waitForRequestOrTimeout(MPI_Request &handle, MPI_Status &status) {
        const auto deadline = std::chrono::steady_clock::now() + GMPICONSUMERWORKERMPITIMEOUT;

        while(true) {
            int isCompleted{0};
            MPI_Test(&handle, &isCompleted, &status);
            if(isCompleted) {
                return true;
            }
            if(halt_() || std::chrono::steady_clock::now() >= deadline) {
                MPI_Cancel(&handle);
                MPI_Wait(&handle, MPI_STATUS_IGNORE);
                return false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds{1});
        }
    }

    /**
         * Polls (with the same bounded give-up behaviour as waitForRequestOrTimeout) for an incoming
         * message from @p source on @p tag, without posting a receive. On success @p status carries the
         * pending message's envelope, from which the caller reads its exact size (MPI_Get_count) before
         * receiving it -- so a message of any size can be received without a fixed-size buffer cap.
         *
         * @param source The expected sender rank
         * @param tag The expected message tag (may be MPI_ANY_TAG)
         * @param status Filled with the pending message's envelope on success
         * @return true if a matching message is pending (status filled); false on timeout / halt.
         */
    [[nodiscard]] bool probeWithTimeout(int source, int tag, MPI_Status &status) {
        const auto deadline = std::chrono::steady_clock::now() + GMPICONSUMERWORKERMPITIMEOUT;

        while(true) {
            int isAvailable{0};
            MPI_Iprobe(source, tag, MPI_COMMUNICATOR, &isAvailable, &status);
            if(isAvailable) {
                return true;
            }
            // A probe posts no request, so on give-up there is nothing to cancel.
            if(halt_() || std::chrono::steady_clock::now() >= deadline) {
                return false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds{1});
        }
    }

    /**
         * Receives the message just reported by a successful probe, sized exactly to its content (no
         * fixed cap). Matches the probed source/tag so it receives precisely that message.
         *
         * @param probeStatus The status filled by a successful probeWithTimeout()
         * @return The received message as a string
         */
    std::string receiveProbedMessage(const MPI_Status &probeStatus) {
        int count{0};
        MPI_Get_count(&probeStatus, MPI_CHAR, &count);
        std::string message(static_cast<std::size_t>(count), '\0');
        MPI_Status recvStatus{};
        MPI_Recv(
            message.data(),
            count,
            MPI_CHAR,
            probeStatus.MPI_SOURCE,
            probeStatus.MPI_TAG,
            MPI_COMMUNICATOR,
            &recvStatus
        );
        return message;
    }

    /**
         * @brief Processes the work item currently stored in commandContainer_.
         *
         * After processing has been finished, the result is put into commandContainer_ i.e. it overrides the old item.
         * In case that commandContainer_ did not contain any work items this method will store a new GETDATA request
         * in commandContainer_ to retrieve new work when sending this message. A STOP command sets the
         * stop flag instead.
         */
    void processWorkItem() {
        switch(commandContainer_.get_command()) {
            using enum Gem::Courtier::networked_consumer_payload_command;
        case COMPUTE: {
            // process item. This will put the result into the container
            commandContainer_.process();

            // increment the counter for processed items
            incrementProcessingCounter_();

            // mark the container as "contains a result"
            commandContainer_.set_command(networked_consumer_payload_command::RESULT);
        } break;
        case NODATA: {
            // Update the NODATA counter for bookkeeping
            ++nNoData_;

            // sleep for a short random time interval
            std::uniform_int_distribution<> dist(
                GMPICONSUMERWORKERNODERETRYINTERVALLOWERBOUNDARYMSEC,
                GMPICONSUMERWORKERNODERETRYINTERVALUPPERBOUNDARYMSEC
            );
            std::this_thread::sleep_for(std::chrono::milliseconds(dist(randomNumberEngine_)));

            // Tell the server again we need work
            commandContainer_.reset(networked_consumer_payload_command::GETDATA);
        } break;
        case STOP: {
            this->stopRequestReceived_ = true;
        } break;
        default: {
            // Emit a warning, ignore item and request new item
            glogger << "GMPIConsumerWorkerNodeT<processable_type>::processWorkItem() with rank="
                    << commRank_ << ":" << '\n'
                    << "Got unknown or invalid command "
                    << commandContainer_.get_command()
                    << '\n'
                    << GWARNING;

            commandContainer_.reset(networked_consumer_payload_command::GETDATA);
        }
        }
    }

    /**
         * @brief Consumes the final master response after the worker has been told to stop.
         *
         * If we have been stopped, we leave the loop here but have sent one more request out
         * because of double buffered requests. The server must await this additional request, because all
         * MPI communication should be completed before shutdown.
         * For this reason, we receive another request that we expect to be a stop request.
         * This last request does not have to be answered, since the server will shut down after each worker has
         * received a last its request.
         */
    void processLastResponse() {
        Gem::Courtier::wireDecode(
            incomingMessage_,
            commandContainer_,
            &wireCtx_,
            config_.serializationMode
        );

        if(commandContainer_.get_command() != networked_consumer_payload_command::STOP) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GMPIConsumerWorkerNodeT<processable_type>::processLastResponse() with rank="
                << commRank_ << ":" << '\n'
                << "Expected to receive the last stop request but instead received message with "
                   "command "
                << commandContainer_.get_command() << '\n'
            );
        }
    }

    /**
         * @brief Worker-side cache-miss fetch (layout send-once): blocks until the master's layout for @p id is in
         * hand, then returns the serialized blob (empty on failure).
         *
         * Sends a REQUEST_LAYOUT message (carrying the wanted id) to the master on TAG_REQUEST_LAYOUT and
         * waits, bounded, for the SEND_LAYOUT reply on TAG_SEND_LAYOUT. The master's receiver loop matches
         * any tag, so it picks up the request and dispatches a session that answers from its registry. A
         * distinct send tag keeps the reply out of the worker's ordinary (double-buffered) MPI_ANY_TAG
         * receive. This is called from inside the work-item deserialise, which is sequenced before this
         * iteration's outgoing send/receive is launched, so it does not overlap other MPI traffic on this
         * rank; the dedicated tag is a belt-and-braces guard.
         *
         * @param id The content id of the layout to fetch from the master.
         * @return The serialized layout blob, or an empty string if the fetch failed / timed out.
         */
    std::string fetchLayoutBlob_(const Gem::Courtier::GWireLayoutId &id) {
        // Build and serialise the REQUEST_LAYOUT message (no genome payload, so no nested wire scope).
        std::string requestStr;
        // Build the REQUEST_LAYOUT message (under a null scope; carries no genome). MPI identifies the
        // worker by rank, so no peer id is needed (the default 0 is sent).
        requestStr = Gem::Courtier::buildLayoutRequest<processable_type>(
            id,
            /* peer = */ 0,
            config_.serializationMode
        );

        // Blocking send of the request to the master.
        MPI_Request sendReq{};
        MPI_Isend(
            requestStr.data(),
            requestStr.size(),
            MPI_CHAR,
            RANK_MASTER_NODE,
            TAG_REQUEST_LAYOUT,
            MPI_COMMUNICATOR,
            &sendReq
        );
        MPI_Status status{};
        if(not waitForRequestOrTimeout(sendReq, status) || status.MPI_ERROR != MPI_SUCCESS) {
            glogger << "In GMPIConsumerWorkerNodeT<processable_type>::fetchLayoutBlob_() with rank="
                    << commRank_ << ":" << '\n'
                    << "Timed out / errored sending a REQUEST_LAYOUT to the master." << '\n'
                    << GWARNING;
            return {};
        }

        // Receive the SEND_LAYOUT reply on its dedicated tag. Probe first so a layout blob of any size
        // can be received (a large layout is exactly what would have exceeded the old fixed cap).
        if(not probeWithTimeout(RANK_MASTER_NODE, TAG_SEND_LAYOUT, status) ||
           status.MPI_ERROR != MPI_SUCCESS) {
            glogger << "In GMPIConsumerWorkerNodeT<processable_type>::fetchLayoutBlob_() with rank="
                    << commRank_ << ":" << '\n'
                    << "Timed out / errored waiting for the SEND_LAYOUT reply from the master." << '\n'
                    << GWARNING;
            return {};
        }

        // Deserialise the reply (again no nested wire scope) and hand back the blob.
        const std::string replyStr = receiveProbedMessage(status);
        return Gem::Courtier::parseLayoutReply<processable_type>(replyStr, config_.serializationMode);
    }

    //-------------------------------------------------------------------------
    // Private data

    /**
         * rank of this node in the cluster
         */
    std::int32_t commRank_;
    /**
         * Callback function that returns true if the halt criterion has been reached
         */
    std::move_only_function<bool()> halt_;
    /**
         * Increments the counter for processed work items of the calling instance of GConsumerBaseT.
         */
    std::move_only_function<void()> incrementProcessingCounter_;
    /**
         * reference to configuration specified by the end-user.
         */
    const MPIConsumerConfig &config_;
    /**
         * Whether a stop request from the master node has been received
         */
    bool stopRequestReceived_{false};

    // only one request is processed at a time. Even in the version with asynchronous request the thread which is
    // responsible for handling the IO will always be joined before the next thread for the next IO-operation is spawned.
    // So the program logic ensures that the access to these resources is exclusive to one thread, so we do not
    // need to enforce this with mutex or something similar and can store the handles as members.
    MPI_Request sendHandle_{};

    /**
         * counter for how many times we have not received data when requesting data from the master node
         */
    std::int32_t nNoData_{0};

    std::random_device randomDevice_; ///< Source of non-deterministic random numbers
    std::mt19937 randomNumberEngine_{
        randomDevice_()
    }; ///< The actual random number engine, seeded by randomDevice_

    std::string incomingMessage_;
    std::string outgoingMessage_;
    // contains the current command and payload (if any)
    GCommandContainerT<processable_type, networked_consumer_payload_command> commandContainer_{
        networked_consumer_payload_command::GETDATA
    };

    /// layout send-once (worker side): this rank's local cache of received layouts and the wire
    /// context engaged around (de)serialisation. The context's fetch_blob resolves a cache miss via a
    /// blocking REQUEST_LAYOUT / SEND_LAYOUT MPI round trip (see fetchLayoutBlob_).
    Gem::Courtier::GWireLayoutRegistry wireRegistry_;
    Gem::Courtier::GWireSerializationContext wireCtx_;
};

/**
     * This class represents one session between the master node and one worker node.
     *
     * A GMPIConsumerSessionT can be opened as soon as the master node has fully received a request from
     * a worker node. The opened GMPIConsumerSessionT will then take care of deserializing and processing
     * the request as well as responding to it with a new work item (if there are items available in the brokers queue
     * at that point in time).
     *
     * @tparam processable_type the type of work item exchanged with the worker node
     */
template <typename processable_type>
class GMPIConsumerSessionT // NOLINT(cppcoreguidelines-special-member-functions)
  : public std::enable_shared_from_this<GMPIConsumerSessionT<processable_type>> {
public:
    /**
         * @brief Constructor for GMPIConsumerSessionT.
         *
         * @param status MPI_Status object that stores information about the received request, most importantly its source
         * i.e. the rank of the worker node sending the request
         * @param requestMessage the payload of the request this session was opened for
         * @param getPayloadItem a callback function to retrieve payload items (raw work items) from their origin / producer
         * @param putPayloadItem a callback function to put payload items (processed work items) to their destination
         * @param serializationMode the mode of serialization between the master nodes and the worker nodes
         * @param stopRequested whether the server is asked to stop and therefore should only send stop requests to clients
         * instead of further work items
         */
    GMPIConsumerSessionT(
        MPI_Status status,
        std::string requestMessage,
        std::move_only_function<std::unique_ptr<processable_type>()> getPayloadItem,
        std::move_only_function<void(std::unique_ptr<processable_type>)> putPayloadItem,
        Gem::Common::serializationMode serializationMode,
        bool stopRequested,
        Gem::Courtier::GWireLayoutRegistry *wireRegistry = nullptr
    )
      : mpiStatus_{status}
      ,
      // avoid copying the string but also not taking it as reference because it should be owned by this object
      requestMessage_{std::move(requestMessage)}
      , serializationMode_{serializationMode}
      , stopRequested_{stopRequested}
      , getPayloadItem_(std::move(getPayloadItem))
      , putPayloadItem_(std::move(putPayloadItem))
      , mpiRequestHandle_{}
      , wireRegistry_{wireRegistry} {
        // Engage the master side of the layout send-once wire form, if a registry was supplied.
        // MPI ranks are persistent, so the peer id is simply the requesting worker's rank
        // (mpiStatus_.MPI_SOURCE) -- naturally stable across the whole run. With no registry the scope is
        // never installed and the genome falls back to its self-contained full-layout encoding.
        wireCtx_.enabled = (wireRegistry_ != nullptr);
        wireCtx_.peer = static_cast<Gem::Courtier::GWirePeerId>(mpiStatus_.MPI_SOURCE);
        wireCtx_.registry = wireRegistry_;
        wireCtx_.mode = serializationMode_;
    }

    //-------------------------------------------------------------------------
    // Deleted constructors and assignment operators

    GMPIConsumerSessionT() = delete;

    GMPIConsumerSessionT(const GMPIConsumerSessionT<processable_type> &) = delete;

    GMPIConsumerSessionT(GMPIConsumerSessionT<processable_type> &&) = delete;

    GMPIConsumerSessionT<processable_type> &
    operator=(const GMPIConsumerSessionT<processable_type> &) = delete;

    GMPIConsumerSessionT<processable_type> &
    operator=(GMPIConsumerSessionT<processable_type> &&) = delete;

    //-------------------------------------------------------------------------
    // public functions that are not constructors or operators

    /**
         * @brief Answers the request this session was created for.
         *
         * Processes the inbound request and, if that succeeded, sends the response.
         */
    void run() {
        // only execute sendResponse if processRequest was successful
        if(processRequest()) {
            sendResponse();
        }
    }

    /**
         * @brief Checks whether sending the response to the worker has completed.
         *
         * This assumes that the run-method has already been called and finished execution.
         *
         * @return true if sending the response has been completed, otherwise false
         */
    [[nodiscard]] bool isCompleted() {
        int isCompleted{0};

        MPI_Test(&mpiRequestHandle_, &isCompleted, MPI_STATUS_IGNORE);

        return isCompleted;
    }

    /**
         * @brief Abandons the outstanding response send if it has not completed, reclaiming its MPI_Request.
         *
         * Reclaims its MPI_Request so
         * it cannot outlive into MPI_Finalize. Used during shutdown to release sessions whose worker
         * died before receiving the response (so the master does not finalize with pending requests).
         */
    void cancelPendingResponse() {
        int isCompleted{0};
        MPI_Test(&mpiRequestHandle_, &isCompleted, MPI_STATUS_IGNORE);
        if(not isCompleted) {
            MPI_Cancel(&mpiRequestHandle_);
            MPI_Wait(&mpiRequestHandle_, MPI_STATUS_IGNORE);
        }
    }

    /**
         * @brief Returns the command this session is sending out to the worker in the response.
         * @return command that the session is sending out to the client in the response
         */
    [[nodiscard]] networked_consumer_payload_command getOutCommand() const {
        return this->commandContainer_.get_command();
    }

private:
    /**
         * @brief Deserializes and acts on the inbound request.
         *
         * On a RESULT command the payload is delivered to the broker; a GETDATA command carries no
         * payload. Unknown commands and deserialization failures are logged.
         *
         * @return true if the request was a valid RESULT or GETDATA, false otherwise
         */
    bool processRequest() {
        try {
            // Deserialize the request under the wire scope (layout send-once): a returned RESULT genome may
            // reference its layout by id, resolved against the master's shared registry (which holds
            // every layout it has sent). The scope's peer is the requesting rank, set in the constructor.
            Gem::Courtier::wireDecode(
                requestMessage_,
                commandContainer_,
                wireCtx_.enabled ? &wireCtx_ : nullptr,
                serializationMode_
            ); // may throw

            // Extract the command
            auto inboundCommand = commandContainer_.get_command();

            // If we have some payload received, add it to its destination
            switch(inboundCommand) {
                using enum Gem::Courtier::networked_consumer_payload_command;
            case RESULT: {
                putWorkItem();
                return true;
            }
            case GETDATA: {
                return true; // no data to process
            }
            case REQUEST_LAYOUT: {
                // Layout cache-miss fetch: remember the requested id; sendResponse() will answer with a
                // SEND_LAYOUT carrying the serialized layout from the registry.
                isLayoutRequest_ = true;
                requestedLayoutId_ = commandContainer_.get_layout_id();
                return true;
            }
            default: { // clients may only send RESULT, GETDATA or REQUEST_LAYOUT commands
                glogger
                    << "GMPIConsumerSessionT<processable_type>::processRequest() connected to rank="
                    << mpiStatus_.MPI_SOURCE << ":" << '\n'
                    << "Got unknown or invalid command "
                    << inboundCommand << '\n'
                    << GWARNING;
            }
            }
        }
        catch(const geneva_exception &ex) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "GMPIConsumerSessionT<processable_type>::processRequest() connected to rank="
                << mpiStatus_.MPI_SOURCE << ":" << '\n'
                << "Caught exception while deserializing request" << '\n'
                << ex.what() << '\n'
            );
        }

        return false;
    }

    /**
         * @brief Releases the processed payload from the command container and hands it to the broker sink.
         *
         * If the container unexpectedly holds no payload, a warning is logged and the request is still
         * answered normally.
         */
    void putWorkItem() {
        // Retrieve the payload from the command container
        auto payloadPtr = commandContainer_.release_payload();

        // Submit the payload to the server (which will send it to the broker)
        if(payloadPtr) {
            putPayloadItem_(std::move(payloadPtr));
            return;
        }

        glogger << "GMPIConsumerSessionT<processable_type>::process_request() connected to rank="
                << mpiStatus_.MPI_SOURCE << ":" << '\n'
                << "payload is empty even though a result was expected." << '\n'
                << "However, this request will also be responded normally." << '\n'
                << GWARNING;
    }

    /**
         * @brief Assigns a new command and payload (if any) to the commandContainer_ member.
         *
         * Fetches a work item from the broker; on success stores it with a COMPUTE command, otherwise
         * stores a NODATA command.
         */
    void prepareDataResponse() {
        // Check a work item out of the queue (the callable includes a timeout that may yield nullptr)
        // and store it as COMPUTE, or NODATA when the queue is empty -- the shared server-side decision.
        Gem::Courtier::serveWorkItem(commandContainer_, this->getPayloadItem_);
    }

    /**
         * @brief Assigns a stop request to the commandContainer_ member.
         */
    void prepareStopResponse() {
        // store a stop request in the command container
        commandContainer_.reset(networked_consumer_payload_command::STOP);
    }

    /**
         * @brief Serializes the commandContainer_ member into outgoingMessage_ for transmission.
         *
         * Any message size is permitted: the receiver probes the incoming message and sizes its receive
         * to fit (see GMPIConsumerMasterNodeT::listenForRequests / the worker's receiveProbedMessage),
         * so there is no fixed send cap.
         */
    void serializeOutgoingMsg() {
        // Serialize the response under the wire scope (layout send-once): a COMPUTE work item's layout is shipped
        // in full to this peer (rank) only the first time it is seen and by content id thereafter. A
        // NODATA / STOP carries no genome, so the scope is harmless there.
        outgoingMessage_ = Gem::Courtier::wireEncode(
            commandContainer_,
            wireCtx_.enabled ? &wireCtx_ : nullptr,
            serializationMode_
        );
    }

    /**
         * @brief Starts an asynchronous send of the response message to the worker.
         *
         * Prepares either a stop or a data response, serializes it, and issues the asynchronous send.
         * The isCompleted()-method can be used to check for the completion of the send operation.
         */
    void sendResponse() {
        // A layout cache-miss fetch is answered on its own tag, independent of work-item flow:
        // the requesting worker is mid-decode and blocked waiting for exactly this reply, so it is served
        // even while the master is shutting down.
        if(isLayoutRequest_) {
            sendLayoutResponse();
            return;
        }

        // prepare the correct type of message in the outgoing command
        if(stopRequested_) {
            prepareStopResponse();
        }
        else {
            prepareDataResponse();
        }

        // serialize the outgoing message
        serializeOutgoingMsg();

        // asynchronously start sending the response
        MPI_Isend(
            outgoingMessage_.data(),
            outgoingMessage_.size(),
            MPI_CHAR,
            mpiStatus_.MPI_SOURCE,
            TAG_SEND_WORK_ITEM,
            MPI_COMMUNICATOR,
            &mpiRequestHandle_
        );

        // the isCompleted method can be used to check if the send-operation has been completed
    }

    /**
         * @brief Answers a worker's REQUEST_LAYOUT with a SEND_LAYOUT carrying the serialized layout from
         * the master's registry (layout cache-miss fetch). Sent on TAG_SEND_LAYOUT so it is not mistaken
         * for a work-item response by the worker's ordinary receive. If the id is not (or no longer)
         * cached the blob is left empty and the worker treats the fetch as failed.
         */
    void sendLayoutResponse() {
        // Build the SEND_LAYOUT reply from the master's shared registry (under a null scope; the reply
        // carries only the raw blob, no genome). Empty blob on a miss -> the worker fails the fetch.
        outgoingMessage_ = Gem::Courtier::buildLayoutReply<processable_type>(
            requestedLayoutId_,
            wireRegistry_,
            serializationMode_
        );

        MPI_Isend(
            outgoingMessage_.data(),
            outgoingMessage_.size(),
            MPI_CHAR,
            mpiStatus_.MPI_SOURCE,
            TAG_SEND_LAYOUT,
            MPI_COMMUNICATOR,
            &mpiRequestHandle_
        );
    }

    //-------------------------------------------------------------------------
    // Data
    /**
         * mpi status of the request this session has been opened for
         */
    const MPI_Status mpiStatus_;
    const std::string requestMessage_;
    const Gem::Common::serializationMode serializationMode_;
    /**
         * Whether the master node is asked to stop and should therefore respond with stop requests to all data requests.
         */
    const bool stopRequested_;
    /**
         * function to retrieve a work item from the broker
         */
    std::move_only_function<std::unique_ptr<processable_type>()> getPayloadItem_;
    /**
         * function to deliver a processed work item to the broker
         */
    std::move_only_function<void(std::unique_ptr<processable_type>)> putPayloadItem_;
    /**
         * Command and payload received/processed (depends on current state of session)
         */
    GCommandContainerT<processable_type, networked_consumer_payload_command> commandContainer_{
        networked_consumer_payload_command::NONE
    };
    MPI_Request mpiRequestHandle_;
    /**
         * serialized command container to be send out to worker
         */
    std::string outgoingMessage_;

    /// layout send-once (master side): the consumer-shared registry (not owned) and the wire
    /// scope installed around (de)serialisation, with the peer set to the requesting worker's rank. When
    /// the inbound request is a REQUEST_LAYOUT, isLayoutRequest_ is set and requestedLayoutId_ holds the
    /// wanted id so sendResponse() answers with a SEND_LAYOUT instead of a work item.
    Gem::Courtier::GWireLayoutRegistry *wireRegistry_ = nullptr;
    Gem::Courtier::GWireSerializationContext wireCtx_;
    bool isLayoutRequest_ = false;
    Gem::Courtier::GWireLayoutId requestedLayoutId_{0, 0};
};

/**
     *
     * This class is responsible for the server side of network communication using MPI.
     *
     * GMPIConsumerMasterNodeT will constantly wait for incoming work items requests, process them and answer them
     * by opening a new GMPIConsumerSessionT for each request.
     *
     * @tparam processable_type a type that is processable (e.g. a GGenome-derived individual)
     *
     *
     * The simplified workflow of the GMPIConsumerMasterNodeT can be described as follows:
     *
     * (1) Create thread pool\n
     * (2) Spawn a new thread that receives request (receiverThread). Then return from the function immediately, because
     * geneva expects consumers to be background processes on other threads than the main thread.\n
     * (3) Once a shutdown is supposed to occur:\n
     *  (3.1) Set a member flag to tell the receiver thread to stop\n
     *  (3.2) Receiver thread will then respond with stop requests to all workers and leave once all workers have shut down
     *  (3.3) Join all threads.
     *
     * Then the main loop in the receiver thread works as follows:\n
     * (2.1) Asynchronously receive message from any worker node if not yet told to stop (check thread safe member variable)\n
     * (2.2) Once a request has been received: schedule a handler on the thread pool and wait for another message
     *     to receive i.e. go back to (2.1)\n\n
     *
     * Then the handler thread works as follows (implemented in the class GMPIConsumerSessionT):\n
     *  (3.1) Deserialize received object\n
     *  (3.2) If the message from the worker includes a processed item, put it into the processed items queue of the broker\n
     *  (3.3) Take an item from the non-processed items queue (if currently there is one available)\n
     *  (3.4) Serialize the response container, which contains a new work item or the NODATA command.\n
     *  (3.5) Asynchronously send the item to the worker node which has requested it.\n
     *  (3.6) Exit the handler. This lets the thread pool use this thread for future incoming requests.\n
     *
     */
template <typename processable_type>
class GMPIConsumerMasterNodeT // NOLINT(cppcoreguidelines-special-member-functions)
  : public std::enable_shared_from_this<GMPIConsumerMasterNodeT<processable_type>> {
public:
    /**
         * @brief Constructor to instantiate the GMPIConsumerMasterNodeT.
         * @param commSize number of nodes in the cluster, which is equal to the number of workers + 1
         * @param config configuration for this node specified by the end user
         */
    explicit GMPIConsumerMasterNodeT(std::int32_t commSize, const MPIConsumerConfig &config)
      : commSize_{commSize}
      , config_{config}
      , isToldToStop_{false} {
        glogger << "GMPIConsumerMasterNodeT started with " << config_.nHandlerThreads
                << " handler threads" << '\n'
                << GLOGGING;
    }

    //-------------------------------------------------------------------------
    // Deleted copy-/move-constructors and assignment operators.
    GMPIConsumerMasterNodeT(const GMPIConsumerMasterNodeT<processable_type> &) = delete;

    GMPIConsumerMasterNodeT(GMPIConsumerMasterNodeT<processable_type> &&) = delete;

    GMPIConsumerMasterNodeT &operator=(const GMPIConsumerMasterNodeT<processable_type> &) = delete;

    GMPIConsumerMasterNodeT &operator=(GMPIConsumerMasterNodeT<processable_type> &&) = delete;

    /**
         * @brief Starts the background threads that run the GMPIConsumerMasterNodeT.
         *
         * First a thread-pool will be created. Then another thread will be created which listens for requests and
         * schedules request handlers on the thread-pool. Furthermore a cleanup thread will be created, which closes open
         * sessions in a cyclic manner.
         * Once the threads have been created, this method will immediately
         * return to the caller while the spawned threads run in the background and fulfil their job.
         * To stop the master node and all its threads again the shutdown()-method can be called.
         */
    void async_startProcessing() {
        handlerThreadPool_ = std::make_unique<Common::Concurrency::GThreadPool>(config_.nHandlerThreads);

        auto self = this->shared_from_this();
        receiverThread_ = std::thread([self] { self->listenForRequests(); });

        cleanUpThread_ = std::thread([self] { self->cleanUpSessionsLoop(); });
    }

    /**
         * @brief Sends a shutdown signal to the GMPIConsumerMasterNodeT and joins all its background threads.
         *
         * Once this method has returned this means that all threads have been joined, all asynchronous mpi communication
         * requests have been either completed, and the server is shut down completely.
         */
    void shutdown() {
        // notify other threads to stop
        isToldToStop_.store(true);

        // wait for the receiver thread to send a stop request to each client
        receiverThread_.join();

        // wait until for threads to finish their work i.e. send the stop requests out to the clients
        handlerThreadPool_->wait();

        // wait for the cleanup thread. This thread will close all open sessions before joining
        cleanUpThread_.join();
    }

private:
    /**
         * @brief Receiver loop: accepts worker requests and schedules a handler for each on the thread pool.
         *
         * Runs until the required number of stop responses has been dispatched (two per worker, due to
         * the workers' double-buffered requests). Once shutdown is requested, live workers get a short
         * grace window to send their final requests before any outstanding receive is abandoned, so a
         * worker that died before its final handshake cannot wedge shutdown.
         */
    void listenForRequests() {
        // number of workers that we have send a stop request to
        uint32_t stopRequestsSendOut{0};
        // Each client sends out one last request although having receive a stop that has to be answered
        // by the server since the clients use double buffering
        const int32_t reqNumStops{2 * (this->commSize_ - 1)};

        while(stopRequestsSendOut < reqNumStops) {
            // Probe (rather than post a fixed-size receive) so a request of ANY size can be received:
            // MPI_Get_count then tells us the exact length and we allocate to fit. This removes the old
            // fixed message-size cap, which a large genome's first (full-layout) work item
            // or a big SEND_LAYOUT reply could exceed. The receive side is single-threaded (only this
            // listener probes/receives; handler threads merely send), so the probe -> receive pair below
            // is race-free.
            int isAvailable{0};
            MPI_Status status{};
            std::optional<std::chrono::steady_clock::time_point> giveUpAt;

            while(true) {
                MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMMUNICATOR, &isAvailable, &status);
                if(isAvailable) {
                    break;
                }
                // Do not busy-spin, and make shutdown observable: once a stop has been requested, give
                // live workers a short grace window to send their final (double-buffered) requests, then
                // stop listening. Otherwise a worker that died before its final handshake would wedge
                // this loop -- and thus shutdown() -- and MPI_Finalize would never be reached.
                if(isToldToStop_.load()) {
                    const auto now = std::chrono::steady_clock::now();
                    if(not giveUpAt) {
                        giveUpAt = now + GMPICONSUMERSHUTDOWNGRACE;
                    }
                    if(now >= *giveUpAt) {
                        break; // isAvailable stays 0
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds{1});
            }

            if(not isAvailable) {
                // Shutting down and a straggler request never arrived: nothing is posted (a probe holds
                // no MPI request), so simply stop listening.
                break;
            }

            // A message is pending; receive it at its exact size. Match the probed source/tag so we
            // receive precisely the message we just probed.
            int count{0};
            MPI_Get_count(&status, MPI_CHAR, &count);
            auto buffer = std::make_shared<char[]>(static_cast<std::size_t>(count));
            MPI_Status recvStatus{};
            MPI_Recv(
                buffer.get(),
                count,
                MPI_CHAR,
                status.MPI_SOURCE,
                status.MPI_TAG,
                MPI_COMMUNICATOR,
                &recvStatus
            );

            // Dispatch it to a handler thread. We capture copies of the smart pointers in the closure,
            // which keeps the underlying data alive.
            const bool stopRequested = isToldToStop_.load();
            if(stopRequested) {
                ++stopRequestsSendOut;
            }
            const auto self = this->shared_from_this();
            handlerThreadPool_->async_schedule([self, recvStatus, buffer, stopRequested] {
                self->handleRequest(recvStatus, buffer, stopRequested);
            });
        }
    }

    /**
         * @brief Handles a single received request by opening, running and tracking a session.
         *
         * Runs on a thread-pool thread. If the receive carried an MPI error the request is not
         * answered. Otherwise a GMPIConsumerSessionT is created, run (which sends the response), and
         * pushed onto the open-session list for the cleanup thread to reap.
         *
         * @param status The MPI_Status of the received request (its source is the requesting worker)
         * @param buffer The shared buffer holding the received (serialized) request message
         * @param stopRequested Whether the master is shutting down and should respond with a stop request
         */
    void handleRequest(
        const MPI_Status &status,
        const std::shared_ptr<char[]> &buffer,
        const bool stopRequested
    ) {
        if(status.MPI_ERROR != MPI_SUCCESS) {
            glogger << "In GMPIConsumerMasterNodeT<processable_type>::handleRequest():" << '\n'
                    << "Received an error:" << '\n'
                    << mpiErrorString(status.MPI_ERROR) << '\n'
                    << "Request from worker node will not be answered." << '\n'
                    << GWARNING;

            // return from this handler, which means not answering the request
            return;
        }

        // start a new session
        auto session = std::make_shared<GMPIConsumerSessionT<processable_type>>(
            status,
            std::string{buffer.get(), static_cast<size_t>(mpiGetCount(status))},
            [this]() -> std::unique_ptr<processable_type> { return getPayloadItem(); },
            [this](std::unique_ptr<processable_type> p) { putPayloadItem(std::move(p)); },
            config_.serializationMode,
            stopRequested,
            &wireRegistry_ // layout send-once: the registry shared by all sessions of this master
        );

        // runs the session but does not close it
        session->run();

        // push the open session to a vector of open sessions
        // the cleanUpThread_ will make sure of waiting until these sessions complete or result in an error
        pushOpenSession(session);

        // This thread of the thread-pool will then be able to be scheduled for further requests by the IO-thread again
    }

    /**
         * @brief Adds a session to the (mutex-protected) list of open sessions awaiting completion.
         * @param session The session whose asynchronous response send is still in flight
         */
    void pushOpenSession(std::shared_ptr<GMPIConsumerSessionT<processable_type>> session) {
        std::scoped_lock guard(openSessionsMutex_);
        openSessions_.push_back(session);
    }

    /**
         * @brief Cleanup loop: reaps completed sessions on a single thread until shutdown is done.
         *
         * This is not a job that must be executed super fast. So we can save resources if we do not let this run on the
         * thread-pool but on a single thread. The thread-pool will then be ready for new work as soon as it has handled
         * the request without having to wait for the completion of the asynchronous operations.
         * The cleanup thread acts as a sort of multiplexer, by handling open sessions from various threads in the thread
         * pool. This is clever because the majority of the time we will be waiting for a session to complete. By handling
         * this by a single thread, we can overlap the waiting time of multiple sessions and only block one thread.
         *
         */
    void cleanUpSessionsLoop() {
        // track number of stop requests, for which the sending has completed
        uint32_t stopSendOutsCompleted{0};
        // two stop requests for each client
        const int32_t reqNumStops{2 * (this->commSize_ - 1)};
        std::optional<std::chrono::steady_clock::time_point> giveUpAt;

        // keep running until all stop requests have been sent out (after that no more sessions should
        // be opened because all clients will shut down) -- or, once shutdown was requested, until the
        // grace window elapses, so a session that will never complete (its worker died mid-exchange)
        // cannot wedge this thread and thus shutdown().
        while(stopSendOutsCompleted < reqNumStops) {
            // wait a short amount of time between checking if the sessions have been completed
            if(config_.masterCleanSessIntervalMSec > 0) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds{config_.masterCleanSessIntervalMSec}
                );
            }

            {
                // lock access to open sessions vector
                std::scoped_lock guard(openSessionsMutex_);

                for(auto sessionIter{openSessions_.begin()}; sessionIter != openSessions_.end();
                    /* no increment */) {
                    if((*sessionIter)->isCompleted()) {
                        // track the completed stop requests
                        if((*sessionIter)->getOutCommand() ==
                           networked_consumer_payload_command::STOP) {
                            ++stopSendOutsCompleted;
                        }

                        // erase this session because it has completed
                        sessionIter = openSessions_.erase(sessionIter);
                    }
                    else {
                        // increment iterator in case no session has been erased
                        ++sessionIter;
                    }
                }
            }

            if(isToldToStop_.load()) {
                const auto now = std::chrono::steady_clock::now();
                if(not giveUpAt) {
                    giveUpAt = now + GMPICONSUMERSHUTDOWNGRACE;
                }
                if(now >= *giveUpAt) {
                    break;
                }
            }
        }

        // Release any sessions still open (only reached on the grace-timeout path): cancel their
        // outstanding response sends so no MPI_Request outlives into MPI_Finalize.
        std::scoped_lock guard(openSessionsMutex_);
        for(auto &session : openSessions_) {
            session->cancelPendingResponse();
        }
        openSessions_.clear();
    }

    /**
         * @brief Retrieves a raw work item from the injected external source (if any).
         *
         * Uses the source set via setPayloadFunctors(); with no source set, no item is produced. The
         * former broker fallback was removed together with the legacy broker.
         *
         * @return A work item, or an empty pointer if no source is set or it produced none
         */
    std::unique_ptr<processable_type> getPayloadItem() {
        // If an external source has been injected (e.g. the courtier reconcile-the-span path),
        // use it instead of the broker. Default (no functor set) is the original broker behaviour.
        // The courtier consumer always injects a source via setPayloadFunctors(); the former broker
        // fallback was removed together with the legacy broker. An unset source yields no item.
        if(getPayloadItemFn_) {
            return getPayloadItemFn_();
        }
        return {};
    }

    //-------------------------------------------------------------------------
    /**
         * @brief Submits a processed work item to the injected external sink (if any).
         *
         * Throws a geneva_exception if @p p is empty. With no sink set the item is silently dropped.
         *
         * @param p The processed work item to deliver (must not be empty)
         */
    void putPayloadItem(std::unique_ptr<processable_type> p) {
        if(not p) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "GMPIConsumerMasterNodeT<>::putPayloadItem():" << '\n'
                << "Function called with empty work item" << '\n'
            );
        }

        // The courtier consumer always injects a sink via setPayloadFunctors(); the former broker
        // fallback was removed together with the legacy broker.
        if(putPayloadItemFn_) {
            putPayloadItemFn_(std::move(p));
        }
    }

public:
    //-------------------------------------------------------------------------
    /**
         * Injects an external source/sink for work items, bypassing the broker. This is the seam the
         * courtier networked-consumer path uses to drive the MPI master node from a span+policy
         * batch instead of the broker's buffer ports. With no functors set the node behaves exactly
         * as before (broker-backed), so this is behaviour-neutral for existing callers.
         *
         * @param getPayloadItemFn Source callback returning the next raw work item (or empty pointer)
         * @param putPayloadItemFn Sink callback receiving each processed work item
         */
    void setPayloadFunctors(
        std::move_only_function<std::unique_ptr<processable_type>()> getPayloadItemFn,
        std::move_only_function<void(std::unique_ptr<processable_type>)> putPayloadItemFn
    ) {
        getPayloadItemFn_ = std::move(getPayloadItemFn);
        putPayloadItemFn_ = std::move(putPayloadItemFn);
    }

    /**
         * @brief The number of distinct genome layouts the master has interned for transport (layout
         * send-once). One per distinct genome structure across all worker ranks.
         * @return The count of interned layouts.
         */
    [[nodiscard]] std::size_t getInternedLayoutCount() const { return wireRegistry_.size(); }

private:

    //-------------------------------------------------------------------------
    // Data

    std::int32_t commSize_;
    const MPIConsumerConfig &config_;

    std::unique_ptr<Common::Concurrency::GThreadPool> handlerThreadPool_;
    /**
         * thread that receives new incoming connections and schedules the handling of those to the thread pool
         */
    std::thread receiverThread_;
    /*
         * Thread that waits for the completion open sessions
         */
    std::thread cleanUpThread_;
    /*
         * Mutex to protect the vector of open sessions
         */
    std::mutex openSessionsMutex_;
    /*
         * Open sessions
         */
    std::vector<std::shared_ptr<GMPIConsumerSessionT<processable_type>>> openSessions_{};
    // whether a stop request for the GMPIConsumerT has been received
    std::atomic_bool isToldToStop_;
    // whether the stop request has been sent to all clients
    /// External source/sink injected by the courtier consumer via setPayloadFunctors().
    std::move_only_function<std::unique_ptr<processable_type>()> getPayloadItemFn_;
    std::move_only_function<void(std::unique_ptr<processable_type>)> putPayloadItemFn_;

    /// layout send-once registry shared by every session this master opens. MPI ranks are
    /// persistent, so each session keys its per-peer ack tracking on the requesting worker's rank
    /// (status.MPI_SOURCE) -- a naturally stable id for the whole run. A late-joining / restarted rank
    /// that misses a layout fetches it back via the REQUEST_LAYOUT / SEND_LAYOUT command pair.
    Gem::Courtier::GWireLayoutRegistry wireRegistry_;
};


} /* namespace Gem::Courtier::Consumers */
