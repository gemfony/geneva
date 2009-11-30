/**
 * @file evaluator.cpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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
#include <boost/random.hpp>
#include <boost/date_time.hpp>

// Geneva headers go here
#include "commandLineParser.hpp"
#include "GDataExchange.hpp"

const std::size_t PARABOLADIM=1000;
const double PARABOLAMIN=-100.;
const double PARABOLAMAX=100.;

int main(int argc, char **argv)
{
	Gem::Util::GDataExchange ge;
	boost::uint16_t executionMode, transferMode;
	bool binary=true;
	std::string paramfile;
	std::string identifyer;

	// Parse the command line
	if(!parseCommandLine(argc, argv,
												executionMode,
												paramfile,
												transferMode,
												identifyer))
	{exit(1);}

	switch(transferMode) {
	case 0:
		binary=true;
		break;
	case 1:
		binary=false;
		break;
	default:
		{
			std::cerr << "Error: Invalid transfer mode" << transferMode << std::endl;
			exit(1);
		}
	}

	// See the file commandLineParser.cpp for the available modes
	switch(executionMode) {
	case 1:	// Perform initialization code
		std::cout << "Initializing ...";
		// put your code here
		std::cout << "... done." << std::endl;
		return 0;
		break;

	case 2:	// Perform finalization code
		std::cout << "Finalizing ...";
		// put your code here
		std::cout << " ... done." << std::endl;
		return 0;
		break;

	case 3: // Evaluate
		{
			// Read in the parameter data
			ge.readFromFile(paramfile, binary);

			// Loop over all parameter sets and do the actual calculation
			int i=0;
			do {
				double result = 0.;
				// Loop over all double values and calculate summed squares ("a parabola")
				std::size_t nDoubles = ge.size<double>(); // The amount of doubles in the current data set
				for(std::size_t pos = 0; pos<nDoubles; pos++) result += pow(ge.at<double>(pos), 2);
				ge.setValue(result);
			}
			while(ge.nextDataSet());

			// Write out the results. We only want to write out one (the best) item
			ge.writeToFile(paramfile, binary, 1, true);
		}
		break;

	case 4: // Write out template
		{
			// Here we simply want PARABOLADIM double values with boundaries [PARABOLAMIN:PARABOLAMAX]
			for(std::size_t i=0; i<PARABOLADIM; i++) {
				ge.append(boost::shared_ptr<Gem::Util::GDoubleParameter>(new Gem::Util::GDoubleParameter(100., PARABOLAMIN, PARABOLAMAX)));
			}

			ge.writeToFile(paramfile, binary);
		}
		break;

	case 5: // Write out template, initializing with random variables
		{
			boost::posix_time::ptime t1;
		    boost::uint32_t seed = (uint32_t)t1.time_of_day().total_milliseconds();
			boost::lagged_fibonacci607 lf(seed);

			for(std::size_t i=0; i<PARABOLADIM; i++) {
				ge.append(boost::shared_ptr<Gem::Util::GDoubleParameter>(new Gem::Util::GDoubleParameter(PARABOLAMIN+lf()*(PARABOLAMAX-PARABOLAMIN), PARABOLAMIN, PARABOLAMAX)));
			}

			ge.writeToFile(paramfile, binary);
		}
		break;

	case 6: // Write out the result for a given parameter set
		{
			// Output the identifyer
			if(identifyer != DEFAULTIDENTIFYER) {
				std::cout << "Printing result with identifyer = " << identifyer << std::endl;
			}

			// Read in the parameter data
			// ge.readFromFile(paramfile, binary);

			// Output the data on the console
			// std::size_t nDoubles = ge.size<double>(); // The amount of doubles in the current data set
			// for(std::size_t pos = 0; pos<nDoubles; pos++) std::cout << ge.at<double>(pos) << std::endl;
		}
		break;

	default:
		std::cerr << "Error: Found invalid execution mode" << std::endl;
		exit(1);
	};


	return 0; // make the compiler happy
}

