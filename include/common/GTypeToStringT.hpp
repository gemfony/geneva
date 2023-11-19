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
#include <cstdint>

// Boost headers go here

// Geneva headers go here

namespace Gem::Common {

/******************************************************************************/
/**
 * This class allows to specify a string for a given type. Useful e.g. for
 * debugging output.
 */
template<typename T>
struct GTypeToStringT {
	static std::string value() {
		return {"unknown"};
	}
};

/******************************************************************************/
/**
 * Specialization for T == double
 */
template<>
struct GTypeToStringT<double> {
	static std::string value() {
		return {"double"};
	}
};

/******************************************************************************/
/**
 * Specialization for T == float
 */
template<>
struct GTypeToStringT<float> {
	static std::string value() {
		return {"float"};
	}
};

/******************************************************************************/
/**
 * Specialization for T == std::int32_t
 */
template<>
struct GTypeToStringT<std::int32_t> {
	static std::string value() {
		return {"int32_t"};
	}
};

/******************************************************************************/
/**
 * Specialization for T == bool
 */
template<>
struct GTypeToStringT<bool> {
	static std::string value() {
		return {"bool"};
	}
};

/******************************************************************************/
/**
 * Specialization for T == string
 */
template<>
struct GTypeToStringT<std::string> {
	static std::string value() {
		return {"string"};
	}
};

/******************************************************************************/

} // namespace Gem::Common

