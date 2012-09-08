/**
 * @file GMutableI.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

// Standard header files go here
#include <iostream>
#include <sstream>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost header files go here
#if BOOST_VERSION < 104200
#include <boost/exception.hpp> // Deprecated as of Boost 1.42
#else
#include <boost/exception/all.hpp>
#endif

#ifndef GMUTABLEI_HPP_
#define GMUTABLEI_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva header files go here

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This is a simple interface class for mutable objects.
 */
class GMutableI {
public:
	/** @brief The standard destructor */
	virtual ~GMutableI(){ /* nothing */ }

	/** @brief Allows derivatives to be adapted */
	virtual void adapt() = 0;
};

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/

#endif /* GMUTABLEI_HPP_ */
