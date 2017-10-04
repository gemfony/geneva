/**
 * @file GHelperFunctionsTest.cpp
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

// Standard headers go here
#include <iostream>
#include <string>
#include <vector>

// Boost headers go here

// Geneva headers go here
#include "common/GCommonHelperFunctions.hpp"

using namespace Gem::Common;

int main(int argc, char** argv) {
	{ // Test of the parsing of an integer string
		std::string in_single = "1, 2, 3, 4";
		std::vector<unsigned int> result_single;
		result_single = stringToUIntVec(in_single);

		std::vector<unsigned int>::iterator it;
		for(it=result_single.begin(); it!=result_single.end(); ++it) {
			std::cout << *it << " ";
		}
		std::cout << std::endl;
	}

	{ // Test of tuple parsing
		std::string in_tuple  = "(1,2), (3,4 ) , (3    ,  5)";
		std::vector<std::tuple<unsigned int, unsigned int>> result_tuple;
		result_tuple = stringToUIntTupleVec(in_tuple);

		std::vector<std::tuple<unsigned int, unsigned int>>::iterator it;
		for(it=result_tuple.begin(); it!=result_tuple.end(); ++it) {
			std::cout << *it << " ";
		}
		std::cout << std::endl;
	}
}
