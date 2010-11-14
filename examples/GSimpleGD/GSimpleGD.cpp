/**
 * @file GSimpleGD.cpp
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
#include <geneva/GBrokerGD.hpp>
#include <geneva/GGradientDescent.hpp>
#include <geneva/GMultiThreadedGD.hpp>
#include <geneva/GIndividual.hpp>
#include <geneva/GDoubleObject.hpp>

// The individual that should be optimized
#include "geneva-individuals/GFunctionIndividual.hpp"

// Declares a function to parse the command line
#include "GArgumentParser.hpp"

// Holds the optimization monitor
// #include "GInfoFunction.hpp"

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
  boost::uint32_t adaptionThreshold;
  double sigma;
  double sigmaSigma;
  double minSigma;
  double maxSigma;
  double adProb;
  bool returnRegardless;
  Gem::Common::serializationMode serMode;
  boost::uint16_t xDim;
  boost::uint16_t yDim;
  bool followProgress;
  bool trackParentRelations;
  bool drawArrows;
  std::size_t nStartingPoints;
  float finiteStep;
  float stepSize;

  if(
   !parseCommandLine(
		  argc
		  , argv
		  , configFile
		  , parallelizationMode
		  , serverMode
		  , ip
		  , port
		  , serMode
  	)
     ||
   !parseConfigFile(
   		configFile
   		, nProducerThreads
   		, nEvaluationThreads
   		, nStartingPoints
   		, finiteStep
   		, stepSize
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
	)
  )
  { exit(1); }

  //***************************************************************************
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
  /*
  std::ofstream resultSummary("./result.C");
  boost::shared_ptr<optimizationMonitor> om_ptr(new optimizationMonitor(df, resultSummary));
  om_ptr->setDims(xDim, yDim);
  om_ptr->setFollowProgress(followProgress); // Shall we take snapshots ?
  om_ptr->setXExtremes(minVar, maxVar);
  om_ptr->setYExtremes(minVar, maxVar);
  om_ptr->setTrackParentRelations(trackParentRelations);
  om_ptr->setDrawArrows(drawArrows);
  */

  //***************************************************************************

  // Create the first set of parent individuals. Initialization of parameters is done randomly.
  std::vector<boost::shared_ptr<GParameterSet> > parentIndividuals;
  for(std::size_t p = 0 ; p<nStartingPoints; p++) {
	  boost::shared_ptr<GParameterSet> functionIndividual_ptr = GFunctionIndividual<>::getFunctionIndividual(df);

	  // Set up a GDoubleCollection with dimension values, each initialized
	  // with a random number in the range [min,max[
	  boost::shared_ptr<GDoubleCollection> gdc_ptr(new GDoubleCollection(parDim,minVar,maxVar));
	  // Let the GDoubleCollection know about its desired initialization range
	  gdc_ptr->setInitBoundaries(minVar, maxVar);

	  // Note: We do not need to set up an adaptor for gradient descents

	  // Make the parameter collection known to this individual
	  functionIndividual_ptr->push_back(gdc_ptr);
	  functionIndividual_ptr->setProcessingCycles(processingCycles);

	  parentIndividuals.push_back(functionIndividual_ptr);
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // We can now start creating populations. We refer to them through the base class

  // This smart pointer will hold the different population types
  boost::shared_ptr<GGradientDescent> pop_ptr;

  // Create the actual populations
  switch (parallelizationMode) {
  //-----------------------------------------------------------------------------------------------------
  case 0: // Serial execution
	  // Create an empty population
	  pop_ptr = boost::shared_ptr<GGradientDescent>(new GGradientDescent(nStartingPoints, finiteStep, stepSize));
	  break;

	  //-----------------------------------------------------------------------------------------------------
  case 1: // Multi-threaded execution
  {
	  // Create the multi-threaded population
	  boost::shared_ptr<GMultiThreadedGD> popPar_ptr(new GMultiThreadedGD(nStartingPoints, finiteStep, stepSize));

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
	  boost::shared_ptr<GBrokerGD> popBroker_ptr(new GBrokerGD());
	  popBroker_ptr->setWaitFactor(waitFactor);

	  // Assignment to the base pointer
	  pop_ptr = popBroker_ptr;
  }
  break;
  }


  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Now we have suitable populations and can fill them with data

  // Add individuals to the population
  for(std::size_t p = 0 ; p<nStartingPoints; p++) {
    pop_ptr->push_back(parentIndividuals[p]);
  }
 
  // Specify some general population settings
  pop_ptr->setMaxIteration(maxIterations);
  pop_ptr->setMaxTime(boost::posix_time::minutes(maxMinutes));
  pop_ptr->setReportIteration(reportIteration);
  // pop_ptr->registerInfoFunction(boost::bind(&optimizationMonitor::informationFunction, om_ptr, _1, _2));
  
  // Do the actual optimization
  pop_ptr->optimize();

  //--------------------------------------------------------------------------------------------

  std::cout << "Done ..." << std::endl;
  return(0);
}
