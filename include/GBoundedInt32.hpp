/**
 * @file GBoundedInt32.hpp
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

#ifndef GBOUNDEDINT32_HPP_
#define GBOUNDEDINT32_HPP_

// GenEvA headers go here
#include "GBoundedNumT.hpp"

namespace Gem
{
namespace GenEvA
{

  /******************************************************************************/
/**
   * This class has now become a simple typedef of the GBoundedNumT<T> class.
   * See there for further explanations.
   */

typedef GBoundedNumT<boost::int32_t> GBoundedInt32;

  /******************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GBOUNDEDINT32_HPP_ */
