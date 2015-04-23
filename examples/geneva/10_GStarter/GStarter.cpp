/**
 * @file GFunctionMinimizer.cpp
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

// Standard header files go here
#include <iostream>

// Boost header files go here

// Geneva header files go here
#include <geneva/Go2.hpp>
#include <geneva/GPluggableOptimizationMonitorsT.hpp>

// The individual that should be optimized
#include "GStarterIndividual.hpp"
#include "GSigmaMonitor.hpp"


using namespace Gem::Geneva;

int main(int argc, char **argv) {
	Go2 go(argc, argv, "./config/Go2.json");

	//---------------------------------------------------------------------
	// Client mode
	if(go.clientMode()) {
		return go.clientRun();
	}

	//---------------------------------------------------------------------
	// Server mode, serial or multi-threaded execution

	// Create an optimization monitor (targeted at evolutionary algorithms) and register
	// it with the global store. This step is OPTIONAL. We recommend checking the chapters
	// on writing custom progress monitors within the Geneva framework.
   GOAMonitorStore->setOnce(
      "ea"
      , std::shared_ptr<GSigmaMonitor>(new GSigmaMonitor("./sigmaProgress.C"))
   );

   // Another possibility: Add a "pluggable optimization monitor" to Go2. This
   // particular monitor will logg all solutions that were found into the file allLog.txt,
   // provided their fitness is better than 1. . The option is usually used for monitors
   // that do not discriminate between optimization algorithms.
   std::vector<double> boundaries;
   boundaries.push_back(1.);
   std::shared_ptr<GAllSolutionFileLoggerT<GParameterSet> >
      allSolutionLogger_ptr(new GAllSolutionFileLoggerT<GParameterSet>("allLog.txt", boundaries));

   go.registerPluggableOM(
      [allSolutionLogger_ptr](const infoMode& im, GOptimizationAlgorithmT<GParameterSet> * const goa){
         allSolutionLogger_ptr->informationFunction(im, goa);
      }
   );

   // Create a factory for GStarterIndividual objects and perform
   // any necessary initial work.
   std::shared_ptr<GStarterIndividualFactory> gsif_ptr(
         new GStarterIndividualFactory("./config/GStarterIndividual.json")
   );

   // Add a content creator so Go2 can generate its own individuals, if necessary
   go.registerContentCreator(gsif_ptr);

	// Perform the actual optimization
	std::shared_ptr<GStarterIndividual> bestIndividual_ptr = go.optimize<GStarterIndividual>();

	// Do something with the best result. Here we simply print the result to stdout.
	std::cout << bestIndividual_ptr << std::endl;
}
