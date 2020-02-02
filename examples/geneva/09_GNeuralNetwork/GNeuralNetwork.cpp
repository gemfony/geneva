/**
 * @file GNeuralNetwork.cpp
 */

/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/


// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>
#include <vector>

// Boost header files go here
#include <boost/lexical_cast.hpp>

// Geneva header files go here
#include <geneva/Go2.hpp>

// The individual that should be optimized
#include <geneva-individuals/GNeuralNetworkIndividual.hpp>

using namespace Gem::Geneva;
using namespace Gem::Courtier;
using namespace Gem::Common;
using namespace Gem::Hap;
namespace po = boost::program_options;


/******************************************************************************/
/**
 * The main function.
 */
int main(int argc, char **argv){
	//---------------------------------------------------------------------------
	// Assemble additional command line options to be passed to Go2
	trainingDataType tdt = Gem::Geneva::trainingDataType::TDTNONE;
	std::string trainingDataFile = "./DataSets/hyper_sphere.dat";
	std::string architecture = "2-4-4-1"; // two input nodes, one output node, two hidden layers with 4 nodes each
	std::size_t nDataSets = 2000;
	std::string resultProgram = "trainedNetwork.hpp";
	std::string visualizationFile = "visualization.C";

	// Assemble command line options
	boost::program_options::options_description user_options;
	user_options.add_options() (
		"traininDataType"
		, po::value<trainingDataType>(&tdt)->default_value(Gem::Geneva::trainingDataType::TDTNONE)
		, "Specify training data to be produced: HYPERCUBE=1, HYPERSPHERE=2, AXISCENTRIC=3, SINUS=4"
	)(
		"trainingDataFile"
		, po::value<std::string>(&trainingDataFile)->default_value(trainingDataFile)
		, "The name of the file to which training data should be written"
	)(
		"architecture"
		, po::value<std::string>(&architecture)->default_value(architecture)
		, "The architecture of the network"
	)(
		"nDataSets"
		, po::value<std::size_t>(&nDataSets)->default_value(nDataSets)
		, "The number of data sets to be produced"
	)(
		"resultProgram"
		, po::value<std::string>(&resultProgram)->default_value(resultProgram)
		, "The name of the result program"
	)(
		"visualizationFile"
		, po::value<std::string>(&visualizationFile)->default_value(visualizationFile)
		, "The name of the visualization file"
	);

	//---------------------------------------------------------------------------
	// Create the main optimizer-wrapper
	Go2 go(argc, argv, "./config/Go2.json", user_options);

	//---------------------------------------------------------------------------
	// Produce data sets if we have been asked to do so, then leave
	if(tdt != Gem::Geneva::trainingDataType::TDTNONE) {
		GNeuralNetworkIndividual::createNetworkData(tdt, trainingDataFile, architecture, nDataSets);
		return 0;
	} else { // Store the trainingDataFile in the global options, so they can be accessed by the individuals
		GNeuralNetworkOptions->set("trainingDataFile", trainingDataFile);
	}

	//---------------------------------------------------------------------------
	// Client mode
	if(go.clientMode()) {
		return go.clientRun();
	} // Execution will end here in client mode

	//---------------------------------------------------------------------------

	// Create a factory for GNeuralNetworkIndividual objects and perform
	// any necessary initial work.
	std::shared_ptr<GNeuralNetworkIndividualFactory>
		gnn_ptr(new GNeuralNetworkIndividualFactory("./config/GNeuralNetworkIndividual.json"));

	// Add a content creator so Go2 can generate its own individuals, if necessary
	go.registerContentCreator(gnn_ptr);

	// Perform the actual optimization and retrieve the best individual
	std::shared_ptr<GNeuralNetworkIndividual> p = go.optimize()->getBestGlobalIndividual<GNeuralNetworkIndividual>();

	//---------------------------------------------------------------------------
	// Output the result- and the visualization-program (if available)
	p->writeTrainedNetwork(resultProgram);
	p->writeVisualizationFile(visualizationFile);

	// Terminate Geneva
	return 0;
}

/******************************************************************************/

