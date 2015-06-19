/**
 * @file GDirectSwarm.cpp
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

/*
 * *****************************************************************************
 * NOTE: This file shows how to access some of the optimization algorithms
 * directly (EA in this case), without going through the Go2 class. Usually, Go2
 * is the recommended way and will relieve you from many burdensome tasks you
 * otherwise have to perform. Thus, if you are new to Geneva, we recommend that
 * you start with example 01 first rather than following what is shown in this file.
 * *****************************************************************************
 */

// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>

// Boost header files go here
#include <boost/lexical_cast.hpp>

// Geneva header files go here
#include "courtier/GAsioTCPConsumerT.hpp"
#include "courtier/GBoostThreadConsumerT.hpp"
#include "courtier/GSerialConsumerT.hpp"
#include "common/GParserBuilder.hpp"
#include "geneva/GenevaInitializer.hpp"
#include "geneva/GSwarmAlgorithmFactory.hpp"

// The individual that should be optimized
#include "geneva-individuals/GFunctionIndividual.hpp"

// Declares a function to parse the command line
#include "GArgumentParser.hpp"

// Holds the optimization monitor
#include "GInfoFunction.hpp"

using namespace Gem::Geneva;
using namespace Gem::Courtier;
using namespace Gem::Hap;
using namespace Gem::Common;

/******************************************************************************/
/**
 * The main function.
 */
