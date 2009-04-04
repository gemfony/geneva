/**
 * @file GDouble.hpp
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

// Standard headers go here

// Boost headers go here

#include <boost/version.hpp>
#include "GGlobalDefines.hpp"
#if BOOST_VERSION < ALLOWED_BOOST_VERSION
#error "Error: Boost has incorrect version !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>

#ifndef GDOUBLE_HPP_
#define GDOUBLE_HPP_

// GenEvA headers go here

#include "GParameterT.hpp"

namespace Gem {
namespace GenEvA {

/************************************************************************/
/**
 * This class encapsulates a single doublel. This might appear heavy weight,
 * and indeed for most applications this is not the recommended solution -
 * use the GDoubleCollection or GBoundedDoubleCollection instead.
 *
 * Bits are mutated by the GDoubleGaussAdaptor in GenEvA. If you want to
 * have individual adaptors (with individual settings) for each double, you need
 * to use this class instead.
 *
 * For reasons of simplicity this class is implemented as a simple typdef of the GParameterT
 * template class.
 */
typedef GParameterT<double> GDouble;

/************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GDOUBLE_HPP_ */
