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

// Standard header files go here
#include <exception>

// Boost header files go here

// Geneva headers go here
#include "hap/GRandomFactory.hpp"
#include "courtier/GBrokerT.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/G_OptimizationAlgorithm_FactoryStore.hpp"
#include "geneva/GConsumerStore.hpp"
#include "geneva/G_OptimizationAlgorithm_InitializerT.hpp"
#include "geneva/GIndividualStandardConsumerInitializerT.hpp"


namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class performs some necessary initialization work. When
 * using the Go2-class, it will be called for the user. When using optimization
 * algorithms directly, the user needs to manually instantiate this class and
 * register any desired optimization algorithm(-factory).
 */
class GenevaInitializer {
public:
	 /** @brief The default constructor */
	 G_API_GENEVA GenevaInitializer();

	 /** @brief The destructor */
	 G_API_GENEVA ~GenevaInitializer();

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

    	 template <typename c_type>
	 void registerConsumer(int size) {
		 // This will register the consumer with the global store
	   GIndividualStandardConsumerInitializerT<c_type> GConsumerStoreRegistrant(size);
	 }

	 /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