int main(int argc, char **argv){
	boost::uint16_t parallelizationMode;
	bool serverMode;
	std::string ip;
	unsigned short port;
	boost::uint32_t maxStalls;
	boost::uint32_t maxConnectionAttempts;
	boost::uint16_t nProducerThreads;
	boost::uint16_t nEvaluationThreads;
	boost::uint32_t maxIterations;
	long maxMinutes;
	boost::uint32_t reportIteration;
	Gem::Common::serializationMode serMode;
	boost::uint16_t xDim;
	boost::uint16_t yDim;
	bool followProgress;
	bool addLocalConsumer;
	std::size_t nNeighborhoods;
	std::size_t nNeighborhoodMembers;
	double cPersonal;
	double cNeighborhood;
	double cGlobal;
	double cVelocity;
	updateRule ur;
	bool allRandomInit;

	/****************************************************************************/
	// Initialization of Geneva
	GenevaInitializer gi;

	/****************************************************************************/
	// Retrieve all necessary configuration data from the command line

	if(!parseCommandLine(
		argc, argv
		, parallelizationMode
		, serverMode
		, ip
		, port
		, maxStalls
		, maxConnectionAttempts
		, serMode
		, addLocalConsumer
		, nNeighborhoods
		, nNeighborhoodMembers
		, cPersonal
		, cNeighborhood
		, cGlobal
		, cVelocity
		, ur
		, allRandomInit
		, nProducerThreads
		, nEvaluationThreads
		, maxIterations
		, maxMinutes
		, reportIteration
		, xDim
		, yDim
		, followProgress
	)) { exit(1); }

	/****************************************************************************/
	// Random numbers are our most valuable good. Set the number of threads
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);

	/****************************************************************************/
	// If this is a client in networked mode, we can just start the listener and
	// return when it has finished
	if(EXECMODE_BROKERAGE==parallelizationMode && !serverMode) {
		std::shared_ptr<GAsioTCPClientT<GParameterSet>>
			p(new GAsioTCPClientT<GParameterSet>(ip, boost::lexical_cast<std::string>(port)));

		p->setMaxStalls(maxStalls); // 0 would mean an infinite number of stalled data retrievals
		p->setMaxConnectionAttempts(maxConnectionAttempts);

		// Start the actual processing loop
		p->run();

		return 0;
	}

	/****************************************************************************/
	// We can now start creating populations. We refer to them through the base class

	// This smart pointer will hold the different population types
	std::shared_ptr<GBaseSwarm> pop_ptr;

	// Create the actual populations
	switch (parallelizationMode) {
		//---------------------------------------------------------------------------
		case 0: // Serial execution
			// Create an empty population
			pop_ptr
				= std::shared_ptr<GSerialSwarm>(new GSerialSwarm(nNeighborhoods, nNeighborhoodMembers));
			break;

			//---------------------------------------------------------------------------
		case 1: // Multi-threaded execution
		{
			// Create the multi-threaded population
			std::shared_ptr<GMultiThreadedSwarm>
				popPar_ptr(new GMultiThreadedSwarm(nNeighborhoods, nNeighborhoodMembers));

			// Population-specific settings
			popPar_ptr->setNThreads(nEvaluationThreads);

			// Assignment to the base pointer
			pop_ptr = popPar_ptr;
		}
			break;

			//---------------------------------------------------------------------------
		case 2: // Networked execution (server-side)
		{
			// Create a network consumer and enrol it with the broker
			std::shared_ptr<GAsioTCPConsumerT<GParameterSet>>
				gatc(new GAsioTCPConsumerT<GParameterSet>(port, 0, serMode));
			GBROKER(Gem::Geneva::GParameterSet)->enrol(gatc);

			if(addLocalConsumer) { // This is mainly for testing and benchmarking
				std::shared_ptr<GBoostThreadConsumerT<GParameterSet>>
					gbtc(new GBoostThreadConsumerT<GParameterSet>());
				gbtc->setNThreadsPerWorker(nEvaluationThreads);
				GBROKER(Gem::Geneva::GParameterSet)->enrol(gbtc);
			}

			// Create the actual broker population
			std::shared_ptr<GBrokerSwarm>
				popBroker_ptr(new GBrokerSwarm(nNeighborhoods, nNeighborhoodMembers));

			// Assignment to the base pointer
			pop_ptr = popBroker_ptr;
		}
			break;

			//----------------------------------------------------------------------------
		default:
		{
			glogger
			<< "In main(): Received invalid parallelization mode " << parallelizationMode << std::endl
			<< GEXCEPTION;
		}
			break;
	}

	/****************************************************************************/
	// Create a factory for GFunctionIndividual objects. It performs
	// all necessary initial work related to the individual (i.e. the optimization problem)
	GFunctionIndividualFactory gfi("./config/GFunctionIndividual.json");

	// Create the first set of parent individuals
	std::vector<std::shared_ptr<GFunctionIndividual>> parentIndividuals;

	// Create initial individuals for the population
	if(allRandomInit) { // Random initialization of all individuals in the population
		for(std::size_t p = 0 ; p<pop_ptr->getDefaultPopulationSize(); p++) {
			parentIndividuals.push_back(gfi.get<GFunctionIndividual>());
		}
	} else { // Individuals of the same neighborhood start from the same location
		for(std::size_t n=0; n<nNeighborhoods; n++) {
			// Initialize the first individual of the neighborhood
			std::shared_ptr<GFunctionIndividual> functionIndividual_ptr
				= gfi.get<GFunctionIndividual>();

			// Now add the required number of clones to the neighborhood
			for(std::size_t p=1; p<nNeighborhoodMembers; p++) {
				parentIndividuals.push_back(functionIndividual_ptr->clone<GFunctionIndividual>());
			}
			parentIndividuals.push_back(functionIndividual_ptr);
		}
	}

	/****************************************************************************/
	// Create an instance of our optimization monitor
	std::shared_ptr<progressMonitor>
		pm_ptr(new progressMonitor(parentIndividuals[0]->getDemoFunction())); // The demo function is only known to the individual
	pm_ptr->setProgressDims(xDim, yDim);
	pm_ptr->setFollowProgress(followProgress); // Shall we take snapshots ?
	pm_ptr->setXExtremes(gfi.getMinVar(), gfi.getMaxVar());
	pm_ptr->setYExtremes(gfi.getMinVar(), gfi.getMaxVar());

	/****************************************************************************/
	// Now we have suitable populations and can fill them with data

	// Add individuals to the population. Many Geneva classes, such as
	// the optimization classes, feature an interface very similar to std::vector.
	for(std::size_t p = 0 ; p<parentIndividuals.size(); p++) {
		pop_ptr->push_back(parentIndividuals[p]);
	}

	// Specify some general population settings
	pop_ptr->setMaxIteration(maxIterations);
	pop_ptr->setMaxTime(boost::posix_time::minutes(maxMinutes));
	pop_ptr->setReportIteration(reportIteration);
	pop_ptr->setCPersonal(cPersonal);
	pop_ptr->setCGlobal(cGlobal);
	pop_ptr->setCNeighborhood(cNeighborhood);
	pop_ptr->setCVelocity(cVelocity);
	pop_ptr->setUpdateRule(ur);
	pop_ptr->registerOptimizationMonitor(pm_ptr);

	// Do the actual optimization
	pop_ptr->optimize();

	/****************************************************************************/
	// Do something with the best individual found
	std::shared_ptr<GFunctionIndividual> p = pop_ptr->getBestIndividual<GFunctionIndividual>();

	// Here you can do something with the best individual ("p") found.
	// We simply print its content here, by means of an operator<< implemented
	// in the GFunctionIndividual code.
	std::cout
	<< "Best result found:" << std::endl
	<< p << std::endl;

	/****************************************************************************/
	// Terminate
	return 0;
}
