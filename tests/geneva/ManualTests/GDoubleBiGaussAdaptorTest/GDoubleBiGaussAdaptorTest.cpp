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
#include "geneva/GDoubleBiGaussAdaptor.hpp"
#include "common/GPlotDesigner.hpp"

// Parsing of the command line
#include "GCommandLineParser.hpp"

using namespace Gem::Tests;
using namespace Gem::Geneva;
using namespace Gem::Common;
using namespace Gem::Hap;

int main(int argc, char **argv) {
	bool verbose;
	double sigma1, sigmaSigma1, minSigma1, maxSigma1;
	double sigma2, sigmaSigma2, minSigma2, maxSigma2;
	double delta, sigmaDelta, minDelta, maxDelta;
	std::uint32_t maxIter, adaptionThreshold;
	std::string resultFile;

	if (!parseCommandLine(argc, argv
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
								 , verbose
	))
	{ exit(1); }

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

		gdbga_ptr->adapt(mutVal, 1.);

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
	gpd.writeToFile("result.C");

	return 0;
}
