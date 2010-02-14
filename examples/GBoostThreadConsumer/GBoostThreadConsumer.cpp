/**
 * @file GBoostThreadConsumer.cpp
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
#include <iostream>
#include <cmath>
#include <sstream>

// Boost header files go here
#include <boost/function.hpp>

// GenEvA header files go here
#include "GRandom.hpp"
#include "GDoubleCollection.hpp"
#include "GParameterSet.hpp"
#include "GDoubleGaussAdaptor.hpp"

// Including this header is mandatory, even if you
// don't manually instantiate any class.
#include "GIndividualBroker.hpp"

#include "GBoostThreadConsumer.hpp"
#include "GBrokerEA.hpp"

// The individual that should be optimized
#include "GFunctionIndividual.hpp"

// Retrieves information from the command line
#include "GCommandLineParser.hpp"

using namespace Gem::GenEvA;
using namespace Gem::Util;

/************************************************************************************************/
/**
 * The main function. We search for the minimum of a parabola, with the help of a GBoostThreadConsumer.
 * This is also a test for the GBroker architecture. Compare the speed with the execution of a simple,
 * multi-threaded population.
 */
int main(int argc, char **argv){
	 std::size_t parabolaDimension, nConsumerThreads, populationSize, nParents;
	 double parabolaMin, parabolaMax;
	 boost::uint16_t nProducerThreads;
	 boost::uint32_t maxGenerations, reportGeneration;
	 boost::uint32_t adaptionThreshold;
	 long maxMinutes;
	 bool verbose;
	 recoScheme rScheme;

	// Retrieve command line options
	if(!parseCommandLine(argc, argv,
						 parabolaDimension,
						 parabolaMin,
						 parabolaMax,
						 adaptionThreshold,
						 nProducerThreads,
						 nConsumerThreads,
						 populationSize,
						 nParents,
						 maxGenerations,
						 maxMinutes,
						 reportGeneration,
						 rScheme,
						 verbose))
	{ exit(1); }

	// Random numbers are our most valuable good. Set the number of threads
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);

	// Set up a single parabola individual
	boost::shared_ptr<GParabolaIndividual>
		parabolaIndividual(new GParabolaIndividual(parabolaDimension, parabolaMin, parabolaMin, adaptionThreshold));

	// Create a consumer and make it known to the global broker
	boost::shared_ptr<GBoostThreadConsumer> gbtc(new GBoostThreadConsumer());
	gbtc->setMaxThreads(nConsumerThreads);
	GINDIVIDUALBROKER->enrol(gbtc);

	GBrokerEA pop;
	pop.push_back(parabolaIndividual);

	// Specify some population settings
	pop.setPopulationSize(populationSize,nParents);
	pop.setMaxIteration(maxGenerations);
	pop.setMaxTime(boost::posix_time::minutes(maxMinutes));
	pop.setReportIteration(reportGeneration);
	pop.setRecombinationMethod(rScheme);

	// Do the actual optimization
	pop.optimize();

	std::cout << "Done ..." << std::endl;

	return 0;
}
