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

// Boost header files go here

// Geneva headers go here
#include "common/GGlobalOptionsT.hpp"
#include "common/GLogger.hpp"
#include "courtier/consumers/GBaseConsumerT.hpp"
#include "geneva/GConsumerStore.hpp"
#include "geneva/par/GParameterSet.hpp"

namespace Gem::Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This base class takes care of adding GParameterSet-based consumer objects
 * to a global store
 */
template <typename c_type> // c_type stands for consumer type
class GIndividualStandardConsumerInitializerT {
public:
    /** @brief The initializing constructor */
    GIndividualStandardConsumerInitializerT() {
        // Wrap a freshly built consumer instance in a provider and register it
        // with the store, if it hasn't happened yet.
        auto provider = std::make_shared<GConsumerProviderT>(
            std::make_shared<c_type>()
        );
        consumerStore()->setOnce(provider->getMnemonic(), provider);
    }
    /** @brief An empty destructor */
    virtual ~GIndividualStandardConsumerInitializerT() = default;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva */
