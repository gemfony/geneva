/**
 * @file GResetToOptimizationStart.cpp
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
#include "geneva/Go2.hpp"

// The individual that should be optimized
#include "geneva-individuals/GFunctionIndividual.hpp"

using namespace Gem::Geneva;

const std::size_t NRESETS = 3; // The number of resets performed for each algorithm

/**
 * This manual test checks the functionality of the
 * resetToOptimizationStart() function of optimization algorithms.
 * We simply use the Go2 class so we may use its command line
 * parsing ability to retrieve algorithms.
 */
int main(int argc, char **argv) {
	Go2 go(argc, argv, "./config/Go2.json");

	//---------------------------------------------------------------------------
	// Client mode
	if(go.clientMode()) {
		return go.clientRun();
	} // Execution will end here in client mode

	//---------------------------------------------------------------------------
	// As we are dealing with a server, register a signal handler that allows us
	// to interrupt execution "on the run"
	signal(G_SIGHUP, GObject::sigHupHandler);

	//---------------------------------------------------------------------------
	// Create a factory for GFunctionIndividual objects and perform
	// any necessary initial work.
	std::shared_ptr<GFunctionIndividualFactory>
		gfif_ptr(new GFunctionIndividualFactory("./config/GFunctionIndividual.json"));

	// Check that algorithms were indeed registered and fix, if this was not the case.
	if(go.getNAlgorithms() == 0) {
		glogger
			<< "In GResetToOptimizationStart:" << std::endl
			<< "No algorithms were registered." << std::endl
			<< "We will add an Evolutionary Algorithm" << std::endl
		   << GLOGGING;

		go & "ea";
	}

	// Retrieve the registered algorithms
	auto algorithms_cnt = go.getRegisteredAlgorithms();

	std::cout << "Got algorithms_cnt of size " << algorithms_cnt.size() << std::endl;

	for(auto const & alg_ptr: algorithms_cnt) {
		for(std::size_t resetCounter=0; resetCounter<NRESETS; resetCounter++) {
			alg_ptr->push_back(gfif_ptr->get());
			alg_ptr->optimize();

			if(resetCounter < NRESETS) {
				alg_ptr->resetToOptimizationStart();
				std::cout << "Algorithm was reset" << std::endl;
			}
		}
	}

	std::cout << "Done ..." << std::endl;
	return(0);
}
