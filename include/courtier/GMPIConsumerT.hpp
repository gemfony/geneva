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
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// utility functions for working with MPI
#include "GMPIHelperFunctions.hpp"

// Standard headers go here
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <array>
#include <optional>

// Boost headers go here
#include <boost/enable_shared_from_this.hpp>
#include <boost/utility.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/lexical_cast.hpp>

// MPI headers go here

#include <mpi.h>

// Geneva headers go here
#include "common/GThreadGroup.hpp"
#include "common/GThreadPool.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonEnums.hpp"
#include "courtier/GCourtierHelperFunctions.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GBaseConsumerT.hpp"
#include "courtier/GBaseClientT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GBaseConsumerT.hpp"
#include "courtier/GCommandContainerT.hpp"

// TODO: extract double buffering to GBaseConsumerClientT
// TODO: adapt documentation
// TODO: make MPI consumer a definitely returning consumer
// TODO: figure out why an exception with stream error is thrown for last stop request received.
// TODO: refactor config structs
// TODO: use MPI wait over MPI_Test if possible when no meaningful work can be done in between

namespace Gem::Courtier {
    // constants that are used by the master and the worker nodes
    constexpr int TAG_REQUEST_WORK_ITEM = 42;
    constexpr int TAG_SEND_WORK_ITEM = 43;
    constexpr int RANK_MASTER_NODE = 0;
    static MPI_Comm MPI_COMMUNICATOR = MPI_COMM_WORLD;

    /**
     * Stores configuration options necessary for a master node
     */
//    struct MasterNodeConfig {
//        /**
//         * The number of threads in a thread pool which is used to handle incoming requests.
//         */
//        boost::uint32_t nHandlerThreads{0};
////        /**
////         * The time in microseconds between each check for a new incoming connection
////         */
////        std::uint32_t receivePollIntervalUSec{0};
//        /**
//         * The time in milliseconds between each check of completion of the send operation of a new work item to a worker node.
//         */
//        std::uint32_t masterCheckSendComplMSec{1'000};
//        /**
//         * The maximum time in seconds to wait until a send operation of a new work item to a worker node is expected to succeed.
//         */
////        std::uint32_t sendPollTimeoutSec{5};
//
//        /**
//         * Adds local command line options to a boost::program_options::options_description object.
//         *
//         * @param visible Command line options that should always be visible
//         * @param hidden Command line options that should only be visible upon request for details
//         */
//        void addCLOptions(
//                boost::program_options::options_description &visible,
//                boost::program_options::options_description &hidden) {
//            // Note that we use the current values of the members as default values, because in a default constructed
//            // instance the defaults are already set. This allows to reduce duplication of those values
//            namespace po = boost::program_options;
//            // add general options for GMPIConsumerT
//            visible.add_options()("mpi_master_nIOThreads",
//                                  po::value<std::uint32_t>(&nHandlerThreads)->default_value(nHandlerThreads),
//                                  "\t[mpi-master-node] The number of threads in a thread pool which is used to handle incoming requests."
//                                  "The default of 0 triggers a dynamic configuration depending on the number of cpu cores on the current system");
//
////            hidden.add_options()("mpi_master_receivePollInterval",
////                                 po::value<std::uint32_t>(&receivePollIntervalUSec)->default_value(
////                                         receivePollIntervalUSec),
////                                 "\t[mpi-master-node] The time in microseconds between each check for a new incoming connection. "
////                                 "Setting this to 0 means repeatedly checking without waiting in between checks.");
//
//            hidden.add_options()("mpi_master_sendPollInterval",
//                                 po::value<std::uint32_t>(&masterCheckSendComplMSec)->default_value(
//                                         masterCheckSendComplMSec),
//                                 "\t[mpi-master-node] The time in milliseconds between each check of completion of the send operation of a new work item to a worker node");
//
////            hidden.add_options()("mpi_master_sendPollTimeout",
////                                 po::value<std::uint32_t>(&sendPollTimeoutSec)->default_value(
////                                         sendPollTimeoutSec),
////                                 "\t[mpi-master-node] The maximum time in seconds to wait until a send operation of a new work item to a worker node succeeds");
//        }
//    };

//    /**
//     * Combines all configuration options necessary for a worker node
//     */
//    struct WorkerNodeConfig {
//        /**
//         * The time in microseconds between each check for completion of the request for a new work item.
//         */
//        std::uint32_t workerCheckRecvComplUSec{100};
//        /**
//         * The maximum time in seconds to wait until a request for a new work item has been answered.
//         * After this timeout is triggered, the worker assumes the server is finished or has crashed and will shut down.
//         */
////        std::uint32_t ioPollTimeoutSec{5};
//
//        /**
//         * Adds local command line options to a boost::program_options::options_description object.
//         *
//         * @param visible Command line options that should always be visible
//         * @param hidden Command line options that should only be visible upon request for details
//         */
//        void addCLOptions(
//                boost::program_options::options_description &visible,
//                boost::program_options::options_description &hidden) {
//            // Note that we use the current values of the members as default values, because in a default constructed
//            // instance the defaults are already set. This allows to reduce duplication of those values
//            namespace po = boost::program_options;
//            hidden.add_options()("mpi_worker_pollInterval",
//                                 po::value<std::uint32_t>(&workerCheckRecvComplUSec)->default_value(
//                                         workerCheckRecvComplUSec),
//                                 "\t[mpi-worker-node] The time in microseconds between each check for completion of the request for a new work item.");
//
////            hidden.add_options()("mpi_worker_pollTimeout",
////                                 po::value<std::uint32_t>(&ioPollTimeoutSec)->default_value(
////                                         ioPollTimeoutSec),
////                                 "\t[mpi-worker-node] The maximum time in seconds to wait until a request for a new work item has been answered");
//        }
//    };

    /**
     * Stores configuration options which are used by master node and worker nodes
     */
    struct MPIConsumerConfig {
        MPIConsumerConfig(){
            // Set the handler threads to the hardware recommendation as a default value
            // This can later be overwritten by user-defined command line options
            this->nHandlerThreads = this->nHandlerThreadsRecommendation();
        }

