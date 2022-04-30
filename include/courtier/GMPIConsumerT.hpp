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

namespace Gem::Courtier {
    // constants that are used by the master and the worker nodes
    const int TAG_REQUEST_WORK_ITEM = 42;
    const int TAG_SEND_WORK_ITEM = 43;
    const int RANK_MASTER_NODE = 0;
    const MPI_Comm MPI_COMMUNICATOR = MPI_COMM_WORLD;

    /**
     * Combines all configuration options necessary for a master node
     */
    struct MasterNodeConfig {
        /**
         * The number of threads in a thread pool which is used to handle incoming requests.
         */
        boost::uint32_t nIOThreads{0};
        /**
         * The time in microseconds between each check for a new incoming connection
         */
        boost::uint32_t receivePollIntervalUSec{10};
        /**
         * The time in microseconds between each check of completion of the send operation of a new work item to a worker node.
         */
        boost::uint32_t sendPollIntervalUSec{10};
        /**
         * The maximum time in microseconds to wait until a send operation of a new work item to a worker node succeeds.
         */
        boost::uint32_t sendPollTimeoutUSec{1'000'000};

        /**
         * Adds local command line options to a boost::program_options::options_description object.
         *
         * @param visible Command line options that should always be visible
         * @param hidden Command line options that should only be visible upon request for details
         */
        void addCLOptions(
                boost::program_options::options_description &visible,
                boost::program_options::options_description &hidden) {
            // Note that we use the current values of the members as default values, because in a default constructed
            // instance the defaults are already set. This allows to reduce duplication of those values
            namespace po = boost::program_options;
            // add general options for GMPIConsumerT
            visible.add_options()
                    ("mpi_master_nIOThreads",
                     po::value<boost::uint32_t>(&nIOThreads)->default_value(nIOThreads),
                     "\t[mpi-master-node] The number of threads in a thread pool which is used to handle incoming requests."
                     "The default of 0 triggers a dynamic configuration depending on the number of cpu cores on the current system");

            hidden.add_options()
                    ("mpi_master_receivePollInterval",
                     po::value<boost::uint32_t>(&receivePollIntervalUSec)->default_value(receivePollIntervalUSec),
                     "\t[mpi-master-node] The time in microseconds between each check for a new incoming connection");

            hidden.add_options()
                    ("mpi_master_sendPollInterval",
                     po::value<boost::uint32_t>(&sendPollIntervalUSec)->default_value(sendPollIntervalUSec),
                     "\t[mpi-master-node] The time in microseconds between each check of completion of the send operation of a new work item to a worker node");

            hidden.add_options()
                    ("mpi_master_sendPollTimeout",
                     po::value<boost::uint32_t>(&sendPollTimeoutUSec)->default_value(sendPollTimeoutUSec),
                     "\t[mpi-master-node] The maximum time in microseconds to wait until a send operation of a new work item to a worker node succeeds");
        }
    };

    /**
     * Combines all configuration options necessary for a master node
     */
    struct WorkerNodeConfig {
        /**
         * Whether to issue a request for the next work item while the current work item is still being processed.
         */
        bool useAsynchronousRequests{true};
        /**
         * The time in microseconds between each check for completion of the request for a new work item.
         */
        boost::uint32_t ioPollIntervalUSec{10};
        /**
         * The maximum time in microseconds to wait until a request for a new work item has been answered.
         */
        boost::uint32_t ioPollTimeoutUSec{1'000'000};

