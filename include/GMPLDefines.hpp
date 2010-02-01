/**
 * @file GMPLDefines.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

// Standard header files go here
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost header files go here
#include <boost/mpl/has_xxx.hpp>

#ifndef GMPLDEFINES_HPP_
#define GMPLDEFINES_HPP_

/*************************************************************************************************/
/**
 * This file defines checks whether we are dealing with objects which have given typedefs.
 * This can be used to check whether template parameters fulfill certain criteria. See
 * "Beyond the C++ Standard Library" by Bjoern Karlsson, p. 99 for an explanation
 */
BOOST_MPL_HAS_XXX_TRAIT_DEF(checkRelationshipWithFunction)
BOOST_MPL_HAS_XXX_TRAIT_DEF(checkRelationshipWithVectorFunction)

#endif /* GMPLDEFINES_HPP_ */
