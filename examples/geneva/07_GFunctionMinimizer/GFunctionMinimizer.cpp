/**
 * @file GFunctionMinimizer.cpp
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

// Boost header files go here

// Geneva header files go here
#include <geneva/Go2.hpp>

// The individual that should be optimized
#include "GFMinIndividual.hpp"

using namespace Gem::Geneva;
namespace po = boost::program_options;

int main(int argc, char **argv) {
	bool printBest = false;
	boost::program_options::options_description user_options;
	user_options.add_options() (
		"print"
		, po::value<bool>(&printBest)->implicit_value(true)->default_value(false) // This allows you say both --print and --print=true
		, "Switches on printing of the best result"
	);

	Go2 go(argc, argv, "./config/Go2.json", user_options);

	//---------------------------------------------------------------------
	// Client mode
	if(go.clientMode()) {
		return go.clientRun();
	}

	//---------------------------------------------------------------------
	// Server mode, serial or multi-threaded execution

	// Create a factory for GFMinIndividual objects and perform
	// any necessary initial work.
	GFMinIndividualFactory gfi("./config/GFMinIndividual.json");

	// Retrieve an individual from the factory and make it known to the optimizer
	go.push_back(gfi());

	// Create an evolutionary algorithm in multi-threaded mode
	GEvolutionaryAlgorithmFactory ea("./config/GEvolutionaryAlgorithm.json");
	std::shared_ptr<GEvolutionaryAlgorithm> ea_ptr = ea.get<GEvolutionaryAlgorithm>();

	// Add the algorithm to the Go2 object. Note that the multi-threaded ea variant will
	// be executed first, regardless of what other algorithms you might have specified
	// on the command line. This example simply shows a different way of adding
	// optimization algorithms to Go2.
	go & ea_ptr;

	// Perform the actual optimization
	std::shared_ptr<GFMinIndividual> bestIndividual_ptr = go.optimize()->getBestGlobalIndividual<GFMinIndividual>();

	// Do something with the best result. Here: Simply print it, if requested
	if(printBest) {
		std::cout
		<< "Best individual found has values" << std::endl
		<< bestIndividual_ptr << std::endl;
	}
}
