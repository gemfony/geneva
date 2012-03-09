/**
 * @file GOptimizationBenchmark.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */

// Standard header files go here
#include <iostream>

// Boost header files go here

// Geneva header files go here
#include <common/GHelperFunctionsT.hpp>
#include <common/GPlotDesigner.hpp>
#include <geneva/Go2.hpp>
#include "GOptimizationBenchmarkConfig.hpp"

// The individual that should be optimized
#include "geneva-individuals/GFunctionIndividual.hpp"

using namespace Gem::Geneva;
using namespace Gem::Common;
using namespace Gem::Tests;

typedef boost::tuple<double,double,double,double> xyWE; // xy-values with errors

int main(int argc, char **argv) {
	Go2::init();

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
	std::vector<boost::uint32_t> dimVec = gbc.getParDim(); // Will hold the dimensions for each test row
	std::vector<boost::uint32_t>::iterator it;
	std::string functionName;
	std::string functionCode;
	boost::tuple<double,double> varBoundaries;

	// Create a factory for GFunctionIndividual objects
	GFunctionIndividualFactory gfi("./config/GFunctionIndividual.json");

	for(it=dimVec.begin(); it!=dimVec.end(); ++it) {
		std::cout << "Starting new measurement with dimension " << *it << std::endl;

		// Set the appropriate dimension of the function individuals
		gfi.setParDim(*it);

		// Individual test results go here
		std::vector<double> bestResult;

		// Run the desired number of tests
		for(std::size_t test=0; test<nTests; test++) {
			// Retrieve an individual from the factory
			boost::shared_ptr<GFunctionIndividual> g = gfi();
			std::cout << "g has fitness " << g->fitness() << std::endl;

#ifdef DEBUG
			if(g->getParameterSize() != *it) {
				raiseException(
					"In main(): parameter size of individual != requested size: " << g->getParameterSize() << " / " << *it << std::endl
				);
			}

			if(!go.empty()) {
				raiseException(
					"In main(): go contains " << go.size() << " items when it should be empty." << std::endl
				);
			}
#endif /* DEBUG */

			// Make an individual known to the optimizer
			go.push_back(g);

			// Perform the actual optimization and extract the best individual
			boost::shared_ptr<GFunctionIndividual> p = go.optimize<GFunctionIndividual>();

			// Extract the function name in the first test row
			if(it==dimVec.begin() && test==0) {
				functionName = GFunctionIndividual::getStringRepresentation(p->getDemoFunction());
				functionCode = GFunctionIndividual::get2DROOTFunction(p->getDemoFunction());
				varBoundaries = gfi.getVarBoundaries();
			}

			// Add the fitness to the result vector
			bestResult.push_back(p->fitness());
			std::cout << "Best result in iteration " << test+1 << "/" << nTests << " with dimension " << *it << " has fitness " << p->fitness() << std::endl;

			// Clear the go object
			go.reset();
		}

		// Post process the vector, extracting mean and sigma
		boost::tuple<double,double> resultY = GStandardDeviation<double>(bestResult);

		std::cout << std::endl
				  << "mean = " << boost::get<0>(resultY) << std::endl
				  << "sigma = " << boost::get<1>(resultY) << std::endl
				  << std::endl;

		xyWE result(double(*it), 0., boost::get<0>(resultY), boost::get<1>(resultY));

		resultVec.push_back(result);
	}

	//-------------------------------------------------------------------------
	// Create plots from the result vector

	// Create a 2d-representation of the chosen function; We use the same boundaries in x- and y-direction
	boost::shared_ptr<GFunctionPlotter2D> optFunction_ptr(new GFunctionPlotter2D(functionCode, varBoundaries, varBoundaries));
	optFunction_ptr->setPlotLabel("2D Representation of the test function");


	boost::shared_ptr<GGraph2ED> gopt_ptr(new GGraph2ED());
	gopt_ptr->setPlotMode(CURVE);
	gopt_ptr->setPlotLabel("Measurements");
	gopt_ptr->setXAxisLabel("Function Dimension");
	gopt_ptr->setYAxisLabel("Best Result");

	// Add the data to the plot
	(*gopt_ptr) & resultVec;

	// Create the canvas
	std::string canvasLabel = "Optimization benchmarks for function " + functionName;
	GPlotDesigner gpd(canvasLabel, 1,2);
	gpd.setCanvasDimensions(1200,1400);

	// Register the two plots
	gpd.registerPlotter(optFunction_ptr);
	gpd.registerPlotter(gopt_ptr);

	// Emit the result file
	gpd.writeToFile(gbc.getResultFileName());

	// Terminate
	return Go2::finalize();
}
