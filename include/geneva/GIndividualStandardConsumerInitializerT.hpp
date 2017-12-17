/**
 * @file GIndividualStandardConsumerInitializerT.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
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
 * http://www.gemfony.eu .
 */

#ifndef GINDIVIDUALSTANDARDCONSUMERINITIALIZERT_HPP_
#define GINDIVIDUALSTANDARDCONSUMERINITIALIZERT_HPP_

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <iostream>

// Boost header files go here

// Geneva headers go here
#include "common/GLogger.hpp"
#include "common/GGlobalOptionsT.hpp"
#include "courtier/GBaseConsumerT.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GConsumerStore.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This base class takes care of adding GParameterSet-based consumer objects
 * to a global store
 */
template <typename c_type> // c_type stands for consumer type
class GIndividualStandardConsumerInitializerT {
public:
	/** @brief The initializing constructor */
	GIndividualStandardConsumerInitializerT() {
		// Create a smart pointer holding the consumer
		std::shared_ptr<Gem::Courtier::GBaseConsumerT<Gem::Geneva::GParameterSet>> p(new c_type());
		std::string mnemonic = p->getMnemonic();

		// Register the consumer with the store, if it hasn't happened yet
		GConsumerStore->setOnce(mnemonic, p);
	}
	/** @brief An empty destructor */
	virtual ~GIndividualStandardConsumerInitializerT() { /* nothing */ }
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GINDIVIDUALSTANDARDCONSUMERINITIALIZERT_HPP_ */