        /**
         * Adds local command line options to a boost::program_options::options_description object.
         *
         * @param visible Command line options that should always be visible
         * @param hidden Command line options that should only be visible upon request for details
         */
        void addCLOptions(
                boost::program_options::options_description &visible,
                boost::program_options::options_description &hidden) {
            // Note that we use the current values of the members as default values, because in a default constructed
            // instance the defaults are already set. This allows to reduce duplication of those values
            namespace po = boost::program_options;
            // add general options for GMPIConsumerT
            visible.add_options()
                    ("mpi_worker_asyncRequests",
                     po::value<bool>(&useAsynchronousRequests)->default_value(useAsynchronousRequests),
                     "\t[mpi-worker-node] Whether to issue a request for the next work item while the current work item is still being processed");

            hidden.add_options()
                    ("mpi_worker_pollInterval",
                     po::value<boost::uint32_t>(&ioPollIntervalUSec)->default_value(ioPollIntervalUSec),
                     "\t[mpi-worker-node] The time in microseconds between each check for completion of the request for a new work item.");

            hidden.add_options()
                    ("mpi_worker_pollTimeout",
                     po::value<boost::uint32_t>(&ioPollTimeoutUSec)->default_value(ioPollTimeoutUSec),
                     "\t[mpi-worker-node] The maximum time in microseconds to wait until a request for a new work item has been answered");
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
 * (5) Asynchronously send the result of the processing to the master node, this message also simultaneously requests a
 *      new work item\n
 * (6) Wait for the response i.e. go back to step (2) again\n
 *
 * All communication between GMPIConsumerWorkerNodeT and GMPIConsumerMasterNodeT works with asynchronous communication
 * using the MPI. If an asynchronous request is not able to complete within a predefined timeframe, a timeout will be triggered.
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
         * @param serializationMode mode of serialization between master node and worker nodes
         * @param worldSize number of nodes in the cluster, which is equal to the number of workers + 1
         * @param worldRank this node's rank within this cluster
         * @param halt function that returns true if halt criterion has been reached
         * @param incrementProcessingCounter function that increments the counter for already processed items on this node
         * @param config configuration for this worker injected by the end user
         */
        explicit GMPIConsumerWorkerNodeT(
                Gem::Common::serializationMode serializationMode,
                boost::int32_t worldSize,
                boost::int32_t worldRank,
                std::function<bool()> halt,
                std::function<void()> incrementProcessingCounter,
                WorkerNodeConfig config)
                : m_serializationMode{serializationMode},
                  m_worldSize{worldSize},
                  m_worldRank{worldRank},
                  m_halt{std::move(halt)},
                  m_incrementProcessingCounter{std::move(incrementProcessingCounter)},
                  m_config{config} {
            glogger << "GMPIConsumerWorkerNodeT with rank " << m_worldRank
                    << " started up" << std::endl << GLOGGING;
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
                    m_serializationMode);

            // send initial GETDATA request to receive first work item
            if (!sendResultAndRequestNewWork()) {
                return; // return if unrecoverable error in networking occurred
            }

            while (!m_halt()) {
                // swap messages in a
                m_outgoingMessage = std::move(Gem::Courtier::container_to_string(
                        m_commandContainer,
                        m_serializationMode));
                Gem::Courtier::container_from_string(
                        m_incomingMessage,
                        m_commandContainer,
                        m_serializationMode);


                if (m_config.useAsynchronousRequests) {
                    std::future<bool> networkingSuccessful = std::async(
                            std::launch::async,
                            [this]() -> bool { return this->sendResultAndRequestNewWork(); });

                    processWorkItem();

                    if (!networkingSuccessful.get()) {
                        return; // return if unrecoverable error in networking occurred
                    }

                } else {
                    if (!sendResultAndRequestNewWork()) {
                        return;  // return if unrecoverable error in networking occurred
                    }
                    processWorkItem();
                }
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
                    &m_receiveHandle
            );

            // time in microseconds that have been spent waiting during communication already
            boost::uint32_t alreadyWaited{0};
            int receiveIsCompleted{0};
            MPI_Status receiveStatus{};

            // continue testing until receive was completed or timeout has been triggered.
            // note that we never wait for the send operation to complete, because receiving will only complete after
            // if the send operation has completed, because the server/master will only send a response if it has received our request
            while (!receiveIsCompleted) {
                MPI_Test(&m_receiveHandle,
                         &receiveIsCompleted,
                         &receiveStatus);

                if (alreadyWaited > m_config.ioPollTimeoutUSec) {
                    glogger
                            << "In GMPIConsumerWorkerNodeT<processable_type>::sendResultAndRequestNewWork() with rank="
                            << m_worldRank << ":"
                            << std::endl
                            << "Timeout triggered when communicating with GMPIConsumerMasterNodeT" << std::endl
                            << "We assume the server is down and will exit the worker." << std::endl
                            << "The reason for the timeout could be that the server has completed computation and exited "
                               "or a potential error or crash on the server side." << std::endl
                            << GLOGGING;

                    cancelActiveRequests();
                    return false;
                }

                alreadyWaited += m_config.ioPollIntervalUSec;
                usleep(m_config.ioPollIntervalUSec);
            }

            if (receiveStatus.MPI_ERROR != MPI_SUCCESS) {
                glogger
                        << "In GMPIConsumerWorkerNodeT<processable_type>::sendResultAndRequestNewWork() with rank="
                        << m_worldRank << ":" << std::endl
                        << "Received an error receiving a message from GMPIConsumerMasterNodeT:" << std::endl
                        << mpiErrorString(receiveStatus.MPI_ERROR) << std::endl
                        << "Worker node will shut down." << std::endl
                        << GWARNING;

                cancelActiveRequests();
                return false;
            }

            // create string with correct size from the fixed sie buffer
            m_incomingMessage = std::string(m_incomingMessageBuffer.get(), mpiGetCount(receiveStatus));

            return true;
        }

        /**
         * Cancels the active requests that are associated with the handles, which are stored as member variables.
         */
        void cancelActiveRequests() {
            MPI_Cancel(&m_receiveHandle);
            MPI_Request_free(&m_sendHandle);
        }

        /**
         * Processes the work item that is stored in the member m_commandContainer.
         * After processing has been finished the result is put into m_commandContainer i.e. it overrides the old item.
         * In case that m_commandContainer did not contain any work items this method will store a new GETDATA request
         * in m_commandContainer to retrieve new work in the next iteration.
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
                default: {
                    // Emit a warning, ignore item and request new item
                    glogger << "GMPIConsumerWorkerNodeT<processable_type>::processWorkItem() with rank=" << m_worldRank
                            << ":" << std::endl
                            << "Got unknown or invalid command "
                            << boost::lexical_cast<std::string>(m_commandContainer.get_command()) << std::endl
                            << GWARNING;

                    m_commandContainer.reset(networked_consumer_payload_command::GETDATA);
                }
            }
        }

        //-------------------------------------------------------------------------
        // Private data

        /**
         * number of nodes in the cluster (number of worker nodes + 1)
         */
        boost::int32_t m_worldSize;
        /**
         * rank of this node in the cluster
         */
        boost::int32_t m_worldRank;
        /**
         * serialization mode to use for messages between master node and worker nodes
         */
        Gem::Common::serializationMode m_serializationMode;
        /**
         * Callback function that returns true if the halt criterion has been reached
         */
        std::function<bool()> m_halt;
        /**
         * Increments the counter for processed work items of the calling instance of GConsumerBaseT.
         */
        std::function<void()> m_incrementProcessingCounter;
        /**
         * Configuration of this client specified by the end-user.
         */
        WorkerNodeConfig m_config;

        // only one request is processed at a time. Even in the version with asynchronous request the thread which is
        // responsible for handling the IO will always be joined before the next thread for the next IO-operation is spawned.
        // So the program logic ensures that the access to these resources is exclusive to one thread, so we do not
        // need to enforce this with mutex or something similar and can store the handles as members.
        MPI_Request m_sendHandle{};
        MPI_Request m_receiveHandle{};

        /**
         * counter for how many times we have not received data when requesting data from the master node
         */
        boost::int32_t m_nNoData{0};

        std::random_device m_randomDevice; ///< Source of non-deterministic random numbers
        std::mt19937 m_randomNumberEngine{
                m_randomDevice()}; ///< The actual random number engine, seeded by m_randomDevice

