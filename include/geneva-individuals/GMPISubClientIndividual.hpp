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

// MPI header files
#include <mpi.h>

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <iostream>

// Boost header files go here

// Geneva header files go here
#include <geneva/GParameterSet.hpp>
#include <geneva/GConstrainedDoubleObject.hpp>

namespace Gem::Geneva {

/******************************************************************/
    /**
     * This individual offers to set and retrieve an MPI communicator.
     * The communicator can be used to communicate with MPI sub-clients to solve the fitnessCalculation in a
     * distributed manner. To use this individual a concrete derived class has to be created and it must be used in
     * conjunction with the GMPISubClientOptimizer.
     */
    class GMPISubClientIndividual : public GParameterSet {
        /** @brief Make the class accessible to Boost.Serialization */
        friend class boost::serialization::access;

        /**
         *
         */
        friend class GMPISubClientOptimizer;

        /**************************************************************/
        /**
         * This function triggers serialization of this class and its
         * base classes.
         */
        template<typename Archive>
        void serialize(Archive &ar, const unsigned int) {
            using boost::serialization::make_nvp;
            // Serialize the base class
            ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet);
            // Add other variables here like this:
            // ar & BOOST_SERIALIZATION_NVP(sampleVariable);
        }
        /**************************************************************/
    public:
        /** @brief The default constructor */
        GMPISubClientIndividual() = default;

        /** @brief A standard copy constructor */
        GMPISubClientIndividual(const GMPISubClientIndividual &) = default;

        static MPI_Comm getCommunicator();

    private:
        /**
         * Sets the MPI communicator that can be used by the individual to communicate with sub-clients in an MPI sub-group
         * @param communicator The communicator to set.
         */
        static void setCommunicator(const MPI_Comm &communicator);

        /**
         * Communicator that can be used by this class
         */
        static MPI_Comm m_communicator;

        // NOTE: the class remains abstract because essential methods of the base class are not implemented
    };

} /* namespace Gem::Geneva */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GMPISubClientIndividual)

