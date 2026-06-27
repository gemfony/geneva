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
#include "geneva/GOptimizableEntityCommandContainerExport.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/oa/GFactoryStore.hpp"
#include "geneva/oa/GInitializerT.hpp"
#include "hap/GRandomFactory.hpp"

namespace Gem::Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class performs some necessary initialization work. When
 * using the Go2-class, it will be called for the user. When using optimization
 * algorithms directly, the user needs to manually instantiate this class.
 */
class GenevaInitializer { // NOLINT(cppcoreguidelines-special-member-functions)
public:
    /**
     * @brief The default constructor; performs the runtime init of the random factory.
     *
     * Brings the global random-number factory online so that all Geneva facilities have a usable RNG
     * source for the lifetime of this object.
     */
    GenevaInitializer();

    /**
     * @brief The destructor; performs the runtime finalize of the random factory.
     *
     * Tears down the global random-number factory that the constructor brought online.
     */
    ~GenevaInitializer();

    /***************************************************************************/
    // Note: optimization-algorithm factories register themselves with the global factory store at
    // library-load time (see the self-registration helpers in each factory's .cpp). Consumers are no
    // longer registered in a store -- they are built on demand by the courtier setup layer.
    // GenevaInitializer therefore exposes no registration API; it only performs the runtime
    // init / finalize of the random factory (see ctor / dtor).

    /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva */
