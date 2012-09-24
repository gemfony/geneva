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
  duplicationScheme rScheme;
  std::size_t parDim;
  double minVar;
  double maxVar;
  sortingMode smode;
  boost::uint32_t nProcessingUnits;
  solverFunction df;
  boost::uint32_t adaptionThreshold;
  double sigma;
  double sigmaSigma;
  double minSigma;
  double maxSigma;
  double adProb;

  if(!parseCommandLine(argc, argv,
		       configFile,
		       parallelizationMode)
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

  // Initialize the Geneva library
  Geneva::init();

  // Random numbers are our most valuable good. Set the number of threads
  GRANDOMFACTORY->setNProducerThreads(nProducerThreads);

  //***************************************************************************
  // Create a factory for GFunctionIndividual objects and perform
  // any necessary initial work.
  GFunctionIndividualFactory gfi("./GFunctionIndividual.json");
  gfi.init();

  // Create the first set of parent individuals. Initialization of parameters is done randomly.
  std::vector<boost::shared_ptr<GParameterSet> > parentIndividuals;
  for(std::size_t p = 0 ; p<nParents; p++) {
	  boost::shared_ptr<GParameterSet> functionIndividual_ptr = gfi();

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

	  parentIndividuals.push_back(functionIndividual_ptr);
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // We can now start creating populations. We refer to them through the base class

  // This smart pointer will hold the different population types
  boost::shared_ptr<GBaseEA> pop_ptr;

  // Create the actual populations
  switch (parallelizationMode) {
    //-----------------------------------------------------------------------------------------------------
  case 0: // Serial execution
	std::cout << "Using serial execution." << std::endl;
    // Create an empty population
    pop_ptr = boost::shared_ptr<GSerialEA>(new GSerialEA());
    break;

    //-----------------------------------------------------------------------------------------------------
  case 1: // Multi-threaded execution
    {
      std::cout << "Using plain multithreaded execution." << std::endl;
      // Create the multi-threaded population
      boost::shared_ptr<GMultiThreadedEA> popPar_ptr(new GMultiThreadedEA());

      // Population-specific settings
      popPar_ptr->setNThreads(nEvaluationThreads);

      // Assignment to the base pointer
      pop_ptr = popPar_ptr;
    }
    break;

    //-----------------------------------------------------------------------------------------------------
  case 2: // Execution with multi-threaded consumer
    {
    	std::cout << "Using the GBoostThreadConsumerT consumer." << std::endl;
		// Create a consumer and make it known to the global broker
		boost::shared_ptr< GBoostThreadConsumerT<GIndividual> > gbtc(new GBoostThreadConsumerT<GIndividual>());
		gbtc->setMaxThreads(nEvaluationThreads);
		GBROKER(Gem::Geneva::GIndividual)->enrol(gbtc);

		// Create the actual broker population and set parameters as needed
		boost::shared_ptr<GBrokerEA> popBroker_ptr(new GBrokerEA());

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
  pop_ptr->setDefaultPopulationSize(populationSize, nParents);
  pop_ptr->setMaxIteration(maxIterations);
  pop_ptr->setMaxTime(boost::posix_time::minutes(maxMinutes));
  pop_ptr->setReportIteration(reportIteration);
  pop_ptr->setRecombinationMethod(rScheme);
  pop_ptr->setSortingScheme(smode);

  // Do the actual optimization
  pop_ptr->optimize();

  //--------------------------------------------------------------------------------------------
  // Shut down the Geneva library collection
  Geneva::finalize();

  std::cout << "Done ..." << std::endl;
  return 0;
}
