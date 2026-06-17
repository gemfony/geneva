/**
 * @file GMPISubClientIndividual.hpp
 */

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

// MPI header files
#include <mpi.h>

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <iostream>

// Boost header files go here

// Geneva header files go here
#include <geneva/ind/GFlatGenome.hpp>

namespace Gem::Geneva {

enum ClientStatus {
    RUNNING,
    FINISHED,
    ERROR
};

enum ClientMode {
    CLIENT,
    SUB_CLIENT
};

/******************************************************************/
/**
     * This individual offers to set and retrieve an MPI communicator.
     * The communicator can be used to communicate with MPI sub-clients to solve the fitnessCalculation in a
     * distributed manner. To use this individual a concrete derived class has to be created and it must be used in
     * conjunction with the GMPISubClientOptimizer.
     */
class GMPISubClientIndividual // NOLINT(cppcoreguidelines-special-member-functions)
  : public gen::GFlatGenome {
    /** @brief Make the class accessible to Boost.Serialization */
    friend class boost::serialization::access;

    /**
         * GMPISubClientOptimizer must be able to set the communicator, other classes should not.
         * Therefore we should get access to private members from GMPISubClientOptimizer
         */
    friend class GMPISubClientOptimizer;

    /**************************************************************/
    /**
         * This function triggers serialization of this class and its
         * base classes.
         */
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        // Serialize the base class
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(gen::GFlatGenome);
        // Add other variables here like this:
        // ar & BOOST_SERIALIZATION_NVP(sampleVariable);
    }
    /**************************************************************/
public:
    /** @brief The default constructor */
    GMPISubClientIndividual() = default;

    /** @brief A standard copy constructor */
    GMPISubClientIndividual(const GMPISubClientIndividual &) = default;

    /**
         * @brief Retrieves the MPI communicator used by this individual to talk to its dedicated sub-client workers.
         *
         * @return The static communicator shared by all instances (MPI_COMM_NULL if none has been set yet).
         */
    static MPI_Comm getCommunicator();

protected:
    /**
         * @brief Retrieves the status of the associated client in the communication group.
         *
         * @return The client status (RUNNING, FINISHED or ERROR), determined from the pending status request.
         */
    static ClientStatus getClientStatus();

    /**
         * @brief Retrieves the role of the current process within the MPI sub-group.
         *
         * @return The client mode of this process, either CLIENT or SUB_CLIENT.
         */
    static ClientMode getClientMode();

private:
    /**
         * @brief Sets the MPI communicator the individual uses to communicate with sub-clients in an MPI sub-group.
         *
         * @param communicator The communicator to store for shared, static use by all instances.
         */
    static void setCommunicator(const MPI_Comm &communicator);

    /**
         * @brief Stores a request that can be used to check the status of the client in the current communication group.
         *
         * @param request The MPI request handle whose completion signals a change in the client status.
         */
    static void setClientStatusRequest(const MPI_Request &request);

    /**
         * @brief Sets the mode of this process to client or sub-client, so individuals can query this property.
         *
         * @param mode The role to record for the current process, either CLIENT or SUB_CLIENT.
         */
    static void setClientMode(const ClientMode &mode);

    /**
         * Communicator that can be used by this class
         */
    static inline MPI_Comm communicator_{MPI_COMM_NULL};

    /**
         * Request which can be used to check the client status
         */
    inline static MPI_Request clientStatusRequest_{MPI_REQUEST_NULL};

    inline static ClientMode clientMode_{ClientMode::CLIENT};

    // NOTE: the class remains abstract because essential methods of the base class are not implemented
};

} /* namespace Gem::Geneva */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GMPISubClientIndividual) // NOLINT
