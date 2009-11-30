/**
 * @file GBoolean.hpp
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

// Standard headers go here

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>

#ifndef GBOOLEAN_HPP_
#define GBOOLEAN_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// GenEvA headers go here
#include "GParameterT.hpp"

namespace Gem {
namespace GenEvA {

/************************************************************************/
/**
 * This class encapsulates a single bit, represented as a bool. This might appear heavy weight,
 * and indeed for most applications this is not the recommended solution -
 * use the GBooleanCollection instead.
 *
 * Bits are mutated by the GBooleanAdaptor in GenEvA. It incorporates a mutable bit-flip probability.
 * The reason for this class is that there might be applications where one might want different flip
 * probabilities for different bits. Where this is the case, separate GBooleanAdaptors must be
 * assigned to each bit value, which cannot be done with the GBooleanCollection. Plus, having
 * a separate bit class adds some consistency to GenEvA, as other values (most
 * notably doubles) have their own class as well (GDouble).
 *
 * For reasons of simplicity this class is implemented as a simple typdef of the GParameterT
 * template class, which implements a specialization of the constructor.
 */
typedef GParameterT<bool> GBoolean;

/************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GBOOLEAN_HPP_ */
