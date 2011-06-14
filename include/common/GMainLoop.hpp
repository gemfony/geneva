/**
 * @file GMainLoop.hpp
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

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost header files go here

#ifndef GMAINLOOP_HPP_
#define GMAINLOOP_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva header files go here

namespace Gem {
namespace Common {

/***************************************************************************************/
/**
 * This interface-class describes a basic main loop, as required by various objects in Geneva.
 */
class GMainLoop {
public:
	/** @brief The default constructor */
	GMainLoop();
	/** @brief The copy constructor */
	GMainLoop(const GMainLoop&);
	/** @brief The destructor */
	virtual ~GMainLoop();

	/** @brief The event loop */
	bool run();

	/** @brief Initialization code */
	virtual void init() = 0;
	/** @brief To be executed in each iteration */
	virtual void cycleLogic() = 0;
	/** @brief A halt criterion */
	virtual bool halt() = 0;
	/** @brief Finalization code */
	virtual bool finalize() = 0;
};

/***************************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GMAINLOOP_HPP_ */
