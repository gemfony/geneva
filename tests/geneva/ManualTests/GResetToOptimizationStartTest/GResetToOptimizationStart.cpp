/**
 * @file GResetToOptimizationStart.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
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
 * http://www.gemfony.eu .
 */

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
		gfi_ptr(new GFunctionIndividualFactory("./config/GFunctionIndividual.json"));

	// Check that algorithms were indeed registered
	if(go.getNAlgorithms() < 1) {
		glogger
		<< "In GResetToOptimizationStart-test: Error!" << std::endl
	   << "No algorithms were registered." << std::endl
		<< GEXCEPTION;
	}

	// Retrieve the registered algorithms
	auto algorithms_vec = go.getRegisteredAlgorithms();

	for(auto alg: algorithms_vec) {
		for(std::size_t resetCounter=0; resetCounter<NRESETS; resetCounter++) {
			alg->push_back(gfi_ptr->get());
			alg->optimize();
			alg->resetToOptimizationStart();

			std::cout << "Algorithm was reset" << std::endl;
		}
	}

	std::cout << "Done ..." << std::endl;
	return(0);
}
