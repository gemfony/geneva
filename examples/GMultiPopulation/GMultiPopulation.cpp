/**
 * @file GMultiPopulation.cpp
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

// Boost header files go here
#include <boost/function.hpp>

// GenEvA header files go here
#include "GRandom.hpp"
#include "GDoubleCollection.hpp"
#include "GParameterSet.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GLogger.hpp"
#include "GLogTargets.hpp"

// Including this header is mandatory, even if you
// don't manually instantiate any class.
#include "GIndividualBroker.hpp"

#include "GBoostThreadConsumer.hpp"
#include "GBrokerPopulation.hpp"
#include "GBoostThreadPopulation.hpp" // We need two population types

// The individual that should be optimized
// This is a simple parabola
#include "GParabolaIndividual.hpp"

// Retrieves information from the command line
#include "GCommandLineParser.hpp"

using namespace Gem::GenEvA;
using namespace Gem::Util;
using namespace Gem::GLogFramework;

/************************************************************************************************/
/**
 * This example demonstrates the use of multipopulations. In Geneva, populations have the
 * GIndividual interface and thus can also compete as part of a "super-population". This can help
 * to start searching the parameter space from various different areas. As in most other
 * examples, we use a high-dimensional parabola as (lowest level) individual. This example also
 * serves as a stress-test for the broker infrastructure, as the competing populations are
 * part of a GBoostThreadPopulation.
 */
int main(int argc, char **argv){
	 std::size_t parabolaDimension, nConsumerThreads, nSuperThreads;
	 std::size_t superPopulationSize, superNParents, subPopulationSize, subNParents;
	 double parabolaMin, parabolaMax;
	 boost::uint16_t nProducerThreads;
	 boost::uint32_t superMaxGenerations, superReportGeneration;
	 boost::uint32_t subMaxGenerations, subReportGeneration;
	 boost::uint32_t adaptionThreshold;
	 long superMaxMinutes, subMaxMinutes;
	 bool verbose;
	 recoScheme superRScheme, subRScheme;

	 // Retrieve command line options
	 if(!parseCommandLine(argc, argv,
	 					  parabolaDimension,
	 					  parabolaMin,
	 					  parabolaMax,
	 					  adaptionThreshold,
	 					  nProducerThreads,
	 					  nConsumerThreads,
	 					  nSuperThreads,
	 					  superPopulationSize,
	 					  superNParents,
	 					  subPopulationSize,
	 					  subNParents,
	 					  superMaxGenerations,
	 					  subMaxGenerations,
	 					  superMaxMinutes,
	 					  subMaxMinutes,
	 					  superReportGeneration,
	 					  subReportGeneration,
	 					  superRScheme,
	 					  subRScheme,
	 					  verbose))
	{ exit(1); }

	// Add some log levels to the logger
	LOGGER->addLogLevel(Gem::GLogFramework::CRITICAL);
	LOGGER->addLogLevel(Gem::GLogFramework::WARNING);
	LOGGER->addLogLevel(Gem::GLogFramework::INFORMATIONAL);
	LOGGER->addLogLevel(Gem::GLogFramework::PROGRESS);

	// Add log targets to the system
	LOGGER->addTarget(boost::shared_ptr<GBaseLogTarget>(new GDiskLogger("GBoostThreadConsumer.log")));
	LOGGER->addTarget(boost::shared_ptr<GBaseLogTarget>(new GConsoleLogger()));

	// Random numbers are our most valuable good. Set the number of threads
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);

	GBoostThreadPopulation super;
	super.setNThreads(nSuperThreads);

	// Create a consumer and make it known to the global broker
	boost::shared_ptr<GBoostThreadConsumer> gbtc(new GBoostThreadConsumer());
	gbtc->setMaxThreads(nConsumerThreads);
	GINDIVIDUALBROKER->enrol(gbtc);

	// Add superNParents parents
	for(std::size_t np=0; np<superNParents; np++){
		boost::shared_ptr<GBrokerPopulation> sub(new GBrokerPopulation());

		// Set up a new parabola individual. Each new instance will be equipped with its
		// own set of random numbers. Hence we start searching the parameter space from different areas.
		boost::shared_ptr<GParabolaIndividual>
			parabolaIndividual(new GParabolaIndividual(parabolaDimension, parabolaMin, parabolaMin, adaptionThreshold));

		// Add the individual to the sub-population
		sub->push_back(parabolaIndividual);

		// Specify some population settings
		sub->setPopulationSize(subPopulationSize,subNParents);
		sub->setMaxGeneration(subMaxGenerations);
		sub->setMaxTime(boost::posix_time::minutes(subMaxMinutes));
		sub->setReportGeneration(subReportGeneration);
		sub->setRecombinationMethod(subRScheme);

		// Do not time-out while waiting for children to return
		sub->setWaitFactor(0);

		// Add the sub-population to the super-population
		super.push_back(sub);
	}

	// Specify some population settings
	super.setPopulationSize(superPopulationSize,superNParents);
	super.setMaxGeneration(superMaxGenerations);
	super.setMaxTime(boost::posix_time::minutes(superMaxMinutes));
	super.setReportGeneration(superReportGeneration);
	super.setRecombinationMethod(superRScheme);

	// Do the actual optimization
	super.optimize();

	std::cout << "Done ..." << std::endl;

	return 0;
}
