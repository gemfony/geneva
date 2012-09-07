/**
 * @file GRateableI.hpp
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
#include <sstream>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost header files go here

#ifndef GRATEABLEI_HPP_
#define GRATEABLEI_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva header files go here

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * A simple interface class for objects that can be evaluated.
 */
class GRateableI {
public:
	/** @brief The standard destructor */
	virtual ~GRateableI(){ /* nothing */ }

	/** @brief Retrieve a value for this class, using a fixed fitness function */
	virtual double fitness() = 0;
	/** @brief Retrieve a value for this class, using a fitness function with a given id */
	virtual double fitness(const std::size_t&) = 0;
};

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/

#endif /* GRATEABLEI_HPP_ */
