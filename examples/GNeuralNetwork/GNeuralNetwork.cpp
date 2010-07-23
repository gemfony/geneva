/**
 * @file GStartProject.cpp
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
#include <vector>

// Boost header files go here
#include <boost/lexical_cast.hpp>

// GenEvA header files go here
#include "random/GRandom.hpp"
#include "communication/GAsioHelperFunctions.hpp"
#include "communication/GAsioTCPClientT.hpp"
#include "communication/GAsioTCPConsumerT.hpp"
#include "optimization/GBrokerEA.hpp"
#include "optimization/GEvolutionaryAlgorithm.hpp"
#include "optimization/GIndividual.hpp"
#include "optimization/GMultiThreadedEA.hpp"

// The individual that should be optimized
#include "GNeuralNetworkIndividual.hpp"

// Declares a function to parse the command line
#include "GArgumentParser.hpp"

// Information retrieval and printing
#include "GInfoFunction.hpp"

using namespace Gem::GenEvA;
using namespace Gem::Communication;
using namespace Gem::Hap;

/************************************************************************************************/
/**
 * Creates a data set of the desired type or throws, if that type is not available
 *
 * @param type The type of network data to be created
 * @param outputFile The name of the output training data file
 * @param architecture The desired architecture of the network
 * @param nDataSets The number of data sets to be produced
 */
