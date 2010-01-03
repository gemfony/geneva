/**
 * @file GSimpleBasePopulation.cpp
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

// GenEvA header files go here
#include "GRandom.hpp"
#include "GEvolutionaryAlgorithm.hpp"
#include "GDoubleCollection.hpp"
#include "GParameterSet.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GMultiThreadedEA.hpp"

// The individual that should be optimized
// This is a neural network.
#include "GNeuralNetworkIndividual.hpp"

// Declares a function to parse the command line
#include "GCommandLineParser.hpp"

using namespace Gem::GenEvA;
using namespace Gem::Util;

/************************************************************************************************/
/**
 * The main function. We search for the minimum of a parabola. This example demonstrates the use
 * of the GEvolutionaryAlgorithm class or (at your choice) of the GMultiThreadedEA class. Note that
 * a number of command line options are available. Call the executable with the "-h" switch
 * to get an overview.
 */
int main(int argc, char **argv){
	 std::size_t nPopThreads;
	 std::size_t populationSize, nParents;
	 std::size_t nData, nDim;
	 std::size_t nHiddenLayer1Nodes, nHiddenLayer2Nodes;
	 double radius, randMin, randMax;
	 boost::uint16_t nProducerThreads;
	 boost::uint32_t maxGenerations, reportGeneration;
	 long maxMinutes;
	 bool verbose;
	 std::string resultFile;
	 recoScheme rScheme;

	// Parse the command line
	if(!parseCommandLine(argc, argv,
		  			     nData,
						 nDim,
						 radius,
						 randMin,
						 randMax,
						 nHiddenLayer1Nodes,
						 nHiddenLayer2Nodes,
						 nProducerThreads,
						 nPopThreads,
						 populationSize,
						 nParents,
						 maxGenerations,
						 maxMinutes,
						 reportGeneration,
						 rScheme,
						 resultFile,
						 verbose))
	{ exit(1); }

	// Random numbers are our most valuable good. Set the number of threads
	// that simultaneously produce random numbers.
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);

	// Create training data for the individual
	// boost::shared_ptr<trainingData> p = GNeuralNetworkIndividual::createHyperSphereTrainingData("", nData, nDim, radius);
	boost::shared_ptr<trainingData> p = GNeuralNetworkIndividual::createHyperCubeTrainingData("", nData, nDim, radius);

	// The neural network architecture is currently hard-wired to two hidden layers
	// Note that this is a restriction of this main function only and not of the network individual.
	std::vector<std::size_t> architecture;
	architecture.push_back(nDim); // Input layer needs exactly the same number of nodes as the dimension of the training data
	architecture.push_back(nHiddenLayer1Nodes);
	architecture.push_back(nHiddenLayer2Nodes);
	architecture.push_back(1); // Output layer has just one node (yes/no decision)

	// Set up a single parabola individual
	boost::shared_ptr<GNeuralNetworkIndividual>
		networkIndividual(new GNeuralNetworkIndividual(p, architecture, randMin, randMax));

	// Now we've got our first individual and can create a population.
	// We choose a multi-threaded population here.

	GMultiThreadedEA pop;
	pop.setNThreads(nPopThreads);

	pop.push_back(networkIndividual);

	// Specify some population settings
	pop.setPopulationSize(populationSize,nParents);
	pop.setMaxIteration(maxGenerations);
	pop.setMaxTime(boost::posix_time::minutes(maxMinutes)); // Calculation should be finished after 5 minutes
	pop.setReportIteration(reportGeneration); // Emit information during every generation
	pop.setRecombinationMethod(rScheme); // The best parents have higher chances of survival

	// Do the actual optimization
	pop.optimize();

	// Save the network
	std::cout << "Saving network ..." << std::endl;
	boost::shared_ptr<GNeuralNetworkIndividual> bestIndividual = pop.individual_cast<GNeuralNetworkIndividual>(0);
    bestIndividual->writeTrainedNetwork("trainingResult.hpp", "testNetwork.cpp");

	std::cout << "Done ..." << std::endl;

	return 0;
}
