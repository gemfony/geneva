/**
 * @file GDoubleBiGaussAdaptorTest.cpp
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
#include "common/GPlotDesigner.hpp"
#include "common/GCommonEnums.hpp"
#include "geneva/GDoubleBiGaussAdaptor.hpp"

using namespace Gem::Geneva;
using namespace Gem::Common;
using namespace Gem::Hap;


/************************************************************************************************/
// Default settings
const double CMD_DEFAULTSIGMA1 = 1.;
const double CMD_DEFAULTSIGMA2 = 1.;
const double CMD_DEFAULTSIGMASIGMA1 = 0.001;
const double CMD_DEFAULTSIGMASIGMA2 = 0.001;
const double CMD_DEFAULTMINSIGMA1 = 0.002;
const double CMD_DEFAULTMAXSIGMA1 = 4.;
const double CMD_DEFAULTMINSIGMA2 = 0.002;
const double CMD_DEFAULTMAXSIGMA2 = 4.;
const double CMD_DEFAULTDELTA = 0.5;
const double CMD_DEFAULTSIGMADELTA = 0.8;
const double CMD_DEFAULTMINDELTA = 0.001;
const double CMD_DEFAULTMAXDELTA = 2.;
const std::uint32_t CMD_DEFAULTMAXITER = 100000;
const std::string CMD_DEFAULTRESULTFILE = "result.C";
const bool CMD_DEFAULTVERBOSE = true;
const std::uint32_t CMD_DEFAULTADAPTIONTHRESHOLD=1;

/************************************************************************************************/
/**
 * A function that parses the command line for all required parameters
 */
