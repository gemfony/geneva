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
#include <geneva/GPluggableOptimizationMonitors.hpp>

// The individual that should be optimized
#include "GStarterIndividual.hpp"

using namespace Gem::Geneva;

int main(int argc, char **argv) {
	Go2 go(argc, argv, "./config/Go2.json");

	//---------------------------------------------------------------------
	// Client mode
	if(go.clientMode()) {
		return go.clientRun();
	}

	//---------------------------------------------------------------------
	// Server mode, serial or multi-threaded execution

	// Add a "pluggable optimization monitor" to Go2. This particular monitor will log
	// solutions that were found into the file allLog.txt.
	std::shared_ptr<GAllSolutionFileLogger>
		allSolutionLogger_ptr(new GAllSolutionFileLogger("allLog.txt"));
	allSolutionLogger_ptr->setPrintInitial(); // Also log the initial population, prior to optimization
	allSolutionLogger_ptr->setShowIterationBoundaries(); // Facilitates reading of the log file

	go.registerPluggableOM(allSolutionLogger_ptr);

	// Create a factory for GStarterIndividual objects and perform
	// any necessary initial work.
	std::shared_ptr<GStarterIndividualFactory> gsif_ptr(
		new GStarterIndividualFactory("./config/GStarterIndividual.json")
	);

	// Add a content creator so Go2 can generate its own individuals, if necessary
	go.registerContentCreator(gsif_ptr);

	// Perform the actual optimization
	std::shared_ptr<GStarterIndividual> bestIndividual_ptr = go.optimize()->getBestGlobalIndividual<GStarterIndividual>();

	// Do something with the best result. Here we simply print the result to stdout.
	std::cout << bestIndividual_ptr << std::endl;
}
