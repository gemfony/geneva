/**
 * @file GExternalEvaluator.cpp
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
#include "geneva-individuals/GExternalEvaluatorIndividual.hpp"

using namespace Gem::Geneva;

int main(int argc, char **argv) {
	Go2 go(argc, argv, "./config/Go2.json");

	//---------------------------------------------------------------------
	// Client mode
	if(go.clientMode()) {
		return go.clientRun();
	} // Execution will end here in client mode

	//---------------------------------------------------------------------

	// Create a factory for GExternalEvaluatorIndividual objects and perform
	// any necessary initial work.
	std::shared_ptr<GExternalEvaluatorIndividualFactory>
		geei_ptr(new GExternalEvaluatorIndividualFactory("./config/GExternalEvaluatorIndividual.json"));

	// Add a content creator so Go2 can generate its own individuals, if necessary^
	go.registerContentCreator(geei_ptr);

	// Add a default optimization algorithm to the Go2 object
	go.registerDefaultAlgorithm("ea");

	// Perform the actual optimization
	std::shared_ptr<GExternalEvaluatorIndividual> p = go.optimize()->getBestGlobalIndividual<GExternalEvaluatorIndividual>();

	// Extract the best individuals found
	std::vector<std::shared_ptr<GExternalEvaluatorIndividual>> bestInds
		= go.getBestGlobalIndividuals<GExternalEvaluatorIndividual>();

	// Note that the "archive" call is specific to the GExternalEvaluatorIndividual
	geei_ptr->archive(bestInds);

	// The GTaoExternalEvaluatorIndividualFactory will, upon its deletion at the end
	// of this function, call the external evaluator with the --finalize switch

	//---------------------------------------------------------------------------
}