        /**
         * Whether to issue a request for the next work item while the current work item is still being processed.
         */
        bool useAsynchronousRequests{true};
        /**
         * Serialization mode to use when serializing messages before transmitting over network between master node and worker nodes
         */
        Gem::Common::serializationMode serializationMode{Gem::Common::serializationMode::BINARY};
        /**
         * The time in microseconds between each check for the completion of synchronization on startup
         */
        std::uint32_t waitForMasterStartupPollIntervalUSec{100};
        /**
         * The maximum time in seconds to wait for the synchronization for all nodes on startup
         */
        std::uint32_t waitForMasterStartupTimeoutSec{60};
        /**
         * The number of threads in a thread pool which is used to handle incoming requests.
         */
        boost::uint32_t nHandlerThreads{0};
        /**
         * The time in milliseconds between each check of completion of the send operation of a new work item to a worker node.
         */
        std::uint32_t masterCheckSendComplMSec{1'000};
        /**
         * The time in microseconds between each check for completion of the request for a new work item.
         */
        std::uint32_t workerCheckRecvComplUSec{100};

        void addCLOptions_(
                boost::program_options::options_description &visible,
                boost::program_options::options_description &hidden) {
            // Note that we use the current values of the members as default values, because in a default constructed
            // instance the defaults are already set. This allows to reduce duplication of those values
            namespace po = boost::program_options;

            visible.add_options()("asyncRequests",
                                  po::value<bool>(&useAsynchronousRequests)->default_value(useAsynchronousRequests),
                                  "\t[mpi] Whether to issue a request for the next work item while the current work item is still being processed");

            visible.add_options()("mpi_handlerThreads",
                                  po::value<std::uint32_t>(&nHandlerThreads)->default_value(nHandlerThreads),
                                  "\t[mpi-master-node] The number of threads in a thread pool which is used to handle incoming requests."
                                  "The default of 0 triggers a dynamic configuration depending on the number of cpu cores on the current system");

            hidden.add_options()("mpi_master_sendPollInterval",
                                 po::value<std::uint32_t>(&masterCheckSendComplMSec)->default_value(
                                         masterCheckSendComplMSec),
                                 "\t[mpi-master-node] The time in milliseconds between each check of completion of the send operation of a new work item to a worker node");

            hidden.add_options()("mpi_serializationMode",
                                 po::value<Gem::Common::serializationMode>(&serializationMode)->default_value(
                                         serializationMode),
                                 "\t[mpi] Specifies whether serialization shall be done in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)");

            hidden.add_options()("mpi_waitForMasterStartupPollInterval",
                                 po::value<std::uint32_t>(&waitForMasterStartupPollIntervalUSec)->default_value(
                                         waitForMasterStartupPollIntervalUSec),
                                 "\t[mpi] The time in microseconds between each check for the completion of synchronization on startup.\n"
                                 "Setting this to 0 means repeatedly checking without waiting in between checks");

            hidden.add_options()("mpi_waitForMasterStartupTimeout",
                                 po::value<std::uint32_t>(&waitForMasterStartupTimeoutSec)->default_value(
                                         waitForMasterStartupTimeoutSec),
                                 "\t[mpi] The maximum time in seconds to wait for the synchronization for all nodes on startup.");

            hidden.add_options()("mpi_worker_pollInterval",
                                 po::value<std::uint32_t>(&workerCheckRecvComplUSec)->default_value(
                                         workerCheckRecvComplUSec),
                                 "\t[mpi-worker-node] The time in microseconds between each check for completion of the request for a new work item.");
        }

        [[nodiscard]] std::uint32_t nHandlerThreadsRecommendation() const {
            // query hint that indicates how many hardware threads are available (might return 0)
            const unsigned int hwThreads{std::thread::hardware_concurrency()};

            // if hint has returned 0, default to 8
            return hwThreads != 0 ? hwThreads : 8;
        }
    };

