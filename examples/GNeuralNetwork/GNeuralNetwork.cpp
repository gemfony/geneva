/**
 * @file GNeuralNetwork.cpp
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
#include <vector>

// Boost header files go here
#include <boost/lexical_cast.hpp>

// Geneva header files go here
#include <geneva/Geneva.hpp>

// The individual that should be optimized
#include <geneva-individuals/GNeuralNetworkIndividual.hpp>

// Declares a function to parse the command line
#include "GArgumentParser.hpp"

using namespace Gem::Geneva;
using namespace Gem::Courtier;
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
		const Gem::Geneva::trainingDataType& t
	  , const std::string& outputFile
	  , const std::vector<std::size_t>& architecture
	  , const std::size_t& nDataSets
) {
	boost::shared_ptr<networkData> nD_ptr;

	switch(t) {
	case Gem::Geneva::HYPERCUBE:
		nD_ptr = GNeuralNetworkIndividual<Gem::Geneva::SIGMOID>::createHyperCubeNetworkData (
				architecture
			  , nDataSets
			  , 0.5 // edgelength
		);

		// Emit a visualization file, suitable for viewing with ROOT (see http://root.cern.ch)
		nD_ptr->toRoot(outputFile + ".C", -0.5, 0.5);

		break;

	case Gem::Geneva::HYPERSPHERE:
		nD_ptr = GNeuralNetworkIndividual<Gem::Geneva::SIGMOID>::createHyperSphereNetworkData (
				architecture
			  , nDataSets
			  , 0.5 // radius
		);

		// Emit a visualization file, suitable for viewing with ROOT (see http://root.cern.ch)
		nD_ptr->toRoot(outputFile + ".C", -1., 1.);

		break;

	case Gem::Geneva::AXISCENTRIC:
		nD_ptr = GNeuralNetworkIndividual<Gem::Geneva::SIGMOID>::createAxisCentricNetworkData (
				architecture
			  , nDataSets
		);

		// Emit a visualization file, suitable for viewing with ROOT (see http://root.cern.ch)
		nD_ptr->toRoot(outputFile + ".C", 0., 1.);

		break;

	case Gem::Geneva::SINUS:
		nD_ptr = GNeuralNetworkIndividual<Gem::Geneva::SIGMOID>::createSinNetworkData (
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
  boost::uint32_t nProcessingUnits;
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
		   , nProcessingUnits
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
  // Initialize Geneva
  Geneva::init();

  // Random numbers are our most valuable good. Set the number of threads
  GRANDOMFACTORY->setNProducerThreads(nProducerThreads);
  GRANDOMFACTORY->setArraySize(arraySize);

  //***************************************************************************
  // Produce data sets if we have been asked to do so, then leave
  if(tdt != Gem::Geneva::TDTNONE) {
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
  case Gem::Geneva::SIGMOID:
	  for(std::size_t p = 0 ; p<nParents; p++) {
		boost::shared_ptr<GNeuralNetworkIndividual<Gem::Geneva::SIGMOID> >
			sigmoid_nn_ptr(new GNeuralNetworkIndividual<Gem::Geneva::SIGMOID>(
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

  case Gem::Geneva::RBF:
	  for(std::size_t p = 0 ; p<nParents; p++) {
		boost::shared_ptr<GNeuralNetworkIndividual<Gem::Geneva::RBF> >
			rbf_nn_ptr(new GNeuralNetworkIndividual<Gem::Geneva::RBF>(
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

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // We can now start creating populations. We refer to them through the base class

  // This smart pointer will hold the different population types
  boost::shared_ptr<GSerialEA> pop_ptr;

  // Create the actual populations
  switch (parallelizationMode) {
    //-----------------------------------------------------------------------------------------------------
  case 0: // Serial execution
    // Create an empty population
    pop_ptr = boost::shared_ptr<GSerialEA>(new GSerialEA());
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
      boost::shared_ptr<GAsioTCPConsumerT<GIndividual> > gatc(new GAsioTCPConsumerT<GIndividual>(port, 0, serMode));
      GBROKER(Gem::Geneva::GIndividual)->enrol(gatc);

      // Create the actual broker population
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
  pop_ptr->setDefaultPopulationSize(populationSize,nParents);
  pop_ptr->setMaxIteration(maxIterations);
  pop_ptr->setMaxTime(boost::posix_time::minutes(maxMinutes));
  pop_ptr->setReportIteration(reportIteration);
  pop_ptr->setRecombinationMethod(rScheme);
  pop_ptr->setSortingScheme(smode);
  pop_ptr->setEmitTerminationReason(true);
  
  // Do the actual optimization
  pop_ptr->optimize();

  //--------------------------------------------------------------------------------------------

  // Output the result- and the visualization-program (if available)
  switch(tF) {
  case Gem::Geneva::SIGMOID:
  {
	  boost::shared_ptr<GNeuralNetworkIndividual<Gem::Geneva::SIGMOID> > best_ptr
		  = pop_ptr->GOptimizationAlgorithmI::getBestIndividual<GNeuralNetworkIndividual<Gem::Geneva::SIGMOID> >();
	  best_ptr->writeTrainedNetwork(resultProgram);
	  best_ptr->writeVisualizationFile(visualizationFile);
  }
  break;

  case Gem::Geneva::RBF:
  {
	  boost::shared_ptr<GNeuralNetworkIndividual<Gem::Geneva::RBF> > best_ptr
		  = pop_ptr->GOptimizationAlgorithmI::getBestIndividual<GNeuralNetworkIndividual<Gem::Geneva::RBF> >();
	  best_ptr->writeTrainedNetwork(resultProgram);
	  best_ptr->writeVisualizationFile(visualizationFile);
  }
  break;
  }

  // Terminate Geneva
  Geneva::finalize();

  // We are done. Leave ...
  std::cout << "Done ..." << std::endl;
  return 0;
}
