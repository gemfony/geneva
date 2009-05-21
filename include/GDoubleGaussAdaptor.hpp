/**
 * @file GDoubleGaussAdaptor.hpp
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
#include <cmath>
#include <string>
#include <sstream>
#include <utility> // For std::pair

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>

#ifndef GDOUBLEGAUSSADAPTOR_HPP_
#define GDOUBLEGAUSSADAPTOR_HPP_

// GenEvA headers go here

#include "GGaussAdaptorT.hpp"

namespace Gem {
namespace GenEvA {

/*************************************************************************/
/**
 * The GDoubleGaussAdaptor represents an adaptor used for the mutation of
 * double values through the addition of gaussian-distributed random numbers.
 * See the documentation of GAdaptorT<T> for further information on adaptors
 * in the GenEvA context. This class is at the core of evolutionary strategies,
 * as implemented by this library. It is now implemented through a generic
 * base class that can also be used to mutate other numeric types.
 */
typedef GGaussAdaptorT<double> GDoubleGaussAdaptor;

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GDOUBLEGAUSSADAPTOR_HPP_ */
