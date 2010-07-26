/**
 * @file GBrokerOverhead.cpp
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
#include <boost/lexical_cast.hpp>

// GenEvA header files go here
#include "hap/GRandom.hpp"
#include "communication/GAsioHelperFunctions.hpp"
#include "communication/GAsioTCPClientT.hpp"
#include "communication/GAsioTCPConsumerT.hpp"
#include "optimization/GBrokerEA.hpp"
#include "optimization/GEvolutionaryAlgorithm.hpp"
#include "optimization/GIndividual.hpp"
#include "optimization/GMultiThreadedEA.hpp"

// The individual that should be optimized
#include "GFunctionIndividual.hpp"
#include "GFunctionIndividualDefines.hpp"

// Declares a function to parse the command line
#include "GArgumentParser.hpp"

using namespace Gem::GenEvA;
using namespace Gem::Communication;
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
  std::size_t populationSize;
  std::size_t nParents;
  boost::uint32_t maxIterations;
  long maxMinutes;
  boost::uint32_t reportIteration;
  recoScheme rScheme;
  std::size_t arraySize;
  std::size_t parDim;
  double minVar;
  double maxVar;
  sortingMode smode;
  boost::uint32_t processingCycles;
  boost::uint32_t waitFactor;
  demoFunction df;
  bool productionPlace;
  boost::uint32_t adaptionThreshold;
  double sigma;
  double sigmaSigma;
  double minSigma;
  double maxSigma;
  double adProb;
  bool returnRegardless;
  Gem::Common::serializationMode serMode;

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
		      maxIterations,
		      maxMinutes,
		      reportIteration,
		      rScheme,
		      smode,
		      arraySize,
		      processingCycles,
			  returnRegardless,
		      waitFactor,
		      productionPlace,
		      adProb,
		      adaptionThreshold,
			  sigma,
			  sigmaSigma,
			  minSigma,
			  maxSigma,
		      parDim,
		      minVar,
		      maxVar,
		      df))
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

  // Create the first set of parent individuals. Initialization of parameters is done randomly.
  std::vector<boost::shared_ptr<GParameterSet> > parentIndividuals;
  for(std::size_t p = 0 ; p<nParents; p++) {
	  boost::shared_ptr<GParameterSet> functionIndividual_ptr;

	  // Set up a single function individual, depending on the expected function type
	  switch(df) {
	  case PARABOLA:
		  functionIndividual_ptr = boost::shared_ptr<GFunctionIndividual<PARABOLA> >(new GFunctionIndividual<PARABOLA>());
		  break;
	  case NOISYPARABOLA:
		  functionIndividual_ptr = boost::shared_ptr<GFunctionIndividual<NOISYPARABOLA> >(new GFunctionIndividual<NOISYPARABOLA>());
		  break;
	  case ROSENBROCK:
		  functionIndividual_ptr = boost::shared_ptr<GFunctionIndividual<ROSENBROCK> >(new GFunctionIndividual<ROSENBROCK>());
		  break;
	  }

	  // Set up a GDoubleCollection with dimension values, each initialized
	  // with a random number in the range [min,max[
	  boost::shared_ptr<GDoubleCollection> gdc_ptr(new GDoubleCollection(parDim,minVar,maxVar));

	  // Set up and register an adaptor for the collection, so it
	  // knows how to be adapted.
	  boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(sigma,sigmaSigma,minSigma,maxSigma));
	  gdga_ptr->setAdaptionThreshold(adaptionThreshold);
	  gdga_ptr->setAdaptionProbability(adProb);
	  if(productionPlace) // Factory means "true"
		  gdga_ptr->setRnrGenerationMode(Gem::Hap::RNRFACTORY);
	  else // Local means "false"
		  gdga_ptr->setRnrGenerationMode(Gem::Hap::RNRLOCAL);
	  gdc_ptr->addAdaptor(gdga_ptr);

	  // Make the parameter collection known to this individual
	  functionIndividual_ptr->push_back(gdc_ptr);
	  functionIndividual_ptr->setProcessingCycles(processingCycles);

	  parentIndividuals.push_back(functionIndividual_ptr);
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
  case 2: // Execution with networked consumer
    {
        // Create a network consumer and enrol it with the broker
        boost::shared_ptr<GAsioTCPConsumerT<GIndividual> > gatc(new GAsioTCPConsumerT<GIndividual>(port));
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
  pop_ptr->setMaxIteration(maxIterations);
  pop_ptr->setMaxTime(boost::posix_time::minutes(maxMinutes));
  pop_ptr->setReportIteration(reportIteration);
  pop_ptr->setRecombinationMethod(rScheme);
  pop_ptr->setSortingScheme(smode);
  
  // Do the actual optimization
  pop_ptr->optimize();

  //--------------------------------------------------------------------------------------------

  std::cout << "Done ..." << std::endl;
  return 0;
}
