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

// Standard headers go here
#include <string>
#include <ostream>
#include <istream>
#include <limits>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/limits.hpp>
#ifndef GCONSTRAINEDVALUELIMITT_HPP_
#define GCONSTRAINEDVALUELIMITT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "GOptimizationEnums.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This struct defines limits used for constrained parameter types in the
 * optimization process.
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
		return MAXCONSTRAINEDDOUBLE;
	}

   static double lowest() {
      return -MAXCONSTRAINEDDOUBLE;
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
		return MAXCONSTRAINEDFLOAT;
	}

   static float lowest() {
      return -MAXCONSTRAINEDFLOAT;
   }
};

/******************************************************************************/
/**
 * Specialization of GConstrainedValueLimitT for boost::int32_t values.
 */
template <>
struct GConstrainedValueLimitT<boost::int32_t>
{
	static boost::int32_t highest() {
		return MAXCONSTRAINEDINT32;
	}

   static boost::int32_t lowest() {
      return -MAXCONSTRAINEDINT32;
   }
};

/******************************************************************************/

} /* Geneva */
} /* Gem */

#endif /* GCONSTRAINEDVALUELIMITT_HPP_ */
