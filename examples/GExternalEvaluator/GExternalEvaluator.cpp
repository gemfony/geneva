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
#include <fstream>
#include <cstdlib>
#include <vector>

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cstdint.hpp>
#include <boost/bind.hpp>

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
#include "GEnums.hpp"

// The individual calls an external program for the evaluation step
#include "GExternalEvaluator.hpp"

// Declares a function to parse the command line
#include "GCommandLineParser.hpp"

using namespace Gem::GenEvA;
using namespace Gem::Util;
using namespace Gem::GLogFramework;

/************************************************************************************************/
/**
 * An information object that will also emit result information in every nth generation,
 * if requested.
 */
class optimizationMonitor{
public:
	/*********************************************************************************************/
	/**
	 * The standard constructor. All collected data will to be written to file
	 *
	 * @param nGenInfo The number of generations after which a result file should be emitted (0 if none is desired)
	 */
	optimizationMonitor(const boost::uint16_t nGenInfo)
		:nGenInfo_(nGenInfo)
	{ /* nothing */  }

	/*********************************************************************************************/
	/**
	 * The function that does the actual collection of data. It can be called in
	 * three modes:
	 *
	 * INFOINIT: is called once before the optimization run.
	 * INFOPROCESSING: is called in regular intervals during the optimization, as determined by the user
	 * INFOEND: is called once after the optimization run
	 *
	 * @param im The current mode in which the function is called
	 * @param gbp A pointer to a GBasePopulation object for which information should be collected
	 */
	void informationFunction(const infoMode& im, GBasePopulation * const gbp){
		// First act on the request to emit result files
		if(nGenInfo_ && im == Gem::GenEvA::INFOPROCESSING) { // > 0 && in processing mode ?
			// Retrieve the current generation
			boost::uint32_t generation = gbp->getGeneration();

			if(generation%nGenInfo_ == 0) {
				// Get access to the best individual in the population (or the one chosen as
				// best parent, in MUCOMMANU mode)
				boost::shared_ptr<GExternalEvaluator> gev_ptr = gbp->getBestIndividual<GExternalEvaluator>();

				// Tell the individual to output the result, offering the external program an identifying string
				gev_ptr->printResult(boost::lexical_cast<std::string>(generation));
			}
		}

		// Then emit the "usual" output
		Gem::GenEvA::GBasePopulation::defaultInfoFunction(im, gbp);
	}

	/*********************************************************************************************/

private:
	boost::uint16_t nGenInfo_;
};

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
	 boost::uint16_t nProcessingThreads;
	 boost::uint32_t maxGenerations, reportGeneration;
	 boost::uint32_t adaptionThreshold;
	 long maxMinutes;
	 boost::uint16_t parallelizationMode;
	 bool serverMode;
	 std::string ip;
	 unsigned short port=10000;
	 bool verbose;
	 recoScheme rScheme;
	 double sigma,sigmaSigma,minSigma,maxSigma;
	 boost::uint32_t nEvaluations;
	 Gem::GenEvA::dataExchangeMode exchangeMode;
	 bool sortingScheme;
	 boost::uint32_t interval;
	 bool maximize;

	// Parse the command line
	if(!parseCommandLine(argc, argv,
				         program,
				         externalArguments,
				         populationSize,
				         nParents,
						 adaptionThreshold,
						 nProducerThreads,
						 nProcessingThreads,
						 maxGenerations,
						 maxMinutes,
						 reportGeneration,
						 rScheme,
						 parallelizationMode,
						 serverMode,
						 ip,
						 port,
						 sigma,
						 sigmaSigma,
						 minSigma,
						 maxSigma,
						 nEvaluations,
						 exchangeMode,
						 sortingScheme,
						 interval,
						 maximize,
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

	// Create an instance of our optimization monitor, telling it to output information in
	// given intervals
	boost::shared_ptr<optimizationMonitor> om(new optimizationMonitor(interval));

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

	// Make sure we perform minimizations
	gev_ptr->setMaximize(false);

	// Set up the populations, as requested
	if(parallelizationMode==0) { // serial execution
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
	  pop_ser.setSortingScheme(sortingScheme); // Determines whether sorting is done in MUPLUSNU or MUCOMMANU mode
	  pop_ser.setMaximize(maximize); // Specifies whether the program should do maximization or minimization

	  // Register the monitor with the population. boost::bind knows how to handle a shared_ptr.
	  pop_ser.registerInfoFunction(boost::bind(&optimizationMonitor::informationFunction, om, _1, _2));

	  // Do the actual optimization
	  pop_ser.optimize();

	  // Retrieve best individual and make it output a result file
	  boost::shared_ptr<GExternalEvaluator> bestIndividual = pop_ser.getBestIndividual<GExternalEvaluator>();
	  bestIndividual->printResult();
	}
	else if(parallelizationMode==1) { // multi-threaded execution
		// Now we can create a simple population with parallel execution.
	  GBoostThreadPopulation pop_par;
	  pop_par.setNThreads(nProcessingThreads);

	  // Attach the individual to the population
	  pop_par.push_back(gev_ptr);

	  // Specify some population settings
	  pop_par.setPopulationSize(populationSize,nParents);
	  pop_par.setMaxGeneration(maxGenerations);
	  pop_par.setMaxTime(boost::posix_time::minutes(maxMinutes)); // Calculation should be finished after this amount of time
	  pop_par.setReportGeneration(reportGeneration); // Emit information during every generation
	  pop_par.setRecombinationMethod(rScheme); // The best parents have higher chances of survival
	  pop_par.setSortingScheme(sortingScheme); // Determines whether sorting is done in MUPLUSNU or MUCOMMANU mode
	  pop_par.setMaximize(maximize); // Specifies whether the program should do maximization or minimization

	  // Register the monitor with the population. boost::bind knows how to handle a shared_ptr.
	  pop_par.registerInfoFunction(boost::bind(&optimizationMonitor::informationFunction, om, _1, _2));

	  // Do the actual optimization
	  pop_par.optimize();

	  // Retrieve best individual and make it output a result file
	  boost::shared_ptr<GExternalEvaluator> bestIndividual = pop_par.getBestIndividual<GExternalEvaluator>();
	  bestIndividual->printResult();
	}
	else if(parallelizationMode==2) { // execution in networked mode
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
			pop_broker.setSortingScheme(sortingScheme);
			pop_broker.setMaximize(maximize); // Specifies whether the program should do maximization or minimization

			// Register the monitor with the population. boost::bind knows how to handle a shared_ptr.
			pop_broker.registerInfoFunction(boost::bind(&optimizationMonitor::informationFunction, om, _1, _2));

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
