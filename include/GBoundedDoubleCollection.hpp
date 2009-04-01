/**
 * @file GBoundedDoubleCollection.hpp
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
#include <sstream>
#include <vector>
#include <algorithm>

// Boost header files go here

#include <boost/version.hpp>
#include "GGlobalDefines.hpp"
#if BOOST_VERSION < ALLOWED_BOOST_VERSION
#error "Error: Boost has incorrect version !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GBOUNDEDDOUBLECOLLECTION_HPP_
#define GBOUNDEDDOUBLECOLLECTION_HPP_

// GenEvA header files go here
#include "GBoundedDouble.hpp"
#include "GParameterTCollectionT.hpp"
#include "GHelperFunctionsT.hpp"

namespace Gem {
namespace GenEvA {

/*************************************************************************/
/**
 * A collection of GBoundedDouble objects, ready for use in a
 * GIndividual derivative.
 */
typedef GParameterTCollectionT<GBoundedDouble> GBoundedDoubleCollection;

/*************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GBOUNDEDDOUBLECOLLECTION_HPP_ */
