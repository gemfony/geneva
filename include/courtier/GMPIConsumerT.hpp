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
    const int TAG_INITIAL_WORK_ITEM_REQUEST = 42;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class is responsible for the client side of network communication through MPI
 * TODO: enhance documentation
 *
 * TODO: change this to asynchronous communication because we need to be able to shutdown and respond to crashed nodes
 *      without being stuck in a never ending blocking call.
 * TODO: finalize draft
 * draft:
 *
 * At the side of the worker nodes the communication only happens when there is no more work item to processed.
 * Therefore, we can work with synchronous send and receive operations without waisting any computation time, because the
 * worker would have to wait for new requests anyways. Furthermore, synchronous operations make the code easier to understand
 * Finally synchronous send calls allow the MPI implementation to directly send the bytes from the user defined buffer
 * through the network without any copying into a second buffer. In asynchronous mode the MPI library might copy the contents
 * of the user defined buffer to an internal second buffer and then return before the network operation has been completed.
 * This is useful in cases where the computation time and buffer shall be reused in the meanwhile. However, as we do not
 * have any useful tasks to complete in the mean time, we can also use synchronous calls which will avoid unnecessary
 * buffer copying.
 * However, one disadvantage of blocking calls is that they might never return if the other process (node 0 in this case)
 * has some issue. Therefore, the blocking call may remain in an endless waiting state. This is also an issue when shutting
 * down a worker. However, if we look at the implementation the GAsioConsumerClientT we can see that it also does not have
 * an option to shut it down remotely, because the call method run blocks until no work remains in the io_context.
 * Therefore, we can implement the GMPIConsumerWorkerNodeT in a similar fashion and only work with synchronous calls and trigger
 * a remote shutdown with a specialized enum variant SHUTDOWN.
 *
 * The simplified workflow of the GMPIConsumerWorkerNodeT can be described as follows:
 *
 * (1) send a synchronous GET request (ask for the first work item)
 * (2) synchronous receive a message.
 * (3) deserialize the received work item
 * (4.1) if the item has DATA: process the received work item
 * (4.2) if the item is a shutdown message: shutdown the worker
 * (5) synchronously send the result of the processing to the master node and simultaneously request a new work item
 * (6) go back to a synchronously receiving the next work item i.e. step (2)
 *
 */
    template<typename processable_type>
    class GMPIConsumerWorkerNodeT final
            : public std::enable_shared_from_this<GMPIConsumerWorkerNodeT<processable_type>> {

    public:
        explicit GMPIConsumerWorkerNodeT(
                Gem::Common::serializationMode serializationMode,
                boost::int32_t worldSize,
                boost::int32_t worldRank)
                : m_serializationMode{serializationMode},
                  m_worldSize{worldSize},
                  m_worldRank{worldRank} { /* nothing */ }

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
            // TODO: implement this

            if (!askForFirstWorkItem()) {
                // TODO: shutdown or try again? -> probably reconnect a number of times
            }
        }

    private:
        [[nodiscard]] bool askForFirstWorkItem() {
            // TODO: use better buffer allocation
            // TODO: add error checking
            std::string test_message{"Test initial worker request"};
            int result = MPI_Ssend(
                    test_message.data(),
                    static_cast<int>(test_message.size()),
                    MPI_CHAR,
                    0,
                    TAG_INITIAL_WORK_ITEM_REQUEST,
                    MPI_COMM_WORLD);

            if (result != MPI_SUCCESS) {
                // TODO: enhance error handling
                std::cout << "some error occurred sending the initial request" << std::endl;
                return false;
            }

            std::cout << "request successfully sent from worker " << m_worldRank << std::endl;

            return true;
        }

        void shutdown() {
            // TODO: perform any cleanup work here (probably MPI_Cancel and a following MPI_Wait)
        }

        //-------------------------------------------------------------------------
        // Data

        Gem::Common::serializationMode m_serializationMode;
        boost::int32_t m_worldSize;
        boost::int32_t m_worldRank;

    };


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * TODO: documentation
 *
 * TODO: finalize draft
 * draft:
 *
 * Contrary to the GMPIConsumerWorkerNodeT the GMPIConsumerMasterNodeT must only work with asynchronous IO-operations and
 * run all operations on other threads than the main thread. The reason for this is because Geneva expects the
 * async_startProcessing method to return immediately and have the actual operations run on a different thread. Furthermore,
 * we want to have the ability to shut the client down, which is also only possible if the actual action is running on another
 * thread. Moreover, if a worker node has an issue we cannot block forever on IO-operations with that worker because the
 * entire system, whose center the GMPIConsumerMasterNodeT is, shall not be effected if single worker nodes encounter issues.
 *
 * The simplified workflow of the GMPIConsumerMasterNodeT can be described as follows:
 *
 * (1) Create thread pool using boost::thread_group and boost::io_service
 * (2) Schedule a main loop that receives requests on the own thread pool (receiver thread). This allows to let the main
 *     thread from return the function immediately, which is expected by Geneva when calling async_startProcessing.
 * (3) Once a shutdown is supposed to occur:
 *  (3.1) Set a member flag to tell the receiver thread to stop
 *  (3.2) Stop the boost::io_service object
 *  (3.3) Join all threads. The receiver thread should exit itself and all handler threads will finish their last task
 *        and then finish.
 *
 *  Then the main loop in the receiver thread works as follows:
 * (1) Asynchronously receive message from any worker node if not yet told to stop (check member variable)
 * (2) Once a request has been received: schedule a handler on the thread pool and wait for another message
 *     to receive i.e. go back to (1)
 *
 * Then the handler thread works as follows:
 *  (3.1) Deserialize received object
 *  (3.2) If the message from the worker includes a processed item, put it into the processed items queue of the broker
 *  (3.3) Take an item from the non-processed items queue, if there is no current item then poll until one is available.
 *  (3.4) Serialize the new work item.
 *  (3.5) Asynchronously send the item to the worker node which has requested it
 *  (3.6) Exit the handler. This lets boost use this thread for future jobs.
 *
 */
    template<typename processable_type>
    class GMPIConsumerMasterNodeT
            : public std::enable_shared_from_this<GMPIConsumerMasterNodeT<processable_type>> {

    public:
        //-------------------------------------------------------------------------
        // TODO: documentation
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
            // TODO: implement this
            createAndStartThreadPool();

            auto self = this->shared_from_this();
            boost::asio::post(
                    [self] { self->listenForRequests(); });
        }

        void shutdown() {
            // TODO: complete the implementation of shutdown

            // notify other threads to stop
            m_isToldToStop.store(true);

            // do not allow scheduling new tasks
            m_ioService.stop();

            // let all threads finish their work and join them
            m_threadGroup.join();
        }

    private:

        void createAndStartThreadPool() {
            // start the m_ioService processing loop
            m_work_ptr = std::make_shared<boost::asio::io_service::work>(m_ioService);

            for (boost::uint32_t i{0}; i < m_nIOThreads; ++i) {
                m_threadGroup.create_thread(
                        [ObjectPtr = &m_ioService] { ObjectPtr->run(); }
                );
            }
        }

        void listenForRequests() {
            // register asynchronous receiving of message from any worker node
            // TODO: improve code quality and use better way of buffer allocation

            while (!m_isToldToStop) {
                MPI_Request requestHandle;
                const int LENGTH = 1024;
                char buf[LENGTH];
                MPI_Irecv(
                        buf,
                        LENGTH,
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
                        auto self = this->shared_from_this();
                        boost::asio::post(
                                [self, status, buf] { self->handleRequest(status, buf); });

                        break;
                    } else {
                        usleep(10); // sleep a short amount of time until polling again for the message status
                    }
                }
            }
        }

        void handleRequest(const MPI_Status &status, char const *const &buffer) {
            // TODO implement this. Also check for errors
            std::cout << "A request handler has been started" << std::endl;

            if (status.MPI_ERROR == MPI_SUCCESS) {
                std::cout << "Request successfully received from node " << status.MPI_SOURCE << std::endl;
                int messageLength{mpiGetCount(status)};
                std::cout << "Size of the message is: " << messageLength << std::endl;
                printf("Value of the message: %.*s\n", messageLength, buffer);
            } else {
                char errorMessage[MPI_MAX_ERROR_STRING];
                int messageLength;
                MPI_Error_string(status.MPI_ERROR, errorMessage, &messageLength);
                std::cout << "Error receiving request: " << std::endl;
                for (int i{0}; i < messageLength; ++i) {
                    std::cout << errorMessage[i];
                }
                std::cout << std::endl;
            }
        }

        //-------------------------------------------------------------------------
        // Data

        Gem::Common::serializationMode m_serializationMode;
        boost::int32_t m_worldSize;
        boost::uint32_t m_nIOThreads;
        boost::asio::detail::thread_group m_threadGroup;
        boost::asio::io_service m_ioService;
        // We do not want to call the constructor of boost::asio::io_service::work at the time of constructing
        // the master node. Therefore, we need a pointer to it which can be set to the location of a boost::asio::io_service::work
        // object at a later point in time.
        std::shared_ptr<boost::asio::io_service::work> m_work_ptr;
        std::atomic<bool> m_isToldToStop;
    };


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class is responsible for checking whether the current process is the master node (rank 0) or a worker node (any other rank).
 * It is derived from both base classes - GBaseClient and GBaseConsumer and can therefore be used as either of these.
 * It instantiates the correct classes (GMPIConsumerMasterNode or GMPIConsumerWorkerNode) and forwards calls to member
 * functions to one of the two classes. This serves as an abstraction of MPI such that the user does not need to explicitly
 * ask for the process's rank.
 *
 */
    template<typename processable_type>
    class GMPIConsumerT final
            : public Gem::Courtier::GBaseConsumerT<processable_type>,
              public Gem::Courtier::GBaseClientT<processable_type>,
              public std::enable_shared_from_this<GMPIConsumerT<processable_type>> {
    public:

        //-------------------------------------------------------------------------
        /**
         * Initialization of a consumer
         *
         * @param serializationMode the method of serialization used by the consumer
         */
        explicit GMPIConsumerT(Gem::Common::serializationMode serializationMode,
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
            MPI_Init(argc, argv);

            // set members regarding the position of the process in the MPI cluster
            MPI_Comm_size(MPI_COMM_WORLD, &m_worldSize);
            MPI_Comm_rank(MPI_COMM_WORLD, &m_worldRank);

            // instantiate the correct class according to the position in the cluster
            if (isMasterNode()) {
                m_masterNode_ptr = std::make_shared<GMPIConsumerMasterNodeT<processable_type>>(
                        serializationMode,
                        m_worldSize,
                        m_nMasterNodeIOThreads);
            } else {
                m_workerNode_ptr = std::make_shared<GMPIConsumerWorkerNodeT<processable_type>>(
                        serializationMode,
                        m_worldSize,
                        m_worldRank);
            }

            std::cout << "MPI node with rank " << m_worldRank << " started up." << std::endl;
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
            return m_worldRank == 0;
        }

        [[nodiscard]] inline bool isWorkerNode() const {
            return m_worldRank != 0;
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
            m_masterNode_ptr->shutdown();
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
                    ("asio_serializationMode",
                     po::value<Gem::Common::serializationMode>(&m_serializationMode)->default_value(
                             GCONSUMERSERIALIZATIONMODE),
                     "\t[asio] Specifies whether serialization shall be done in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)");
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

            m_masterNode_ptr->async_startProcessing();
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

            m_workerNode_ptr->run();
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

        std::shared_ptr<GMPIConsumerMasterNodeT<processable_type>> m_masterNode_ptr;
        std::shared_ptr<GMPIConsumerWorkerNodeT<processable_type>> m_workerNode_ptr;
        Gem::Common::serializationMode m_serializationMode;
        boost::int32_t m_worldSize;
        boost::int32_t m_worldRank;
        boost::uint32_t m_nMasterNodeIOThreads;
    };


} /* namespace Gem::courtier */