    // forward declare class because we have a cyclic dependency between MPIConsumerT and MPIConsumerWorkerNodeT
    template<typename processable_type>
    class GMPIConsumerT;

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
     * (5) Asynchronously send the processed work item to the master node, this message also implicitly requests a
     *      new work item\n
     * (6) Wait for the response i.e. go back to step (2) again\n
     *
     * All communication between GMPIConsumerWorkerNodeT and GMPIConsumerMasterNodeT works with asynchronous communication
     * using MPI. If an asynchronous request is not able to complete within a predefined timeframe, a timeout will be triggered.
     * Depending on where the timeout happened the worker might be able to continue working or needs to shutdown.
     * Timeouts and shutdowns will be logged.
     *
     */
    template<typename processable_type>
    class GMPIConsumerWorkerNodeT final
            : public std::enable_shared_from_this<GMPIConsumerWorkerNodeT<processable_type>> {

    public:
        /**
         *
         * Constructor with arguments for all configuration options for this class.
         *
         * @param commSize number of nodes in the cluster, which is equal to the number of workers + 1
         * @param commRank this node's rank within this cluster
         * @param halt function that returns true if halt criterion has been reached
         * @param incrementProcessingCounter function that increments the counter for already processed items on this node
         * @param commonConfig general configuration specified by the end-user
         * @param workerConfig configuration for this worker injected by the end-user
         */
        explicit GMPIConsumerWorkerNodeT(
                std::int32_t commSize,
                std::int32_t commRank,
                std::function<bool()> halt,
                std::function<void()> incrementProcessingCounter,
                const MPIConsumerConfig &config)
                : m_commSize{commSize},
                  m_commRank{commRank},
                  m_halt{std::move(halt)},
                  m_incrementProcessingCounter{std::move(incrementProcessingCounter)},
                  m_config{config} {
            glogger << "GMPIConsumerWorkerNodeT with rank " << m_commRank
                    << " started up" << std::endl
                    << GLOGGING;
            // create the buffer for incoming messages
            m_incomingMessageBuffer = std::unique_ptr<char[]>(new char[GMPICONSUMERMAXMESSAGESIZE]);
        }

        /**
         * The destructor
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

        GMPIConsumerWorkerNodeT<processable_type> &operator=(GMPIConsumerWorkerNodeT<processable_type> &&) = delete;

        /**
         * Advises the worker to start requesting and processing work items.
         * Once this call completes the worker was not able to receive any messages from the master node.
         * This means that either optimization has been completed at the master node, or that there is an error (e.g. the
         * network connection is lost)
         */
        void run() {
            // set message for initial GETDATA request
            m_outgoingMessage = Gem::Courtier::container_to_string(
                    m_commandContainer,
                    m_config.serializationMode);

            // send initial GETDATA request to receive first work item
            if (!sendResultAndRequestNewWork()) {
                return; // return if unrecoverable error in networking occurred
            }

            // stop if server tells this worker to stop or if the optimization stop criteria is fulfilled
            while (!m_stopRequestReceived && !m_halt()) {
                // swap messages
                // serialize (processed) container and store in m_outgoingMessage
                // afterwards deserialize m_incomingMessage into the container
                m_outgoingMessage = std::move(Gem::Courtier::container_to_string(
                        m_commandContainer,
                        m_config.serializationMode));
                Gem::Courtier::container_from_string(
                        m_incomingMessage,
                        m_commandContainer,
                        m_config.serializationMode);

                if (m_config.useAsynchronousRequests) {
                    std::future<bool> networkingSuccessful = std::async(
                            std::launch::async,
                            [this]() -> bool { return this->sendResultAndRequestNewWork(); });

                    // process the currently available work item (might be a stop request)
                    processWorkItem();

                    if (!networkingSuccessful.get()) {
                        return; // return if unrecoverable error in networking occurred
                    }
                } else {
                    // process the currently available work item (might be a stop request)
                    processWorkItem();

                    if (!sendResultAndRequestNewWork()) {
                        return; // return if unrecoverable error in networking occurred
                    }
                }
            }

            // TODO: remove print
            std::cout << "Worker with rank " << m_commRank << " leaving Loop after stop request" << std::endl;

            // await last server response for the due to double buffering unnecessarily send our request
            if (this->m_stopRequestReceived) {
                // TODO: remove print
                std::cout << "Worker with rank " << m_commRank << " preparing to wait for last stop request"
                          << std::endl;
                waitForLastResponse();

                glogger << "GMPIConsumerWorkerNodeT with rank " << this->m_commRank
                        << " has received a stop request and will shut down."
                        << std::endl << GLOGGING;
            }
        }

    private:
        /**
         * Sends the current contents of m_outgoingMessage to the master node and requests a new work item which will
         * be assigned to m_IncomingMessage. Therefore both members m_outgoingMessage and m_incomingMessage are mutated
         * inside of this function and shall not be mutated from other threads at the same time.
         *
         * @return true if successful, otherwise false.\n
         * If unsuccessful, the members m_incomingMessage and m_outgoingMessage are undefined.
         * Once this call returns the asynchronous mpi communication requests associated with the handles, which are
         * stored as member variables of this class, are guaranteed to be either completed or in case of an error canceled.
         *
         */
        [[nodiscard]] bool sendResultAndRequestNewWork() {
            // start asynchronous send call to send result of last computation (or GETDATA command if no result available)
            MPI_Isend(
                    m_outgoingMessage.data(),
                    m_outgoingMessage.size(),
                    MPI_CHAR,
                    RANK_MASTER_NODE,
                    TAG_REQUEST_WORK_ITEM,
                    MPI_COMMUNICATOR,
                    &m_sendHandle);

            // start asynchronous receive call to receive the servers result. By starting this call before we even have
            // confirmed that the send operation has completed, we can save some time. I.e. after the server has fully received
            // the request it can immediately respond without having to wait for the client side starting to receive
            MPI_Irecv(
                    m_incomingMessageBuffer.get(),
                    GMPICONSUMERMAXMESSAGESIZE,
                    MPI_CHAR,
                    RANK_MASTER_NODE,
                    MPI_ANY_TAG,
                    MPI_COMMUNICATOR,
                    &m_receiveHandle);

            // time in microseconds that have been spent waiting during communication already
            int receiveIsCompleted{0};
            MPI_Status receiveStatus{};

            // TODO: to decrease CPU utilization, use MPI_Wait instead of MPI_Test.
            //  Since the computation is running on another thread, this should be possible

            // continue testing until receive was completed or timeout has been triggered.
            // This can only happen after the server has received our message, so send must also be complete
            while (true) {
                MPI_Test(&m_receiveHandle,
                         &receiveIsCompleted,
                         &receiveStatus);

                // using break instead of while-condition allows skipping the sleep as soon as receiving has been completed
                if (receiveIsCompleted) {
                    // Since we have received the response to our message, the message must have been sent out and
                    // we can therefore allow the send request and send buffer to be freed.
                    MPI_Request_free(&m_sendHandle);
                    break;
                }

                if (m_config.workerCheckRecvComplUSec > 0) {
                    std::this_thread::sleep_for(std::chrono::microseconds{m_config.workerCheckRecvComplUSec});
                }
            }

            if (receiveStatus.MPI_ERROR != MPI_SUCCESS) {
                glogger
                        << "In GMPIConsumerWorkerNodeT<processable_type>::sendResultAndRequestNewWork() with rank="
                        << m_commRank << ":" << std::endl
                        << "Received an error receiving a message from GMPIConsumerMasterNodeT:" << std::endl
                        << mpiErrorString(receiveStatus.MPI_ERROR) << std::endl
                        << "Worker node will shut down." << std::endl
                        << GWARNING;

                return false;
            }

            // create string with correct size from the fixed size buffer
            m_incomingMessage = std::string(m_incomingMessageBuffer.get(), mpiGetCount(receiveStatus));

            return true;
        }

        // TODO: remove unused code
//        /**
//         * Cancels the active requests that are associated with the handles, which are stored as member variables.
//         */
//        void cancelActiveRequests() {
//            MPI_Cancel(&m_receiveHandle);
//            MPI_Request_free(&m_receiveHandle);
//
//            MPI_Cancel(&m_sendHandle);
//            MPI_Request_free(&m_sendHandle);
//        }

        /**
         * Processes the work item that is stored in the member m_commandContainer.
         * After processing has been finished the result is put into m_commandContainer i.e. it overrides the old item.
         * In case that m_commandContainer did not contain any work items this method will store a new GETDATA request
         * in m_commandContainer to retrieve new work when sending this message.
         */
        void processWorkItem() {
            switch (m_commandContainer.get_command()) {
                case networked_consumer_payload_command::COMPUTE: {
                    // process item. This will put the result into the container
                    m_commandContainer.process();

                    // increment the counter for processed items
                    m_incrementProcessingCounter();

                    // mark the container as "contains a result"
                    m_commandContainer.set_command(networked_consumer_payload_command::RESULT);
                }
                    break;
                case networked_consumer_payload_command::NODATA: {

                    // Update the NODATA counter for bookkeeping
                    ++m_nNoData;

                    // sleep for a short random time interval
                    std::uniform_int_distribution<> dist(GMPICONSUMERWORKERNODERETRYINTERVALLOWERBOUNDARYMSEC,
                                                         GMPICONSUMERWORKERNODERETRYINTERVALUPPERBOUNDARYMSEC);
                    std::this_thread::sleep_for(std::chrono::milliseconds(dist(m_randomNumberEngine)));

                    // Tell the server again we need work
                    m_commandContainer.reset(networked_consumer_payload_command::GETDATA);
                }
                    break;
                case networked_consumer_payload_command::STOP: {
                    this->m_stopRequestReceived = true;
                    // TODO: remove print
                    std::cout << "Worker " << this->m_commRank << ": Stop request received" << std::endl;
                }
                    break;
                default: {
                    // Emit a warning, ignore item and request new item
                    glogger << "GMPIConsumerWorkerNodeT<processable_type>::processWorkItem() with rank=" << m_commRank
                            << ":" << std::endl
                            << "Got unknown or invalid command "
                            << boost::lexical_cast<std::string>(m_commandContainer.get_command()) << std::endl
                            << GWARNING;

                    m_commandContainer.reset(networked_consumer_payload_command::GETDATA);
                }
            }
        }

        /**
         *  If we have been stopped, we leave the loop here but have sent one more request out
         * because of double buffered requests. The server must await this additional request, because all
         * MPI communication should be completed before shutdown.
         * For this reason, we have to receive the server's response for this last request and keep it unanswered
         */
        void waitForLastResponse() {
            MPI_Request request{};

            // issue async call to receive the server's last response
            MPI_Irecv(
                    m_incomingMessageBuffer.get(),
                    GMPICONSUMERMAXMESSAGESIZE,
                    MPI_CHAR,
                    RANK_MASTER_NODE,
                    MPI_ANY_TAG,
                    MPI_COMMUNICATOR,
                    &request);

            // wait for completion of last server response
            MPI_Status receiveStatus{};
//            MPI_Wait(&request, &receiveStatus);

            // TODO: use MPI_Wait if possible
            int isCompleted{0};
            while (!isCompleted) {
                MPI_Test(&m_receiveHandle, &isCompleted, &receiveStatus);
            }

            if (receiveStatus.MPI_ERROR != MPI_SUCCESS) {
                glogger
                        << "In GMPIConsumerWorkerNodeT<processable_type>::waitForLastResponse() with rank="
                        << m_commRank << ":" << std::endl
                        << "Received an error receiving the last message from GMPIConsumerMasterNodeT:" << std::endl
                        << mpiErrorString(receiveStatus.MPI_ERROR) << std::endl
                        << "Worker node will shut down." << std::endl
                        << GTERMINATION;
                return;
            }

            // TODO: remove print
            std::cout << "Worker " << this->m_commRank << " received last stop received" << std::endl;

            // get command container from received bytes for sanity check
            m_incomingMessage = std::string(m_incomingMessageBuffer.get(), mpiGetCount(receiveStatus));
            Gem::Courtier::container_from_string(
                    m_incomingMessage,
                    m_commandContainer,
                    m_config.serializationMode);

            // sanity check: Server should only return stop request once the first stop request has been returned
            if (m_commandContainer.get_command() != networked_consumer_payload_command::STOP) {
                glogger
                        << "In GMPIConsumerWorkerNodeT<processable_type>::waitForLastResponse() with rank="
                        << m_commRank << ":" << std::endl
                        << "Expected to receive the last stop request but instead received message with command "
                        << m_commandContainer.get_command()
                        << GTERMINATION;
            }
        }

        //-------------------------------------------------------------------------
        // Private data

        /**
         * number of nodes in the cluster (number of worker nodes + 1)
         */
        std::int32_t m_commSize;
        /**
         * rank of this node in the cluster
         */
        std::int32_t m_commRank;
        /**
         * Callback function that returns true if the halt criterion has been reached
         */
        std::function<bool()> m_halt;
        /**
         * Increments the counter for processed work items of the calling instance of GConsumerBaseT.
         */
        std::function<void()> m_incrementProcessingCounter;
        /**
         * Configuration specified by the end-user.
         */
        const MPIConsumerConfig &m_config;
        /**
         * Whether a stop request from the master node has been received
         */
        bool m_stopRequestReceived{false};

        // only one request is processed at a time. Even in the version with asynchronous request the thread which is
        // responsible for handling the IO will always be joined before the next thread for the next IO-operation is spawned.
        // So the program logic ensures that the access to these resources is exclusive to one thread, so we do not
        // need to enforce this with mutex or something similar and can store the handles as members.
        MPI_Request m_sendHandle{};
        MPI_Request m_receiveHandle{};

        /**
         * counter for how many times we have not received data when requesting data from the master node
         */
        std::int32_t m_nNoData{0};

        std::random_device m_randomDevice; ///< Source of non-deterministic random numbers
        std::mt19937 m_randomNumberEngine{
                m_randomDevice()}; ///< The actual random number engine, seeded by m_randomDevice

        // we only work with one IO-thread here. So we can store the buffer as a member variable without concurrency issues
        std::unique_ptr<char[]> m_incomingMessageBuffer;
        std::string m_incomingMessage;
        std::string m_outgoingMessage;
        GCommandContainerT<processable_type, networked_consumer_payload_command> m_commandContainer{
                networked_consumer_payload_command::GETDATA}; ///< Holds the current command and payload (if any)
    };

