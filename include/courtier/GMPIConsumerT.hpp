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
#include <boost/enable_shared_from_this.hpp>
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
 * down a worker. However so far we
 *
 * The simplified workflow of the GMPIConsumerWorkerNodeT can be described as follows:
 *
 * (1) send an synchronous GET request (ask for the first work item)
 * (2) synchronous receive a message.
 * (3) deserialize the received work item
 * (4) process the received work item
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
            std::cout << "Worker Node with rank " << m_worldRank << " called run()" << std::endl;

            /*
             * Simplified workflow of GAsioConsumerClient:
             *
             * 1) set m_outgoing_message_str to an empty GET request to request work items
             * 2) check if halt criterion is already reached, if so shutdown
             * 3) resolve IP hostname
             * 4) connect to socket of GAsioConsumer (server)
             * 5.1) if connection failed
             *      + try to reconnect until reconnection limit reached
             * 5.2) if successfully connected
             *      + send GET request to GAsioConsumer (server)
             * 6) when writing finishes:
             *      + read server response
             * 7) when data received form server, check its variant:
             *      + case NODATA: poll again after a few milliseconds
             *      + case COMPUTE: process item and send a result back to server
             *      + case default: shutdown and throw exception
             * 8) start with 2) again which means issue a new request if the halt criterion is not reached yet
             */
        }

    private:
        void shutdown() {
            // TODO: perform any cleanup work here (probably MPI_Cancel and a following MPI_Wait)
        }

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
 * The simplified workflow of the GMPIConsumerMasterNodeT can be described as follows:
 *
 * (1) Create thread pool using boost::thread_group and boost::io_service
 * (2) Dispatch a main loop that receives requests to the own thread pool (receiver thread). This allow to let the main
 *     thread return the function immediately, which is expected by Geneva when calling async_startProcessing.
 * (3) Once a shutdown is supposed to occur:
 *  (3.1) Set a member flag to tell the receiver thread to stop
 *  (3.2) Stop the boost::io_service object
 *  (3.3) Join all threads. The receiver thread should exit itself and all handler threads will finish their last task
 *        and then finish.
 *
 *  Then the main loop in the receiver thread works as follows:
 * (1) Asynchronously receive message from any worker node if not yet told to stop (check member variable)
 * (2) Dispatch a handler to the thread pool and wait for another message to receive i.e. back to (2)
 *
 * Then the handler thread works as follows:
 *  (3.1) Deserialize received object
 *  (3.2) If the message from the worker includes a processed item, put it into the processed items queue of the broker
 *  (3.3) Take an item from the non-processed items queue, if there is no current item then poll until one is available.
 *  (3.4) Serialize the new work item.
 *  (3.5) Asynchronously send the item to the worker node which has requested it
 *  (3.6) Exit the handler. This lets boost allocate this thread for future jobs.
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
                boost::int32_t worldSize)
                : m_serializationMode{serializationMode},
                  m_worldSize{worldSize} { /* nothing */ }

        //-------------------------------------------------------------------------
        // Deleted copy-/move-constructors and assignment operators.
        GMPIConsumerMasterNodeT(const GMPIConsumerMasterNodeT<processable_type> &) = delete;

        GMPIConsumerMasterNodeT(GMPIConsumerMasterNodeT<processable_type> &&) = delete;

        GMPIConsumerMasterNodeT &operator=(const GMPIConsumerMasterNodeT<processable_type> &) = delete;

        GMPIConsumerMasterNodeT &operator=(GMPIConsumerMasterNodeT<processable_type> &&) = delete;

        void async_startProcessing() const {
            // TODO: implement this
            std::cout << "Master node (rank 0) called async_startProcessing" << std::endl;

            /*
             * Simplified workflow of GAsioConsumerT:
             *
             * 1) open server endpoint
             * 2) connect to server address (to own address)
             * 3) start listening for connections
             * 4) register async callback for when connection was accepted
             * 5) start multiple threads to handle async callbacks on the io_context
             *
             * ((4)) when a connection is accepted this callback will be executed)
             * 4.1) create a GAsioConsumerSessionT and call the async_start_run() method
             * 4.2) go back to 4) (register another connection callback to wait for new connecitons)
             *
             * ((4.1)) The async_start_run() method registers a callback that will
             * be executed as soon as data can be read from the connection
             * 4.1.1) when there is incoming data to read on the connection:
             *      + read bytes from socket (receive data from client)
             * 4.1.2) when all data has been read:
             *      + deserialize received object
             *          * CASE GETDATA:
             *              - getAndSerializeWorkItem()
             *          * CASE RESULT:
             *              - m_put_payload_item();
             *              - getAndSerializeWorkItem()
             * 4.1.3) send serialized workItem to client
             * 4.1.4) when work item was transmitted:
             *      + close socket ins send direction, which indicates end of write at client side
             *      + clear the outgoing message string
             *
             */
        }

    private:

        Gem::Common::serializationMode m_serializationMode;
        boost::int32_t m_worldSize;
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
                               int *argc = nullptr,
                               char ***argv = nullptr)
                : m_serializationMode{serializationMode},
                  m_worldRank{},
                  m_worldSize{} {

            // initialize MPI
            MPI_Init(argc, argv);

            // set members regarding the position of the process in the MPI cluster
            MPI_Comm_size(MPI_COMM_WORLD, &m_worldSize);
            MPI_Comm_rank(MPI_COMM_WORLD, &m_worldRank);

            // instantiate the correct class according to the position in the cluster
            if (isMasterNode()) {
                m_masterNode = std::make_shared<GMPIConsumerMasterNodeT<processable_type>>(
                        serializationMode,
                        m_worldSize);
            } else {
                m_workerNode = std::make_shared<GMPIConsumerWorkerNodeT<processable_type>>(
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
         * Inherited from GBaseConsumerT
         */
        void shutdown_() override {
            GBaseConsumerT<processable_type>::shutdown_();
            // TODO: perform any work that needs to be done to shutdown consumer
        }

    private:
        //-------------------------------------------------------------------------
        // Private methods

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

            m_masterNode->async_startProcessing();
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
            return true; // assume that MPI cluster will always return responses
            // TODO: Is it correct to make that assumption?
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

            m_workerNode->run();
        }

        //-------------------------------------------------------------------------
        // Data

        std::shared_ptr<GMPIConsumerMasterNodeT<processable_type>> m_masterNode;
        std::shared_ptr<GMPIConsumerWorkerNodeT<processable_type>> m_workerNode;
        Gem::Common::serializationMode m_serializationMode;
        boost::int32_t m_worldSize;
        boost::int32_t m_worldRank;
    };


} /* namespace Gem::courtier */

