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

// Standard headers go here
#include <sstream>
#include <type_traits>
#include <vector>

// Boost headers go here

// Our own headers go here
#include "common/GExceptions.hpp"
#include "geneva/par/GAdaptorT.hpp"
#include "geneva/GOptimizationEnums.hpp"

namespace Gem::Geneva {

/******************************************************************************/
/**
 * This factory function returns default adaptors for a given base type. This function is a trap.
 * Specializations are responsible for the actual implementation.
 *
 * @return The default adaptor for a given base type
 */
template <typename T>
std::shared_ptr<gpar::GAdaptorT<T>> getDefaultAdaptor() {
    throw geneva_exception(
        g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
        << "In getDefaultAdaptor():" << '\n'
        << "Function called with invalid type." << '\n'
    );

    // Make the compiler happy
    return std::shared_ptr<gpar::GAdaptorT<T>>();
}

// Specializations for double, std::int32_t and bool
/******************************************************************************/
template <>
std::shared_ptr<gpar::GAdaptorT<double>> getDefaultAdaptor<double>();
template <>
std::shared_ptr<gpar::GAdaptorT<std::int32_t>> getDefaultAdaptor<std::int32_t>();
template <>
std::shared_ptr<gpar::GAdaptorT<bool>> getDefaultAdaptor<bool>();

/******************************************************************************/

} /* namespace Gem::Geneva */