        // we only work with one IO-thread here. So we can store the buffer as a member variable without concurrency issues
        std::unique_ptr<char[]> m_incomingMessageBuffer;
        std::string m_incomingMessage;
        std::string m_outgoingMessage;
        GCommandContainerT <processable_type, networked_consumer_payload_command> m_commandContainer{
                networked_consumer_payload_command::GETDATA}; ///< Holds the current command and payload (if any)
    };


/**
 * This class represents one session between the master node and one worker node.
 *
 * A GMPIConsumerSessionT can be opened as soon as the master node has fully received a request from
 * a worker node. The opened GMPIConsumerSessionT will then take care of putting deserializing and processing
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
         * @param isToldToStop a reference to a thread safe atomic bool variable indicating whether the caller decided abort this session.
         * @param ioPollIntervalUSec time in microseconds between each poll for the status of the send operation of the response
         * @param ioPollTimeoutUSec maximum time in microseconds for a sending a response until a timeout will get triggered
         */
        GMPIConsumerSessionT(
                MPI_Status status,
                std::string requestMessage,
                std::function<std::shared_ptr<processable_type>()> getPayloadItem,
                std::function<void(std::shared_ptr<processable_type>)> putPayloadItem,
                Gem::Common::serializationMode serializationMode,
                std::atomic<bool> &isToldToStop,
                boost::uint32_t ioPollIntervalUSec,
                boost::uint32_t ioPollTimeoutUSec)
                : m_mpiStatus{status},
                // avoid copying the string but also not taking it as reference because it should be owned by this object
                  m_requestMessage{std::move(requestMessage)},
                  m_getPayloadItem(std::move(getPayloadItem)),
                  m_putPayloadItem(std::move(putPayloadItem)),
                  m_serializationMode{serializationMode},
                  m_isToldToStop{isToldToStop},
                  m_ioPollIntervalUSec{ioPollIntervalUSec},
                  m_ioPollTimeoutUSec{ioPollTimeoutUSec},
                  m_mpiRequestHandle{} {}

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
        bool run() {
            // only execute sendResponse if processRequest was successful
            return processRequest() && sendResponse();
        }

    private:

