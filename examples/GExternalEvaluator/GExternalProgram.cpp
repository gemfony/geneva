/**
 * @file GExternalProgram.cpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
 * Copyright (C) 2009 Forschungszentrum Karlsruhe GmbH
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
#include <string>
#include <sstream>
#include <cstdlib>
#include <vector>

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>

// GenEvA header files go here
#include "GRandom.hpp"
#include "GBasePopulation.hpp"
#include "GBoostThreadPopulation.hpp"
#include "GDoubleCollection.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GInt32Collection.hpp"
#include "GInt32FlipAdaptor.hpp"
#include "GCharFlipAdaptor.hpp"
#include "GBooleanCollection.hpp"
#include "GLogger.hpp"
#include "GLogTargets.hpp"
#include "GBrokerPopulation.hpp"
#include "GIndividualBroker.hpp"
#include "GAsioTCPConsumer.hpp"
#include "GAsioTCPClient.hpp"

// The individual calls an external program for the evaluation step
#include "GExternalEvaluator.hpp"

// Declares a function to parse the command line
#include "GCommandLineParser.hpp"

using namespace Gem::GenEvA;
using namespace Gem::Util;
using namespace Gem::GLogFramework;

/************************************************************************************************/
/**
 * The main function.  The actual calculation is handled by an external program, hence we
 * do not know what the purpose of this optimization is.
 */