    /**
     * This class represents one session between the master node and one worker node.
     *
     * A GMPIConsumerSessionT can be opened as soon as the master node has fully received a request from
     * a worker node. The opened GMPIConsumerSessionT will then take care of deserializing and processing
     * the request as well as responding to it with a new work item (if there are items available in the brokers queue
     * at that point in time).
     */
    template<typename processable_type>
    class GMPIConsumerSessionT
            : public std::enable_shared_from_this<GMPIConsumerSessionT<processable_type>> {
    public:
        /**
         * Constructor for GMPIConsumerSessionT.
         *
         * @param status MPI_Status object that stores information about the received request, most importantly its source
         * i.e. the rank of the worker node sending the request
         * @param requestMessage the payload of the request this session was opened for
         * @param getPayloadItem a callback function to retrieve payload items (raw work items) from their origin / producer
         * @param putPayloadItem a callback function to put payload items (processed work items) to their destination
         * @param serializationMode the mode of serialization between the master nodes and the worker nodes
         */
        GMPIConsumerSessionT(
                MPI_Status status,
                std::string requestMessage,
                std::function<std::shared_ptr<processable_type>()> getPayloadItem,
                std::function<void(std::shared_ptr<processable_type>)> putPayloadItem,
                Gem::Common::serializationMode serializationMode,
                bool stopRequested)
                : m_mpiStatus{status},
                // avoid copying the string but also not taking it as reference because it should be owned by this object
                  m_requestMessage{std::move(requestMessage)},
                  m_getPayloadItem(std::move(getPayloadItem)),
                  m_putPayloadItem(std::move(putPayloadItem)),
                  m_serializationMode{serializationMode},
                  m_stopRequested{stopRequested},
                  m_mpiRequestHandle{} {
        }

        //-------------------------------------------------------------------------
        // Deleted constructors and assignment operators

        GMPIConsumerSessionT() = delete;

        GMPIConsumerSessionT(const GMPIConsumerSessionT<processable_type> &) = delete;

        GMPIConsumerSessionT(GMPIConsumerSessionT<processable_type> &&) = delete;

        GMPIConsumerSessionT<processable_type> &operator=(const GMPIConsumerSessionT<processable_type> &) = delete;

        GMPIConsumerSessionT<processable_type> &operator=(GMPIConsumerSessionT<processable_type> &&) = delete;

        //-------------------------------------------------------------------------
        // public functions that are not constructors or operators

        /**
         * Answers the request this session was created for
         */
        void run() {
            // only execute sendResponse if processRequest was successful
            if (processRequest()) {
                sendResponse();
            }
        }

        /**
         * Allows to check whether the sending the response of the request to the worker has been completed
         *
         * This assumes that the run-method has already been called and finished execution.
         *
         * @return true if sending the response has been completed, otherwise false
         */
        [[nodiscard]] bool isCompleted() {
            int isCompleted{0};

            MPI_Test(
                    &m_mpiRequestHandle,
                    &isCompleted,
                    MPI_STATUS_IGNORE);

            return isCompleted;
        }

        /**
         *
         * Allows retrieving the rank of the worker that this session was opened for
         *
         * @return the rank of the worker this session was opened for
         */
        [[nodiscard]] std::uint32_t getConnectedWorker() {
            return m_mpiStatus.MPI_SOURCE;
        }

        /**
         * @return command that the session is sending out to the client in the response
         */
        [[nodiscard]] networked_consumer_payload_command getOutCommand() const {
            return this->m_commandContainer.get_command();
        }

    private:
        bool processRequest() {
            try {
                // deserialize request string
                Gem::Courtier::container_from_string(
                        m_requestMessage, m_commandContainer, m_serializationMode); // may throw

                // Extract the command
                auto inboundCommand = m_commandContainer.get_command();

                // If we have some payload received, add it to its destination
                switch (inboundCommand) {
                    case networked_consumer_payload_command::RESULT: {
                        putWorkItem();
                        return true;
                    }
                    case networked_consumer_payload_command::GETDATA: {
                        return true; // no data to process
                    }
                    default: { // clients may only send RESULT or GETDATA commands
                        glogger
                                << "GMPIConsumerSessionT<processable_type>::process_request() connected to rank="
                                << m_mpiStatus.MPI_SOURCE << ":" << std::endl
                                << "Got unknown or invalid command " << boost::lexical_cast<std::string>(inboundCommand)
                                << std::endl
                                << GWARNING;
                    }
                }
            }
            catch (const gemfony_exception &ex) {
                auto ePtr = std::current_exception();
                glogger
                        << "GMPIConsumerSessionT<processable_type>::processRequest() connected to rank="
                        << m_mpiStatus.MPI_SOURCE << ":" << std::endl
                        << ": Caught exception while deserializing request" << std::endl
                        << ex.what() << std::endl
                        << GEXCEPTION;
            }

            return false;
        }

        void putWorkItem() {
            // Retrieve the payload from the command container
            auto payloadPtr = m_commandContainer.get_payload();

            // Submit the payload to the server (which will send it to the broker)
            if (payloadPtr) {
                m_putPayloadItem(payloadPtr);
                return;
            }

            glogger
                    << "GMPIConsumerSessionT<processable_type>::process_request() connected to rank="
                    << m_mpiStatus.MPI_SOURCE << ":" << std::endl
                    << "payload is empty even though a result was expected." << std::endl
                    << "However, this request will also be responded normally." << std::endl
                    << GWARNING;
        }

        void prepareDataResponse() {
            // Obtain a container_payload object from the queue, serialize it and send it off
            // this function includes a timeout that might result in a nullptr being returned
            auto payloadPtr = this->m_getPayloadItem();

            if (payloadPtr) {
                m_commandContainer.reset(networked_consumer_payload_command::COMPUTE, payloadPtr);
            } else {
                m_commandContainer.reset(networked_consumer_payload_command::NODATA);
            }
        }

        void prepareStopResponse() {
            // store a stop request in the command container
            m_commandContainer.reset(networked_consumer_payload_command::STOP);
        }

        void serializeOutgoingMsg() {
            // set the outgoing message to the string representation of the
            m_outgoingMessage = Gem::Courtier::container_to_string(
                    m_commandContainer, m_serializationMode);

            if (m_outgoingMessage.size() > GMPICONSUMERMAXMESSAGESIZE) {
                throw gemfony_exception(
                        g_error_streamer(DO_LOG, time_and_place)
                                << "GMPIConsumerSessionT<processable_type>::serializeOutgoingMsg():" << std::endl
                                << "Size of individual to send after serialization greater than maximum configured message size."
                                << std::endl
                                << "Size of Individual is " << m_outgoingMessage.size() << std::endl
                                << "Maximum message size is " << GMPICONSUMERMAXMESSAGESIZE << std::endl
                                << "Serialization mode is " << m_serializationMode << std::endl
                                << "To overcome this issue, change the serialization mode or adjust the maximum message size.");
            }
        }

        /**
         * Starts an asynchronous send call of the response message.
         * isCompleted() can be used to check for the completion of the send operation.
         */
        void sendResponse() {
            // prepare the correct type of message in the outgoing command
            if (m_stopRequested) {
                // TODO: remove print
                std::cout << "Sending stop request" << std::endl;
                prepareStopResponse();
            } else {
                prepareDataResponse();
            }

            // serialize the outgoing message
            serializeOutgoingMsg();

            // asynchronously start sending the response
            MPI_Isend(
                    m_outgoingMessage.data(),
                    m_outgoingMessage.size(),
                    MPI_CHAR,
                    m_mpiStatus.MPI_SOURCE,
                    TAG_SEND_WORK_ITEM,
                    MPI_COMMUNICATOR,
                    &m_mpiRequestHandle);

            // the isCompleted method can be used to check if the send-operation has been completed
        }


        //-------------------------------------------------------------------------
        // Data
        const MPI_Status m_mpiStatus;
        const std::string m_requestMessage;
        const Gem::Common::serializationMode m_serializationMode;
        const bool m_stopRequested;

        std::function<std::shared_ptr<processable_type>()> m_getPayloadItem;
        std::function<void(std::shared_ptr<processable_type>)> m_putPayloadItem;

        GCommandContainerT<processable_type, networked_consumer_payload_command> m_commandContainer{
                networked_consumer_payload_command::NONE}; ///< Holds the current command and payload (if any)
        MPI_Request m_mpiRequestHandle;
        std::string m_outgoingMessage;

        std::optional<std::chrono::steady_clock::time_point> m_finishedSince{}; // empty start time before run called
    };

