/**
 * @file GParaboloid.cpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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

// Boost header files go here

// Geneva header files go here
#include <geneva/Go.hpp>

// The individual that should be optimized
#include "GParaboloidIndividual.hpp"

using namespace Gem::Geneva;

int main(int argc, char **argv) {
	Go go(argc, argv, "GParaboloid.cfg");

	//---------------------------------------------------------------------
	// Client mode (networked)
	if(go.clientMode()) {
		go.clientRun();
		return 0;
	}

	//---------------------------------------------------------------------
	// Server mode, serial or multi-threaded execution

	// Create a factory for GParaboloidIndividual objects and perform
	// any necessary initial work.
	GParaboloidIndividualFactory gpi("./GParaboloidIndividual.cfg");
	gpi.init();

	// Retrieve an individual from the factory and make it known to the optimizer
	go.push_back(gpi());

	// Perform the actual optimization
	boost::shared_ptr<GParaboloidIndividual> bestIndividual_ptr = go.optimize<GParaboloidIndividual>();

	// Do something with the best result

	std::cout << "Done ..." << std::endl;
	return 0;
}
