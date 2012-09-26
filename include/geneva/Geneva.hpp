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
#include <cstdlib>

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
#include <hap/GRandomFactory.hpp>
#include <geneva/GIndividual.hpp>
#include <courtier/GAsioHelperFunctions.hpp>
#include <courtier/GBrokerT.hpp>
#include <courtier/GAsioTCPClientT.hpp>
#include <courtier/GAsioTCPConsumerT.hpp>
#include <courtier/GBoostThreadConsumerT.hpp>
#include <geneva/GBaseEA.hpp>
#include <geneva/GSerialEA.hpp>
#include <geneva/GMultiThreadedEA.hpp>
#include <geneva/GBrokerEA.hpp>
#include <geneva/GBaseGD.hpp>
#include <geneva/GMultiThreadedGD.hpp>
#include <geneva/GBrokerGD.hpp>
#include <geneva/GBaseSwarm.hpp>
#include <geneva/GSerialSwarm.hpp>
#include <geneva/GMultiThreadedSwarm.hpp>
#include <geneva/GBrokerSwarm.hpp>
#include <geneva/GIndividual.hpp>
#include <geneva/GMultiThreadedEA.hpp>
#include <geneva/GDoubleObject.hpp>
#include <geneva/Go.hpp>

namespace Gem {
namespace Geneva {

/******************************************************************************/
/** @brief A termination handler */
void GTerminate();

/******************************************************************************/
/**
 * This class serves as a hold-all for functions governing the overall running and progress
 * of optimizations carried out with Geneva. Most notably, this class holds the startup
 * and finalization code.
 */
class Geneva {
public:
	/***************************************************************************/
	/**
	 * Initialization code for the Geneva library collection. Most notably, we enforce
	 * the initialization of various singletons needed for Geneva.
	 */
	static void init() {
		GRANDOMFACTORY->init();
		GBROKER(Gem::Geneva::GIndividual)->init();
	}

	/***************************************************************************/
	/**
	 * Finalization code for the Geneva library collection. Most notably, we enforce
	 * shutdown of various singleton services needed for Geneva. Note that we shut down
	 * in reverse order to the startup procedure.
	 */
	static int finalize() {
		GBROKER(Gem::Geneva::GIndividual)->finalize();
		RESETGBROKER(Gem::Geneva::GIndividual);

		GRANDOMFACTORY->finalize();
		RESETGRANDOMFACTORY;

#ifdef GEM_INT_FORCE_TERMINATION // Defined in GGlobalDefines.hpp.in
		std::set_terminate(GTerminate);
		std::terminate();
#endif /* GEM_INT_FORCE_TERMINATION */

		std::cout << "Done ..." << std::endl;
		return 0;
	}

	/***************************************************************************/
};

/******************************************************************************/

} /* Geneva */
} /* Gem */

#endif /* GENEVA_HPP_ */

using namespace Gem::Tests;
using namespace Gem::Geneva;
using namespace Gem::Courtier;
using namespace Gem::Hap;
