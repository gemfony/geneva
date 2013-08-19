/**
 * @file GFunctionMinimizer.cpp
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
#include "GFMinIndividual.hpp"
#include "GSigmaMonitor.hpp"

using namespace Gem::Geneva;
namespace po = boost::program_options;

int main(int argc, char **argv) {
   bool printBest = false;
   std::vector<boost::shared_ptr<po::option_description> > od;
   boost::shared_ptr<po::option_description> print_option(
      new po::option_description(
         "print"
         , po::value<bool>(&printBest)->default_value(false)
         , "Switches on printing of the best result"
      )
   );
   od.push_back(print_option);

	Go2 go(argc, argv, "./config/Go2.json", od);

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

	// Register an optimization monitor for evolutionary algorithms. This allows the
	// GEvolutionaryAlgorithmFactory to find suitable monitors in the global store.
	GOAMonitorStore->setOnce("ea", boost::shared_ptr<GSigmaMonitor> (new GSigmaMonitor("./sigmaProgress.C")));

	// Create an evolutionary algorithm in multi-threaded mode
	GEvolutionaryAlgorithmFactory ea("./config/GEvolutionaryAlgorithm.json", EXECMODE_MULTITHREADED);
	boost::shared_ptr<GBaseEA> ea_ptr = ea.get<GBaseEA>();

	// Add the algorithm to the Go2 object. Note that the multi-threaded variant will
	// be executed first, regardless of what other algorithms you might have specified
	// on the command line. This example simply shows a different way of adding
	// optimization algorithms to Go2.
	go & ea_ptr;

	// Perform the actual optimization
	boost::shared_ptr<GFMinIndividual> bestIndividual_ptr = go.optimize<GFMinIndividual>();

	// Do something with the best result. Here: Simply print it, if requested
	if(printBest) {
	   std::cout
	   << "Best individual found has values" << std::endl
	   << bestIndividual_ptr << std::endl;
	}
}
