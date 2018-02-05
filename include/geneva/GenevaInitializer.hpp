/**
 * @file Geneva.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

#ifndef INCLUDE_GENEVA_GENEVA_HPP_
#define INCLUDE_GENEVA_GENEVA_HPP_

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

	 /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* INCLUDE_GENEVA_GENEVA_HPP_ */
