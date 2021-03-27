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
#include <iostream>
#include <type_traits>

// Boost header files go here

// Geneva headers go here
#include "common/GGlobalStoreT.hpp"
#include "common/GLogger.hpp"
#include "courtier/GExecutorT.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/G_OptimizationAlgorithm_Base.hpp"
#include "geneva/G_OptimizationAlgorithm_FactoryStore.hpp"
#include "geneva/G_OptimizationAlgorithm_FactoryT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This base class takes care of adding optimization algorithm factories to
 * the global algorithm store
 */
template <typename oaf_type>
class G_OptimizationAlgorithm_InitializerT {
	// Make sure oaf_type has the expected type
	static_assert(
		std::is_base_of<G_OptimizationAlgorithm_FactoryT<G_OptimizationAlgorithm_Base> , oaf_type>::value
		, "G_OptimizationAlgorithm_FactoryT<G_OptimizationAlgorithm_Base is no base of oaf_type"
	);

public:
	/** @brief The initializing constructor */
	G_OptimizationAlgorithm_InitializerT() {
		// Create a smart pointer holding the algorithm
		std::shared_ptr<G_OptimizationAlgorithm_FactoryT<G_OptimizationAlgorithm_Base>> p(new oaf_type());
		std::string mnemonic = p->getMnemonic();

		// Add the factory to the store, if it hasn't been stored there yet
		GOAFactoryStore->setOnce(mnemonic, p);
	}

	/** @brief Defaulted destructor */
	virtual ~G_OptimizationAlgorithm_InitializerT() BASE = default;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