void createNetworkData(
		const Gem::GenEvA::trainingDataType& t
	  , const std::string& outputFile
	  , const std::vector<std::size_t>& architecture
	  , const std::size_t& nDataSets
) {
	boost::shared_ptr<networkData> nD_ptr;

	switch(t) {
	case Gem::GenEvA::HYPERCUBE:
		nD_ptr = GNeuralNetworkIndividual<Gem::GenEvA::SIGMOID>::createHyperCubeNetworkData (
				architecture
			  , nDataSets
			  , 0.5 // edgelength
		);

		// Emit a visualization file, suitable for viewing with ROOT (see http://root.cern.ch)
		nD_ptr->toRoot(outputFile + ".C", -0.5, 0.5);

		break;

	case Gem::GenEvA::HYPERSPHERE:
		nD_ptr = GNeuralNetworkIndividual<Gem::GenEvA::SIGMOID>::createHyperSphereNetworkData (
				architecture
			  , nDataSets
			  , 0.5 // radius
		);

		// Emit a visualization file, suitable for viewing with ROOT (see http://root.cern.ch)
		nD_ptr->toRoot(outputFile + ".C", -1., 1.);

		break;

	case Gem::GenEvA::AXISCENTRIC:
		nD_ptr = GNeuralNetworkIndividual<Gem::GenEvA::SIGMOID>::createAxisCentricNetworkData (
				architecture
			  , nDataSets
		);

		// Emit a visualization file, suitable for viewing with ROOT (see http://root.cern.ch)
		nD_ptr->toRoot(outputFile + ".C", 0., 1.);

		break;

	case Gem::GenEvA::SINUS:
		nD_ptr = GNeuralNetworkIndividual<Gem::GenEvA::SIGMOID>::createSinNetworkData (
				architecture
			  , nDataSets
		);

		// Emit a visualization file, suitable for viewing with ROOT (see http://root.cern.ch)
		nD_ptr->toRoot(outputFile + ".C", -6., 6.);

		break;

	default:
		{ // Error
			std::ostringstream error;
			error << "In createDataset(): Error!" << std::endl
				  << "Received invalid data type " << t << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}
		break;
	}

	// Write distribution to file
	nD_ptr->saveToDisk(outputFile);
}

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
  sortingMode smode;
  boost::uint32_t processingCycles;
  bool returnRegardless;
  boost::uint32_t waitFactor;
  trainingDataType tdt;
  std::string trainingDataFile;
  std::size_t nDataSets;
  std::vector<std::size_t> architecture;
  std::string trainingInputData;
  std::string resultProgram;
  std::string visualizationFile;
  transferFunction tF;
  double sigma;
  double sigmaSigma;
  double minSigma;
  double maxSigma;
  double adProb;
  Gem::Common::serializationMode serMode;

  //***************************************************************************
  // Parse command line and configuration file
  if(!parseCommandLine(
			  argc, argv
			, configFile
			, parallelizationMode
			, serverMode
			, ip
			, port
		    , serMode
			, tdt
			, trainingDataFile
			, nDataSets
			, architecture
	 )
     ||
     !parseConfigFile(
    		 configFile
		   , nProducerThreads
		   , nEvaluationThreads
		   , populationSize
		   , nParents
		   , maxIterations
		   , maxMinutes
		   , reportIteration
		   , rScheme
		   , smode
		   , arraySize
		   , processingCycles
		   , returnRegardless
		   , waitFactor
		   , tF
		   , trainingInputData
		   , resultProgram
		   , visualizationFile
		   , sigma
		   , sigmaSigma
		   , minSigma
		   , maxSigma
		   , adProb
		   , true
	)
  )
  { exit(1); }

  //***************************************************************************
  // Random numbers are our most valuable good. Set the number of threads
  GRANDOMFACTORY->setNProducerThreads(nProducerThreads);
  GRANDOMFACTORY->setArraySize(arraySize);

  //***************************************************************************
  // Produce data sets if we have been asked to do so, then leave
  if(tdt != Gem::GenEvA::TDTNONE) {
	  createNetworkData(tdt, trainingDataFile, architecture, nDataSets);
	  return 0;
  }
  
  //***************************************************************************
  // If this is a client in networked mode, we can just start the listener and
  // leave when it has finished
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
  std::vector<boost::shared_ptr<GIndividual> > parentIndividuals;
  switch(tF) {
  case Gem::GenEvA::SIGMOID:
	  for(std::size_t p = 0 ; p<nParents; p++) {
		boost::shared_ptr<GNeuralNetworkIndividual<Gem::GenEvA::SIGMOID> >
			sigmoid_nn_ptr(new GNeuralNetworkIndividual<Gem::GenEvA::SIGMOID>(
					  trainingInputData
				    , -1.
				    , 1.
				    , sigma
				    , sigmaSigma
				    , minSigma
				    , maxSigma
				    , adProb
			)
		);
		sigmoid_nn_ptr->setProcessingCycles(processingCycles);

		parentIndividuals.push_back(sigmoid_nn_ptr);
	  }
	  break;

  case Gem::GenEvA::RBF:
	  for(std::size_t p = 0 ; p<nParents; p++) {
		boost::shared_ptr<GNeuralNetworkIndividual<Gem::GenEvA::RBF> >
			rbf_nn_ptr(new GNeuralNetworkIndividual<Gem::GenEvA::RBF>(
					  trainingInputData
					, -1.
					, 1.
					, sigma
					, sigmaSigma
					, minSigma
					, maxSigma
					, adProb
			)
		);
		rbf_nn_ptr->setProcessingCycles(processingCycles);

		parentIndividuals.push_back(rbf_nn_ptr);
	  }
	  break;
  }

  // Create an instance of our optimization monitor, telling it to output information in given intervals
  std::ofstream resultSummary("./result.C");
  boost::shared_ptr<optimizationMonitor> om(new optimizationMonitor(nParents, resultSummary, tF));

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
  pop_ptr->registerInfoFunction(boost::bind(&optimizationMonitor::informationFunction, om, _1, _2));
  pop_ptr->setEmitTerminationReason(true);
  
  // Do the actual optimization
  pop_ptr->optimize();

  //--------------------------------------------------------------------------------------------

  // Make sure we close the result file
  resultSummary.close();

  // Output the result- and the visualization-program (if available)
  switch(tF) {
  case Gem::GenEvA::SIGMOID:
  {
	  boost::shared_ptr<GNeuralNetworkIndividual<Gem::GenEvA::SIGMOID> > best_ptr
		  = pop_ptr->getBestIndividual<GNeuralNetworkIndividual<Gem::GenEvA::SIGMOID> >();
	  best_ptr->writeTrainedNetwork(resultProgram);
	  best_ptr->writeVisualizationFile(visualizationFile);
  }
  break;

  case Gem::GenEvA::RBF:
  {
	  boost::shared_ptr<GNeuralNetworkIndividual<Gem::GenEvA::RBF> > best_ptr
		  = pop_ptr->getBestIndividual<GNeuralNetworkIndividual<Gem::GenEvA::RBF> >();
	  best_ptr->writeTrainedNetwork(resultProgram);
	  best_ptr->writeVisualizationFile(visualizationFile);
  }
  break;
  }

  // We are done. Leave ...
  std::cout << "Done ..." << std::endl;
  return 0;
}
