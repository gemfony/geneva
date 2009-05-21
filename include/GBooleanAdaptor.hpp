/**
 * @file GBooleanAdaptor.hpp
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
#include <sstream>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>

#ifndef GBOOLEANADAPTOR_HPP_
#define GBOOLEANADAPTOR_HPP_

// GenEvA headers go here

#include "GAdaptorT.hpp"
#include "GObject.hpp"
#include "GBoundedDouble.hpp"
#include "GEnums.hpp"
#include "GenevaExceptions.hpp"
#include "GIntFlipAdaptorT.hpp"

namespace Gem {
namespace GenEvA {

/*************************************************************************/
/**
 * The GBooleanAdaptor represents an adaptor used for the mutation of
 * bool values by flipping its value. See the documentation of GAdaptorT<T> for
 * further information on adaptors in the GenEvA context. All functionality is
 * currently implemented in the GIntFlipAdaptorT class.
 */
typedef GIntFlipAdaptorT<bool> GBooleanAdaptor;

/***********************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GBOOLEANADAPTOR_HPP_ */
