/**
 * @file GMultiCriterionParabola.cpp
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
#include <geneva/Go2.hpp>

// The individual that should be optimized
#include "GMultiCriterionParabolaIndividual.hpp"

using namespace Gem::Geneva;

int main(int argc, char **argv) {
	Go2 go(argc, argv, "./config/Go2.json");

	//---------------------------------------------------------------------
	// Client mode (networked)
	if(go.clientMode()) {
		go.clientRun();
		return 0;
	}

	//---------------------------------------------------------------------
	// Server mode, serial or multi-threaded execution

	// Create a factory for GMultiCriterionParabolaIndividual objects and perform
	// any necessary initial work.
	std::shared_ptr<GMultiCriterionParabolaIndividualFactory>
		gpi_ptr(new GMultiCriterionParabolaIndividualFactory("./config/GMultiCriterionParabolaIndividual.json"));

	// Add a content creator so Go2 can generate its own individuals, if necessary
	go.registerContentCreator(gpi_ptr);

	// Add a default optimization algorithm to the Go2 object.
	// Note that this is the only algorithm that currently can handle multi-criterion optimization
	go.registerDefaultAlgorithm("ea");

	// Perform the actual optimization
	std::shared_ptr<GMultiCriterionParabolaIndividual> bestIndividual_ptr = go.optimize<GMultiCriterionParabolaIndividual>();

	// Do something with the best result
	std::cout << bestIndividual_ptr << std::endl;
}
