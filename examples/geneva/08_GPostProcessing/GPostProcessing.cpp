/**
 * @file GPostProcessing.cpp
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

// Boost header files go here
#include <boost/program_options.hpp>

// Geneva header files go here
#include "geneva/Go2.hpp"
#include "geneva/GPluggableOptimizationMonitors.hpp"
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

	std::string monitorTimings = "empty";
	bool usePostProcessor = false;
	execMode execModePP = execMode::SERIAL;

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
		, po::value<execMode>(&execModePP)->default_value(execMode::SERIAL)
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
		std::shared_ptr<GEvolutionaryAlgorithmPostOptimizer> eaPostOptimizer_ptr(
			new GEvolutionaryAlgorithmPostOptimizer(
				execModePP
				, "./config/GPostEvolutionaryAlgorithm.json"
				, "./config/GSerialExecutor.json"
			)
		);

		// Make sure only evolutionary algorithms may perform postprocessing
		eaPostOptimizer_ptr->allowPostProcessingFor("ea");

		// Make the post-processor known to the factory
		gfi_ptr->registerPostProcessor(eaPostOptimizer_ptr);
	}

	//---------------------------------------------------------------------------
	// Register pluggable optimization monitors, if requested by the user
	// See example 13 for more monitors

	if(monitorTimings != "empty") {
		std::shared_ptr<GProcessingTimesLogger>
			processingTimesLogger_ptr(new GProcessingTimesLogger(
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
	std::shared_ptr<GFunctionIndividual> p = go.optimize()->getBestGlobalIndividual<GFunctionIndividual>();

	// Here you can do something with the best individual ("p") found.
	// We simply print its content here, by means of an operator<< implemented
	// in the GFunctionIndividual code.
	std::cout
	<< "Best result found:" << std::endl
	<< p << std::endl;
}
