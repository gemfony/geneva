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
#include <iostream>
#include <type_traits>

// Boost header files go here

// Geneva headers go here
#include "common/GGlobalOptionsT.hpp"
#include "common/GLogger.hpp"
#include "courtier/GExecutorT.hpp"
#include "geneva/par/GParameterSet.hpp"
#include "geneva/oa/GBase.hpp"
#include "geneva/oa/GFactoryStore.hpp"
#include "geneva/oa/GOAFactoryT.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This base class takes care of adding optimization algorithm factories to
 * the global algorithm store
 */
template <typename oaf_type>
class GInitializerT {
    // Make sure oaf_type has the expected type
    static_assert(
        std::is_base_of_v<GOAFactoryT<GBase>, oaf_type>,
        "GOAFactoryT<GBase> is not a base of oaf_type"
    );

public:
    /** @brief The initializing constructor */
    GInitializerT() {
        // Create a smart pointer holding the algorithm
        std::shared_ptr<GOAFactoryT<GBase>> p(
            new oaf_type()
        );
        std::string mnemonic = p->getMnemonic();

        // Add the factory to the store, if it hasn't been stored there yet
        GOAFactoryStore->setOnce(mnemonic, p);
    }

    /** @brief Defaulted destructor */
    virtual ~GInitializerT() = default;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */

