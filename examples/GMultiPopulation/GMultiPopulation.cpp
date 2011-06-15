/**
 * @file GBrokerOverhead.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */


// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>

// Boost header files go here
#include <boost/lexical_cast.hpp>

// Geneva header files go here
#include "geneva/Geneva.hpp"

// The individual that should be optimized
#include "geneva-individuals/GFunctionIndividual.hpp"

// Declares a function to parse the command line
#include "GArgumentParser.hpp"

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
  std::size_t populationSizeSuper;
  std::size_t nParentsSuper;
  boost::uint32_t maxIterationsSuper;
  long maxMinutesSuper;
  boost::uint32_t reportIterationSuper;
  recoScheme rSchemeSuper;
  sortingMode smodeSuper;
  std::size_t populationSizeSub;
  std::size_t nParentsSub;
  boost::uint32_t maxIterationsSub;
  long maxMinutesSub;
  boost::uint32_t reportIterationSub;
  recoScheme rSchemeSub;
  sortingMode smodeSub;
  std::size_t arraySize;
  std::size_t parDim;
  double minVar;
  double maxVar;
  boost::uint32_t processingCycles;
  bool returnRegardless;
  boost::uint32_t nProcessingUnits;
  demoFunction df;
  boost::uint32_t adaptionThreshold;
  double sigma;
  double sigmaSigma;
  double minSigma;
  double maxSigma;
  double adProb;
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
		      populationSizeSuper,
		      nParentsSuper,
		      maxIterationsSuper,
		      maxMinutesSuper,
		      reportIterationSuper,
		      rSchemeSuper,
		      smodeSuper,
		      populationSizeSub,
		      nParentsSub,
		      maxIterationsSub,
		      maxMinutesSub,
		      reportIterationSub,
		      rSchemeSub,
		      smodeSub,
		      arraySize,
		      processingCycles,
			  returnRegardless,
			  nProcessingUnits,
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

  // Initialize Geneva
  Geneva::init();

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

  // Create the first set of parent populations.
  std::vector<boost::shared_ptr<GEvolutionaryAlgorithm> > parentPopulations;
  for(std::size_t psuper = 0; psuper<nParentsSuper; psuper++) {
	  // This smart pointer holds a parent population. Note that it can be useful
	  // not to parallelize on the level of sub-populations. Using a GBrokerEA here
	  // will likely lead to failure.
	  boost::shared_ptr<GEvolutionaryAlgorithm> sub_pop_ptr
		  = boost::shared_ptr<GEvolutionaryAlgorithm>(new GEvolutionaryAlgorithm());

	  // Create the first set of parent individuals. Initialization of parameters is done randomly.
	  for(std::size_t psub = 0 ; psub<nParentsSub; psub++) {
		  boost::shared_ptr<GParameterSet> functionIndividual_ptr = GFunctionIndividual::getFunctionIndividual(df);

		  // Set up a GDoubleCollection with dimension values, each initialized
		  // with a random number in the range [min,max[
		  boost::shared_ptr<GDoubleCollection> gdc_ptr(new GDoubleCollection(parDim,minVar,maxVar));

		  // Set up and register an adaptor for the collection, so it
		  // knows how to be adapted.
		  boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(sigma,sigmaSigma,minSigma,maxSigma));
		  gdga_ptr->setAdaptionThreshold(adaptionThreshold);
		  gdga_ptr->setAdaptionProbability(adProb);
		  gdc_ptr->addAdaptor(gdga_ptr);

		  // Make the parameter collection known to this individual
		  functionIndividual_ptr->push_back(gdc_ptr);
		  functionIndividual_ptr->setProcessingCycles(processingCycles);

		  sub_pop_ptr->push_back(functionIndividual_ptr);
	  }

	  // Specify some general population settings
	  sub_pop_ptr->setDefaultPopulationSize(populationSizeSub,nParentsSub);
	  sub_pop_ptr->setMaxIteration(maxIterationsSub);
	  sub_pop_ptr->setMaxTime(boost::posix_time::minutes(maxMinutesSub));
	  sub_pop_ptr->setReportIteration(reportIterationSub);
	  sub_pop_ptr->setRecombinationMethod(rSchemeSub);
	  sub_pop_ptr->setSortingScheme(smodeSub);

	  // Add the sub population to the vector
	  parentPopulations.push_back(sub_pop_ptr);
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // We can now start creating populations. We refer to them through the base class

  // This smart pointer will hold the different population types
  boost::shared_ptr<GEvolutionaryAlgorithm> super_pop_ptr;

  // Create the actual populations
  switch (parallelizationMode) {
    //-----------------------------------------------------------------------------------------------------
  case 0: // Serial execution
    // Create an empty population
    super_pop_ptr = boost::shared_ptr<GEvolutionaryAlgorithm>(new GEvolutionaryAlgorithm());
    break;

    //-----------------------------------------------------------------------------------------------------
  case 1: // Multi-threaded execution
    {
      // Create the multi-threaded population
      boost::shared_ptr<GMultiThreadedEA> popPar_ptr(new GMultiThreadedEA());

      // Population-specific settings
      popPar_ptr->setNThreads(nEvaluationThreads);

      // Assignment to the base pointer
      super_pop_ptr = popPar_ptr;
    }
    break;

    //-----------------------------------------------------------------------------------------------------
  case 2: // Networked execution (server-side)
    {
      // Create a network consumer and enrol it with the broker
      boost::shared_ptr<GAsioTCPConsumerT<GIndividual> > gatc(new GAsioTCPConsumerT<GIndividual>(port));
      gatc->setSerializationMode(serMode);
      GBROKER(Gem::Geneva::GIndividual)->enrol(gatc);

      // Create the actual broker population
      boost::shared_ptr<GBrokerEA> popBroker_ptr(new GBrokerEA());
      popBroker_ptr->setNProcessingUnits(nProcessingUnits);

      // Assignment to the base pointer
      super_pop_ptr = popBroker_ptr;
    }
    break;
  }


  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Now we have suitable populations and can fill them with data

  // Add individuals to the population
  for(std::size_t psuper = 0 ; psuper<nParentsSuper; psuper++) {
	  super_pop_ptr->push_back(parentPopulations[psuper]);
  }
 
  // Specify some general population settings
  super_pop_ptr->setDefaultPopulationSize(populationSizeSuper,nParentsSuper);
  super_pop_ptr->setMaxIteration(maxIterationsSuper);
  super_pop_ptr->setMaxTime(boost::posix_time::minutes(maxMinutesSuper));
  super_pop_ptr->setReportIteration(reportIterationSuper);
  super_pop_ptr->setRecombinationMethod(rSchemeSuper);
  super_pop_ptr->setSortingScheme(smodeSuper);
  super_pop_ptr->setEmitTerminationReason(true);
  
  // Do the actual optimization
  super_pop_ptr->optimize();

  //--------------------------------------------------------------------------------------------
  // Terminate Geneva
  Geneva::finalize();

  std::cout << "Done ..." << std::endl;
  return 0;
}
