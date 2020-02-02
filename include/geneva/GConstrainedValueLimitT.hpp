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
#include <string>
#include <ostream>
#include <istream>
#include <limits>

// Boost headers go here
#include <boost/limits.hpp>

// Geneva headers go here
#include "GOptimizationEnums.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This struct defines limits used for constrained parameter types in the
 * optimization process. It has been introduced, as we want limits smaller
 * than the allowed maximum value for floating point types.
 */
template <typename T>
struct GConstrainedValueLimitT
{
	static T highest() {
		return boost::numeric::bounds<T>::highest();
	}

	static T lowest() {
		return boost::numeric::bounds<T>::lowest();
	}
};

/******************************************************************************/
/**
 * Specialization of GConstrainedValueLimitT for double values.
 */
template <>
struct GConstrainedValueLimitT<double>
{
	static double highest() {
		return GMAXCONSTRAINEDDOUBLE;
	}

	static double lowest() {
		return -GMAXCONSTRAINEDDOUBLE;
	}
};

/******************************************************************************/
/**
 * Specialization of GConstrainedValueLimitT for float values.
 */
template <>
struct GConstrainedValueLimitT<float>
{
	static float highest() {
		return GMAXCONSTRAINEDFLOAT;
	}

	static float lowest() {
		return -GMAXCONSTRAINEDFLOAT;
	}
};

/******************************************************************************/
/**
 * Specialization of GConstrainedValueLimitT for std::int32_t values.
 */
template <>
struct GConstrainedValueLimitT<std::int32_t>
{
	static std::int32_t highest() {
		return GMAXCONSTRAINEDINT32;
	}

	static std::int32_t lowest() {
		return -GMAXCONSTRAINEDINT32;
	}
};

/******************************************************************************/
/**
 * Specialization of GConstrainedValueLimitT for bool values.
 */
template <>
struct GConstrainedValueLimitT<bool>
{
	static bool highest() {
		return true;
	}

	static bool lowest() {
		return false;
	}
};

/******************************************************************************/

} /* Geneva */
} /* Gem */

