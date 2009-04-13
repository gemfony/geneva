/**
 * @file GBoundedDouble.hpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
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
#include <vector>
#include <sstream>
#include <cmath>

// Boost headers go here
#include <boost/version.hpp>
#include "GGlobalDefines.hpp"
#if BOOST_VERSION < ALLOWED_BOOST_VERSION
#error "Error: Boost has incorrect version !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/cast.hpp> // For boost::numeric_cast<>

#ifndef GBOUNDEDDOUBLE_HPP_
#define GBOUNDEDDOUBLE_HPP_

// GenEvA headers go here
#include "GBoundedNumT.hpp"

namespace Gem
{
namespace GenEvA
{

  /******************************************************************************/
  /* The GBoundedDouble class represents a double value, equipped with the
   * ability to mutate itself. The value range can have an upper and a lower
   * limit. The range has to be set during the initial creation. Once set, it cannot
   * be changed anymore. Mutated values will only appear inside the given range.
   * Note that appropriate adaptors (see e.g the GDoubleGaussAdaptor class) need
   * to be loaded in order to benefit from the mutation capabilities. Note that this
   * class intentionally does not provide an ability to reset the upper and lower
   * boundaries, due to the intended usage of this class in the GBoundedDoubleWithGaps
   * class. An ability to reset the boundaries would make implementation really
   * difficult. Just recreate a new GBoundedDouble object with different values
   * instead.
   *
   * This class has now become a simple typedef of the GBoundedNumT<T> class.
   */

typedef GBoundedNumT<double> GBoundedDouble;

  /******************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GBOUNDEDDOUBLE_HPP_ */
