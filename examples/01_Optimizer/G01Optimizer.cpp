/**
 * @file G01Optimizer.cpp
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
#include <cmath>
#include <sstream>

// Boost header files go here
#include <boost/lexical_cast.hpp>

// Geneva header files go here
#include <geneva/GO.hpp>

// The individual that should be optimized
#include "GFunctionIndividual.hpp"
#include "GFunctionIndividualDefines.hpp"

using namespace Gem::Geneva;

int main(int argc, char **argv) {
	GO go(argc, argv);

	//---------------------------------------------------------------------
	// Client mode
	if(go.clientRun()) return 0;

	//---------------------------------------------------------------------
	// Server mode

	// Create the first set of individuals.
	for(std::size_t p = 0 ; p<nParents; p++) {
	  boost::shared_ptr<GParameterSet> functionIndividual_ptr = GFunctionIndividual<>::getFunctionIndividual(df);

	  // Make the parameter collection known to this individual
	  go.push_back(functionIndividual_ptr);
	}

	// Perform the actual optimization
	boost::shared_ptr<GParameterSet> functionIndividual_ptr = go.optimize();

	std::cout << "Done ..." << std::endl;
	return 0;
}