        bool processRequest() {
            try {
                // deserialize request string
                Gem::Courtier::container_from_string(
                        m_requestMessage, m_commandContainer, m_serializationMode
                ); // may throw

                // Extract the command
                auto inboundCommand = m_commandContainer.get_command();

                // If we have some payload received, add it to its destination
                // Either way retrieve the next work item and send it to the client for processing
                switch (inboundCommand) {
                    case networked_consumer_payload_command::GETDATA: {
                        getAndSerializeWorkItem();
                        return true;
                    }
                    case networked_consumer_payload_command::RESULT: {
                        putWorkItem();
                        getAndSerializeWorkItem();
                        return true;
                    }
                    default: {
                        glogger
                                << "GMPIConsumerSessionT<processable_type>::process_request() connected to rank="
                                << m_mpiStatus.MPI_SOURCE << ":" << std::endl
                                << "Got unknown or invalid command " << boost::lexical_cast<std::string>(inboundCommand)
                                << std::endl
                                << GWARNING;
                    }
                }

            } catch (const gemfony_exception &ex) {
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

        void getAndSerializeWorkItem() {
            // Obtain a container_payload object from the queue, serialize it and send it off
            // this function includes a timeout that might result in a nullptr being returned
            auto payloadPtr = this->m_getPayloadItem();

            if (payloadPtr) {
                m_commandContainer.reset(networked_consumer_payload_command::COMPUTE, payloadPtr);
            } else {
                m_commandContainer.reset(networked_consumer_payload_command::NODATA);
            }

            // serialize container, save as member and return
            m_outgoingMessage = Gem::Courtier::container_to_string(
                    m_commandContainer, m_serializationMode
            );
        }

        bool sendResponse() {
            // asynchronously start sending the response
            MPI_Isend(
                    m_outgoingMessage.data(),
                    m_outgoingMessage.size(),
                    MPI_CHAR,
                    m_mpiStatus.MPI_SOURCE,
                    TAG_SEND_WORK_ITEM,
                    MPI_COMMUNICATOR,
                    &m_mpiRequestHandle);

            boost::uint32_t alreadyWaited{0};

            // wait for the completion of the asynchronous call to verify no errors have occurred
            while (!m_isToldToStop) {
                int isCompleted{0};
                MPI_Status status{};

                MPI_Test(
                        &m_mpiRequestHandle,
                        &isCompleted,
                        &status);

                if (isCompleted) {
                    return true;
                }

                if (alreadyWaited > m_ioPollTimeoutUSec) {
                    glogger
                            << "In GMPIConsumerSessionT<processable_type>::sendResponse() connected to rank="
                            << m_mpiStatus.MPI_SOURCE << ":" << std::endl
                            << "Configured timeout of " << m_ioPollTimeoutUSec
                            << " microseconds has been reached for sending a work item to a worker node" << std::endl
                            << "Response will be canceled." << std::endl
                            << GWARNING;
                    cancelActiveRequest();
                    return false;
                }

                // if send not yet, sleep a short amount of time until polling again for the message status
                usleep(m_ioPollIntervalUSec);
                alreadyWaited += m_ioPollIntervalUSec;
            }

            glogger
                    << "In GMPIConsumerSessionT<processable_type>::sendResponse() connected to rank="
                    << m_mpiStatus.MPI_SOURCE << ":" << std::endl
                    << "Handler thread was told to stop before sending the response was completed." << std::endl
                    << "Response will be canceled." << std::endl
                    << GLOGGING;
            cancelActiveRequest();
            return false;
        }

        /**
         * cancels an active send request that is referenced by the member variable m_mpiRequestHandle
         */
        void cancelActiveRequest() {
            MPI_Cancel(&m_mpiRequestHandle);
            MPI_Request_free(&m_mpiRequestHandle);
        }

        //-------------------------------------------------------------------------
        // Data
        const MPI_Status m_mpiStatus;
        const std::string m_requestMessage;
        const Gem::Common::serializationMode m_serializationMode;
        const boost::uint32_t m_ioPollIntervalUSec;
        const boost::uint32_t m_ioPollTimeoutUSec;

        std::function<std::shared_ptr<processable_type>()> m_getPayloadItem;
        std::function<void(std::shared_ptr<processable_type>)> m_putPayloadItem;

        // references a thread-safe indicator for shutdown that is stored in the scope calling the constructor
        std::atomic<bool> &m_isToldToStop;

        GCommandContainerT <processable_type, networked_consumer_payload_command> m_commandContainer{
                networked_consumer_payload_command::NONE
        }; ///< Holds the current command and payload (if any)
        MPI_Request m_mpiRequestHandle;
        std::string m_outgoingMessage;
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
     * (1) Create thread pool using boost::thread_group and boost::io_service\n
     * (2) Spawn a new thread that receives request (receiverThread). Then return from the function immediately, because
     * geneva expects consumers to be background processes on other threads than the main thread.\n
     * (3) Once a shutdown is supposed to occur:\n
     *  (3.1) Set a member flag to tell the receiver thread to stop\n
     *  (3.2) Stop the boost::io_service object\n
     *  (3.3) Join all threads. The receiver thread should exit itself and all handler threads will finish their last task
     *        and then finish.\n\n
     *
     * Then the main loop in the receiver thread works as follows:\n
     * (2) Asynchronously receive message from any worker node if not yet told to stop (check thread safe member variable)\n
     * (2) Once a request has been received: schedule a handler on the thread pool and wait for another message
     *     to receive i.e. go back to (1)\n\n
     *
     * Then the handler thread works as follows (implemented in the class GMPIConsumerSessionT):\n
     *  (3.1) Deserialize received object\n
     *  (3.2) If the message from the worker includes a processed item, put it into the processed items queue of the broker\n
     *  (3.3) Take an item from the non-processed items queue (if there currently is one available)\n
     *  (3.4) Serialize the response container, which contains a new work item or the NODATA command.\n
     *  (3.5) Asynchronously send the item to the worker node which has requested it.\n
     *  (3.6) Exit the handler. This lets boost use this thread for further jobs.\n
     *
     */
    template<typename processable_type>
    class GMPIConsumerMasterNodeT
            : public std::enable_shared_from_this<GMPIConsumerMasterNodeT<processable_type>> {

    public:
        /**
         * Constructor to instantiate the GMPIConsumerMasterNodeT
         * @param serializationMode mode of serialization to use for messages transferred between master node and worker nodes.
         * @param worldSize number of nodes in the cluster, which is equal to the number of workers + 1
         * @param config configuration for this node specified by the end user
         */
        explicit GMPIConsumerMasterNodeT(
                Gem::Common::serializationMode serializationMode,
                boost::int32_t worldSize,
                MasterNodeConfig config)
                : m_serializationMode{serializationMode},
                  m_worldSize{worldSize},
                  m_isToldToStop{false},
                  m_config{config} {
            configureNHandlerThreads();
            glogger << "GMPIConsumerMasterNodeT started with n=" << m_config.nIOThreads << " IO-threads" << std::endl
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
         * First a thread-pool will be created. Then another thread will be created which listens for requests and
         * schedules request handlers on the thread-pool. Once the threads have been created this method will immediately
         * return to the caller while the spawned threads run in the background and fulfil their job.
         * To stop the master node and all its threads again the shutdown method can be called.
         */
        void async_startProcessing() {
            m_handlerThreadPool = std::make_unique<Common::GThreadPool>(m_config.nIOThreads);

            auto self = this->shared_from_this();
            m_receiverThread = std::thread(
                    [self] { self->listenForRequests(); }
            );
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

            // stop the receiver thread
            m_receiverThread.join();

            // wait until all threads have finished their job
            m_handlerThreadPool->wait();

            // call the destructor of the thread pool
            m_handlerThreadPool.reset();
        }

    private:
        void listenForRequests() {
            while (!m_isToldToStop) {
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
                        &requestHandle
                );

                while (true) {
                    int isCompleted{0};
                    MPI_Status status{};

                    MPI_Test(
                            &requestHandle,
                            &isCompleted,
                            &status);

                    if (isCompleted) {
                        // let a new thread handle this request and listen for further requests
                        // we capture copies of smart pointers in the closure, which keeps the underlying data alive
                        const auto self = this->shared_from_this();
                        boost::asio::post(
                                [&self, status, buffer] { self->handleRequest(status, buffer); });

                        break;
                    }

                    if (m_isToldToStop) {
                        // cancel pending request and return in case we receive a stop signal
                        cancelRequest(requestHandle);
                        return;
                    }

                    // if receive not completed yet, sleep a short amount of time until polling again for the message status
                    usleep(m_config.receivePollIntervalUSec);
                }
            }
        }

        void handleRequest(const MPI_Status &status, const std::shared_ptr<char[]> &buffer) {
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

            // start a new session i.e. answering the request
            GMPIConsumerSessionT<processable_type>{
                    status,
                    std::string{buffer.get(), static_cast<size_t>(mpiGetCount(status))},
                    [this]() -> std::shared_ptr<processable_type> { return getPayloadItem(); },
                    [this](std::shared_ptr<processable_type> p) { putPayloadItem(p); },
                    m_serializationMode,
                    m_isToldToStop,
                    m_config.sendPollIntervalUSec,
                    m_config.sendPollTimeoutUSec
            }.run();

            // at time of leaving this method the request has been handled and the job for this thread is finished.
            // This thread will then be able to be scheduled for further requests by the IO-thread again
        }

        /**
         * Cancels an active mpi request.
         * @param request the request to be canceled.
         */
        void cancelRequest(MPI_Request &request) {
            MPI_Cancel(&request);
            MPI_Request_free(&request);
        }

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
                                << "Function called with empty work item" << std::endl
                );
            }

            if (not m_brokerPtr->put(p, m_timeout)) {
                glogger
                        << "In GMPIConsumerMasterNodeT<>::putPayloadItem():" << std::endl
                        << "Work item could not be submitted to the broker" << std::endl
                        << "The item will be discarded" << std::endl
                        << GWARNING;
            }
        }