int main(int argc, char **argv){
	// Variables for the command line parsing
	 std::string program;
	 std::string externalArguments;
	 std::size_t populationSize, nParents;
	 boost::uint16_t nProducerThreads;
	 boost::uint32_t maxGenerations, reportGeneration;
	 boost::uint32_t adaptionThreshold;
	 long maxMinutes;
	 boost::uint16_t parallel;
	 bool serverMode;
	 std::string ip;
	 unsigned short port=10000;
	 bool verbose;
	 recoScheme rScheme;
	 double sigma,sigmaSigma,minSigma,maxSigma;
	 boost::uint32_t nEvaluations;
	 Gem::GenEvA::dataExchangeMode exchangeMode;

	// Parse the command line
	if(!parseCommandLine(argc, argv,
				         program,
				         externalArguments,
				         populationSize,
				         nParents,
						 adaptionThreshold,
						 nProducerThreads,
						 maxGenerations,
						 maxMinutes,
						 reportGeneration,
						 rScheme,
						 parallel,
						 serverMode,
						 ip,
						 port,
						 sigma,
						 sigmaSigma,
						 minSigma,
						 maxSigma,
						 nEvaluations,
						 exchangeMode,
						 verbose))
	{ exit(1); }

	// Add some log levels to the logger
	LOGGER->addLogLevel(Gem::GLogFramework::CRITICAL);
	LOGGER->addLogLevel(Gem::GLogFramework::WARNING);
	LOGGER->addLogLevel(Gem::GLogFramework::INFORMATIONAL);
	LOGGER->addLogLevel(Gem::GLogFramework::PROGRESS);

	// Add log targets to the system
	LOGGER->addTarget(boost::shared_ptr<GBaseLogTarget>(new GDiskLogger("GExternalEvaluator.log")));
	LOGGER->addTarget(boost::shared_ptr<GBaseLogTarget>(new GConsoleLogger()));

	// Random numbers are our most valuable good. Set the number of threads
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);

	// Tell the evaluation program to do any initial work
	GExternalEvaluator::initialize(program, externalArguments);

	// Create a number of adaptors to be used in the individual
	boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(sigma,sigmaSigma,minSigma,maxSigma));
	gdga_ptr->setAdaptionThreshold(adaptionThreshold);
	boost::shared_ptr<GInt32FlipAdaptor> gifa_ptr(new GInt32FlipAdaptor());
	gifa_ptr->setAdaptionThreshold(adaptionThreshold);
	boost::shared_ptr<GBooleanAdaptor> gba_ptr(new GBooleanAdaptor());
	gba_ptr->setAdaptionThreshold(adaptionThreshold);
	boost::shared_ptr<GCharFlipAdaptor> gcfa_ptr(new GCharFlipAdaptor());
	gcfa_ptr->setAdaptionThreshold(adaptionThreshold);

	// Create an initial individual (it will get the necessary information
	// from the external executable)
	boost::shared_ptr<GExternalEvaluator> gev_ptr(
			new GExternalEvaluator(
					program,
					externalArguments,
					false,  // random initialization of template data
					exchangeMode,
					gdga_ptr,
					gifa_ptr,
					gba_ptr,
					gcfa_ptr
			)
	);

	// Make each external program evaluate a number of data sets, if nEvaluations > 1
	gev_ptr->setNEvaluations(nEvaluations);

	// Set up the populations, as requested
	if(parallel==0) { // serial execution
	  // Now we've got our first individual and can create a simple population with serial execution.
	  GBasePopulation pop_ser;

	  // Attach all individuals to the population
	  pop_ser.push_back(gev_ptr);

	  // Specify some population settings
	  pop_ser.setPopulationSize(populationSize,nParents);
	  pop_ser.setMaxGeneration(maxGenerations);
	  pop_ser.setMaxTime(boost::posix_time::minutes(maxMinutes)); // Calculation should be finished after this amount of time
	  pop_ser.setReportGeneration(reportGeneration); // Emit information during every generation
	  pop_ser.setRecombinationMethod(rScheme); // The best parents have higher chances of survival

	  // Do the actual optimization
	  pop_ser.optimize();

	  // Retrieve best individual and make it output a result file
	  boost::shared_ptr<GExternalEvaluator> bestIndividual = pop_ser.getBestIndividual<GExternalEvaluator>();
	  bestIndividual->printResult();
	}
	else if(parallel==1) { // multi-threaded execution
		// Now we can create a simple population with parallel execution.
	  GBoostThreadPopulation pop_par;
	  pop_par.setNThreads(4);

	  // Attach the individual to the population
	  pop_par.push_back(gev_ptr);

	  // Specify some population settings
	  pop_par.setPopulationSize(populationSize,nParents);
	  pop_par.setMaxGeneration(maxGenerations);
	  pop_par.setMaxTime(boost::posix_time::minutes(maxMinutes)); // Calculation should be finished after this amount of time
	  pop_par.setReportGeneration(reportGeneration); // Emit information during every generation
	  pop_par.setRecombinationMethod(rScheme); // The best parents have higher chances of survival

	  // Do the actual optimization
	  pop_par.optimize();

	  // Retrieve best individual and make it output a result file
	  boost::shared_ptr<GExternalEvaluator> bestIndividual = pop_par.getBestIndividual<GExternalEvaluator>();
	  bestIndividual->printResult();
	}
	else if(parallel==2) { // execution in networked mode
		if(serverMode) {
			// Create a consumer and enrol it with the broker
			boost::shared_ptr<GAsioTCPConsumer> gatc(new GAsioTCPConsumer(port));
			// gatc->setSerializationMode(BINARYSERIALIZATION);
			GINDIVIDUALBROKER->enrol(gatc);

			// Create the actual population
			GBrokerPopulation pop_broker;

			// Make the individual known to the population
			pop_broker.push_back(gev_ptr);

			// Specify some population settings
			pop_broker.setPopulationSize(populationSize,nParents);
			pop_broker.setMaxGeneration(maxGenerations);
			pop_broker.setMaxTime(boost::posix_time::minutes(maxMinutes));
			pop_broker.setReportGeneration(reportGeneration);
			pop_broker.setRecombinationMethod(rScheme);

			// Do the actual optimization
			pop_broker.optimize();

		    // Retrieve best individual and make it output a result file
		    boost::shared_ptr<GExternalEvaluator> bestIndividual = pop_broker.getBestIndividual<GExternalEvaluator>();
		    bestIndividual->printResult();
		}
		else { // Client mode
			// Just start the client with the required parameters
		    GAsioTCPClient gasiotcpclient(ip,boost::lexical_cast<std::string>(port));
		    gasiotcpclient.run();
		}
	}

	// Tell the evaluation program to perform any final work
	GExternalEvaluator::finalize(program, externalArguments);

	std::cout << "Done ..." << std::endl;

	return 0;
}
