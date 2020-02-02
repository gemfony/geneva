/**
 * @file GDoubleGaussAdaptorTest.cpp
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

/*
 * This test adapts a double a given number of times and records the values of
 * different entities of a GDoubleGaussAdaptor as a function of the iteration.
 * The output can be processed with the help of the ROOT analysis toolkit (see
 * http://root.cern.ch .
 */

// Standard header files go here
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>

// Boost header files go here

// Geneva header files go here
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "common/GPlotDesigner.hpp"
#include "common/GParserBuilder.hpp"

using namespace Gem::Geneva;
using namespace Gem::Common;
using namespace Gem::Hap;

/************************************************************************************************/

// Default settings
const double CMD_DEFAULTSIGMA = 0.025;
const double CMD_DEFAULTSIGMASIGMA = 0.1;
const double CMD_DEFAULTMINSIGMA = 0.001;
const double CMD_DEFAULTMAXSIGMA = 1.;
const std::uint32_t CMD_DEFAULTMAXITER = 100000;
const std::string CMD_DEFAULTRESULTFILE = "result.C";
const bool CMD_DEFAULTVERBOSE = true;
const std::uint32_t CMD_DEFAULTADAPTIONTHRESHOLD=1;

/************************************************************************************************/
/**
 * A function that parses the command line for all required parameters
 */

bool parseCommandLine(
	int argc, char **argv
	, double& sigma
	, double& sigmaSigma
	, double& minSigma
	, double& maxSigma
	, std::uint32_t& adaptionThreshold
	, std::string& resultFile
	, std::uint32_t& maxIter
) {
	// Create the parser builder
	Gem::Common::GParserBuilder gpb;

	gpb.registerCLParameter<double>(
		"sigma"
		, sigma
		, CMD_DEFAULTSIGMA
		, "Width of the gaussian"
	);

	gpb.registerCLParameter<double>(
		"sigmaSigma"
		, sigmaSigma
		, CMD_DEFAULTSIGMASIGMA
		, "Width of the gaussian used to adapt sigma"
	);

	gpb.registerCLParameter<double>(
		"minSigma"
		, minSigma
		, CMD_DEFAULTMINSIGMA
		, "Minimal allowed value of sigma"
	);

	gpb.registerCLParameter<double>(
		"maxSigma"
		, maxSigma
		, CMD_DEFAULTMAXSIGMA
		, "Maximum allowed value of sigma"
	);

	gpb.registerCLParameter<std::uint32_t>(
		"adaptionThreshold,a"
		, adaptionThreshold
		, CMD_DEFAULTADAPTIONTHRESHOLD
		, "Number of calls to adapt() after which the adaption parameters should be modified"
	);

	gpb.registerCLParameter<std::string>(
		"resultFile,F"
		, resultFile
		, CMD_DEFAULTRESULTFILE
		, "The file to write the result to"
	);

	gpb.registerCLParameter<std::uint32_t>(
		"maxIter,I"
		, maxIter
		, CMD_DEFAULTMAXITER
		, "The maximum number of test cycles"
	);

	// Parse the command line and leave if the help flag was given. The parser
	// will emit an appropriate help message by itself
	if(Gem::Common::GCL_HELP_REQUESTED == gpb.parseCommandLine(argc, argv, true /*verbose*/)) {
		return false; // Do not continue
	}

	return true;
}

/************************************************************************************************/

int main(int argc, char **argv) {
	// Get a random number generator
	Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

	bool verbose;
	double sigma, sigmaSigma, minSigma, maxSigma;
	std::uint32_t maxIter, adaptionThreshold;
	std::string resultFile;

	if (!parseCommandLine(
		argc, argv
		, sigma
		, sigmaSigma
		, minSigma
		, maxSigma
		, adaptionThreshold
		, resultFile
		, maxIter
	)) { exit(1); }

	// The adaptor object to be tested
	std::shared_ptr<GDoubleGaussAdaptor> gdga(
		new GDoubleGaussAdaptor(sigma, sigmaSigma, minSigma, maxSigma));
	gdga->setAdaptionThreshold(adaptionThreshold);

	// Create the GPlotDesigner object
	GPlotDesigner gpd("GDoubleGaussAdaptor Tests", 2,3);
	gpd.setCanvasDimensions(1200,1200);

	// 2D Graph for the value of the mutation-subject
	std::shared_ptr<GGraph2D> gmutpar_iter_ptr(new GGraph2D());
	gmutpar_iter_ptr->setPlotMode(Gem::Common::graphPlotMode::SCATTER);
	gmutpar_iter_ptr->setPlotLabel("Mutation parameter (iteration)");
	gmutpar_iter_ptr->setXAxisLabel("Iteration");
	gmutpar_iter_ptr->setYAxisLabel("Value of mutation parameter");

	// 2D Graph for the difference between current and last mutation parameter
	std::shared_ptr<GGraph2D> gmutpardiff_iter_ptr(new GGraph2D());
	gmutpardiff_iter_ptr->setPlotMode(Gem::Common::graphPlotMode::SCATTER);
	gmutpardiff_iter_ptr->setPlotLabel("Difference between consecutive mutation parameters (iteration)");
	gmutpardiff_iter_ptr->setXAxisLabel("Iteration");
	gmutpardiff_iter_ptr->setYAxisLabel("Difference");

	// 2D Graph for sigma as a function of the iteration
	std::shared_ptr<GGraph2D> gsigma_iter_ptr(new GGraph2D());
	gsigma_iter_ptr->setPlotMode(Gem::Common::graphPlotMode::SCATTER);
	gsigma_iter_ptr->setPlotLabel("Sigma as a function of the iteration");
	gsigma_iter_ptr->setXAxisLabel("Iteration");
	gsigma_iter_ptr->setYAxisLabel("Sigma");

	// Fill the objects with data
	double mutVal = 0.;
	double mutValOld=0;
	for (std::uint32_t i = 0; i < maxIter; i++) {
		mutValOld = mutVal;

		gdga->adapt(mutVal, 1., gr);

		(*gmutpar_iter_ptr) & std::tuple<double, double>((double)i, mutVal); // The new value of sigma
		(*gmutpardiff_iter_ptr) & std::tuple<double, double>((double)i, mutVal - mutValOld); // Difference between last known value and current value
		(*gsigma_iter_ptr) & std::tuple<double, double>((double)i, gdga->getSigma()); // Value of sigma after sigma-adaption
	}

	// Register the plots (and projections) with the plot designer
	gpd.registerPlotter(gmutpar_iter_ptr);
	gpd.registerPlotter(gmutpar_iter_ptr->projectY(DEFAULTNBINSGPD, std::tuple<double, double>()));
	gpd.registerPlotter(gmutpardiff_iter_ptr);
	gpd.registerPlotter(gmutpardiff_iter_ptr->projectY(DEFAULTNBINSGPD, std::tuple<double, double>()));
	gpd.registerPlotter(gsigma_iter_ptr);
	gpd.registerPlotter(gsigma_iter_ptr->projectY(DEFAULTNBINSGPD, std::tuple<double, double>()));

	// Write the result to disk
	gpd.writeToFile(resultFile);

	return 0;
}
