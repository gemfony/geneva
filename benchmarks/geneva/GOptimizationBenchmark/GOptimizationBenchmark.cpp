/**
 * @file GOptimizationBenchmark.cpp
 */

/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

// Standard header files go here
#include <iostream>
#include <algorithm>
#include <chrono>

// Boost header files go here

// Geneva header files go here
#include "common/GCommonHelperFunctionsT.hpp"
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
			// Create a Go2-object for the loop
			Go2* go_loop = new Go2(argc, argv, "./config/Go2.json");

			// Retrieve an individual from the factory
			std::shared_ptr<GFunctionIndividual> g = gfi.get_as<GFunctionIndividual>();

#ifdef DEBUG
			if(g->getParameterSize() != *it) {
				throw gemfony_exception(
					g_error_streamer(DO_LOG,  time_and_place)
						<< "In main(): parameter size of individual != requested size: " << g->getParameterSize() << " / " << *it << std::endl
				);
			}

			if(!go_loop->empty()) {
				throw gemfony_exception(
					g_error_streamer(DO_LOG,  time_and_place)
						<< "In main(): go contains " << go_loop->size() << " items when it should be empty." << std::endl
				);
			}
#endif /* DEBUG */

			// Make an individual known to the optimizer
			go_loop->push_back(g);

			// Start recording of time
			startTime = std::chrono::system_clock::now();

			// Perform the actual optimization and extract the best individual
			std::shared_ptr<GFunctionIndividual> p = go_loop->optimize()->getBestGlobalIndividual<GFunctionIndividual>();

			endTime = std::chrono::system_clock::now();

			// Extract the function name in the first test row
			if(it==dimVec.begin() && test==0) {
				functionName = GFunctionIndividual::getStringRepresentation(p->getDemoFunction());
				functionCode = GFunctionIndividual::get2DROOTFunction(p->getDemoFunction());
				varBoundaries = gfi.getVarBoundaries();
			}

			// Add the fitness to the result vector
			bestResult.push_back(p->raw_fitness(0));

			// Add timing information to the result vector
			timeConsumed.push_back(std::chrono::duration<double>(endTime-startTime).count());

            delete go_loop;
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
