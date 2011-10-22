/**
 * @file GParameterObjectUsagePatterns.cpp
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
#include "geneva/Go2.hpp" // Includes all of the parameter object types

// All GRandom-related code is in the namespace Gem::Hap
using namespace Gem::Geneva;

int main(int argc, char **argv) {
	{ // Usage patterns for the GDoubleObject class
		std::cout << "GDoubleObject:" << std::endl;
		GDoubleObject o1; // Default construction
		o1 = 1.; // Assigning a value
		GDoubleObject o2(o1); // Copy construction
		GDoubleObject o3(2.); // Initialization by value
		GDoubleObject o4(0.,2.); // Random initialization in a given range
		std::cout << o4.value() << std::endl; // Value retrieval
		std::cout << o4.getLowerInitBoundary() << std::endl; // Retrieval of lower init boundary
		std::cout << o4.getUpperInitBoundary() << std::endl; // Retrieval of upper init boundary
		o4 = o1; // Assignment of another object
	}

	{ // Usage patterns for the GConstrainedDoubleObject class
		std::cout << "GConstrainedDoubleObject:" << std::endl;
		GConstrainedDoubleObject o1; // Default construction
		o1 = 1.; // Assigning a value
		GConstrainedDoubleObject o2(o1); // Copy construction
		GConstrainedDoubleObject o3(2.); // Initialization by value
		GConstrainedDoubleObject o4(0.,2.); // Initialization of value boundaries
		std::cout << o4.getLowerBoundary() << std::endl; // Retrieval of lower value boundary
		std::cout << o4.getUpperBoundary() << std::endl; // Retrieval of upper value boundary
		GConstrainedDoubleObject o5(1.,0.,2.); // Initialization with value and value boundaries
		std::cout << o4.value() << " " << o5.value() << std::endl; // Value retrieval; value is set to lower boundary
		o5 = o1; // Assignment of another object
	}

	return 0;
}
