/**
 * @file GOptimizationBenchmark.cpp
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
#include <algorithm>
#include <chrono>

// Boost header files go here

// Geneva header files go here
#include "common/GHelperFunctionsT.hpp"
#include "common/GPlotDesigner.hpp"
#include "geneva/Go2.hpp"

#include "GOptimizationBenchmarkConfig.hpp"

// The individual that should be optimized
#include "geneva-individuals/GFunctionIndividual.hpp"

using namespace Gem::Geneva;
using namespace Gem::Common;
using namespace Gem::Tests;

using xyWE = std::tuple<double,double,double,double>; // xy-values with errors

int main(int argc, char **argv) {
	// Create the algorithm container
	Go2 go(argc, argv, "./config/Go2.json");

	//---------------------------------------------------------------------
	// Client mode
	if(go.clientMode()) {
		return go.clientRun();
	}

	//---------------------------------------------------------------------
	// Server mode, serial or multi-threaded execution

	// Create a copy so we may reset the Go2 object to its original settings later
	Go2 goTmp = go;

	// Load benchmark configuration options
	GOptimizationBenchmarkConfig gbc("./config/GOptimizationBenchmark.json");

	// Loop over all dimensions and the number of tests in each dimension
	std::size_t nTests = gbc.getNTests();
	std::vector<xyWE> resultVec; // Will hold the results for each dimension
	std::vector<xyWE> timingVec; // Will hold the results for each dimension
	std::vector<std::uint32_t> dimVec = gbc.getParDim(); // Will hold the dimensions for each test row
	std::vector<std::uint32_t>::iterator it;
	std::string functionName;
	std::string functionCode;
	std::tuple<double,double> varBoundaries;

	// Create a factory for GFunctionIndividual objects
	GFunctionIndividualFactory gfi("./config/GFunctionIndividual.json");

	for(it=dimVec.begin(); it!=dimVec.end(); ++it) {
		// Individual test results go here
		std::vector<double> bestResult;
		// The time consumed until the optimization was terminated
		std::vector<double> timeConsumed;
		// Holds timing measurements
		std::chrono::system_clock::time_point startTime, endTime;

		std::cout << "Starting new measurement with dimension " << *it << std::endl;

		// Set the appropriate dimension of the function individuals
		gfi.setParDim(*it);

		// Run the desired number of tests
		for(std::size_t test=0; test<nTests; test++) {
			// Retrieve an individual from the factory
			std::shared_ptr<GFunctionIndividual> g = gfi.get_as<GFunctionIndividual>();

#ifdef DEBUG
			if(g->getParameterSize() != *it) {
				glogger
				<< "In main(): parameter size of individual != requested size: " << g->getParameterSize() << " / " << *it << std::endl
				<< GEXCEPTION;
			}

			if(!go.empty()) {
				glogger
				<< "In main(): go contains " << go.size() << " items when it should be empty." << std::endl
				<< GEXCEPTION;
			}
#endif /* DEBUG */

			// Make an individual known to the optimizer
			go.push_back(g);

			// Start recording of time
			startTime = std::chrono::system_clock::now();

			// Perform the actual optimization and extract the best individual
			std::shared_ptr<GFunctionIndividual> p = go.optimize<GFunctionIndividual>();

			endTime = std::chrono::system_clock::now();

			// Extract the function name in the first test row
			if(it==dimVec.begin() && test==0) {
				functionName = GFunctionIndividual::getStringRepresentation(p->getDemoFunction());
				functionCode = GFunctionIndividual::get2DROOTFunction(p->getDemoFunction());
				varBoundaries = gfi.getVarBoundaries();
			}

			// Add the fitness to the result vector
			bestResult.push_back(p->fitness());

			// Add timing information to the result vector
			timeConsumed.push_back(std::chrono::duration<double>(endTime-startTime).count());

			// Reset the go object
			go = goTmp;
		}

		// Post process the vector, extracting mean and sigma
		std::tuple<double,double> resultY = GStandardDeviation<double>(bestResult);
		std::tuple<double,double> timing2 = GStandardDeviation<double>(timeConsumed);

		std::cout
		<< std::endl
		<< "best result = " << std::get<0>(resultY) << " +/- " << std::get<1>(resultY) << std::endl
		<< "timing      = " << std::get<0>(timing2) << " +/- " << std::get<1>(timing2) << " s" << std::endl
		<< std::endl;

		xyWE resultE(double(*it), 0., std::get<0>(resultY), std::get<1>(resultY));
		xyWE timingE(double(*it), 0., std::get<0>(timing2), std::get<1>(timing2));

		resultVec.push_back(resultE);
		timingVec.push_back(timingE);
	}

	//-------------------------------------------------------------------------
	// Create plots from the result vector

	std::shared_ptr<GGraph2ED> timing_ptr(new GGraph2ED());
	timing_ptr->setPlotMode(graphPlotMode::CURVE);
	timing_ptr->setPlotLabel("Timings of optimization runs [s]");
	timing_ptr->setXAxisLabel("Function Dimension");
	timing_ptr->setYAxisLabel("Seconds consumed");

	std::shared_ptr<GGraph2ED> gopt_ptr(new GGraph2ED());
	gopt_ptr->setPlotMode(graphPlotMode::CURVE);
	gopt_ptr->setPlotLabel("Best measurements and errors");
	gopt_ptr->setXAxisLabel("Function Dimension");
	gopt_ptr->setYAxisLabel("Best Result");

	// Add the data to the plots
	(*timing_ptr) & timingVec;
	(*gopt_ptr)   & resultVec;

	// Create the canvas
	std::string canvasLabel = "Optimization benchmarks for function " + functionName;
	GPlotDesigner gpd(canvasLabel, 1,2);
	gpd.setCanvasDimensions(800,1200);

	// Register the two plots
	gpd.registerPlotter(timing_ptr);
	gpd.registerPlotter(gopt_ptr);

	// Emit the result file
	gpd.writeToFile(gbc.getResultFileName());
}
