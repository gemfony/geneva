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
#include "GDataExchange.hpp"

using namespace std;
using namespace Gem::Util;

const std::size_t PARABOLADIM=1000;
const std::size_t POPSIZE=100;
const std::size_T NPARENTS=5;

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

	// Check whether we've been asked to emit the parameter structure
	if(writeTemplate) {
		// Create a vector of double values. This will serve as the
		// starting point for the optimization
		std::vector<double> dParm;
		for(std::size_t i=0; i<PARABOLADIM; i++) dParm.push_back(100.);

		// Create a GPopulationData object
		GPopulationData popData;
		popData.setPopulationSize(POPSITE, NPARENTS);

		// Create a single GIndividualData object
		boost::shared_ptr<GIndividualData> indDataPtr(new GIndividualData());
		// Add the parabola data
		indDataPtr->appendArray(dParm);
		// Add the object to the population data
		popData.appendIndividual(indDataPtr);

		// Write the data to file and leave
		if(!popData.saveToFile(paramFile)) exit(1);
		else exit(0);
	}

	// O.k., at this point we are sure we need to load an individual's
	// data from a parameter file.
	GIndividualData indData;
	if(!indData.loadFromFile(paramFile)) exit(1);

	// Check that we have the correct number of double arrays (1 in this case)
	std::size_t nArrays=indData.numberOfDoubleArrays();
	if(nArrays != 1) {
		std::cerr << "Error: Retrieved invalid number of double arrays: " << nArrays << std::endl;
		exit(1);
	}

	// Load the double array
	const std::vector<double>& dParmRef = indData.d_at(0);
	std::vector<double>::const_iterator cit;

	// If our mission is to write out the result, do so and leave
	if(writeResult) {
		// Say what we're doing
		std::cout << "And the result is " << std::endl << std::endl;

		// Loop over all parameters in the array
		for(cit=dParmRef.begin(); cit!=dParmRef.end(); ++cit) std::cout << *cit << std::endl;

		// Leave
		exit(0);
	}

	// At this point the only thing left to do is to calculate the result
	double result = 0.;
	for(cit=dParmRef.begin(); cit!=dParmRef.end(); ++cit) result += pow(dParm[i], 2);

	// Create a suitable GResultData object
	GResultData resultData(result);
	if(!resultData.saveToFile(paramFile)) exit(1);
	else exit(0);

	return 0;
}
