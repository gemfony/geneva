/**
 * @file GInt32FlipAdaptor.hpp
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

// Standard header files go here
#include <string>
#include <sstream>
#include <vector>

// Boost header files go here

#include <boost/version.hpp>
#include "GGlobalDefines.hpp"
#if BOOST_VERSION < ALLOWED_BOOST_VERSION
#error "Error: Boost has incorrect version !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>

#ifndef GINT32FLIPADAPTOR_HPP_
#define GINT32FLIPADAPTOR_HPP_

// GenEvA header files go here

#include "GIntFlipAdaptorT.hpp"


namespace Gem {
namespace GenEvA {

/** All "real" functionality is implemented in GIntFlipAdaptorT and its parent classes */
typedef GIntFlipAdaptorT<boost::int32_t> GInt32FlipAdaptor;

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GINT32FLIPADAPTOR_HPP_ */
