/**
 * @file evaluator.cpp
 */

/*
 * Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
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
 * This file complements the GExternalProgram example. It represents the
 * fitness calculation triggered by the corresponding function. If you follow
 * the same pattern as in this file, you should be able to use the GExternalProgram
 * example without modification in order to run optimizations with an external
 * program.
 */

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>

#include "commandLineParser.hpp"

using namespace std;

const std::size_t PARABOLADIM=1000;

void writeTemplateFile(const std::string& fileName,
		               const std::vector<std::vector<double> >& dParm,
		               const std::vector<std::vector<long> >& lParm,
		               const std::vector<std::vector<bool> >& bParm)
{

}

int main(int argc, char **argv)
{
	std::vector<double> dParm;
	std::size_t nDArrays, nBArrays, nLArrays;
	double current;

	std::string paramfile;
	bool writeTemplate, writeResult, verbose;

	// Parse the command line
	if(!parseCommandLine(argc, argv,
						 paramfile,
						 writeTemplate,
						 writeResult,
						 verbose))
    { exit(1); }

	std::string fname = argv[1];
	std::ifstream parameters(fname.c_str(), ios_base::in | ios_base::binary);

	// Check whether we've been asked to emit the parameter structure
	if(writeTemplate) {
		nDArrays = 1; // The number of double parameters
		nLArrays = 0; // The number of long arrays
		nBArrays = 0; // The number of boolean arrays

		// Open the parameter file
		std::ofstream templateFile(fname.c_str(), ios_base::out | ios_base::binary | ios_base::trunc);
		if(!templateFile) { // Have we successfully opened the file ?
			std::cerr << "Error: Could not emit the parameter structure" << std::endl;
			exit(1);
		}

		// First emit the number of double arrays
		templateFile.write((char *)&nDArrays, sizeof(std::size_t));

		// Then emit the size of each double array in sequence


		// Emit the number of boolean arrays
		// Then emit the size of each boolean array in sequence
		// Emit the number of long arrays
		// Then emit the size of each long array in sequence

		// Close the parameter file
		templateFile.close();

		// Leave - nothing else to do
		exit(0);
	}

	parameters.read((char *)&nDParm, sizeof(std::size_t));
	for(std::size_t i=0; i<nDParm; i++){
		parameters.read((char *)&current, sizeof(double));
		dParm.push_back(current);
	}

  parameters.close();

  double result;
  for(unsigned int i=0; i<dParm.size(); i++){
    result += pow(dParm[i], 2);
  }

  std::ofstream resultFile(fname.c_str(), ios_base::out | ios_base::binary | ios_base::trunc);
  resultFile.write((char *)&result, sizeof(double));
  resultFile.close();

  return 0;
}