bool parseCommandLine (
	int argc, char **argv
	, double& sigma1
	, double& sigmaSigma1
	, double& minSigma1
	, double& maxSigma1
	, double& sigma2
	, double& sigmaSigma2
	, double& minSigma2
	, double& maxSigma2
	, double& delta
	, double& sigmaDelta
	, double& minDelta
	, double& maxDelta
	, std::uint32_t& adaptionThreshold
	, std::string& resultFile
	, std::uint32_t& maxIter
){
	// Create the parser builder
	Gem::Common::GParserBuilder gpb;

	gpb.registerCLParameter<double>(
		"sigma1"
		, sigma1
		, CMD_DEFAULTSIGMA1
		, "Width of the first gaussian"
	);

	gpb.registerCLParameter<double>(
		"sigmaSigma1"
		, sigmaSigma1
		, CMD_DEFAULTSIGMASIGMA1
		, "Width of the gaussian used to adapt sigma1"
	);

	gpb.registerCLParameter<double>(
		"minSigma1"
		, minSigma1
		, CMD_DEFAULTMINSIGMA1
		, "Minimal allowed value of sigma1"
	);

	gpb.registerCLParameter<double>(
		"maxSigma1"
		, maxSigma1
		, CMD_DEFAULTMAXSIGMA1
		, "Maximum allowed value of sigma1"
	);

	gpb.registerCLParameter<double>(
		"sigma2"
		, sigma2
		, CMD_DEFAULTSIGMA2
		, "Width of the second gaussian"
	);

	gpb.registerCLParameter<double>(
		"sigmaSigma2"
		, sigmaSigma2
		, CMD_DEFAULTSIGMASIGMA2
		, "Width of the gaussian used to adapt sigma2"
	);

	gpb.registerCLParameter<double>(
		"minSigma2"
		, minSigma2
		, CMD_DEFAULTMINSIGMA2
		, "Minimal allowed value of sigma2"
	);

	gpb.registerCLParameter<double>(
		"maxSigma2"
		, maxSigma2
		, CMD_DEFAULTMAXSIGMA2
		, "Maximum allowed value of sigma2"
	);

	gpb.registerCLParameter<double>(
		"delta"
		, delta
		, CMD_DEFAULTDELTA
		, "Distance between both gaussians"
	);

	gpb.registerCLParameter<double>(
		"sigmaDelta"
		, sigmaDelta
		, CMD_DEFAULTSIGMADELTA
		, "Width of the gaussian used to adapt delta"
	);

	gpb.registerCLParameter<double>(
		"minDelta"
		, minDelta
		, CMD_DEFAULTMINDELTA
		, "Minimal allowed value for delta"
	);

	gpb.registerCLParameter<double>(
		"maxDelta"
		, maxDelta
		, CMD_DEFAULTMAXDELTA
		, "Maximum allowed value for delta"
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
	Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::PROXY> gr;

	double sigma1, sigmaSigma1, minSigma1, maxSigma1;
	double sigma2, sigmaSigma2, minSigma2, maxSigma2;
	double delta, sigmaDelta, minDelta, maxDelta;
	std::uint32_t maxIter, adaptionThreshold;
	std::string resultFile;

	if (!parseCommandLine(
		argc, argv
		, sigma1
	   , sigmaSigma1
	   , minSigma1
	   , maxSigma1
	   , sigma2
	   , sigmaSigma2
	   , minSigma2
	   , maxSigma2
	   , delta
	   , sigmaDelta
	   , minDelta
	   , maxDelta
	   , adaptionThreshold
	   , resultFile
	   , maxIter
	)) { exit(1); }

	// The adaptor object to be tested
	std::shared_ptr<GDoubleBiGaussAdaptor> gdbga_ptr(new GDoubleBiGaussAdaptor());

	gdbga_ptr->setUseSymmetricSigmas(false);
	gdbga_ptr->setSigma1(sigma1);
	gdbga_ptr->setSigma2(sigma2);
	gdbga_ptr->setSigma1AdaptionRate(sigmaSigma1);
	gdbga_ptr->setSigma2AdaptionRate(sigmaSigma2);
	gdbga_ptr->setSigma1Range(minSigma1,maxSigma1);
	gdbga_ptr->setSigma2Range(minSigma2,maxSigma2);
	gdbga_ptr->setDeltaAdaptionRate(sigmaDelta);
	gdbga_ptr->setDeltaRange(minDelta, maxDelta);
	gdbga_ptr->setDelta(delta);
	gdbga_ptr->setAdaptionThreshold(adaptionThreshold);

	// Create the GPlotDesigner object
	GPlotDesigner gpd("GDoubleBiGaussAdaptor Tests", 2,5);
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

	// 2D Graph for sigma1 as a function of the iteration
	std::shared_ptr<GGraph2D> gsigma1_iter_ptr(new GGraph2D());
	gsigma1_iter_ptr->setPlotMode(Gem::Common::graphPlotMode::SCATTER);
	gsigma1_iter_ptr->setPlotLabel("Sigma1 as a function of the iteration");
	gsigma1_iter_ptr->setXAxisLabel("Iteration");
	gsigma1_iter_ptr->setYAxisLabel("Sigma1");

	// 2D Graph for sigma2 as a function of the iteration
	std::shared_ptr<GGraph2D> gsigma2_iter_ptr(new GGraph2D());
	gsigma2_iter_ptr->setPlotMode(Gem::Common::graphPlotMode::SCATTER);
	gsigma2_iter_ptr->setPlotLabel("Sigma2 as a function of the iteration");
	gsigma2_iter_ptr->setXAxisLabel("Iteration");
	gsigma2_iter_ptr->setYAxisLabel("Sigma2");

	// 2D Graph for delta as a function of the iteration
	std::shared_ptr<GGraph2D> gdelta_iter_ptr(new GGraph2D());
	gdelta_iter_ptr->setPlotMode(Gem::Common::graphPlotMode::SCATTER);
	gdelta_iter_ptr->setPlotLabel("Delta as a function of the iteration");
	gdelta_iter_ptr->setXAxisLabel("Iteration");
	gdelta_iter_ptr->setYAxisLabel("Delta");


	// Fill the objects with data
	double mutVal = 0.;
	double mutValOld=0;
	for (std::uint32_t i = 0; i < maxIter; i++) {
		mutValOld = mutVal;

		gdbga_ptr->adapt(mutVal, 1., gr);

		(*gmutpar_iter_ptr) & std::tuple<double, double>((double)i, mutVal); // The new value of sigma
		(*gmutpardiff_iter_ptr) & std::tuple<double, double>((double)i, mutVal - mutValOld); // Difference between last known value and current value
		(*gsigma1_iter_ptr) & std::tuple<double, double>((double)i, gdbga_ptr->getSigma1()); // Value of sigma1 after sigma-adaption
		(*gsigma2_iter_ptr) & std::tuple<double, double>((double)i, gdbga_ptr->getSigma2()); // Value of sigma2 after sigma-adaption
		(*gdelta_iter_ptr) & std::tuple<double, double>((double)i, gdbga_ptr->getDelta()); // Value of delta after adaption
	}

	// Register the plots (and projections) with the plot designer
	gpd.registerPlotter(gmutpar_iter_ptr);
	gpd.registerPlotter(gmutpar_iter_ptr->projectY(DEFAULTNBINSGPD, std::tuple<double, double>()));
	gpd.registerPlotter(gmutpardiff_iter_ptr);
	gpd.registerPlotter(gmutpardiff_iter_ptr->projectY(DEFAULTNBINSGPD, std::tuple<double, double>()));
	gpd.registerPlotter(gsigma1_iter_ptr);
	gpd.registerPlotter(gsigma1_iter_ptr->projectY(DEFAULTNBINSGPD, std::tuple<double, double>()));
	gpd.registerPlotter(gsigma2_iter_ptr);
	gpd.registerPlotter(gsigma2_iter_ptr->projectY(DEFAULTNBINSGPD, std::tuple<double, double>()));
	gpd.registerPlotter(gdelta_iter_ptr);
	gpd.registerPlotter(gdelta_iter_ptr->projectY(DEFAULTNBINSGPD, std::tuple<double, double>()));

	// Write the result to disk
	gpd.writeToFile(resultFile);

	return 0;
}
