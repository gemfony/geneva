/**
 * @file Geneva.hpp
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

// Standard headers go here

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here

#ifndef GENEVA_HPP_
#define GENEVA_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "hap/GRandomFactory.hpp"
#include "courtier/GBrokerT.hpp"
#include "geneva/GIndividual.hpp"
#include "courtier/GBoostThreadConsumerT.hpp"
#include "geneva/GMultiThreadedEA.hpp"
#include "geneva/GBrokerEA.hpp"

namespace Gem {
namespace Geneva {

/****************************************************************************************/
/**
 * This class serves as a hold-all for functions govnering the overall running and progress
 * of optimizations carried out with Geneva. Most notably, this class holds the startup
 * and finalization code.
 */
class Geneva {
public:
	/************************************************************************************/
	/**
	 * Initialization code for the Geneva library collection. Most notably, we enforce
	 * the initialization of various singletons needed for Geneva.
	 */
	static void init() {
		GRANDOMFACTORY->init();
		GBROKER(Gem::Geneva::GIndividual)->init();
	}

	/************************************************************************************/
	/**
	 * Finalization code for the Geneva library collection. Most notably, we enforce
	 * shutdown of various singleton services needed for Geneva. Note that we shut down
	 * in reverse order to the startup procedure.
	 */
	static void finalize() {
		GBROKER(Gem::Geneva::GIndividual)->finalize();
		RESETGBROKER(Gem::Geneva::GIndividual);

		GRANDOMFACTORY->finalize();
		RESETGRANDOMFACTORY;
	}

	/************************************************************************************/
};

/****************************************************************************************/

} /* Geneva */
} /* Gem */

#endif /* GENEVA_HPP_ */
