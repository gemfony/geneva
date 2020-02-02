/**
 * @file GMetaOptimizer.cpp
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
#include "geneva-individuals/GMetaOptimizerIndividualT.hpp"

using namespace Gem::Geneva;

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
		gfi_ptr(new GFunctionIndividualFactory("./config/GFunctionIndividual.json"));

	// Create a factory for GMetaOptimizerIndividual objects and perform
	// any necessary initial work.
	std::shared_ptr<GMetaOptimizerIndividualFactoryT<GFunctionIndividual>> gmoi_ptr(
		new GMetaOptimizerIndividualFactoryT<GFunctionIndividual>("./config/GMetaOptimizerIndividual.json")
	);

	// Add a pluggable optimization monitor (targeted at evolutionary algorithms) and register
	// it with Go2.
	go.registerPluggableOM(
		std::shared_ptr<GOptOptMonitorT<GFunctionIndividual>>(new GOptOptMonitorT<GFunctionIndividual>("./optProgress.C"))
	);

	// Register the GFunctionIndividualFactory with the meta-optimizer,
	// so it can be handed to the meta-optimization individuals later
	gmoi_ptr->registerIndividualFactory(gfi_ptr);

	// Add a content creator so Go2 can generate its own individuals, if necessary
	go.registerContentCreator(gmoi_ptr);

	// Add a default optimization algorithm to the Go2 object
	go.registerDefaultAlgorithm("ea");

	// Perform the actual optimization
	std::shared_ptr<GMetaOptimizerIndividualT<GFunctionIndividual>> bestIndividual_ptr
		= go.optimize()->getBestGlobalIndividual<GMetaOptimizerIndividualT<GFunctionIndividual>>();

	// Do something with the best result. Here we simply print the result to std-out.
	std::cout
	<< "Best Result was:" << std::endl
	<< *bestIndividual_ptr << std::endl;

	//---------------------------------------------------------------------------
}
