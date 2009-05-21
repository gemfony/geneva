/**
 * @file GDoubleObjectCollection.hpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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
 */

// Standard header files go here

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost header files go here

#ifndef GDOUBLEOBJECTCOLLECTION_HPP_
#define GDOUBLEOBJECTCOLLECTION_HPP_

// GenEvA header files go here
#include "GDouble.hpp"
#include "GParameterTCollectionT.hpp"

namespace Gem {
namespace GenEvA {

/*************************************************************************/
/**
 * A collection of GDouble objects, ready for use in a GIndividual derivative.
 */
typedef GParameterTCollectionT<GDouble> GDoubleObjectCollection;

/*************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GDOUBLEOBJECTCOLLECTION_HPP_ */
