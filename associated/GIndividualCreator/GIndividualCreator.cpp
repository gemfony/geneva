/**
 * @file GIndividualCreator.cpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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
 */

// Standard header files go here
#include <iostream>
#include <cmath>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <vector>

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cstdint.hpp>
#include <boost/bind.hpp>

// GenEvA header files go here

// Declares a function to parse the command line
#include "GCommandLineParser.hpp"

using namespace Gem::GenEvA;
using namespace Gem::Util;
using namespace Gem::GLogFramework;

/************************************************************************************************/
/**
 * The main function.  This program emits a .cpp and .hpp file for the class name
 * chosen by the user
 */
int main(int argc, char **argv){
	// Variables for the command line parsing
	 std::string className;

	// Parse the command line
	if(!parseCommandLine(argc, argv, className,  verbose))	{ exit(1); }



	std::cout << "Done ..." << std::endl;

	return 0;
}
