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
 */
    template<typename processable_type>
    class GMPIConsumerWorkerNodeT final
            : public std::enable_shared_from_this<GMPIConsumerWorkerNodeT<processable_type>> {

    public:
        explicit GMPIConsumerWorkerNodeT(Gem::Common::serializationMode serializationMode)
                : m_serializationMode{serializationMode} { /* nothing */ }

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
            std::cout << "Worker Node called run()" << std::endl;
        }

    private:
        Gem::Common::serializationMode m_serializationMode;

    };


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * TODO: documentation
 */
    template<typename processable_type>
    class GMPIConsumerMasterNodeT
            : public std::enable_shared_from_this<GMPIConsumerMasterNodeT<processable_type>> {

    public:
        //-------------------------------------------------------------------------
        // note: this was changed in order to comply with the new MPI interface
        explicit GMPIConsumerMasterNodeT(Gem::Common::serializationMode serializationMode)
                : m_serializationMode{serializationMode} {}

        //-------------------------------------------------------------------------
        // Deleted copy-/move-constructors and assignment operators.
        GMPIConsumerMasterNodeT(const GMPIConsumerMasterNodeT<processable_type> &) = delete;

        GMPIConsumerMasterNodeT(GMPIConsumerMasterNodeT<processable_type> &&) = delete;

        GMPIConsumerMasterNodeT &operator=(const GMPIConsumerMasterNodeT<processable_type> &) = delete;

        GMPIConsumerMasterNodeT &operator=(GMPIConsumerMasterNodeT<processable_type> &&) = delete;

        void async_startProcessing() const {
            // TODO: implement this
            std::cout << "Master node called async_startProcessing" << std::endl;
        }

    private:

        Gem::Common::serializationMode m_serializationMode;

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
                m_masterNode = std::make_shared<GMPIConsumerMasterNodeT<processable_type>>(serializationMode);
            } else {
                m_workerNode = std::make_shared<GMPIConsumerWorkerNodeT<processable_type>>(serializationMode);
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
        int m_worldSize;
        int m_worldRank;
    };


} /* namespace Gem::courtier */