    /**
     *
     * This class is responsible for the server side of network communication using MPI.
     *
     * GMPIConsumerMasterNodeT will constantly wait for incoming work items requests, process them and answer them
     * by opening a new GMPIConsumerSessionT for each request.
     *
     * @tparam processable_type a type that is processable like GParameterSet
     *
     *
     * The simplified workflow of the GMPIConsumerMasterNodeT can be described as follows:
     *
     * (1) Create thread pool\n
     * (2) Spawn a new thread that receives request (receiverThread). Then return from the function immediately, because
     * geneva expects consumers to be background processes on other threads than the main thread.\n
     * (3) Once a shutdown is supposed to occur:\n
     *  (3.1) Set a member flag to tell the receiver thread to stop\n
     *  (3.2) Join all threads. The receiver thread should exit itself and all handler threads will finish their last task
     *        and then finish.\n\n
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
    template<typename processable_type>
    class GMPIConsumerMasterNodeT
            : public std::enable_shared_from_this<GMPIConsumerMasterNodeT<processable_type>> {

    public:
        /**
         * Constructor to instantiate the GMPIConsumerMasterNodeT
         * @param commSize number of nodes in the cluster, which is equal to the number of workers + 1
         * @param config configuration for this node specified by the end user
         */
        explicit GMPIConsumerMasterNodeT(
                std::int32_t commSize,
                const MPIConsumerConfig &config)
                : m_commSize{commSize},
                  m_isToldToStop{false},
                  m_config{config} {
//            configureNHandlerThreads();
            glogger << "GMPIConsumerMasterNodeT started with n=" << m_config.nHandlerThreads << " IO-threads"
                    << std::endl
                    << GLOGGING;
        }

        //-------------------------------------------------------------------------
        // Deleted copy-/move-constructors and assignment operators.
        GMPIConsumerMasterNodeT(const GMPIConsumerMasterNodeT<processable_type> &) = delete;

        GMPIConsumerMasterNodeT(GMPIConsumerMasterNodeT<processable_type> &&) = delete;

        GMPIConsumerMasterNodeT &operator=(const GMPIConsumerMasterNodeT<processable_type> &) = delete;

        GMPIConsumerMasterNodeT &operator=(GMPIConsumerMasterNodeT<processable_type> &&) = delete;

        /**
         * Starts background threads that run the GMPIConsumerMasterNodeT.
         *
         * First a thread-pool will be created.Then another thread will be created which listens for requests and
         * schedules request handlers on the thread-pool. Furthermore a cleanup thread will be created, which closes open
         * sessions in a cyclic manner.
         * Once the threads have been created this method will immediately
         * return to the caller while the spawned threads run in the background and fulfil their job.
         * To stop the master node and all its threads again the shutdown method can be called.
         */
        void async_startProcessing() {
            m_handlerThreadPool = std::make_unique<Common::GThreadPool>(m_config.nHandlerThreads);

            auto self = this->shared_from_this();
            m_receiverThread = std::thread(
                    [self] { self->listenForRequests(); });

            m_cleanUpThread = std::thread(
                    [self] { self->cleanUpSessionsLoop(); });
        }

        /**
         * Sends a shutdown signal to the GMPIConsumerMasterNodeT and then joins all its background threads.
         *
         * Once this method has returned this means that all threads have been joined, all asynchronous mpi communication
         * requests have been either completed or have been canceled and the server is shut down completely.
         */
        void shutdown() {
            // notify other threads to stop
            m_isToldToStop.store(true);

            // wait for the receiver thread to send a stop request to each client
            m_receiverThread.join();

            // wait until for threads to finish their work i.e. send the stop requests out to the clients
            m_handlerThreadPool->wait();

            // wait for the cleanup thread. This thread will close all open sessions before joining
            m_cleanUpThread.join();

            // TODO: remove print
            std::cout << "Server finished shutdown" << std::endl;
        }

//        void configureNHandlerThreads() {
//            // if thread pool size is set to 0 by user, set it to a recommendation
//            if (m_config.nHandlerThreads == 0) {
//                m_config.nHandlerThreads = nHandlerThreadsRecommendation();
//            }
//        }

    private:
        void listenForRequests() {
            // number of workers that we have send a stop request to
            uint32_t stopRequestsSendOut{0};
            // Each client sends out one last request although having receive a stop that has to be answered
            // by the server since the clients use double buffering
            const int32_t reqNumStops{2 * (this->m_commSize - 1)};

            while (stopRequestsSendOut < reqNumStops) {
                MPI_Request requestHandle{};
                // create a buffer for each request.
                // once the session finished handling the request, it will release the handle and the memory will be freed
                auto buffer = std::shared_ptr<char[]>(new char[GMPICONSUMERMAXMESSAGESIZE]);

                // register asynchronous receiving of message from any worker node
                MPI_Irecv(
                        buffer.get(),
                        GMPICONSUMERMAXMESSAGESIZE,
                        MPI_CHAR,
                        MPI_ANY_SOURCE,
                        MPI_ANY_TAG,
                        MPI_COMMUNICATOR,
                        &requestHandle);

                while (true) {
                    int isCompleted{0};
                    MPI_Status status{};

                    MPI_Test(
                            &requestHandle,
                            &isCompleted,
                            &status);

                    if (isCompleted) {
                        // save atomic variable value
                        const bool stopRequested = m_isToldToStop.load();

                        if (stopRequested) {
                            ++stopRequestsSendOut;
                        }
                        // let a new thread handle this request and listen for further requests
                        // we capture copies of smart pointers in the closure, which keeps the underlying data alive
                        const auto self = this->shared_from_this();
                        m_handlerThreadPool->async_schedule(
                                [&self, status, buffer, stopRequested] {
                                    self->handleRequest(status, buffer, stopRequested);
                                });
                        break;
                    }

                    // if receive not completed yet, sleep a short amount of time until polling again for the message status
//                    if (m_masterConfig.receivePollIntervalUSec > 0) {
//                        std::this_thread::sleep_for(std::chrono::microseconds{m_masterConfig.receivePollIntervalUSec});
//                    }
                }
            }
            // TODO: remove waiting if shutdown bug found
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }

        void handleRequest(const MPI_Status &status, const std::shared_ptr<char[]> &buffer, const bool stopRequested) {
            if (status.MPI_ERROR != MPI_SUCCESS) {
                glogger
                        << "In GMPIConsumerMasterNodeT<processable_type>::handleRequest():" << std::endl
                        << "Received an error:" << std::endl
                        << mpiErrorString(status.MPI_ERROR) << std::endl
                        << "Request from worker node will not be answered." << std::endl
                        << GWARNING;

                // return from this handler, which means not answering the request
                return;
            }

            // start a new session
            auto session = std::make_shared<GMPIConsumerSessionT<processable_type>>(
                    status,
                    std::string{buffer.get(), static_cast<size_t>(mpiGetCount(status))},
                    [this]() -> std::shared_ptr<processable_type> { return getPayloadItem(); },
                    [this](std::shared_ptr<processable_type> p) { putPayloadItem(p); },
                    m_config.serializationMode,
                    stopRequested);

            // runs the session but does not close it
            session->run();

            // push the open session to a vector of open sessions
            // the m_cleanUpThread will make sure of waiting until these sessions complete or result in an error
            pushOpenSession(session);

            // This thread of the thread-pool will then be able to be scheduled for further requests by the IO-thread again
        }

