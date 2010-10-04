/**
 * @file GSimpleSwarm.cpp
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
#include <iostream>
#include <cmath>
#include <sstream>

// Boost header files go here
#include <boost/lexical_cast.hpp>

// Geneva header files go here
#include <courtier/GAsioHelperFunctions.hpp>
#include <courtier/GAsioTCPClientT.hpp>
#include <courtier/GAsioTCPConsumerT.hpp>
#include <geneva/GBrokerEA.hpp>
#include <geneva/GIndividual.hpp>
#include <geneva/GSwarm.hpp>
#include <geneva/GMultiThreadedSwarm.hpp>
#include <geneva/GBrokerSwarm.hpp>

// The individual that should be optimized
#include "GFunctionIndividual.hpp"
#include "GFunctionIndividualDefines.hpp"

// Declares a function to parse the command line
#include "GArgumentParser.hpp"

// Holds the optimization monitor
#include "GInfoFunction.hpp"

using namespace Gem::Geneva;
using namespace Gem::Courtier;
using namespace Gem::Hap;

/************************************************************************************************/
/**
 * The main function.
 */
int main(int argc, char **argv){
	std::string configFile;
	boost::uint16_t parallelizationMode;
	bool serverMode;
	std::string ip;
	unsigned short port;
	boost::uint16_t nProducerThreads;
	boost::uint16_t nEvaluationThreads;
	boost::uint32_t maxIterations;
	long maxMinutes;
	boost::uint32_t reportIteration;
	std::size_t arraySize;
	std::size_t parDim;
	double minVar;
	double maxVar;
	boost::uint32_t processingCycles;
	bool returnRegardless;
	boost::uint32_t waitFactor;
	std::size_t nNeighborhoods;
	std::size_t nNeighborhoodMembers;
	demoFunction df;
	double cLocal;
	double cGlobal;
	double cDelta;
	updateRule ur;
	boost::uint16_t xDim;
	boost::uint16_t yDim;
	bool followProgress;
	bool allRandomInit;

	if(!parseCommandLine (
			argc
			, argv
			, configFile
			, parallelizationMode
			, serverMode
			, ip
			, port
	) ||
	!parseConfigFile (
			configFile
			, nProducerThreads
			, nEvaluationThreads
			, nNeighborhoods
			, nNeighborhoodMembers
			, maxIterations
			, maxMinutes
			, reportIteration
			, arraySize
			, processingCycles
			, returnRegardless
			, waitFactor
			, parDim
			, minVar
			, maxVar
			, df
			, cLocal
			, cGlobal
			, cDelta
			, ur
			, xDim
			, yDim
			, followProgress
			, allRandomInit
	))
	{ exit(1); }

	// Random numbers are our most valuable good. Set the number of threads
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);
	GRANDOMFACTORY->setArraySize(arraySize);

	//***************************************************************************
	// If this is a client in networked mode, we can just start the listener and
	// return when it has finished
	if(parallelizationMode==2 && !serverMode) {
		boost::shared_ptr<GAsioTCPClientT<GIndividual> > p(new GAsioTCPClientT<GIndividual>(ip, boost::lexical_cast<std::string>(port)));

		p->setMaxStalls(0); // An infinite number of stalled data retrievals
		p->setMaxConnectionAttempts(100); // Up to 100 failed connection attempts

		// Prevent return of unsuccessful adaption attempts to the server
		p->returnResultIfUnsuccessful(returnRegardless);

		// Start the actual processing loop
		p->run();

		return 0;
	}

	//***************************************************************************
	// Create an instance of our optimization monitor
	boost::shared_ptr<progressMonitor> pm_ptr(new progressMonitor(df));
	pm_ptr->setDims(xDim, yDim);
	pm_ptr->setFollowProgress(followProgress); // Shall we take snapshots ?
	pm_ptr->setXExtremes(minVar, maxVar);
	pm_ptr->setYExtremes(minVar, maxVar);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// We can now start creating populations. We refer to them through the base class

	// This smart pointer will hold the different population types
	boost::shared_ptr<GSwarm> pop_ptr;

	// Create the actual populations
	switch (parallelizationMode) {
	//-----------------------------------------------------------------------------------------------------
	case 0: // Serial execution
		// Create an empty population
		pop_ptr = boost::shared_ptr<GSwarm>(new GSwarm(nNeighborhoods, nNeighborhoodMembers));
		break;

		//-----------------------------------------------------------------------------------------------------
	case 1: // Multi-threaded execution
	{
		// Create the multi-threaded population
		boost::shared_ptr<GMultiThreadedSwarm> popPar_ptr(new GMultiThreadedSwarm(nNeighborhoods, nNeighborhoodMembers));

		// Population-specific settings
		popPar_ptr->setNThreads(nEvaluationThreads);

		// Assignment to the base pointer
		pop_ptr = popPar_ptr;
	}
	break;

	//-----------------------------------------------------------------------------------------------------
	case 2: // Networked execution (server-side)
	{
		// Create a network consumer and enrol it with the broker
		boost::shared_ptr<GAsioTCPConsumerT<GIndividual> > gatc(new GAsioTCPConsumerT<GIndividual>(port));
		GINDIVIDUALBROKER->enrol(gatc);

		// Create the actual broker population
		boost::shared_ptr<GBrokerSwarm> popBroker_ptr(new GBrokerSwarm(nNeighborhoods, nNeighborhoodMembers));
		popBroker_ptr->setWaitFactor(waitFactor);

		// Assignment to the base pointer
		pop_ptr = popBroker_ptr;
	}
	break;
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Now we have suitable populations and can fill them with data

	// Add individuals to the population.
	// NOTE: Unlike evolutionary algorithms, we do not have to add an adaptor to the population
	if(allRandomInit) { // Random initialization of all individuals in the population
		for(std::size_t p = 0 ; p<pop_ptr->getDefaultPopulationSize(); p++) {
			boost::shared_ptr<GParameterSet> functionIndividual_ptr = GFunctionIndividual<>::getFunctionIndividual(df);

			// Set up a GDoubleCollection with dimension values, each initialized
			// with a random number in the range [min,max[
			boost::shared_ptr<GDoubleCollection> gdc_ptr(new GDoubleCollection(parDim,minVar,maxVar));
			// Let the collection know its initialization range
			gdc_ptr->setInitBoundaries(minVar, maxVar);

			// Make the parameter collection known to this individual
			functionIndividual_ptr->push_back(gdc_ptr);

			// Add tp the population
			pop_ptr->push_back(functionIndividual_ptr);
		}
	}
	else { // Individuals of the same neighborhood start from the same location
		for(std::size_t n=0; n<nNeighborhoods; n++) {
			// Initialize the first individual of the neighborhood
			boost::shared_ptr<GParameterSet> functionIndividual_ptr = GFunctionIndividual<>::getFunctionIndividual(df);
			// Set up a GDoubleCollection with dimension values, each initialized
			// with a random number in the range [min,max[
			boost::shared_ptr<GDoubleCollection> gdc_ptr(new GDoubleCollection(parDim,minVar,maxVar));
			// Let the collection know its initialization range
			gdc_ptr->setInitBoundaries(minVar, maxVar);

			// Make the parameter collection known to this individual
			functionIndividual_ptr->push_back(gdc_ptr);

			// Add tp the population
			pop_ptr->push_back(functionIndividual_ptr);

			// Now add the required number of clones to the neighborhood
			for(std::size_t p=1; p<nNeighborhoodMembers; p++) {
				pop_ptr->push_back(functionIndividual_ptr->clone<GParameterSet>());
			}
		}
	}

	// Specify some general population settings
	pop_ptr->setMaxIteration(maxIterations);
	pop_ptr->setMaxTime(boost::posix_time::minutes(maxMinutes));
	pop_ptr->setReportIteration(reportIteration);
	pop_ptr->setCLocal(cLocal);
	pop_ptr->setCGlobal(cGlobal);
	pop_ptr->setCDelta(cDelta);
	pop_ptr->setUpdateRule(ur);
	pop_ptr->registerOptimizationMonitor(pm_ptr);

	// Do the actual optimization
	pop_ptr->optimize();

	//--------------------------------------------------------------------------------------------

	std::cout << "Done ..." << std::endl;
	return 0;
}
