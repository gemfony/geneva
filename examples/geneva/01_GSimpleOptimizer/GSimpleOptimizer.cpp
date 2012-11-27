/**
 * @file GSimpleOptimizer.cpp
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

// Boost header files go here

// Geneva header files go here
#include <geneva/Go2.hpp>

// The individual that should be optimized
#include "geneva-individuals/GFunctionIndividual.hpp"

using namespace Gem::Geneva;

int main(int argc, char **argv) {
	Go2::init();
	Go2 go(argc, argv, "./config/Go2.json");

	//---------------------------------------------------------------------
	// Client mode
	if(go.clientMode()) {
		return go.clientRun();
	}

	//---------------------------------------------------------------------
	// Server mode, serial or multi-threaded execution

	// Add an evolutionary algorithm in multi-threaded mode
	GEvolutionaryAlgorithmFactory ea("./config/GEvolutionaryAlgorithm.json", PARMODE_MULTITHREADED);

	// Retrieve an evolutionary algorithm
	boost::shared_ptr<GBaseEA> ea_ptr = ea();

	// Retrieve the default population size
	std::size_t popSize = ea_ptr->getDefaultPopulationSize();

   // Create a factory for GFunctionIndividual objects and perform
   // any necessary initial work.
   GFunctionIndividualFactory gfi("./config/GFunctionIndividual.json");

	// Add the desired number of individuals
	for(std::size_t i=0; i<popSize; i++) {
	   // Make an individual known to the optimizer
	   boost::shared_ptr<GParameterSet> ind_ptr = gfi();
	   ind_ptr->randomInit();
	   go.push_back(ind_ptr);
	}

	// Add the algorithm to the Go2 object
	go & ea_ptr;

	// Perform the actual optimization
	boost::shared_ptr<GFunctionIndividual> p = go.optimize<GFunctionIndividual>();

	// Terminate
	return Go2::finalize();
}
