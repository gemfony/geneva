/**
 * @file GOptimizer.cpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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

#include "GOptimizer.hpp"

namespace Gem {
namespace Geneva {

/**************************************************************************************/
/**
 * The standard constructor. Loads the data from the configuration file
 *
 * @param serverMode A boolean indicating whether this class acts in server mode
 * @param fileName The name of the configuration file
 */
GOptimizer::GOptimizer(const bool& serverMode, const std::string& fileName)
	: fileName_(fileName)
	, serverMode_(serverMode)
{
	loadConfigurationData(fileName_);
}

/**************************************************************************************/
/**
 * Allows to register a function object that performs necessary initialization work
 */
void GOptimizer::registerInitFunction(/*some boost::function object*/) {

}

/**************************************************************************************/
/**
 * Allows to add individuals to the class. These will later be used to initialize the
 * optimization algorithms. Note that the class will take ownership of the objects by
 * cloning them.
 *
 * @param ps_ptr A pointer to an individual
 */
void GOptimizer::registerParameterSet(boost::shared_ptr<GParameterSet> ps_ptr) {

}

/**************************************************************************************/
/**
 * Allows to specify an optimization monitor to be used with evolutionary algorithms
 */
void GOptimizer::registerOptimizationMonitor(boost::shared_ptr<GEvolutionaryAlgorithm::GEAOptimizationMonitor> ea_om_ptr) {

}

/**************************************************************************************/
/**
 * Allows to specify an optimization monitor to be used with swarm algorithms
 */
void GOptimizer::registerOptimizationMonitor(boost::shared_ptr<GSwarm::GSwarmOptimizationMonitor> swarm_om_ptr) {

}

/**************************************************************************************/
/**
 * Allows to specify an optimization monitor to be used with gradient descents
 */
void GOptimizer::registerOptimizationMonitor(boost::shared_ptr<GGradientDescent::GGDOptimizationMonitor> gd_om_ptr) {

}

/**************************************************************************************/
/**
 * Starts the optimization cycle, using the optimization algorithm that has been requested.
 * Returns the best individual found.
 *
 * @return The best individual found during the optimization process
 */
boost::shared_ptr<GParameterSet> GOptimizer::optimize() {
	// Add the initial individuals to the collections
#ifdef DEBUG
	if(initialParameterSets_.empty()) {
		std::ostringstream error;
		error << "In GOptimizer::optimize(): Error!" << std::endl
			  << "You need to register at least one individual." << std::endl
			  << "Found none." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}
#endif

	// TODO: This is ugly and many parameters aren't recorded yet.
	// Ugly because we leave function with exit() that is supposed
	// to return a result.
	if(parMode_ == ASIONETWORKED && !serverMode_) {
	    boost::shared_ptr<GAsioTCPClientT<GIndividual> > p(new GAsioTCPClientT<GIndividual>(ip, boost::lexical_cast<std::string>(port)));

	    p->setMaxStalls(0); // An infinite number of stalled data retrievals
	    p->setMaxConnectionAttempts(100); // Up to 100 failed connection attempts

	    // Prevent return of unsuccessful adaption attempts to the server
	    p->returnResultIfUnsuccessful(returnRegardless);

	    // Start the actual processing loop
	    p->run();

	    // Terminate execution when finished
	    exit(0);
	} else {
		// Which algorithm are we supposed to use ?
		switch(pers_) {
		case EA: // Evolutionary algorithms
			return eaOptimize();
			break;

		case SWARM: // Swarm algorithms
			return swarmOptimize();
			break;

		case GD: // Gradient descents
			return gdOptimize();
			break;

		case NONE:
			{
				std::ostringstream error;
				error << "In GOptimizer::optimize(): Error!" << std::endl
						<< "No optimization algorithm was specified." << std::endl;
				throw(Gem::Common::gemfony_error_condition(error.str()));
			}
		break;
		};
	}
}

/**************************************************************************************/
/**
 * Performs an EA optimization cycle
 *
 * @return The best individual found during the optimization process
 */
boost::shared_ptr<GParameterSet> GOptimizer::eaOptimize() {
	switch(parMode_) {
	case SERIAL:
		break;
	case MULTITHREADED:
		break;
	case ASIONETWORKED:
		break;
	};
}

/**************************************************************************************/
/**
 * Performs a swarm optimization cycle
 *
 * @return The best individual found during the optimization process
 */
boost::shared_ptr<GParameterSet> GOptimizer::swarmOptimize() {
	switch(parMode_) {
	case SERIAL:
		break;
	case MULTITHREADED:
		break;
	case ASIONETWORKED:
		break;
	};
}

/**************************************************************************************/
/**
 * Performs a GD optimization cycle
 *
 * @return The best individual found during the optimization process
 */
boost::shared_ptr<GParameterSet> GOptimizer::gdOptimize() {
	switch(parMode_) {
	case SERIAL:
		break;
	case MULTITHREADED:
		break;
	case ASIONETWORKED:
		{
			std::ostringstream error;
			error << "In GOptimizer::gdOptimize(): Error!" << std::endl
				  << "ASIONETWORKED mode not implemented yet for gradient descents." << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}
		break;
	};
}

/**************************************************************************************/
/**
 * Writes out descriptions in XML format of the requested number of individuals
 */
void GOptimizer::saveResults(const std::size_t& nInds) {

}

/**************************************************************************************/
/**
 * Loads the configuration data from a given configuration file
 */
void GOptimizer::loadConfigurationData(const std::string& fileName) {

}

/**************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
