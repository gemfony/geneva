/**
 * @file GExternalEvaluator.cpp
 */

/* Copyright (C) Dr. Ruediger Berlich, Dr. Ariel Garcia, Dr. Sven Gabriel
 * and Karlsruhe Institute of Technology (University of the State of
 * Baden-Wuerttemberg and National Laboratory of the Helmholtz Association)
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
#include <boost/lexical_cast.hpp>

// GenEvA header files go here
#include "GRandom.hpp"
#include "GEvolutionaryAlgorithm.hpp"
#include "GMultiThreadedEA.hpp"
#include "GBrokerEA.hpp"
#include "GIndividualBroker.hpp"
#include "GAsioTCPConsumerT.hpp"
#include "GAsioTCPClient.hpp"
#include "GAsioHelperFunctions.hpp"

// The individual that should be optimized
#include "GExternalEvaluatorIndividual.hpp"

// Declares a function to parse the command line
#include "GArgumentParser.hpp"

// Information retrieval and printing
#include "GInfoFunction.hpp"

using namespace Gem::GenEvA;
using namespace Gem::Util;

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
  std::size_t populationSize;
  std::size_t nParents;
  boost::uint32_t maxGenerations;
  long maxMinutes;
  boost::uint32_t reportGeneration;
  recoScheme rScheme;
  std::size_t arraySize;
  sortingMode smode;
  boost::uint32_t processingCycles;
  bool returnRegardless;
  boost::uint32_t waitFactor;
  std::string program;
  std::string externalArguments;
  boost::uint32_t adaptionThreshold;
  double sigma;
  double sigmaSigma;
  double minSigma;
  double maxSigma;
  boost::uint32_t nEvaluations;
  Gem::GenEvA::dataExchangeMode exchangeMode;
  bool maximize;
  bool productionPlace;
  bool randomFill;
  serializationMode serMode;

  if(!parseCommandLine(argc, argv,
		       configFile,			  
		       parallelizationMode,
		       serverMode,
		       ip,
		       port,
		       serMode)
     ||
     !parseConfigFile(configFile,
		      nProducerThreads,
		      nEvaluationThreads,
		      populationSize,
		      nParents,
		      maxGenerations,
		      maxMinutes,
		      reportGeneration,
		      rScheme,
		      smode,
		      arraySize,
		      processingCycles,
		      returnRegardless,
		      waitFactor,
		      program,
		      externalArguments,
			  adaptionThreshold,
			  sigma,
		      sigmaSigma,
		      minSigma,
		      maxSigma,
		      nEvaluations,
		      exchangeMode,
		      maximize,
		      productionPlace,
		      randomFill))
    {
	  std::cout << "Error parsing the command line or the configuration file. Leaving ..." << std::endl;
	  exit(1);
    }

  // Random numbers are our most valuable good. Set the number of threads
  GRANDOMFACTORY->setNProducerThreads(nProducerThreads);
  GRANDOMFACTORY->setArraySize(arraySize);
  
  //***************************************************************************
  // If this is a client in networked mode, we can just start the listener and
  // return when it has finished
  if(parallelizationMode==2 && !serverMode) {
    boost::shared_ptr<GAsioTCPClient> p(new GAsioTCPClient(ip, boost::lexical_cast<std::string>(port)));

    p->setMaxStalls(0); // An infinite number of stalled data retrievals
    p->setMaxConnectionAttempts(100); // Up to 100 failed connection attempts

    // Possibly prevent return of unsuccessful adaption attempts to the server
    p->returnResultIfUnsuccessful(returnRegardless);

    // Start the actual processing loop
    p->run();

    return 0;
  }
  //***************************************************************************

  // Create an instance of our optimization monitor, telling it to output information in given intervals
  std::ofstream resultSummary("./result.C");
  boost::shared_ptr<optimizationMonitor> om(new optimizationMonitor(nParents, resultSummary));

  // Tell the evaluation program to do any initial work
  GExternalEvaluatorIndividual::initialize(program, externalArguments);

  // Create the first set of parent individuals. Initialization of parameters is done randomly.
  std::vector<boost::shared_ptr<GExternalEvaluatorIndividual> > parentIndividuals;
  for(std::size_t p = 0 ; p<nParents; p++) {
	// Create a number of adaptors to be used in the individual
	boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(sigma,sigmaSigma,minSigma,maxSigma));
	boost::shared_ptr<GInt32FlipAdaptor> gifa_ptr(new GInt32FlipAdaptor());
	boost::shared_ptr<GBooleanAdaptor> gba_ptr(new GBooleanAdaptor());

	// Set the adaption threshold
	gdga_ptr->setAdaptionThreshold(adaptionThreshold);
	gifa_ptr->setAdaptionThreshold(adaptionThreshold);
	gba_ptr->setAdaptionThreshold(adaptionThreshold);

	// Check whether random numbers should be produced locally or in the factory
	if(productionPlace) { // Factory means "true"
		gdga_ptr->setRnrGenerationMode(Gem::Util::RNRFACTORY);
		gifa_ptr->setRnrGenerationMode(Gem::Util::RNRFACTORY);
		gba_ptr->setRnrGenerationMode(Gem::Util::RNRFACTORY);
	}
	else {
		gdga_ptr->setRnrGenerationMode(Gem::Util::RNRLOCAL);
		gifa_ptr->setRnrGenerationMode(Gem::Util::RNRLOCAL);
		gba_ptr->setRnrGenerationMode(Gem::Util::RNRLOCAL);
	}

	// Create an initial individual (it will get the necessary information
	// from the external executable)
	boost::shared_ptr<GExternalEvaluatorIndividual> gev_ptr(
			new GExternalEvaluatorIndividual(
					program,
					externalArguments,
					p==0?randomFill:true, // Allow choice whether the first parent is filled with random data or not
					exchangeMode,
					gdga_ptr,
					gifa_ptr,
					gba_ptr
			)
	);

	// Make each external program evaluate a number of data sets, if nEvaluations > 1
	gev_ptr->setNEvaluations(nEvaluations);

	// Set the desired maximization/minimization mode
	gev_ptr->setMaximize(maximize);

	// Set the amount of processing cycles used in a remote individual
    gev_ptr->setProcessingCycles(processingCycles);

    // Store the newly created individual
    parentIndividuals.push_back(gev_ptr);
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // We can now start creating populations. We refer to them through the base class

  // This smart pointer will hold the different population types
  boost::shared_ptr<GEvolutionaryAlgorithm> pop_ptr;

  // Create the actual populations
  switch (parallelizationMode) {
    //-----------------------------------------------------------------------------------------------------
  case 0: // Serial execution
    // Create an empty population
    pop_ptr = boost::shared_ptr<GEvolutionaryAlgorithm>(new GEvolutionaryAlgorithm());
    break;

    //-----------------------------------------------------------------------------------------------------
  case 1: // Multi-threaded execution
    {
      // Create the multi-threaded population
      boost::shared_ptr<GMultiThreadedEA> popPar_ptr(new GMultiThreadedEA());

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
      boost::shared_ptr<GAsioTCPConsumerT<GIndividual> > gatc(new GAsioTCPConsumerT<GIndividual>(port, 0));
      gatc->setSerializationMode(serMode);
      GINDIVIDUALBROKER->enrol(gatc);

      // Create the actual broker population
      boost::shared_ptr<GBrokerEA> popBroker_ptr(new GBrokerEA());
      popBroker_ptr->setWaitFactor(waitFactor);

      // Assignment to the base pointer
      pop_ptr = popBroker_ptr;
    }
    break;
  }


  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Now we have suitable populations and can fill them with data

  // Add individuals to the population
  for(std::size_t p = 0 ; p<nParents; p++) {
    pop_ptr->push_back(parentIndividuals[p]);
  }
 
  // Specify some general population settings
  pop_ptr->setDefaultPopulationSize(populationSize,nParents);
  pop_ptr->setMaxIteration(maxGenerations);
  pop_ptr->setMaxTime(boost::posix_time::minutes(maxMinutes));
  pop_ptr->setReportIteration(reportGeneration);
  pop_ptr->setRecombinationMethod(rScheme);
  pop_ptr->setSortingScheme(smode);
  pop_ptr->registerInfoFunction(boost::bind(&optimizationMonitor::informationFunction, om, _1, _2));
  pop_ptr->setMaximize(maximize);
  
  // Do the actual optimization
  pop_ptr->optimize();

  //--------------------------------------------------------------------------------------------

  // Make sure we close the result file
  resultSummary.close();

  // Tell the evaluation program to perform any necessary final work
  GExternalEvaluatorIndividual::finalize(program, externalArguments);

  std::cout << "Done ..." << std::endl;
  return 0;
}