        void configureNHandlerThreads() {
            // if thread pool size is set to 0 by user, set it to a recommendation
            if (m_config.nIOThreads == 0) {
                m_config.nIOThreads = nHandlerThreadsRecommendation();
            }
        }

        [[nodiscard]] boost::uint32_t nHandlerThreadsRecommendation() const {
            // At least the main thread (consumer runs not on main thread)
            // and the thread for receiving incoming connections should get their own processor
            const long long threadsReservedForOtherTasks{2};
            long long numCPUCores = sysconf(_SC_NPROCESSORS_ONLN);

            long long recommendation{numCPUCores - threadsReservedForOtherTasks};

            // at least one thread must be used for handling requests, even if system has not as many cores
            if (recommendation < 1) {
                recommendation = 1;
            }

            return recommendation > 0 ? recommendation : 1;
        }

        //-------------------------------------------------------------------------
        // Data

        Gem::Common::serializationMode m_serializationMode;
        boost::int32_t m_worldSize;
        MasterNodeConfig m_config;

        std::unique_ptr<Common::GThreadPool> m_handlerThreadPool;
        std::thread m_receiverThread;
        std::atomic<bool> m_isToldToStop;

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
     * It is derived from both base classes - GBaseClientT and GBaseConsumerT and can therefore be used as either of these.
     * It instantiates the correct classes (GMPIConsumerMasterNodeT or GMPIConsumerWorkerNodeT) and forwards calls to its methods
     * to one methods of the two classes. This serves as an abstraction of MPI such that the user does not need to explicitly
     * ask for the process's rank.
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
         * Initialized the MPI framework. Then initialized the consumer to fulfill its role inside the cluster.
         * Depending on the rank (0 = master node (server), 1-n = worker node (client)) the member variables will be
         * set accordingly.
         *
         * NOTE: The constructor of GMPIConsumerT is only allowed to be called once per process. The reason for this is
         * because the constructor of GMPIConsumerT calls MPI_Init_Thread and the destructor calls MPI_Finalize. Both
         * those functions are not allowed to be called more than once per process. Therefore if you want to get a reference
         * to GMPIConsumerT in multiple locations of your code, it is convenient to use the GSingletonT class.
         *
         * @param argc argument count passed to main function, which will be forwarded to the MPI_Init call
         * @param argv argument vector passed to main function, which will be forwarded to MPI_Init call
         * @param serializationMode serialization mode to use for messages send between the master node and worker nodes
         * @param masterNodeConfig configuration specifically for the case in which this instance is the master node
         * @param workerNodeConfig configuration specifically for the case in which this instance is a worker node
         */
        explicit GMPIConsumerT(
                int *argc = nullptr,
                char ***argv = nullptr,
                Gem::Common::serializationMode serializationMode = Gem::Common::serializationMode::BINARY,
                MasterNodeConfig masterNodeConfig = MasterNodeConfig{},
                WorkerNodeConfig workerNodeConfig = WorkerNodeConfig{})
                : m_serializationMode{serializationMode},
                  m_masterNodeConfig{masterNodeConfig},
                  m_workerNodeConfig{workerNodeConfig},
                  m_worldRank{},
                  m_worldSize{} {

            // initialize MPI
            int providedThreadingLevel = 0;
            MPI_Init_thread(argc, argv, MPI_THREAD_MULTIPLE, &providedThreadingLevel);

            if (providedThreadingLevel != MPI_THREAD_MULTIPLE) {
                throw gemfony_exception(
                        g_error_streamer(DO_LOG, time_and_place)
                                << "GMPIConsumerT<> constructor" << std::endl
                                << "Geneva requires MPI implementation with level MPI_THREAD_MULTIPLE (a.k.a. "
                                << MPI_THREAD_MULTIPLE
                                << ") but the runtime environment implementation of MPI only supports level "
                                << providedThreadingLevel << std::endl
                );
            }

            // set members regarding the position of the process in the MPI cluster
            MPI_Comm_size(MPI_COMMUNICATOR, &m_worldSize);
            MPI_Comm_rank(MPI_COMMUNICATOR, &m_worldRank);
        }

