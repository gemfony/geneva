/**
 * @file evaluator.cpp
 */

/*
 * Copyright (C) 2009 Dr. Ruediger Berlich
 * Copyright (C) 2009 Forschungszentrum Karlsruhe GmbH
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

/*
 * This program performs a value calculation for parameters that have been
 * handed to it by the Geneva library. It serves as an example on how it is
 * possible to use external evaluation programs with the Geneva library.
 */

// standard headers go here
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>

// Boost headers go here

// Geneva headers go here
#include "commandLineParser.hpp"

const long MAXGEN=2000;
const std::size_t PARABOLADIM=1000;
const std::size_t POPSIZE=100;
const std::size_t NPARENTS=5;

int main(int argc, char **argv)
{
	std::string paramfile;
	bool init, finalize, templ, result, eval;

	// Parse the command line
	if(!parseCommandLine(argc, argv,
			  init,
			  finalize,
			  paramfile,
		      templ,
		      result))
    { exit(1); }

	if(init) {
		// Perform initialization code
		std::cout << "Initializing ..." << std::endl;
		return 0;
	}

	if(finalize) {
		// Perform finalization code
		std::cout << "Finalizing ..." << std::endl;
		return 0;
	}

	if(paramfile != "unknown" && !paramfile.empty()) {
		/*
		 * We have been asked to write out a template file. It has the following format:
		 * Number of generations (int)
		 * Desired population size (int)
		 * Number of parents (int)
		 * Structure of parent:
		 *   Number n of double values (int)
		 *   n initial values (double)
		 */
		if(templ) {
			// Open the parameter file
			std::ofstream ofstr(paramfile.c_str()); // will automatically truncate the file

			if(!ofstr.good()){
				std::cerr << "Error: output stream is in a bad state" << std::endl;
				ofstr.close();
				return 1;
			}

			// Write out the required information
			ofstr << MAXGEN << std::endl
				  << POPSIZE << std::endl
				  << NPARENTS << std::endl
				  << PARABOLADIM << std::endl;

			for(std::size_t i = 0; i<PARABOLADIM; i++) {
				ofstr << 1.25 << std::endl;
			}

			// Make sure the output file is closed.
			ofstr.close();
			return 0;
		}

		if(result) { // We have been asked to write out a result file for the parameter file
			std::cout << "Writing out result file" << std::endl;
			return 0;
		}

		// O.k. so we are supposed to evaluate the content of the parameter file.
		// Its structure is as follows:
		// Number n of double values (int)
		// n double values
		std::ifstream paramStream(paramfile.c_str(), std::ios_base::in);
		if(!paramStream) {
			std::cerr << "Error: Could not open file " << paramfile << ". Leaving ..." << std::endl;
			return 1;
		}

		std::size_t pDim = 0;
		paramStream >> pDim;
		if(pDim != PARABOLADIM) {
			std::cerr << "Error: Invalid dimensions: " << pDim << " " << PARABOLADIM << std::endl;
			return 1;
		}

		std::vector<double> parabola;
		for(std::size_t i=0; i<PARABOLADIM; i++) {
			double tmp;
			paramStream >> tmp;
			parabola.push_back(tmp);
		}

		// Now we can do the actual calculation
		std::vector<double>::iterator it;
		double result = 0.;
		for(it=parabola.begin(); it!=parabola.end(); ++it) result += std::pow(*it,2);

		// Make sure the stream is closed
		paramStream.close();

		// Finally we write the result to the targer file
		// Open the parameter file
		std::ofstream resfile(paramfile.c_str()); // will automatically truncate the file

		if(!resfile.good()){
			std::cerr << "Error: output stream is in a bad state" << std::endl;
			resfile.close();
			return 1;
		}

		// Write out the required information and close the file
		resfile << result << std::endl;
		resfile.close();

		return 0;
	}
	else {
		std::cerr << "Error: you did not specify a valid parameter file" << std::endl;
		return 1;
	}

	return 0; // make the compiler happy
}

