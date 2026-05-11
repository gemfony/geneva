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

// Standard header files go here
#include <exception>

// Boost header files go here

// Geneva headers go here
#include "courtier/GBrokerT.hpp"
#include "geneva/GConsumerStore.hpp"
#include "geneva/GIndividualStandardConsumerInitializerT.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/G_OptimizationAlgorithm_FactoryStore.hpp"
#include "geneva/G_OptimizationAlgorithm_InitializerT.hpp"
#include "hap/GRandomFactory.hpp"

namespace Gem::Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class performs some necessary initialization work. When
 * using the Go2-class, it will be called for the user. When using optimization
 * algorithms directly, the user needs to manually instantiate this class and
 * register any desired optimization algorithm(-factory).
 */
class GenevaInitializer { // NOLINT(cppcoreguidelines-special-member-functions)
public:
    /** @brief The default constructor */
    GenevaInitializer();

    /** @brief The destructor */
    ~GenevaInitializer();

    /***************************************************************************/
    /**
	  * Allows to register optimization algorithm factories
	  */
    template <typename oaf_type>
    void registerOAF() {
        // This will register the factory in the global factory store
        G_OptimizationAlgorithm_InitializerT<oaf_type> GOAFStoreRegistrant;
    }

    /***************************************************************************/
    /**
	  * Allows to register consumers
	  */
    template <typename c_type>
    void registerConsumer() {
        // This will register the consumer with the global store
        GIndividualStandardConsumerInitializerT<c_type> GConsumerStoreRegistrant;
    }

    /**
      * Allows to register an existing consumer. This is important when working with singleton consumers such as the
      * GMPIConsumerT.
      */
    void registerConsumer(
        const std::shared_ptr<Gem::Courtier::GBaseConsumerT<Gem::Geneva::GParameterSet>> &consumer
    ) {
        std::string mnemonic = consumer->getMnemonic(); // NOLINT(cppcoreguidelines-init-variables)
        GConsumerStore->setOnce(mnemonic, consumer);
    }

    /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva */
