/**
 * @file GDoubleGaussAdaptorTest.cpp
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

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_io.hpp>
#include <boost/tuple/tuple_comparison.hpp>

// Geneva header files go here
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "common/GPlotDesigner.hpp"

// Parsing of the command line
#include "GCommandLineParser.hpp"

using namespace Gem::Tests;
using namespace Gem::Geneva;
using namespace Gem::Common;
using namespace Gem::Hap;

int main(int argc, char **argv) {
	bool verbose;
	double sigma, sigmaSigma, minSigma, maxSigma;
	boost::uint32_t maxIter, adaptionThreshold;
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
      , verbose))
	{ exit(1); }

	// The adaptor object to be tested
	std::shared_ptr<GDoubleGaussAdaptor> gdga(
			new GDoubleGaussAdaptor(sigma, sigmaSigma, minSigma, maxSigma));
	gdga->setAdaptionThreshold(adaptionThreshold);

	// Create the GPlotDesigner object
	GPlotDesigner gpd("GDoubleGaussAdaptor Tests", 2,3);
	gpd.setCanvasDimensions(1200,1200);

	// 2D Graph for the value of the mutation-subject
	std::shared_ptr<GGraph2D> gmutpar_iter_ptr(new GGraph2D());
	gmutpar_iter_ptr->setPlotMode(Gem::Common::SCATTER);
	gmutpar_iter_ptr->setPlotLabel("Mutation parameter (iteration)");
	gmutpar_iter_ptr->setXAxisLabel("Iteration");
	gmutpar_iter_ptr->setYAxisLabel("Value of mutation parameter");

	// 2D Graph for the difference between current and last mutation parameter
	std::shared_ptr<GGraph2D> gmutpardiff_iter_ptr(new GGraph2D());
	gmutpardiff_iter_ptr->setPlotMode(Gem::Common::SCATTER);
	gmutpardiff_iter_ptr->setPlotLabel("Difference between consecutive mutation parameters (iteration)");
	gmutpardiff_iter_ptr->setXAxisLabel("Iteration");
	gmutpardiff_iter_ptr->setYAxisLabel("Difference");

	// 2D Graph for sigma as a function of the iteration
	std::shared_ptr<GGraph2D> gsigma_iter_ptr(new GGraph2D());
	gsigma_iter_ptr->setPlotMode(Gem::Common::SCATTER);
	gsigma_iter_ptr->setPlotLabel("Sigma as a function of the iteration");
	gsigma_iter_ptr->setXAxisLabel("Iteration");
	gsigma_iter_ptr->setYAxisLabel("Sigma");

	// Fill the objects with data
	double mutVal = 0.;
	double mutValOld=0;
	for (boost::uint32_t i = 0; i < maxIter; i++) {
		mutValOld = mutVal;

		gdga->adapt(mutVal, 1.);

		(*gmutpar_iter_ptr) & boost::tuple<double, double>((double)i, mutVal); // The new value of sigma
		(*gmutpardiff_iter_ptr) & boost::tuple<double, double>((double)i, mutVal - mutValOld); // Difference between last known value and current value
		(*gsigma_iter_ptr) & boost::tuple<double, double>((double)i, gdga->getSigma()); // Value of sigma after sigma-adaption
	}

	// Register the plots (and projections) with the plot designer
	gpd.registerPlotter(gmutpar_iter_ptr);
	gpd.registerPlotter(gmutpar_iter_ptr->projectY(DEFAULTNBINSGPD, boost::tuple<double, double>()));
	gpd.registerPlotter(gmutpardiff_iter_ptr);
	gpd.registerPlotter(gmutpardiff_iter_ptr->projectY(DEFAULTNBINSGPD, boost::tuple<double, double>()));
	gpd.registerPlotter(gsigma_iter_ptr);
	gpd.registerPlotter(gsigma_iter_ptr->projectY(DEFAULTNBINSGPD, boost::tuple<double, double>()));

	// Write the result to disk
	gpd.writeToFile("result.C");

	return 0;
}