        void pushOpenSession(std::shared_ptr<GMPIConsumerSessionT<processable_type>> session) {
            std::lock_guard<std::mutex> guard(m_openSessionsMutex);
            m_openSessions.push_back(session);
        }

        /**
         * Waits for open sessions to be completed.
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
            const int32_t reqNumStops{2 * (this->m_commSize - 1)};

            // keep running until all stop requests have been send out,
            // since after that no more sessions should be opened because all clients will shut down
            while (stopSendOutsCompleted < reqNumStops) {
                // wait a short amount of time between checking if the sessions have been completed
                if (m_config.masterCheckSendComplMSec > 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds{m_config.masterCheckSendComplMSec});
                }

                const auto timeCurr = std::chrono::steady_clock::now();

                // lock access to open sessions vector
                std::lock_guard<std::mutex> guard(m_openSessionsMutex);
                // TODO: remove print
                std::cout << "Number of open sessions before clean up: " << m_openSessions.size() << std::endl;

                for (auto sessionIter{m_openSessions.begin()};
                     sessionIter != m_openSessions.end(); /* no increment */) {
                    if ((*sessionIter)->isCompleted()) {
                        // track the completed stop requests
                        if ((*sessionIter)->getOutCommand() == networked_consumer_payload_command::STOP) {
                            ++stopSendOutsCompleted;
                            // TODO: remove print
                            std::cout << "stop request sending completed" << std::endl;
                        }

                        // erase this session because it has completed
                        sessionIter = m_openSessions.erase(sessionIter);
                    } else {
                        // increment iterator in case no session has been erased
                        ++sessionIter;
                    }
                }

                // TODO: remove print
                std::cout << "Number of open sessions after clean up: " << m_openSessions.size() << std::endl;
            }
        }

        // TODO: remove unused code
