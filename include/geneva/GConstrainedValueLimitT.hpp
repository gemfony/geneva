/**
 * @file GConstrainedValueLimitT.hpp
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
#include <ostream>
#include <istream>
#include <limits>

// Boost headers go here
#include <boost/limits.hpp>

#ifndef GCONSTRAINEDVALUELIMITT_HPP_
#define GCONSTRAINEDVALUELIMITT_HPP_

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

#endif /* GCONSTRAINEDVALUELIMITT_HPP_ */
