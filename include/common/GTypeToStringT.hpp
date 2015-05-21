/**
 * @file GTypeToStringT.hpp
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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <string>

// Boost headers go here
#include <boost/cstdint.hpp>

#ifndef INCLUDE_COMMON_GTYPETOSTRINGT_HPP_
#define INCLUDE_COMMON_GTYPETOSTRINGT_HPP_

// Geneva headers go here

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * This class allows to specify a string for a given type. Useful e.g. for
 * debugging output.
 */
template<typename T>
struct GTypeToStringT {
	static std::string value() {
		return std::string("unknown");
	}
};

/******************************************************************************/
/**
 * Specialization for T == double
 */
template<>
struct GTypeToStringT<double> {
	static std::string value() {
		return std::string("double");
	}
};

/******************************************************************************/
/**
 * Specialization for T == float
 */
template<>
struct GTypeToStringT<float> {
	static std::string value() {
		return std::string("float");
	}
};

/******************************************************************************/
/**
 * Specialization for T == boost::int32_t
 */
template<>
struct GTypeToStringT<boost::int32_t> {
	static std::string value() {
		return std::string("int32_t");
	}
};

/******************************************************************************/
/**
 * Specialization for T == bool
 */
template<>
struct GTypeToStringT<bool> {
	static std::string value() {
		return std::string("bool");
	}
};

/******************************************************************************/
/**
 * Specialization for T == string
 */
template<>
struct GTypeToStringT<std::string> {
	static std::string value() {
		return std::string("string");
	}
};

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* INCLUDE_COMMON_GTYPETOSTRINGT_HPP_ */
