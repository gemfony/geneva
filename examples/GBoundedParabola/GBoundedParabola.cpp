/**
 * @file GBoundedParabola.cpp
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
#include <sstream>

// Boost header files go here

// GenEvA header files go here
#include "GRandom.hpp"
#include "GBasePopulation.hpp"
#include "GBoostThreadPopulation.hpp"

// The individual that should be optimized
// This is a simple parabola
#include "GBoundedParabolaIndividual.hpp"

// Declares a function to parse the command line
#include "GCommandLineParser.hpp"

using namespace Gem::GenEvA;
using namespace Gem::Util;

/************************************************************************************************/
/**
 * The main function. We search for the minimum of a parabola. This example demonstrates the use
 * of the GBasePopulation class or (at your choice) of the GBoostThreadPopulation class. Note that
 * a number of command line options are available. Call the executable with the "-h" switch
 * to get an overview.
 */

int main(int argc, char **argv){
	// Variables for the command line parsing
	 std::size_t parabolaDimension;
	 std::size_t populationSize, nParents;
	 double parabolaMin, parabolaMax;
	 boost::uint16_t nProducerThreads;
	 boost::uint16_t nEvaluationThreads;
	 boost::uint32_t maxGenerations, reportGeneration;
	 boost::uint32_t adaptionThreshold;
	 long maxMinutes;
	 bool parallel;
	 bool verbose;
	 recoScheme rScheme;
	  std::size_t arraySize;
	  bool productionPlace;

	// Parse the command line
	if(!parseCommandLine(argc, argv,
		  			     parabolaDimension,
						 parabolaMin,
						 parabolaMax,
						 adaptionThreshold,
						 nProducerThreads,
						 nEvaluationThreads,
						 populationSize,
						 nParents,
						 maxGenerations,
						 maxMinutes,
						 reportGeneration,
						 rScheme,
						 parallel,
						 arraySize,
						 productionPlace,
						 verbose))
	{ exit(1); }

	// Random numbers are our most valuable good. Set the number of threads
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);
	GRANDOMFACTORY->setArraySize(arraySize);

	// Set up a single parabola individual
	boost::shared_ptr<GBoundedParabolaIndividual>
		parabolaIndividual(new GBoundedParabolaIndividual(parabolaDimension,
				parabolaMin,
				parabolaMax,
				adaptionThreshold));

	if(productionPlace) // Factory means "true"
		parabolaIndividual->setRnrGenerationMode(Gem::Util::RNRFACTORY);
	else
		parabolaIndividual->setRnrGenerationMode(Gem::Util::RNRLOCAL);

	if(parallel) {
	  // Now we've got our first individual and can create a simple population with parallel execution.
	  GBoostThreadPopulation pop_par;
	  pop_par.setNThreads(nEvaluationThreads);

	  pop_par.push_back(parabolaIndividual);

	  // Specify some population settings
	  pop_par.setPopulationSize(populationSize,nParents);
	  pop_par.setMaxGeneration(maxGenerations);
	  pop_par.setMaxTime(boost::posix_time::minutes(maxMinutes)); // Calculation should be finished after 5 minutes
	  pop_par.setReportGeneration(reportGeneration); // Emit information during every generation
	  pop_par.setRecombinationMethod(rScheme); // The best parents have higher chances of survival

	  // Do the actual optimization
	  pop_par.optimize();
	}
	else {
	  // Now we've got our first individual and can create a simple population with serial execution.
	  GBasePopulation pop_ser;

	  pop_ser.push_back(parabolaIndividual);

	  // Specify some population settings
	  pop_ser.setPopulationSize(populationSize,nParents);
	  pop_ser.setMaxGeneration(maxGenerations);
	  pop_ser.setMaxTime(boost::posix_time::minutes(maxMinutes)); // Calculation should be finished after 5 minutes
	  pop_ser.setReportGeneration(reportGeneration); // Emit information during every generation
	  pop_ser.setRecombinationMethod(rScheme); // The best parents have higher chances of survival

	  // Do the actual optimization
	  pop_ser.optimize();
	}

	std::cout << "Done ..." << std::endl;

	return 0;
}
