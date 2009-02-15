/**
 * @file GExternalProgram.cpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
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
#include "GDataExchange.hpp"

// The individual calls an external program for the evaluation step
#include "GExecIndividual.hpp"

// Declares a function to parse the command line
#include "GCommandLineParser.hpp"

using namespace Gem::GenEvA;
using namespace Gem::Util;
using namespace Gem::GLogFramework;

const std::string POPULATIONDATA = "./populationData";
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
	 std::size_t parabolaDimension;
	 std::size_t populationSize, nParents;
	 double parabolaMin, parabolaMax;
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

	// Ask the evaluation program to emit information about the population and individuals
	std::string commandLine = fileName + " -t -p " + POPULATIONDATA;
	system(commandLine.c_str());

	// Read in the population data
	GPopulationData populationData;
	populationData.loadFromFile(POPULATIONDATA);

	// Retrieve the population partcularities
	populationSize = populationData.getPopulationSize();
	nParents = populationData.getNumberOfParents();

	// Retrieve all individual templates stored in the file and
	// transfer them into an individual
	std::vector<boost::shared_ptr<GExecIndividual> > execIndPtrArray;
	std::size_t i, j, k, nIndividuals = populationData.numberOfIndividuals();
	for(i=0; i<nIndividuals; i++) {
		boost::shared_ptr<GIndividualData> indDatPtr = populationData.at(i);
		boost::shared_ptr<GExecIndividual> execIndPtr(new GExecIndividual(PARAMETERDATA));

		// Find out about the number of arrays of different type in the GIndividualData object
		std::size_t nDoubleArrays = indDatPtr->numberOfDoubleArrays();
		std::size_t nLongArrays = indDatPtr->numberOfLongArrays();
		std::size_t nBooleanArrays = indDatPtr->numberOfBooleanArrays();

		// Create a suitable number of GDoubleCollection objects
		for(j=0; j<nDoubleArrays; j++) {
			// Retrieve a reference to the first double array
			const std::vector<double>& doubleArrayRef = indDatPtr.d_at(j);
			// Get the size of the array
			std::size_t sz = doubleArrayRef.size();

			// Set up an empty GDoubleCollection
			boost::shared_ptr<GDoubleCollection> gdc(new GDoubleCollection());

			// Attach the values of the double array to the collection
			for(k=0; k<sz; k++) gdc->push_back(doubleArrayRef.at(k));

			// Set up an adaptor for the collection, so it knows how to be mutated. sigma,
			// sigmaSigma, minSigma and maxSigma are supplied on the command line (alternatively
			// default values are used)
			boost::shared_ptr<GDoubleGaussAdaptor> gdga(new GDoubleGaussAdaptor(sigma,sigmaSigma,minSigma,maxSigma));
			gdga->setAdaptionThreshold(adaptionThreshold);

			// Register the adaptor
			gdc->addAdaptor(gdga);

			// Finally add the GDoubleCollection to the individual
			execIndPtr->push_back(gdc);
		}

		// Create a suitable number of GInt32Collection objects
		for(j=0; j<nLongArrays; j++) {
			// Retrieve a reference to the first double array
			const std::vector<boost::uint32_t>& longArrayRef = indDatPtr.l_at(j);
			// Get the size of the array
			std::size_t sz = longArrayRef.size();

			// Set up an empty GInt32Collection
			boost::shared_ptr<GInt32Collection> glc(new GInt32Collection());

			// Attach the values of the double array to the collection
			for(k=0; k<sz; k++) glc->push_back(longArrayRef.at(k));

			// Set up an adaptor for the collection, so it knows how to be mutated.
			boost::shared_ptr<GInt32FlipAdaptor> gifa(new GInt32FlipAdaptor());
			gifa->setAdaptionThreshold(adaptionThreshold);

			// Register the adaptor
			glc->addAdaptor(gifa);

			// Finally add the GInt32Collection to the individual
			execIndPtr->push_back(glc);
		}

		// Create a suitable number of GBooleanCollection objects
		for(j=0; j<nBooleanArrays; j++) {
			// Retrieve a reference to the first double array
			const std::vector<bool>& boolArrayRef = indDatPtr.b_at(j);
			// Get the size of the array
			std::size_t sz = boolArrayRef.size();

			// Set up an empty GInt32Collection
			boost::shared_ptr<GBooleanCollection> gbc(new GBooleanCollection());

			// Attach the values of the double array to the collection
			for(k=0; k<sz; k++) glc->push_back(boolArrayRef.at(k));

			// Set up an adaptor for the collection, so it knows how to be mutated.
			boost::shared_ptr<GBooleanAdaptor> gba(new GBooleanAdaptor());
			gba->setAdaptionThreshold(adaptionThreshold);

			// Register the adaptor
			gbc->addAdaptor(gba);

			// Finally add the GBooleanCollection to the individual
			execIndPtr->push_back(gbc);
		}

		// The individual has been set up. Store it for later use.
		execIndPtrArray.push_back(execIndPtr);
	}

	// Set up the populations, as requested
	if(parallel) {
	  // Now we can create a simple population with parallel execution.
	  GBoostThreadPopulation pop_par;
	  pop_par.setNThreads(4);

	  // Attach all individuals to the population
	  std::vector<boost::shared_ptr<GExecIndividual> >::iterator it;
	  for(it=execIndPtrArray.begin(); it!=execIndPtrArray.end(); ++it)  pop_par.push_back(*it);

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

	  // Attach all individuals to the population
	  std::vector<boost::shared_ptr<GExecIndividual> >::iterator it;
	  for(it=execIndPtrArray.begin(); it!=execIndPtrArray.end(); ++it)  pop_par.push_back(*it);

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
