/**
 * @file
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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <vector>
#include <sstream>
#include <type_traits>

// Boost headers go here

// Our own headers go here
#include "common/GExceptions.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GAdaptorT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This factory function returns default adaptors for a given base type. This function is a trap.
 * Specializations are responsible for the actual implementation.
 *
 * @return The default adaptor for a given base type
 */
template <typename T>
std::shared_ptr<GAdaptorT<T>> getDefaultAdaptor() {
	throw gemfony_exception(
		g_error_streamer(DO_LOG, time_and_place)
			<< "In getDefaultAdaptor():" << std::endl
			<< "Function called with invalid type." << std::endl
	);

	// Make the compiler happy
	return std::shared_ptr<GAdaptorT<T>>();
}

// Specializations for double, std::int32_t and bool
/******************************************************************************/
template <> G_API_GENEVA std::shared_ptr<GAdaptorT<double>> getDefaultAdaptor<double>();
template <> G_API_GENEVA std::shared_ptr<GAdaptorT<std::int32_t>> getDefaultAdaptor<std::int32_t>();
template <> G_API_GENEVA std::shared_ptr<GAdaptorT<bool>> getDefaultAdaptor<bool>();

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

