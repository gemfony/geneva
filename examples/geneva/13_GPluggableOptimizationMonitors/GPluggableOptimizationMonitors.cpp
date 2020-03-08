/**
 * @file GPluggableOptimizationMonitors.cpp
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

	bool printValid = false;
	bool useRawFitness = false;
	std::string monitorSpec = "empty";
	bool bestOnly = false;
	bool observeBoundaries = false;
	std::string logAll = "empty";
	std::string monitorNAdaptions = "empty";
	std::string logSigma = "empty";
	std::string monitorTimings = "empty";
	bool addOneOnly = false;
	bool initPerimeter = false;
	bool printInitial = false;
	bool showIterationBoundaries = false;


	// Assemble command line options
	boost::program_options::options_description user_options;
	user_options.add_options()(
		"validOnly"
		, po::value<bool>(&printValid)->implicit_value(true)->default_value(false) // This allows you to say both --validOnly and --validOnly=true
		, "Enforces output of valid solutions only"
	)(
		"useRawFitness"
		, po::value<bool>(&useRawFitness)->implicit_value(true)->default_value(false) // This allows you to say both --useRawFitness and --useRawFitness=true
		, "Plot untransformed fitness value, even if a transformation takes place for the purpose of optimization"
	)(
		"monitorSpec"
		, po::value<std::string>(&monitorSpec)->default_value(std::string("empty"))
		, "Allows you to specify variables to be monitored like this: \"d(var0, -10, 10)\""
	)(
		"bestOnly"
		, po::value<bool>(&bestOnly)->implicit_value(true)->default_value(false)
		, "Allows you to specify whether only the best solutions should be monitored. This option only has an effect when monitorSpec is set."
	)(
		"observeBoundaries"
		, po::value<bool>(&observeBoundaries)->implicit_value(true)->default_value(false) // This allows you say both --observeBoundaries and --observeBoundaries=true
		, "Only plot inside of specified boundaries (no effect, when monitorSpec hasn't been set)"
	)(
		"logAll"
		, po::value<std::string>(&logAll)->implicit_value(std::string("./logAll.txt"))->default_value("empty")
		, "Logs all solutions to the file name provided as argument to this switch"
	)(
		"monitorAdaptions"
		, po::value<std::string>(&monitorNAdaptions)->implicit_value(std::string("./nAdaptions.C"))->default_value("empty")
		, "Logs the number of adaptions for all individuals over the course of the optimization. Useful for evolutionary algorithms only."
	)(
		"logSigma"
		, po::value<std::string>(&logSigma)->implicit_value(std::string("./sigmaLog.C"))->default_value("empty")
		, "Logs the value of sigma for all or the best adaptors, if GDoubleGaussAdaptors are being used"
	)(
		"monitorTimings"
		, po::value<std::string>(&monitorTimings)->implicit_value(std::string("timingsLog"))->default_value("empty")
		, "Logs the times for all processing steps"
	)(
		"addOneIndividualOnly"
		, po::value<bool>(&addOneOnly)->implicit_value(true)->default_value(false)
		, "When set, results in a single individual being added to the collection. This may be useful for debugging in conjunction with the INITPERIMETER option"
	)(
		"initPerimeter"
		, po::value<bool>(&initPerimeter)->implicit_value(true)->default_value(false)
		, "When set, results in the initialization of the GFunctionIndividual on the perimeter of the allowed value range. Otherwise the individual will be initialized rendomly"
	)(
		"printInitial"
		, po::value<bool>(&printInitial)->implicit_value(true)->default_value(false)
		, "[logAll] When set, forces the printout of the initial population prior to the optimization"
	)(
		"showIterationBoundaries"
		, po::value<bool>(&showIterationBoundaries)->implicit_value(true)->default_value(false)
		, "[logAll] When set, prints a comment inbetween iterations"
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

	if(initPerimeter) {
		gfi_ptr->setIM(initMode::INITPERIMETER);
	} else {
		gfi_ptr->setIM(initMode::INITRANDOM);
	}

	//---------------------------------------------------------------------------
	// Register pluggable optimization monitors, if requested by the user

	// Register a progress plotter with the global optimization algorithm factory
	if(monitorSpec != "empty") {
		std::shared_ptr<GProgressPlotter> progplot_ptr(new GProgressPlotter());

		progplot_ptr->setProfileSpec(monitorSpec);
		progplot_ptr->setObserveBoundaries(observeBoundaries);
		progplot_ptr->setMonitorValidOnly(printValid); // Only record valid parameters, when printValid is set to true
		progplot_ptr->setUseRawEvaluation(useRawFitness); // Use untransformed evaluation values for logging
		progplot_ptr->setMonitorBestOnly(bestOnly); // Whether only the best solutions should be monitored

		// Request printing of png files (upon processing of the .C file with ROOT)
		progplot_ptr->setAddPrintCommand(true);

		go.registerPluggableOM(progplot_ptr);
	}

	if(logAll != "empty") {
		std::shared_ptr<GAllSolutionFileLogger> allSolutionLogger_ptr(new GAllSolutionFileLogger(logAll));

		allSolutionLogger_ptr->setPrintWithNameAndType(true); // Output information about variable names and types
		allSolutionLogger_ptr->setPrintWithCommas(true); // Output commas between values
		allSolutionLogger_ptr->setUseTrueFitness(false); // Output "transformed" fitness, not the "true" value
		allSolutionLogger_ptr->setShowValidity(true); // Indicate, whether this is a valid solution

		if(printInitial) {
			allSolutionLogger_ptr->setPrintInitial();
		}

		if(showIterationBoundaries) {
			allSolutionLogger_ptr->setShowIterationBoundaries();
		}

		go.registerPluggableOM(allSolutionLogger_ptr);
	}

	if(monitorNAdaptions != "empty") {
		std::shared_ptr<GNAdpationsLogger> nAdaptionsLogger_ptr(new GNAdpationsLogger(monitorNAdaptions));

		nAdaptionsLogger_ptr->setMonitorBestOnly(false); // Output information for all individuals
		nAdaptionsLogger_ptr->setAddPrintCommand(true); // Create a PNG file if Root-file is executed

		go.registerPluggableOM(nAdaptionsLogger_ptr);
	}

	if(logSigma != "empty") {
		std::shared_ptr<GAdaptorPropertyLogger<double>>
			sigmaLogger_ptr(new GAdaptorPropertyLogger<double>(logSigma, "GDoubleGaussAdaptor", "sigma"));

		sigmaLogger_ptr->setMonitorBestOnly(false); // Output information for all individuals
		sigmaLogger_ptr->setAddPrintCommand(true); // Create a PNG file if Root-file is executed

		go.registerPluggableOM(sigmaLogger_ptr);
	}

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

	// Either add a single individual or take all individuals from the content provider.
	// Adding a single individual is useful for debugging purposes, e.g. in order to check,
	// whether the added individual is retained in INITPERIMETER mode.
	if(addOneOnly) {
	   go.push_back(gfi_ptr->get());
	} else {
		// Add a content creator so Go2 can generate its own individuals, if necessary
		go.registerContentCreator(gfi_ptr);
	}

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
