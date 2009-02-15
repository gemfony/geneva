/**
 * @file GExternalProgram.cpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
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

// Standard header files go here
#include <iostream>
#include <cmath>
#include <string>
#include <sstream>
#include <cstdlib>
#include <vector>

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>

// GenEvA header files go here
#include "GRandom.hpp"
#include "GBasePopulation.hpp"
#include "GBoostThreadPopulation.hpp"
#include "GDoubleCollection.hpp"
#include "GInt32Collection.hpp"
#include "GInt32FlipAdaptor.hpp"
#include "GBooleanCollection.hpp"
#include "GLogger.hpp"
#include "GLogTargets.hpp"

// The individual calls an external program for the evaluation step
#include "GExecIndividual.hpp"

// Declares a function to parse the command line
#include "GCommandLineParser.hpp"

using namespace Gem::GenEvA;
using namespace Gem::Util;
using namespace Gem::GLogFramework;

const std::string PARAMETERDATA = "./parameterData";

/************************************************************************************************/
/**
 * The main function. We search for the minimum of a parabola. The actual calculation is handled by
 * an external program. This example demonstrates the use of the GExecIndividual class. Note that
 * a number of command line options are available. Call the executable with the "-h" switch
 * to get an overview.
 */
int main(int argc, char **argv){
	// Variables for the command line parsing
	 std::string fileName;
	 std::size_t populationSize, nParents;
	 boost::uint16_t nProducerThreads;
	 boost::uint32_t maxGenerations, reportGeneration;
	 boost::uint32_t adaptionThreshold;
	 long maxMinutes;
	 bool parallel;
	 bool verbose;
	 recoScheme rScheme;
	 double sigma,sigmaSigma,minSigma,maxSigma;

	// Parse the command line
	if(!parseCommandLine(argc, argv,
				         fileName,
				         populationSize,
				         nParents,
						 adaptionThreshold,
						 nProducerThreads,
						 maxGenerations,
						 maxMinutes,
						 reportGeneration,
						 rScheme,
						 parallel,
						 sigma,
						 sigmaSigma,
						 minSigma,
						 maxSigma,
						 verbose))
	{ exit(1); }

	// Add some log levels to the logger
	LOGGER->addLogLevel(Gem::GLogFramework::CRITICAL);
	LOGGER->addLogLevel(Gem::GLogFramework::WARNING);
	LOGGER->addLogLevel(Gem::GLogFramework::INFORMATIONAL);
	LOGGER->addLogLevel(Gem::GLogFramework::PROGRESS);

	// Add log targets to the system
	LOGGER->addTarget(boost::shared_ptr<GBaseLogTarget>(new GDiskLogger("GExternalProgram.log")));
	LOGGER->addTarget(boost::shared_ptr<GBaseLogTarget>(new GConsoleLogger()));

	// Random numbers are our most valuable good. Set the number of threads
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);

	// Ask the evaluation program to emit information about the individuals
	std::string commandLine = fileName + " -t -p " + PARAMETERDATA;
	system(commandLine.c_str());

	// Read in the population data and set up a GDoubleCollection
	boost::shared_ptr<GDoubleCollection> gdc(new GDoubleCollection());

	std::ifstream paramStream(PARAMETERDATA.c_str());
	if(!paramStream) {
		std::cerr << "Error: Could not open file " << PARAMETERDATA << ". Leaving ..." << std::endl;
		return 1;
	}

	std::size_t pDim = 0;
	paramStream >> pDim;

	for(std::size_t i=0; i<pDim; i++) {
		double tmp;
		paramStream >> tmp;
		gdc->push_back(tmp);
	}

	// Close the stream
	paramStream.close();

	// Set up and register an adaptor for the collection, so it
	// knows how to be mutated. We use the values given to us on the command line
	// (or as default values).
	boost::shared_ptr<GDoubleGaussAdaptor> gdga(new GDoubleGaussAdaptor(sigma,sigmaSigma,minSigma,maxSigma));
	gdga->setAdaptionThreshold(adaptionThreshold);
	gdc->addAdaptor(gdga);

	// Set up a single "master individual"
	boost::shared_ptr<GExecIndividual> execIndPtr(new GExecIndividual(fileName));
	execIndPtr->push_back(gdc);

	// Set up the populations, as requested
	if(parallel) {
	  // Now we can create a simple population with parallel execution.
	  GBoostThreadPopulation pop_par;
	  pop_par.setNThreads(4);

	  // Attach the individual to the population
	  pop_par.push_back(execIndPtr);

	  // Specify some population settings
	  pop_par.setPopulationSize(populationSize,nParents);
	  pop_par.setMaxGeneration(maxGenerations);
	  pop_par.setMaxTime(boost::posix_time::minutes(maxMinutes)); // Calculation should be finished after this amount of time
	  pop_par.setReportGeneration(reportGeneration); // Emit information during every generation
	  pop_par.setRecombinationMethod(rScheme); // The best parents have higher chances of survival

	  // Do the actual optimization
	  pop_par.optimize();
	}
	else {
	  // Now we've got our first individual and can create a simple population with serial execution.
	  GBasePopulation pop_ser;

	  // Attach all individuals to the population
	  pop_ser.push_back(execIndPtr);

	  // Specify some population settings
	  pop_ser.setPopulationSize(populationSize,nParents);
	  pop_ser.setMaxGeneration(maxGenerations);
	  pop_ser.setMaxTime(boost::posix_time::minutes(maxMinutes)); // Calculation should be finished after this amount of time
	  pop_ser.setReportGeneration(reportGeneration); // Emit information during every generation
	  pop_ser.setRecombinationMethod(rScheme); // The best parents have higher chances of survival

	  // Do the actual optimization
	  pop_ser.optimize();
	}

	std::cout << "Done ..." << std::endl;

	return 0;
}
