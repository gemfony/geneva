/**
 * @file GPostProcessing.cpp
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
#include <boost/ref.hpp>
#include <boost/program_options.hpp>

// Geneva header files go here
#include "geneva/Go2.hpp"
#include "geneva/GPluggableOptimizationMonitorsT.hpp"
#include "geneva/GPostProcessorT.hpp"

// The individual that should be optimized
#include "geneva-individuals/GFunctionIndividual.hpp"

using namespace Gem::Geneva;
namespace po = boost::program_options;

/******************************************************************************/
/**
 * The main function
 */
int main(int argc, char **argv) {
	//---------------------------------------------------------------------------
	// We want to add additional command line options

	bool useRawFitness = false;
	std::string monitorTimings = "empty";
	bool usePostProcessor = false;
	execMode execModePP = execMode::EXECMODE_SERIAL;

	// Assemble command line options
	boost::program_options::options_description user_options;
	user_options.add_options()(
		"monitorTimings"
		, po::value<std::string>(&monitorTimings)->implicit_value(std::string("timingsLog"))->default_value("empty")
		, "Logs the times for all processing steps"
	)(
		"usePostProcessor"
		, po::value<bool>(&usePostProcessor)->implicit_value(true)->default_value(false)
		, "Whether or not to post-process individuals (using evolutionary algorithms in this example)"
	)(
		"execModePostProcessing"
		, po::value<execMode>(&execModePP)->default_value(execMode::EXECMODE_SERIAL)
		, "The execution mode for post-optimization (0: serial; 1: multithreaded)"
	);

	Go2 go(argc, argv, "./config/Go2.json", user_options);

	//---------------------------------------------------------------------------
	// Client mode
	if(go.clientMode()) {
		return go.clientRun();
	} // Execution will end here in client mode

	//---------------------------------------------------------------------------
	// Create a factory for GFunctionIndividual objects and perform
	// any necessary initial work.
	std::shared_ptr<GFunctionIndividualFactory> gfi_ptr(new GFunctionIndividualFactory("./config/GFunctionIndividual.json"));

	//---------------------------------------------------------------------------
	// Register a post-processor, if this was requested by the user
	if(usePostProcessor) {
		std::shared_ptr<GEvolutionaryAlgorithmPostOptimizerT<GFunctionIndividual>> eaPostOptimizer_ptr(
			new GEvolutionaryAlgorithmPostOptimizerT<GFunctionIndividual>(
				execModePP
				, "./config/GPostEvolutionaryAlgorithm.json"
			)
		);

		gfi_ptr->registerPostProcessor(eaPostOptimizer_ptr);
	}

	//---------------------------------------------------------------------------
	// Register pluggable optimization monitors, if requested by the user
	// See example 13 for more monitors

	if(monitorTimings != "empty") {
		std::shared_ptr<GProcessingTimesLoggerT<GParameterSet>>
			processingTimesLogger_ptr(new GProcessingTimesLoggerT<GParameterSet>(
				"hist_" + monitorTimings + ".C"
				, "hist2D_" + monitorTimings + ".C"
				, monitorTimings + ".txt"
				, 100 // nBins in x-direction
				, 100 // nBins in y-direction
			)
		);
		go.registerPluggableOM(processingTimesLogger_ptr);
	}

	//---------------------------------------------------------------------------

	// Add a content creator so Go2 can generate its own individuals, if necessary
	go.registerContentCreator(gfi_ptr);

	// Add a default optimization algorithm to the Go2 object. This is optional.
	// Indeed "ea" is the default setting anyway. However, if you do not like it, you
	// can register another default algorithm here, which will then be used, unless
	// you specify other algorithms on the command line. You can also add a smart
	// pointer to an optimization algorithm here instead of its mnemonic.
	go.registerDefaultAlgorithm("ea");

	// Perform the actual optimization
	std::shared_ptr<GFunctionIndividual> p = go.optimize<GFunctionIndividual>();

	// Here you can do something with the best individual ("p") found.
	// We simply print its content here, by means of an operator<< implemented
	// in the GFunctionIndividual code.
	std::cout
	<< "Best result found:" << std::endl
	<< p << std::endl;
}
