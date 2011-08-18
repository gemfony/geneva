/**
 * @file Go.cpp
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

#include "geneva/Go.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Go2)

namespace Gem {
namespace Geneva {

/**************************************************************************************/
/**
 * Triggers execution of the client loop. Note that it is up to you to terminate
 * the program after calling this function.
 *
 * @return A boolean indicating whether we are running in client mode
 */
bool Go2::clientRun() {
	if(serverMode_) {
		return false;
	}
	else {
		// Instantiate the client worker
		boost::shared_ptr<Gem::Courtier::GAsioTCPClientT<GIndividual> > p(new Gem::Courtier::GAsioTCPClientT<GIndividual>(ip_, boost::lexical_cast<std::string>(port_)));

		p->setMaxStalls(maxStalledDataTransfers_); // Set to 0 to allow an infinite number of stalls
		p->setMaxConnectionAttempts(maxConnectionAttempts_); // Set to 0 to allow an infinite number of failed connection attempts
		p->returnResultIfUnsuccessful(returnRegardless_);  // Prevent return of unsuccessful adaption attempts to the server

		// Start the actual processing loop
		p->run();

		return true;
	}
}

/**************************************************************************************/
/**
 * Checks whether server mode has been requested for this object
 *
 * @return A boolean which indicates whether the server mode has been set for this object
 */
bool Go2::serverMode() const {
	return serverMode_;
}

/**************************************************************************************/
/**
 * Checks whether this object is running in client mode
 *
 * @return A boolean which indicates whether the client mode has been set for this object
 */
bool Go2::clientMode() const {
	return !serverMode_;
}

/**************************************************************************************/

/**************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
