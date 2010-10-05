/**
 * @file GOptimizer.hpp
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

// Standard header files go here
#include <vector>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost header files go here
#include <boost/utility.hpp>

#ifndef GOPTIMIZER_HPP_
#define GOPTIMIZER_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GEvolutionaryAlgorithm.hpp"
#include "geneva/GMultiThreadedEA.hpp"
#include "geneva/GBrokerEA.hpp"
#include "geneva/GSwarm.hpp"
#include "geneva/GMultiThreadedSwarm.hpp"
#include "geneva/GBrokerSwarm.hpp"
#include "geneva/GGradientDescent.hpp"
#include "geneva/GMultiThreadedGD.hpp"
#include "common/GExceptions.hpp"
#include "hap/GRandomT.hpp"

namespace Gem {
namespace Geneva {

/**************************************************************************************/
/**
 * This class acts as a wrapper around the various optimization algorithms in the
 * Geneva library. Its aim is to facilitate the usage of the various algorithms,
 * relieving users from having to write any other code than is needed by their parameter
 * descriptions. The class parses a configuration file covering the most common options of
 * the various optimization algorithms. The class will not touch the command line. The
 * user can make the name of a configuration file known to the class. If none is provided,
 * the class will attempt to load the data from a default file name.
 */
class GOptimizer
	:boost::noncopyable
{
public:
	/** @brief The standard constructor. Loads the data from the configuration file */
	explicit GOptimizer(const bool&, const std::string& fileName = "optimizationAlgorithm.cfg");

	/** @brief Allows to register a function object that performs necessary initialization work */
	void registerInitFunction(/*some boost::function object*/);

	/** @brief Allows to add individuals to the class. These will later be used to initialize the optimization algorithms. Note that the class will take ownership of the objects by cloning them */
	void registerParameterSet(boost::shared_ptr<GParameterSet>);

	/** @brief Allows to specify an optimization monitor to be used with evolutionary algorithms */
	void registerOptimizationMonitor(boost::shared_ptr<GEvolutionaryAlgorithm::GEAOptimizationMonitor>);
	/** @brief Allows to specify an optimization monitor to be used with swarm algorithms */
	void registerOptimizationMonitor(boost::shared_ptr<GSwarm::GSwarmOptimizationMonitor>);
	/** @brief Allows to specify an optimization monitor to be used with gradient descents */
	void registerOptimizationMonitor(boost::shared_ptr<GGradientDescent::GGDOptimizationMonitor>);

	/** @brief Starts the optimization cycle, using the optimization algorithm that has been requested. Returns the best individual found */
	boost::shared_ptr<GParameterSet> optimize();

	/** @brief Writes out descriptions in XML format of the requested number of individuals */
	void saveResults(const std::size_t&);

private:
	/** @brief The default constructor. Intentionally private and undefined */
	GOptimizer();
	/** @brief Loads the configuration data from a given configuration file */
	void loadConfigurationData(const std::string&);

	/** @brief Performs an EA optimization cycle */
	boost::shared_ptr<GParameterSet> eaOptimize();
	/** @brief Performs a swarm optimization cycle */
	boost::shared_ptr<GParameterSet> swarmOptimize();
	/** @brief Performs a GD optimization cycle */
	boost::shared_ptr<GParameterSet> gdOptimize();

	/**********************************************************************/
	// Data
	std::vector<boost::shared_ptr<GParameterSet> > initialParameterSets_; ///< Holds the individuals used for the initialization of the algorithm
	std::string configFilename_; ///< Indicates where the configuration file is stored
	bool serverMode_; ///< Specifies whether this object (and the executable) are running in server or client mode
	personality pers_; ///< Indicates which optimization algorithm should be used
	parMode parMode_; ///< The chosen parallelization mode

};

/**************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GOPTIMIZER_HPP_ */