        /**
         * The destructor finalazes the MPI framework.
         *
         * This constructor must call the MPI_Finalize function as this function encapsulates all MPI-specific action.
         */
        ~GMPIConsumerT() override {
            MPI_Finalize();
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
         * Returns true if the current process has rank 0 within the mpi cluster, otherwise returns false.
         */
        [[nodiscard]] inline bool isMasterNode() const {
            return m_worldRank == RANK_MASTER_NODE;
        }

        /**
         * Returns true if the current process has a rank different from 0 within the mpi cluster, otherwise returns false.
         */
        [[nodiscard]] inline bool isWorkerNode() const {
            return m_worldRank != RANK_MASTER_NODE;
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
                        << "But the calling node with rank " << m_worldRank << " is a worker node." << std::endl
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
                boost::program_options::options_description &hidden
        ) override {
            // TODO: test that it works
            // Note that we use the current values of the members as default values, because in a default constructed
            // instance the defaults are already set. This allows to reduce duplication of those values
            namespace po = boost::program_options;

            // add specific options for worker and master node
            m_workerNodeConfig.addCLOptions(visible, hidden);
            m_masterNodeConfig.addCLOptions(visible, hidden);

            // add general options for GMPIConsumerT
            hidden.add_options()
                    ("mpi_serializationMode",
                     po::value<Gem::Common::serializationMode>(&m_serializationMode)->default_value(
                             m_serializationMode),
                     "\t[mpi] Specifies whether serialization shall be done in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)");
        }

        /**
         * Takes a boost::program_options::variables_map object and acts on
         * the received command line options.
         *
         * Inherited from GBaseConsumerT
         *
         */
        void actOnCLOptions_(const boost::program_options::variables_map &vm) override { /* nothing */ }

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
         */
        void async_startProcessing_() BASE override {
            if (!isMasterNode()) {
                glogger
                        << "In GMPIConsumerT<>::async_startProcessing_():" << std::endl
                        << "async_startProcessing_ method is only supposed to be called by instances running master mode."
                        << std::endl
                        << "But the calling node with rank " << m_worldRank << " is a worker node." << std::endl
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
            return m_worldSize;
        }

        /**
         * Inherited from GBaseConsumerT
         * @return
         */
        [[nodiscard]] bool capableOfFullReturn_() const BASE override {
            return false; // in network operations we must always expect that error can occur
        }

        /**
         * This method returns a client associated with this consumer.
         *
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
                                << "But still the getClient_ method has been called."
                );
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

        /**
         * Inherited from GBaseClientT
         */
        void run_() BASE override {
            if (!isWorkerNode()) {
                glogger
                        << "In GMPIConsumerT<>::run_():" << std::endl
                        << "run_ method is only supposed to be called by instances running worker mode." << std::endl
                        << "But the calling node with rank " << m_worldRank << " is the master node." << std::endl
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
                        m_serializationMode,
                        m_worldSize,
                        m_masterNodeConfig);
            } else {
                // note that we cannot create a shared pointer from this because we are currently in the constructor
                // and therefore the precondition that there must already exist one shared pointer pointing to this
                // is not met. But as the lambdas are passed an instance that is a member of the consumer, we are pretty
                // safe already with raw pointers, because the lifetime of the GMPIConsumerWorkerNodeT is bound to the
                // lifetime of this object
                m_workerNodePtr = std::make_shared<GMPIConsumerWorkerNodeT<processable_type>>(
                        m_serializationMode,
                        m_worldSize,
                        m_worldRank,
                        [this]() -> bool { return this->halt(); },
                        [this]() -> void { this->incrementProcessingCounter(); },
                        m_workerNodeConfig);
            }
        }

        //-------------------------------------------------------------------------
        // Data
        Gem::Common::serializationMode m_serializationMode;
        MasterNodeConfig m_masterNodeConfig;
        WorkerNodeConfig m_workerNodeConfig;

        // it might seem like unique pointers are sufficient in the first place.
        // However, we need to call shared_from_this in the objects themselves to pass a reference to them
        // to lambda functions which are used in different threads. shared_from_this has the precondition
        // that there is already a shared pointer pointing to this. So we must use shared_ptr here already.
        std::shared_ptr<GMPIConsumerMasterNodeT<processable_type>> m_masterNodePtr;
        std::shared_ptr<GMPIConsumerWorkerNodeT<processable_type>> m_workerNodePtr;

        boost::int32_t m_worldSize;
        boost::int32_t m_worldRank;
    };


} /* namespace Gem::courtier */

