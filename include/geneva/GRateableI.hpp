/**
 * @file GRateableI.hpp
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

/**************************************************************************************************/
/**
 * A simple interface class for objects that can be evaluated.
 */
class GRateableI {
public:
	/** @brief The standard destructor */
	virtual ~GRateableI(){ /* nothing */ }

	/** @brief Retrieve a value for this class */
	virtual double fitness() = 0;

	/** @brief Retrieve a value for this class and check for exceptions. Useful for threads */
	virtual double checkedFitness() = 0;
};

} /* namespace Geneva */
} /* namespace Gem */

/**************************************************************************************************/

#endif /* GRATEABLEI_HPP_ */
