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

// TODO: enhance in-code documentation of public methods
// TODO: do not issue warning but only logging once handler threads have been stopped due to member flag
// TODO: replace std::cout with logging

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// utility functions for working with MPI
#include "GMPIHelperFunctions.hpp"

// Standard headers go here
#include <iostream>
#include <sstream>
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

    // forward declare class because we have a cyclic dependency between MPIConsumerT and MPIConsumerWorkerNodeT
    template<typename processable_type>
    class GMPIConsumerT;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class is responsible for the client side of network communication using MPI.
 * It will repeatedly request work items from the GMPIConsumerMasterNodeT, process those,
 * send them back to the worker and request more work items.
 *
 * The simplified workflow of the GMPIConsumerWorkerNodeT can be described as follows:
 *
 * (1) Send a synchronous GET request (ask for the first work item)
 * (2) Synchronously receive a message containing either the work item or the information that currently no items are available
 * (3) Deserialize the received message
 * (4.1) If the message contains DATA (a raw work item): process the received work item
 * (4.2) If the message contains no data: poll later again i.e. back to step (1)
 * (4.3) If the message contains a SHUTDOWN signal: shut this worker down.
 * (5) Synchronously send the result of the processing to the master node and simultaneously request a new work item
 * (6) Wait for the response i.e. go back to step (2) again
 *
 */

/*
 * Notes on the decision of implementation design:
 * At the side of the worker nodes the communication only happens when there is no more work item to processed.
 * Therefore, we can work with synchronous send and receive operations without waisting any computation time, because at
 * the time of communication the worker would have to wait for new requests anyway. Furthermore, synchronous operations
 * make the code easier to understand. Finally, synchronous send calls allow the MPI implementation to directly send the
 * bytes from the user defined buffer through the network without any copying into a second buffer. In asynchronous mode
 * the MPI library might copy the contents of the user defined buffer to an internal second buffer and then return before
 * the network operation has been completed. This is useful in cases where the computation time and buffer shall be reused
 * in the meanwhile. However, as we do not have any useful tasks to complete in the meantime, we can also use synchronous
 * calls which will avoid unnecessary buffer copying.
 * However, one disadvantage of blocking calls is that they might never return if the other process (node 0 in this case)
 * has some issue. Therefore, the blocking call may remain in an endless waiting state. This is also an issue when shutting
 * down a worker. However, if we look at the implementation the GAsioConsumerClientT we can see that it also does not have
 * an option to shut it down remotely, because the call method run blocks until no work remains in the io_context, which
 * is never the case unless an error or halt criterion breaks the never ending chain of requests.
 */
    template<typename processable_type>
    class GMPIConsumerWorkerNodeT final
            : public std::enable_shared_from_this<GMPIConsumerWorkerNodeT<processable_type>> {

    public:
        explicit GMPIConsumerWorkerNodeT(
                Gem::Common::serializationMode serializationMode,
                boost::int32_t worldSize,
                boost::int32_t worldRank,
                GMPIConsumerT<processable_type> &callingConsumer,
                boost::int32_t nMaxReconnects = GMPICONSUMERMAXCONNECTIONATTEMPTS)
                : m_serializationMode{serializationMode},
                  m_worldSize{worldSize},
                  m_worldRank{worldRank},
                  m_nMaxReconnects{nMaxReconnects},
                  m_callingConsumer{callingConsumer} {
            std::cout << "MPIConsumerWorkerNodeT with rank " << m_worldRank << " started up" << std::endl;
            m_incomingMessageBuffer = std::unique_ptr<char[]>(new char[m_maxIncomingMessageSize]);
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

        void run() {
            // Prepare the outgoing string for the first request
            m_outgoingMessage = Gem::Courtier::container_to_string(
                    m_commandContainer,
                    m_serializationMode
            );

            while (!m_callingConsumer.halt()) {
                // if not all functions succeed (return true), we shut down the server and stop the request loop
                if (!(sendRequestMaybeWithResult()
                      && receiveWorkItem()
                      && processWorkItem())) {
                    shutdown();
                    return;
                }
            }
        }

    private:
        /**
         * Sends the current contents of m_outgoingMessage to the master node.
         * This message contains either just a get request (if it was the first request).
         * Or delivers a result and requests more data (for all following requests).
         * @return true if the communication was successful within the maximum number of reconnects, otherwise false.
         */
        [[nodiscard]] bool sendRequestMaybeWithResult() {
            for (int i{0}; i < m_nMaxReconnects; ++i) {
                int sendResult = MPI_Ssend(
                        m_outgoingMessage.data(),
                        static_cast<int>(m_outgoingMessage.size()),
                        MPI_CHAR,
                        RANK_MASTER_NODE,
                        TAG_REQUEST_WORK_ITEM,
                        MPI_COMM_WORLD);

                if (sendResult == MPI_SUCCESS) {
                    return true;
                }

                glogger
                        << "GMPIConsumerWorkerNodeT<processable_type>::sendRequestMaybeWithResult():" << std::endl
                        << "Request for work item was not successful. Error message: " << std::endl
                        << mpiErrorString(sendResult) << std::endl
                        << "The worker will try to request a work item again." << std::endl
                        << GWARNING;
            }
            glogger
                    << "GMPIConsumerWorkerNodeT<processable_type>::sendRequestMaybeWithResult():" << std::endl
                    << "Request for work item was not successful for " << m_nMaxReconnects << " times."
                    << std::endl
                    << "This is the configures maximum number for reconnects." << std::endl
                    << "The worker node will therefore exit now." << std::endl
                    << GWARNING;

            return false;
        }

        bool receiveWorkItem() {
            MPI_Status status;

            MPI_Recv( // blocking receive
                    m_incomingMessageBuffer.get(),
                    m_maxIncomingMessageSize,
                    MPI_CHAR,
                    RANK_MASTER_NODE,
                    MPI_ANY_TAG,
                    MPI_COMM_WORLD,
                    &status);

            if (status.MPI_ERROR != MPI_SUCCESS) {
                glogger
                        << "GMPIConsumerWorkerNodeT<processable_type>::receiveWorkItem():" << std::endl
                        << "Worker was not able to successfully retrieve work item from master node" << std::endl
                        << std::endl
                        << "The worker node will therefore exit now." << std::endl
                        << GWARNING;

                return false;
            }
            std::string receivedMessage{m_incomingMessageBuffer.get(), static_cast<size_t>(mpiGetCount(status))};

            try {
                // Extract the string from the buffer and de-serialize the object
                Gem::Courtier::container_from_string(
                        receivedMessage, m_commandContainer, m_serializationMode
                ); // may throw
            } catch (const gemfony_exception &ex) {
                glogger
                        << "GMPIConsumerWorkerNodeT<processable_type>::receiveWorkItem(): Caught exception while deserializing command container:"
                        << std::endl << ex.what() << std::endl
                        << GLOGGING;
                return false;
            }

            return true;
        }

        [[nodiscard]] bool processWorkItem() {
            switch (m_commandContainer.get_command()) {
                case networked_consumer_payload_command::COMPUTE: {
                    // process item. This will put the result into the container
                    m_commandContainer.process();

                    // increment the counter for processed items
                    m_callingConsumer.incrementProcessingCounter();

                    // mark the container as "contains a result"
                    m_commandContainer.set_command(networked_consumer_payload_command::RESULT);
                }
                    break;
                case networked_consumer_payload_command::NODATA: {

                    // Update the NODATA counter for bookkeeping
                    ++m_nNoData;

                    // sleep for a short while (between 50 and 200 milliseconds, randomly) before asking again for new work
                    std::uniform_int_distribution<> dist(50, 200);
                    std::this_thread::sleep_for(std::chrono::milliseconds(dist(m_randomNumberEngine)));

                    // Tell the server again we need work
                    m_commandContainer.reset(networked_consumer_payload_command::GETDATA);
                }
                    break;
                default: {
                    // Emit an exception
                    glogger << "GMPIConsumerWorkerNodeT<processable_type>::processWorkItem():" << std::endl
                            << "Got unknown or invalid command "
                            << boost::lexical_cast<std::string>(m_commandContainer.get_command()) << std::endl
                            << GWARNING;

                    return false;
                }
            }

            // serialize the container and set the outgoing message accordingly
            // the next call of sendRequestMaybeWithResult() will transmit the message
            m_outgoingMessage = Gem::Courtier::container_to_string(
                    m_commandContainer,
                    m_serializationMode
            );
            return true;
        }

        void shutdown() {
            // TODO: perform any cleanup work here (probably MPI_Cancel and a following MPI_Wait, or MPI_Request_free?)
        }

        //-------------------------------------------------------------------------
        // Data

        boost::int32_t m_worldSize;
        boost::int32_t m_worldRank;
        Gem::Common::serializationMode m_serializationMode;
        boost::int32_t m_nMaxReconnects;
        GMPIConsumerT<processable_type> &m_callingConsumer;
        const boost::uint32_t m_maxIncomingMessageSize = 1024 * 4;

        // counter for how many times we have not received data when requesting data from the master node
        boost::int32_t m_nNoData = 0;

        std::random_device m_randomDevice; ///< Source of non-deterministic random numbers
        std::mt19937 m_randomNumberEngine{
                m_randomDevice()}; ///< The actual random number engine, seeded by m_randomDevice

        // we only work with one IO-thread here. So we can store the buffer as a member variable without concurrency issues
        std::unique_ptr<char[]> m_incomingMessageBuffer;
        std::string m_outgoingMessage;
        GCommandContainerT<processable_type, networked_consumer_payload_command> m_commandContainer{
                networked_consumer_payload_command::GETDATA}; ///< Holds the current command and payload (if any)
    };


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class represents one session between the master node and one worker node.
 *
 * A GMPIConsumerSessionT can be opened as soon as the master node has fully received a request from
 * a worker node. The opened GMPIConsumerSessionT will then take care of putting deserializing and processing
 * the request as well as responding to it with a new work item (if there are items availible in the brokers queue
 * at that point in time).
 */
    template<typename processable_type>
    class GMPIConsumerSessionT
            : public std::enable_shared_from_this<GMPIConsumerSessionT<processable_type>> {
    public:

        GMPIConsumerSessionT(
                MPI_Status status,
                std::string requestMessage,
                std::function<std::shared_ptr<processable_type>()> getPayloadItem,
                std::function<void(std::shared_ptr<processable_type>)> putPayloadItem,
                Gem::Common::serializationMode serializationMode,
                std::atomic<bool> &isToldToStop)
                : m_mpiStatus{status},
                // avoid copying the string but also not taking it as reference because it should be owned by this object
                  m_requestMessage{std::move(requestMessage)},
                  m_getPayloadItem(std::move(getPayloadItem)),
                  m_putPayloadItem(std::move(putPayloadItem)),
                  m_serializationMode{serializationMode},
                  m_isToldToStop{isToldToStop} {}

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
            if (processRequest() && sendResponse()) {
                return true;
            }
            return false;
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
                                << "GMPIConsumerSessionT<processable_type>::process_request():" << std::endl
                                << "Got unknown or invalid command " << boost::lexical_cast<std::string>(inboundCommand)
                                << std::endl
                                << GWARNING;
                    }
                }

            } catch (const gemfony_exception &ex) {
                auto ePtr = std::current_exception();
                glogger
                        << "GMPIConsumerSessionT<processable_type>::processRequest(): Caught exception while deserializing request"
                        << std::endl
                        << ex.what() << std::endl
                        << GLOGGING;
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
                    << "GMPIConsumerSessionT<processable_type>::process_request():" << std::endl
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
            MPI_Request requestHandle;
            MPI_Isend(
                    m_outgoingMessage.data(),
                    m_outgoingMessage.size(),
                    MPI_CHAR,
                    m_mpiStatus.MPI_SOURCE,
                    TAG_SEND_WORK_ITEM,
                    MPI_COMM_WORLD,
                    &requestHandle);

            while (!m_isToldToStop) {
                int isCompleted;
                MPI_Status status;

                MPI_Test(
                        &requestHandle,
                        &isCompleted,
                        &status);

                if (isCompleted) {
                    return true;
                }
                // if send not yet, sleep a short amount of time until polling again for the message status
                usleep(m_sendResponseCompletionPollInterval);
            }

            glogger
                    << "In GMPIConsumerSessionT<processable_type>::sendResponse():" << std::endl
                    << "Handler thread was told to stop before sending the response was completed." << std::endl
                    << "Response will be canceled." << std::endl
                    << GLOGGING;
            return false;
        }

        //-------------------------------------------------------------------------
        // Data
        const MPI_Status m_mpiStatus;
        const std::string m_requestMessage;
        const boost::uint32_t m_sendResponseCompletionPollInterval = 10;

        std::function<std::shared_ptr<processable_type>()> m_getPayloadItem;
        std::function<void(std::shared_ptr<processable_type>)> m_putPayloadItem;

        const Gem::Common::serializationMode m_serializationMode;

        // references a thread-safe indicator for shutdown that is stored in the scope calling the constructor
        std::atomic<bool> &m_isToldToStop;

        GCommandContainerT<processable_type, networked_consumer_payload_command> m_commandContainer{
                networked_consumer_payload_command::NONE
        }; ///< Holds the current command and payload (if any)
        std::string m_outgoingMessage;
    };


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class is responsible for the server side of network communication using MPI.
 * It will constantly wait for incoming work items requests, process them and answer them
 * by opening a new GMPIConsumerSessionT for each request.

 * The simplified workflow of the GMPIConsumerMasterNodeT can be described as follows:
 *
 * (1) Create thread pool using boost::thread_group and boost::io_service
 * (2) Spawn a new thread that receives request (receiverThread). Then return from the function immediately, because
 * geneva expects consumers to be background processes on other threads than the main thread.
 * (3) Once a shutdown is supposed to occur:
 *  (3.1) Set a member flag to tell the receiver thread to stop
 *  (3.2) Stop the boost::io_service object
 *  (3.3) Join all threads. The receiver thread should exit itself and all handler threads will finish their last task
 *        and then finish.
 *
 * Then the main loop in the receiver thread works as follows:
 * (2) Asynchronously receive message from any worker node if not yet told to stop (check thread safe member variable)
 * (2) Once a request has been received: schedule a handler on the thread pool and wait for another message
 *     to receive i.e. go back to (1)
 *
 * Then the handler thread works as follows (implemented in the class GMPIConsumerSessionT):
 *  (3.1) Deserialize received object
 *  (3.2) If the message from the worker includes a processed item, put it into the processed items queue of the broker
 *  (3.3) Take an item from the non-processed items queue (if there currently is one available)
 *  (3.4) Serialize the response container, which contains a new work item or the NODATA command.
 *  (3.5) Asynchronously send the item to the worker node which has requested it
 *  (3.6) Exit the handler. This lets boost use this thread for further jobs.
 *
 * Contrary to the GMPIConsumerWorkerNodeT the GMPIConsumerMasterNodeT must only work with asynchronous IO-operations and
 * run all operations on other threads than the main thread. The reason for this is that Geneva expects the
 * async_startProcessing method to return immediately and have the actual operations run on a different thread. Furthermore,
 * we want to have the ability to shut the server down, which is also only possible if the actual action is running on another
 * thread. Moreover, if a worker node has an issue we cannot block forever on IO-operations with that worker because the
 * entire system, whose center the GMPIConsumerMasterNodeT is, shall not be effected if single worker nodes encounter issues.
 *
 *
 */
    template<typename processable_type>
    class GMPIConsumerMasterNodeT
            : public std::enable_shared_from_this<GMPIConsumerMasterNodeT<processable_type>> {

    public:
        /**
         * Constructor to instantiate the GMPIConsumerMasterNodeT
         *
         * @param serializationMode mode of serialization to use for messages transferred between master node and worker nodes.
         * @param worldSize the number of nodes in the MPI cluster which is equal to the number of worker nodes + 1.
         * @param nIOThreads the number of threads in the thread pool which handles client connections.
         */
        explicit GMPIConsumerMasterNodeT(
                Gem::Common::serializationMode serializationMode,
                boost::int32_t worldSize,
                boost::uint32_t nIOThreads)
                : m_serializationMode{serializationMode},
                  m_worldSize{worldSize},
                  m_nIOThreads{nIOThreads},
                  m_isToldToStop{false} {
            std::cout << "GMPIConsumerMasterNodeT started with n=" << m_nIOThreads << " IO-threads" << std::endl;
        }

        //-------------------------------------------------------------------------
        // Deleted copy-/move-constructors and assignment operators.
        GMPIConsumerMasterNodeT(const GMPIConsumerMasterNodeT<processable_type> &) = delete;

        GMPIConsumerMasterNodeT(GMPIConsumerMasterNodeT<processable_type> &&) = delete;

        GMPIConsumerMasterNodeT &operator=(const GMPIConsumerMasterNodeT<processable_type> &) = delete;

        GMPIConsumerMasterNodeT &operator=(GMPIConsumerMasterNodeT<processable_type> &&) = delete;

        void async_startProcessing() {
            createAndStartThreadPool();

            auto self = this->shared_from_this();
            m_receiverThread = std::thread(
                    [self] { self->listenForRequests(); }
            );
        }

        void shutdown() {
            // TODO: complete the implementation of shutdown

            // notify other threads to stop
            m_isToldToStop.store(true);

            // do not allow scheduling new tasks
            m_ioService.stop();

            // stop the receiver thread
            m_receiverThread.join();

            // let all threads finish their work and join them
            m_handlerThreadGroup.join();
        }

    private:

        void createAndStartThreadPool() {
            // start the m_ioService processing loop
            m_workPtr = std::make_shared<boost::asio::io_service::work>(m_ioService);

            for (boost::uint32_t i{0}; i < m_nIOThreads; ++i) {
                m_handlerThreadGroup.create_thread(
                        [ObjectPtr = &m_ioService] { ObjectPtr->run(); }
                );
            }
        }

        void listenForRequests() {
            // register asynchronous receiving of message from any worker node

            while (!m_isToldToStop) {
                MPI_Request requestHandle;
                // create a buffer for each request.
                // once the session finished handling the request, it will release the handle and the memory will be freed
                auto buffer = std::shared_ptr<char[]>(new char[m_maxIncomingMessageSize]);
                MPI_Irecv(
                        buffer.get(),
                        m_maxIncomingMessageSize,
                        MPI_CHAR,
                        MPI_ANY_SOURCE,
                        MPI_ANY_TAG,
                        MPI_COMM_WORLD,
                        &requestHandle
                );

                while (!m_isToldToStop) {
                    int isCompleted;
                    MPI_Status status;

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
                    // if receive not completed yet, sleep a short amount of time until polling again for the message status
                    usleep(m_testForNewConnectionRequestPollInterval);
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
                        << GLOGGING;

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
                    m_isToldToStop
            }.run();

            // at time of leaving this method the request has been handled and the job for this thread is finished.
            // This thread will then be able to be scheduled for further requests by the IO-thread again
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

        //-------------------------------------------------------------------------
        // Data

        Gem::Common::serializationMode m_serializationMode;
        boost::int32_t m_worldSize;
        boost::uint32_t m_nIOThreads;
        const boost::uint32_t m_maxIncomingMessageSize = 1024 * 4;
        const boost::uint32_t m_testForNewConnectionRequestPollInterval = 100;

        boost::asio::detail::thread_group m_handlerThreadGroup;
        std::thread m_receiverThread;
        boost::asio::io_service m_ioService;
        // We do not want to call the constructor of boost::asio::io_service::work at the time of constructing
        // the master node. Therefore, we need a pointer to it which can be set to the location of a boost::asio::io_service::work
        // object at a later point in time.
        std::shared_ptr<boost::asio::io_service::work> m_workPtr;
        std::atomic<bool> m_isToldToStop;

        std::shared_ptr<typename Gem::Courtier::GBrokerT<processable_type>> m_brokerPtr = GBROKER(
                processable_type); ///< Simplified access to the broker
        const std::chrono::duration<double> m_timeout = std::chrono::milliseconds(
                GMPICONSUMERBROKERACCESSBROKERTIMEOUT); ///< A timeout for put- and get-operations via the broker
    };


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class is a wrapper around:
 * - GMPIConsumerMasterNodeT
 * - GMPIConsumerWorkerNodeT
 * - MPI initialization and finalization
 *
 * This class is responsible for checking whether the current process is the master node (rank 0) or a worker node (any other rank).
 * It is derived from both base classes - GBaseClient and GBaseConsumer and can therefore be used as either of these.
 * It instantiates the correct classes (GMPIConsumerMasterNode or GMPIConsumerWorkerNode) and forwards calls to member
 * functions to one of the two classes. This serves as an abstraction of MPI such that the user does not need to explicitly
 * ask for the process's rank.
 */
    template<typename processable_type>
    class GMPIConsumerT final
            : public Gem::Courtier::GBaseConsumerT<processable_type>,
              public Gem::Courtier::GBaseClientT<processable_type>,
              public std::enable_shared_from_this<GMPIConsumerT<processable_type>> {
        // worker nodes need to access private methods that are inherited from GBaseClientT
        friend GMPIConsumerWorkerNodeT<processable_type>;
    public:

        //-------------------------------------------------------------------------
         /**
          *
          * Constructor for a GMPIConsumerT
          *
          * @param serializationMode serialization mode to use for messages send between the master node and worker nodes
          * @param nMasterNodeIOThreads the number of threads in the thread pool of the master node to handle connections with worker nodes
          * @param argc argument count passed to main function, which will be forwarded to the MPI_Init call - optional
          * @param argv argument vector passed to main funciton, which will be forwarded to MPI_Init call - opitonal
          */
        explicit GMPIConsumerT(
                Gem::Common::serializationMode serializationMode = Gem::Common::serializationMode::BINARY,
                boost::uint32_t nMasterNodeIOThreads = 0,
                int *argc = nullptr,
                char ***argv = nullptr)
                : m_serializationMode{serializationMode},
                  m_worldRank{},
                  m_worldSize{},
                  m_nMasterNodeIOThreads{nMasterNodeIOThreads} {

            // 0 indicates that the number of io threads shall be set to the recommended number of threads of for this system
            if (m_nMasterNodeIOThreads == 0) {
                m_nMasterNodeIOThreads = masterNodeIOThreadsRecommendation();
            }

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
            MPI_Comm_size(MPI_COMM_WORLD, &m_worldSize);
            MPI_Comm_rank(MPI_COMM_WORLD, &m_worldRank);

            // instantiate the correct class according to the position in the cluster
            if (isMasterNode()) {
                m_masterNodePtr = std::make_shared<GMPIConsumerMasterNodeT<processable_type>>(
                        m_serializationMode,
                        m_worldSize,
                        m_nMasterNodeIOThreads);
            } else {
                m_workerNodePtr = std::make_shared<GMPIConsumerWorkerNodeT<processable_type>>(
                        m_serializationMode,
                        m_worldSize,
                        m_worldRank,
                        *this);
            }
        }

        //-------------------------------------------------------------------------
        /**
         * The destructor
         *
         * This constructor must call the MPI_Finalize function as this function encapsulates all
         * MPI-specific action.
         *
         */
        ~GMPIConsumerT() override {
            MPI_Finalize();
        }

        //-------------------------------------------------------------------------
        // Deleted functions

        // Deleted default-constructor -- enforce usage of a particular constructor
        GMPIConsumerT() = delete;

        // Deleted copy-constructors and assignment operators -- the client is non-copyable
        GMPIConsumerT(const GMPIConsumerT<processable_type> &) = delete;

        GMPIConsumerT(GMPIConsumerT<processable_type> &&) = delete;

        GMPIConsumerT<processable_type> &operator=(const GMPIConsumerT<processable_type> &) = delete;

        GMPIConsumerT<processable_type> &operator=(GMPIConsumerT<processable_type> &&) = delete;

        //-------------------------------------------------------------------------

        [[nodiscard]] inline bool isMasterNode() const {
            return m_worldRank == RANK_MASTER_NODE;
        }

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
        // override inherited private methods

        //-------------------------------------------------------------------------
        /**
         * Adds local command line options to a boost::program_options::options_description object.
         *
         * Inherited from GBaseConsumerT
         *
         * @param visible Command line options that should always be visible
         * @param hidden Command line options that should only be visible upon request
         */
        void addCLOptions_(
                boost::program_options::options_description &visible,
                boost::program_options::options_description &hidden
        ) override {
            namespace po = boost::program_options;

            hidden.add_options()
                    ("mpi_serializationMode",
                     po::value<Gem::Common::serializationMode>(&m_serializationMode)->default_value(
                             GCONSUMERSERIALIZATIONMODE),
                     "\t[MPI] Specifies whether serialization shall be done in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)");
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
         * Inherited from GBaseClient
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

            m_workerNodePtr->run();
        }

        //-------------------------------------------------------------------------
        // non-inherited private methods

        //-------------------------------------------------------------------------
        /**
         * Allows to retrieve the number of processing threads to be used for processing incoming connections.
         */
        [[nodiscard]] uint32_t getMasterNodeIOThreads() const {
            return m_nMasterNodeIOThreads;
        }

        [[nodiscard]] uint32_t masterNodeIOThreadsRecommendation() const {
            // TODO: calculate a more sophisticated recommendation that is dependent on the systems number of CPU-cores
            return 4;
        }

        //-------------------------------------------------------------------------
        // Data

        std::shared_ptr<GMPIConsumerMasterNodeT<processable_type>> m_masterNodePtr;
        std::shared_ptr<GMPIConsumerWorkerNodeT<processable_type>> m_workerNodePtr;
        Gem::Common::serializationMode m_serializationMode;
        boost::int32_t m_worldSize;
        boost::int32_t m_worldRank;
        boost::uint32_t m_nMasterNodeIOThreads;
    };


} /* namespace Gem::courtier */

