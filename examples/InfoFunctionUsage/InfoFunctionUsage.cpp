/**
 * @file InfoFunctionUsage.cpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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
 */

// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>
#include <fstream>
#include <string>

// Boost header files go here
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

// GenEvA header files go here
#include "GRandom.hpp"
#include "GBasePopulation.hpp"
#include "GIndividual.hpp"
#include "GDoubleCollection.hpp"
#include "GParameterSet.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GLogger.hpp"
#include "GLogTargets.hpp"
#include "GBoostThreadPopulation.hpp"

// The individual that should be optimized
// This is a "noisy" parabola
#include "GNoisyParabolaIndividual.hpp"

// Holds the function object used for the information retrieval
#include "GOptimizationMonitor.hpp"

// Declares a function to parse the command line
#include "GCommandLineParser.hpp"

using namespace Gem::GenEvA;
using namespace Gem::Util;
using namespace Gem::GLogFramework;


/************************************************************************************************/
/**
 * The main function - similar to the GBasePopulation example. We search for the minimum of a
 * simple function, such as provided by the "parabola" or "noisy parabola" individuals.
 */
int main(int argc, char **argv){
	 std::size_t nPopThreads;
	 std::size_t populationSize, nParents;
	 std::size_t parabolaDim;
	 double parabolaMin, parabolaMax;
	 boost::uint16_t nProducerThreads;
	 boost::uint32_t maxGenerations, reportGeneration;
	 boost::uint32_t adaptionThreshold;
	 long maxMinutes;
	 bool verbose;
	 recoScheme rScheme;

	// Parse the command line
	if(!parseCommandLine(argc, argv,
						 parabolaMin,
						 parabolaMax,
						 parabolaDim,
						 adaptionThreshold,
						 nProducerThreads,
						 nPopThreads,
						 populationSize,
						 nParents,
						 maxGenerations,
						 maxMinutes,
						 reportGeneration,
						 rScheme,
						 verbose))
	{ exit(1); }

	// Add some log levels to the logger
	LOGGER->addLogLevel(Gem::GLogFramework::CRITICAL);
	LOGGER->addLogLevel(Gem::GLogFramework::WARNING);
	LOGGER->addLogLevel(Gem::GLogFramework::INFORMATIONAL);
	LOGGER->addLogLevel(Gem::GLogFramework::PROGRESS);

	// Add log targets to the system
	LOGGER->addTarget(boost::shared_ptr<GBaseLogTarget>(new GDiskLogger("InfoFunctionUsage.log")));
	LOGGER->addTarget(boost::shared_ptr<GBaseLogTarget>(new GConsoleLogger()));

	// Random numbers are our most valuable good. Set the number of threads
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);

	// Set up a single "noisy" parabola individual.
	boost::shared_ptr<GNoisyParabolaIndividual> noisyParabolaIndividual(new GNoisyParabolaIndividual(parabolaDim,
																									 parabolaMin,
																									 parabolaMax,
																									 adaptionThreshold));

	// Create the optimizationMonitor
	boost::shared_ptr<optimizationMonitor> om(new optimizationMonitor("optimization.xml"));

	// Create the population
	boost::shared_ptr<GBoostThreadPopulation> pop(new GBoostThreadPopulation());
	pop->setNThreads(nPopThreads);

	// Register the monitor with the population. boost::bind knows how to handle a shared_ptr.
	pop->registerInfoFunction(boost::bind(&optimizationMonitor::informationFunction, om, _1, _2));

	pop->push_back(noisyParabolaIndividual);

	// Specify some population settings
	pop->setPopulationSize(populationSize,nParents);
	pop->setMaxGeneration(maxGenerations);
	pop->setMaxTime(boost::posix_time::minutes(maxMinutes)); // Calculation should be finished after 5 minutes
	pop->setReportGeneration(reportGeneration); // Emit information during every generation
	pop->setRecombinationMethod(rScheme); // The best parents have higher chances of survival

	// Do the actual optimization
	pop->optimize();

	// At this point we should have a file named "optimization.xml" in the same directory as this executable.

	std::cout << "Done ..." << std::endl;

	return 0;
}
