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
	 bool useCommonAdaptor;
	 double sigma;
	 double sigmaSigma;
	 double minSigma;
	 double maxSigma;

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
						 useCommonAdaptor,
						 sigma,
						 sigmaSigma,
						 minSigma,
						 maxSigma,
						 verbose))
	{ exit(1); }

	// Random numbers are our most valuable good. Set the number of threads
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);
	GRANDOMFACTORY->setArraySize(arraySize);

	// Set up a single parabola individual
	boost::shared_ptr<GBoundedParabolaIndividual> parabolaIndividual(new GBoundedParabolaIndividual());

	// Set up a GBoundedDoubleCollection
	boost::shared_ptr<GBoundedDoubleCollection> gbdc_ptr(new GBoundedDoubleCollection());

	// Set up an adaptor.
	boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(sigma, sigmaSigma, minSigma, maxSigma));
	gdga_ptr->setAdaptionThreshold(adaptionThreshold);

	// Check whether random numbers should be produced locally or in the factory
	if(productionPlace) // Factory means "true"
		gdga_ptr->setRnrGenerationMode(Gem::Util::RNRFACTORY);
	else
		gdga_ptr->setRnrGenerationMode(Gem::Util::RNRLOCAL);

	if(useCommonAdaptor) {
		// Adding the adaptor to the collection instead of individual GBoundedDouble objects will
		// result in a joint adaptor for all GBoundedDoubles
		gbdc_ptr->addAdaptor(gdga_ptr);

		// Set up parabolaDimension GBoundedDouble objects in the desired value range,
		// and register them with the double collection
		for(std::size_t i=0; i<parabolaDimension; i++) {
			// GBoundedDouble will start with value "max"
			boost::shared_ptr<GBoundedDouble> gbd_ptr(new GBoundedDouble(parabolaMax, parabolaMin, parabolaMax));

			// Make the GBoundedDouble known to the collection
			gbdc_ptr->push_back(gbd_ptr);
		}
	}
	else {
		// Set up parabolaDimension GBoundedDouble objects in the desired value range,
		// and register them with the double collection
		for(std::size_t i=0; i<parabolaDimension; i++) {
			// GBoundedDouble will start with value "max"
			boost::shared_ptr<GBoundedDouble> gbd_ptr(new GBoundedDouble(parabolaMax, parabolaMin, parabolaMax));

			// Registering the adaptor with each GBoundedDouble results in individual adaptors for each object
			gbd_ptr->addAdaptor(gdga_ptr);

			// Make the GBoundedDouble known to the collection
			gbdc_ptr->push_back(gbd_ptr);
		}
	}

	// Make the GBoundedDouble collection known to the individual
	parabolaIndividual->push_back(gbdc_ptr);

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

	  // Check whether random numbers should be produced locally or in the factory
	  if(productionPlace) // Factory means "true"
		  pop_par.setRnrGenerationMode(Gem::Util::RNRFACTORY);
	  else
		  pop_par.setRnrGenerationMode(Gem::Util::RNRLOCAL);

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

	  // Check whether random numbers should be produced locally or in the factory
	  if(productionPlace) // Factory means "true"
		  pop_ser.setRnrGenerationMode(Gem::Util::RNRFACTORY);
	  else
		  pop_ser.setRnrGenerationMode(Gem::Util::RNRLOCAL);

	  // Do the actual optimization
	  pop_ser.optimize();
	}

	std::cout << "Done ..." << std::endl;

	return 0;
}