//        /**
//         * Cancels an active mpi request.
//         * @param request the request to be canceled.
//         */
//        void cancelRequest(MPI_Request &request) {
//            MPI_Cancel(&request);
//            MPI_Request_free(&request);
//        }

        /**
         * Tries to retrieve a work item from the server, observing a timeout. If timeout is reached a nullptr is returned
         *
         * @return A work item (possibly empty)
         */
        std::shared_ptr<processable_type> getPayloadItem() {
            std::shared_ptr<processable_type> p;

            // Try to retrieve a work item from the broker
            m_brokerPtr->get(p, m_timeout);

            // May be empty, if we ran into a timeout
            return p;
        }

        //-------------------------------------------------------------------------
        /**
         * Submits a work item to the server, observing a timeout
         */
        void putPayloadItem(std::shared_ptr<processable_type> p) {
            if (not p) {
                throw gemfony_exception(
                        g_error_streamer(DO_LOG, time_and_place)
                                << "GMPIConsumerMasterNodeT<>::putPayloadItem():" << std::endl
                                << "Function called with empty work item" << std::endl);
            }

            if (not m_brokerPtr->put(p, m_timeout)) {
                glogger
                        << "In GMPIConsumerMasterNodeT<>::putPayloadItem():" << std::endl
                        << "Work item could not be submitted to the broker" << std::endl
                        << "The item will be discarded" << std::endl
                        << GWARNING;
            }
        }

        //-------------------------------------------------------------------------
        // Data

        std::int32_t m_commSize;
        const MPIConsumerConfig &m_config;

        std::unique_ptr<Common::GThreadPool> m_handlerThreadPool;
        /**
         * thread that receives new incoming connections and schedules the handling of those to the thread pool
         */
        std::thread m_receiverThread;
        /*
         * Thread that waits for the completion open sessions
         */
        std::thread m_cleanUpThread;
        /*
         * Mutex to protect the vector of open sessions
         */
        std::mutex m_openSessionsMutex{};
        /*
         * Open sessions
         */
        std::vector<std::shared_ptr<GMPIConsumerSessionT<processable_type>>> m_openSessions{};
        // whether a stop request for the GMPIConsumerT has been received
        std::atomic_bool m_isToldToStop;
        // whether the stop request has been sent to all clients
        std::shared_ptr<typename Gem::Courtier::GBrokerT<processable_type>> m_brokerPtr = GBROKER(
                processable_type); ///< Simplified access to the broker
        const std::chrono::duration<double> m_timeout = std::chrono::milliseconds(
                GMPICONSUMERBROKERACCESSBROKERTIMEOUT); ///< A timeout for put- and get-operations via the broker
    };

    /**
     *
     * This class manages the initialization of MPI and is a wrapper around GMPIConsumerMasterNodeT and GMPIConsumerWorkerNodeT.
     *
     * This class is responsible for checking whether the current process is the master node (rank 0) or a worker node (any other rank).
     * It is derived from both base classes - GBaseClientT and GBaseConsumerT - and can therefore be used as either of these.
     * It instantiates the correct classes (GMPIConsumerMasterNodeT or GMPIConsumerWorkerNodeT) and forwards calls to its methods
     * to one methods of the underlying object. This serves as an abstraction of MPI such that the user does not need to explicitly
     * ask for the process's rank.
     *
     * The GMPIConsumerMasterNodeT is a server using MPI to wait for and serve connections initiated by GMPIConsumerWorkerNodeTs.
     * The server waits for a request from any worker node and answers it.
     *
     * The GMPIConsumerT only uses asynchronous MPI communication and point-to-point messaging. This might be unusual
     * when working with MPI, as most MPI-applications use synchronous communication e.g. with scatter and gather.
     * While synchronous communication with multiple clients at a time could maybe be slightly more performant, the asynchronous
     * client server model is much more fault tolerant and was therefore chosen.
     * Asynchronous MPI calls with the client-server model allow realizing time-outs in case of crashing or unavailable workers.
     * This means no matter how many clients crash, the remaining clients are able to fulfil their job and the optimization can
     * be completed as soon as the server does not crash.
     * Geneva claims to be fault tolerant, because the probability that one node crashes is very high in a large scale
     * distributed system that runs for a long time. So the decision has been made to use a client-server model for GMPIConsumerT.
     *
     * @tparam processable_type a type that is processable like GParameterSet
     */
    template<typename processable_type>
    class GMPIConsumerT
            : public Gem::Courtier::GBaseConsumerT<processable_type>,
              public Gem::Courtier::GBaseClientT<processable_type>,
              public std::enable_shared_from_this<GMPIConsumerT<processable_type>> {
    public:
        /**
         *
         * Constructor for a GMPIConsumerT
         *
         * @param argc argument count passed to main function, which will be forwarded to the MPI_Init call
         * @param argv argument vector passed to main function, which will be forwarded to MPI_Init call
         * @param commonConfig configuration options for users, default values are defined through the default values of the struct
         */
        explicit GMPIConsumerT(
                int *argc = nullptr,
                char ***argv = nullptr,
                MPIConsumerConfig config = MPIConsumerConfig{})
                : m_argc{argc},
                  m_argv{argv},
                  m_config{config},
                  m_commRank{},
                  m_commSize{} {}

        /**
         * The destructor finalizes the MPI framework.
         *
         * This destructor must call the MPI_Finalize function as this function encapsulates all MPI-specific action.
         */
        ~GMPIConsumerT() override {
            this->finalizeMPI();
        }

        //-------------------------------------------------------------------------
        // Deleted functions

        // Deleted copy-constructors and assignment operators -- the client is non-copyable
        GMPIConsumerT(const GMPIConsumerT<processable_type> &) = delete;

        GMPIConsumerT(GMPIConsumerT<processable_type> &&) = delete;

        GMPIConsumerT<processable_type> &operator=(const GMPIConsumerT<processable_type> &) = delete;

        GMPIConsumerT<processable_type> &operator=(GMPIConsumerT<processable_type> &&) = delete;

        //-------------------------------------------------------------------------

        /**
         *
         * Initializes MPI in the required mode. This method is static, so that it can be called without a reference to
         * an instance of GMPIConsumerT in case the user wants to initialize MPI on his own. This is useful in case of
         * using MPI in the user code as well e.g. for creating MPI-subClients (look at examples/geneva/17_GMPISubClients/)
         *
         * @param argc argument count passed to main function, which will be forwarded to the MPI_Init call
         * @param argv argument vector passed to main function, which will be forwarded to MPI_Init call
         * @return true if MPI has been initialized by this call, false if it has already been initialized and therefore not initialized again
         */
        static bool initializeMPI(int *argc = nullptr, char ***argv = nullptr) {
            int isAlreadyInitialized{0};
            MPI_Initialized(&isAlreadyInitialized);

            if (!isAlreadyInitialized) {
                int providedThreadingLevel = 0;
                MPI_Init_thread(argc, argv, MPI_THREAD_MULTIPLE, &providedThreadingLevel);

                if (providedThreadingLevel != MPI_THREAD_MULTIPLE) {
                    throw gemfony_exception(
                            g_error_streamer(DO_LOG, time_and_place)
                                    << "GMPIConsumerT<> constructor" << std::endl
                                    << "Geneva requires MPI implementation with level MPI_THREAD_MULTIPLE (a.k.a. "
                                    << MPI_THREAD_MULTIPLE
                                    << ") but the runtime environment implementation of MPI only supports level "
                                    << providedThreadingLevel << std::endl);
                }
            }

            return !isAlreadyInitialized;
        }

        void finalizeMPI() {
            // Do not finalize MPI if MPI Consumer has never been initialized.
            // Note that it is still possible that MPI is initialized since the user code might use MPI
            // Therefore we should not check for MPI_Initialized() but rather for our own flag
            if (!this->isClusterPositionDefined) {
                return;
            }

            int isAlreadyFinalized{0};
            MPI_Finalized(&isAlreadyFinalized);

            if (!isAlreadyFinalized) {
                MPI_Finalize();
            } else {
                glogger
                        << "In GMPIConsumerT<>::finalizeMPI():" << std::endl
                        << "MPI has been finalized GMPIConsumerT::finalizeMPI() has been called."
                        << std::endl
                        << "Happened on node with rank " << m_commRank << std::endl
                        << "This might indicate issues in the user code." << std::endl
                        << GWARNING;
            }
        }

        /**
          * Sets the nodes position in the cluster with regard to the MPI_COMMUNICATOR
          * This method is required to be called before any instance method except the constructor is called
          *
          * @return a reference to itself to allow chaining
          */
        GMPIConsumerT<processable_type> &setPositionInCluster() {
            // initialize MPI if not already happened
            initializeMPI(m_argc, m_argv);

            // set members regarding the position of the process in the MPI cluster
            MPI_Comm_size(MPI_COMMUNICATOR, &m_commSize);
            MPI_Comm_rank(MPI_COMMUNICATOR, &m_commRank);
            isClusterPositionDefined = true;

            return *this;
        }

        /**
         * Sets the base communicator for all communication between the master and worker nodes.
         * This allows also working with MPI in the user code by splitting the communicators.
         * GMPIConsumerT must receive a user defined communicator in such case and cannot create its own, because
         * MPI_Comm_split is a collective call that must be called by all ranks.
         *
         * @param communicator the new communicator for communication between master node and worker nodes
         */
        static void setMPICommunicator(MPI_Comm communicator) {
            MPI_COMMUNICATOR = communicator;
        }

        /**
         * Tries to synchronize with all other processes in the communicator.
         * If m_config.waitForMasterStartupTimeoutSec seconds elapse without all processes synchronizing,
         * the synchronization attempt is stopped.
         * @return true in case of all processing synchronizing, otherwise false
         */
        [[nodiscard]] bool trySynchronize() const {
            // handle for asynchronous MPI request
            MPI_Request requestHandle{};

            // asynchronously ask all processes to synchronize here.
            MPI_Ibarrier(MPI_COMMUNICATOR, &requestHandle);

            auto timeStart{std::chrono::steady_clock::now()};

            while (true) {

                int isCompleted{0};
                MPI_Status status{};

                MPI_Test(
                        &requestHandle,
                        &isCompleted,
                        &status);

                if (isCompleted) {
                    if (status.MPI_ERROR != MPI_SUCCESS) {
                        glogger
                                << "In GMPIConsumerT<processable_type>::trySynchronize():" << std::endl
                                << "Received an error:" << std::endl
                                << mpiErrorString(status.MPI_ERROR) << std::endl
                                << "We will try to continue execution anyways." << std::endl
                                << GWARNING;

                        return false; // synchronize stopped but unsuccessful
                    }
                    return true; // synchronize successful
                }

                // TODO: consider removing timeout check here since we expect returning consumer anyways

                if (std::chrono::steady_clock::now() - timeStart >
                    std::chrono::seconds(m_config.waitForMasterStartupTimeoutSec)) {
                    glogger
                            << "In GMPIConsumerT<processable_type>::trySynchronize() with rank="
                            << m_commRank << ":" << std::endl
                            << "Synchronization on startup timed out after "
                            << m_config.waitForMasterStartupTimeoutSec << "s." << std::endl
                            << "Trying to continue unsynchronized." << std::endl
                            << GWARNING;

                    MPI_Cancel(&requestHandle);
                    MPI_Request_free(&requestHandle);

                    return false; // synchronize stopped but unsuccessful
                } else if (m_config.waitForMasterStartupPollIntervalUSec > 0) {
                    // sleep in this thread in case the poll interval is not set to zero and we are not timed out
                    std::this_thread::sleep_for(
                            std::chrono::microseconds{m_config.waitForMasterStartupPollIntervalUSec});
                }
            }
        }

        /**
         * Returns true if the current process has rank 0 within the mpi cluster, otherwise returns false.
         *
         * Requires that cluster position has already been set with  setPositionInCluster
         */
        [[nodiscard]] inline bool isMasterNode() const {
            if (!isClusterPositionDefined) {
                throw gemfony_exception(
                        g_error_streamer(DO_LOG, time_and_place)
                                << "GMPIConsumerT<>::isMasterNode():" << std::endl
                                << "The position of the process in the cluster is undefined." << std::endl
                                << "Use GMPIConsumerT<>::setPositionInCluster() to let the node figure out its position "
                                   "before calling any methods that require this information.");
            }
            return m_commRank == RANK_MASTER_NODE;
        }

        /**
         * Returns true if the current process has a rank different from 0 within the mpi cluster, otherwise returns false.
         *
         * Requires that cluster position has already been set with  setPositionInCluster
         */
        [[nodiscard]] inline bool isWorkerNode() const {
            return !isMasterNode();
        }

    protected:
        /**
         * Shuts down the consumer. This method shall only be called if the process is running a master node and if it
         * has already started.
         *
         * Inherited from GBaseConsumerT
         */
        void shutdown_() override {
            if (!isMasterNode()) {
                glogger
                        << "In GMPIConsumerT<>::shutdown_():" << std::endl
                        << "shutdown_ method is only supposed to be called by instances running master mode."
                        << std::endl
                        << "But the calling node with rank " << m_commRank << " is a worker node." << std::endl
                        << "The method will therefore exit." << std::endl
                        << GWARNING;
                return;
            }
            m_masterNodePtr->shutdown();
        }

    private:
        //-------------------------------------------------------------------------
        // private methods that override methods of the base class

        /**
         * Adds local command line options to a boost::program_options::options_description object.
         *
         * Inherited from GBaseConsumerT
         *
         * @param visible Command line options that should always be visible
         * @param hidden Command line options that should only be visible upon request for details
         */
        void addCLOptions_(
                boost::program_options::options_description &visible,
                boost::program_options::options_description &hidden) override {
            // Note that we use the current values of the members as default values, because in a default constructed
            // instance the defaults are already set. This allows to reduce duplication of those values
            namespace po = boost::program_options;

            // add command line options from our configuration struct
            m_config.addCLOptions_(visible, hidden);
        }

        /**
         * Takes a boost::program_options::variables_map object and acts on
         * the received command line options.
         *
         * Inherited from GBaseConsumerT
         *
         */
        void actOnCLOptions_(const boost::program_options::variables_map &vm) override { /* nothing */
        }

        /**
         * A unique identifier for a given consumer
         *
         * Inherited from GBaseConsumerT
         *
         * @return A unique identifier for a given consumer
         */
        [[nodiscard]] std::string getConsumerName_() const BASE override {
            return {"GMPIConsumerT"};
        }

        /**
         * Returns a short identifier for this consumer.
         *
         * Inherited form GBaseConsumerT
         * @return short identifier for this consumer
         */
        [[nodiscard]] std::string getMnemonic_() const BASE override {
            return {"mpi"};
        }

        /**
         * Inherited from GBaseConsumerT
         *
         * Requires that cluster position has already been set with  setPositionInCluster
         */
        void async_startProcessing_() BASE override {
            if (!isMasterNode()) {
                glogger
                        << "In GMPIConsumerT<>::async_startProcessing_():" << std::endl
                        << "async_startProcessing_ method is only supposed to be called by instances running master mode."
                        << std::endl
                        << "But the calling node with rank " << m_commRank << " is a worker node." << std::endl
                        << "The method will therefore exit." << std::endl
                        << GWARNING;
                return;
            }
            instantiateNode();

            m_masterNodePtr->async_startProcessing();
        }

        /**
         *
         * Inherited from GBaseConsumerT
         * @return true if the consumer needs clients to submit the work to, otherwise false
         */
        [[nodiscard]] bool needsClient_() const noexcept override {
            return true;
        }

        /**
         * Inherited from GBaseConsumerT
         * @param exact
         * @return the amount of worker nodes in the cluster used by the instance of GMPIConsumerT
         */
        size_t getNProcessingUnitsEstimate_(bool &exact) const BASE override {
            exact = true; // mark the answer as exact
            return m_commSize - 1;
        }

        /**
         * Inherited from GBaseConsumerT
         * @return
         */
        [[nodiscard]] bool capableOfFullReturn_() const BASE override {
            return true; // mpi errors are fatal in most cases, assume everything returns or we have an error
        }

        /**
         * This method returns a client associated with this consumer.
         *
         * Inherited from GBaseonsumerT
         * This function makes only sense to be called if the current node has rank 1-n (i.e. is a worker node).
         * Therefore we can simply return the current object, because it is already a client.
         */
        std::shared_ptr<typename Gem::Courtier::GBaseClientT<processable_type>> getClient_() const override {
            if (isMasterNode()) {
                throw gemfony_exception(
                        g_error_streamer(DO_LOG, time_and_place)
                                << "GMPIConsumerT<>::getClient_():" << std::endl
                                << "The current node is the master node in the MPI cluster." << std::endl
                                << "Trying to construct a client a.k.a. worker from this node is not permitted."
                                << std::endl
                                << "But still the getClient_ method has been called.");
            }

            // As this is a const method we only get a const pointer to this.
            // For the most consumers this method will simply create a new object of type GBaseClientT and then return it,
            // which does not affect this and therefore does not collide with the const qualifier of this.
            // However, as the GMPIConsumerT has the functionalities of a GBaseClientT and GBaseServerT in order to abstract
            // away the calls to MPI_Init and friends, we must return this very object itself instead of instantiate a new client.
            // As the returned object is expected to be mutable, we must advise the compiler to drop the const qualifier
            // at this point. Of course, it would be nicer to not even have the const qualifier in the first place, but because
            // this is an overridden method we can not change this. For the other consumers it makes sense to have the const
            // qualifier here.
            auto mutable_self = const_cast<GMPIConsumerT<processable_type> *>(this);

            return std::dynamic_pointer_cast<GBaseClientT<processable_type>>(
                    mutable_self->shared_from_this());
        }

        void init_() override {
            setPositionInCluster();
        }

        /**
         * Inherited from GBaseClientT
         */
        void run_() BASE override {
            if (!isWorkerNode()) {
                glogger
                        << "In GMPIConsumerT<>::run_():" << std::endl
                        << "run_ method is only supposed to be called by instances running worker mode." << std::endl
                        << "But the calling node with rank " << m_commRank << " is the master node." << std::endl
                        << "The method will therefore exit." << std::endl
                        << GWARNING;
                return;
            }
            instantiateNode();

            m_workerNodePtr->run();
        }

        /**
         * Instantiates the node depending on what isWorkerNode() / isMasterNode() returns
         *
         * This is decoupled from the constructor in order to be able to construct the object first and later change its
         * configuration (members) before constructing the contained node with the adjusted configuration.
         */
        void instantiateNode() {
            // instantiate the correct class according to the position in the cluster
            if (isMasterNode()) {
                m_masterNodePtr = std::make_shared<GMPIConsumerMasterNodeT<processable_type>>(
                        m_commSize,
                        m_config);
            } else {
                // note that we cannot create a shared pointer from this because we are currently in the constructor
                // and therefore the precondition that there must already exist one shared pointer pointing to this
                // is not met. But as the lambdas are passed an instance that is a member of the consumer, we are pretty
                // safe already with raw pointers, because the lifetime of the GMPIConsumerWorkerNodeT is bound to the
                // lifetime of this object
                m_workerNodePtr = std::make_shared<GMPIConsumerWorkerNodeT<processable_type>>(
                        m_commSize,
                        m_commRank,
                        [this]() -> bool { return this->halt(); },
                        [this]() -> void { this->incrementProcessingCounter(); },
                        m_config);
            }
        }

        //-------------------------------------------------------------------------
        // Data

        MPIConsumerConfig m_config;
//        MasterNodeConfig m_masterNodeConfig;
//        WorkerNodeConfig m_workerNodeConfig;

        // it might seem like unique pointers are sufficient in the first place.
        // However, we need to call shared_from_this in the objects themselves to pass a reference to them
        // to lambda functions which are used in different threads. shared_from_this has the precondition
        // that there is already a shared pointer pointing to this. So we must use shared_ptr here already.
        std::shared_ptr<GMPIConsumerMasterNodeT<processable_type>> m_masterNodePtr;
        std::shared_ptr<GMPIConsumerWorkerNodeT<processable_type>> m_workerNodePtr;

        std::int32_t m_commSize;
        std::int32_t m_commRank;

        int *m_argc{nullptr};
        char ***m_argv{nullptr};

        bool isClusterPositionDefined{false};

        // This instance of a shared_ptr keeps it alive as long as the GMPIConsumerT exists
        // Normally this should not be necessary, because we have a static instance of shared_ptr to the logger in the
        // GSingleton class. However, there is a bug that was worked around by inserting this member variable.
        // We are as of right now not sure why the bug persists.
        // If nobody resets the GSingleton or the pointer manually that would mean that some parts of the Consumer are
        // still running after the destructor of the static shared_ptr instance in GSingleton has been called. That would
        // mean that there are threads in GMPIConsumerT that run longer than the main thread. This should not be the case
        // as all threads are joined once GMPIConsumerT::shutdown() is called.
        // TODO: investigate why there is a "pure virtual method called" error when compiling without the below line
        //  NOTE: the error is non-deterministic and occurs roughly in 34% of the run, especially when using Go2
        //      It occurs after the optimization has been completed in the GMPIConsumerSessionT class when logging that
        //      a session has been told to stop and will be canceled.
        std::shared_ptr<Gem::Common::GLogger<Gem::Common::GLogStreamer>> m_logger = glogger_ptr; // DO NOT DELETE, unused but keeps instance behind shared_ptr alive
    };

} /* namespace Gem::courtier */
