/**
 * @file GFloatGaussAdaptorTest.cpp
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

/*
 * This test adapts a float a given number of times and records the values of
 * different entities of a GFloatGaussAdaptor as a function of the iteration.
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
#include "geneva/GFloatGaussAdaptor.hpp"
#include "common/GPlotDesigner.hpp"

// Parsing of the command line
#include "GCommandLineParser.hpp"

using namespace Gem::Tests;
using namespace Gem::Geneva;
using namespace Gem::Common;
using namespace Gem::Hap;

int main(int argc, char **argv) {
	bool verbose;
	float sigma, sigmaSigma, minSigma, maxSigma;
	boost::uint32_t maxIter, adaptionThreshold;
	std::string resultFile;

	if (!parseCommandLine(argc, argv,
		                  sigma,
		                  sigmaSigma,
		                  minSigma,
		                  maxSigma,
		                  adaptionThreshold,
		                  resultFile,
		                  maxIter,
		                  verbose))
	{ exit(1); }

	// The adaptor object to be tested
	boost::shared_ptr<GFloatGaussAdaptor> gdga(
			new GFloatGaussAdaptor(sigma, sigmaSigma, minSigma, maxSigma));
	gdga->setAdaptionThreshold(adaptionThreshold);

	// Create the GPlotDesigner object
	GPlotDesigner gpd("GFloatGaussAdaptor Tests", 2,3);
	gpd.setCanvasDimensions(1200,1200);

	// 2D Graph for the value of the mutation-subject
	boost::shared_ptr<GGraph2D> gmutpar_iter_ptr(new GGraph2D());
	gmutpar_iter_ptr->setPlotMode(Gem::Common::SCATTER);
	gmutpar_iter_ptr->setPlotLabel("Mutation parameter (iteration)");
	gmutpar_iter_ptr->setXAxisLabel("Iteration");
	gmutpar_iter_ptr->setYAxisLabel("Value of mutation parameter");

	// 2D Graph for the difference between current and last mutation parameter
	boost::shared_ptr<GGraph2D> gmutpardiff_iter_ptr(new GGraph2D());
	gmutpardiff_iter_ptr->setPlotMode(Gem::Common::SCATTER);
	gmutpardiff_iter_ptr->setPlotLabel("Difference between consecutive mutation parameters (iteration)");
	gmutpardiff_iter_ptr->setXAxisLabel("Iteration");
	gmutpardiff_iter_ptr->setYAxisLabel("Difference");

	// 2D Graph for sigma as a function of the iteration
	boost::shared_ptr<GGraph2D> gsigma_iter_ptr(new GGraph2D());
	gsigma_iter_ptr->setPlotMode(Gem::Common::SCATTER);
	gsigma_iter_ptr->setPlotLabel("Sigma as a function of the iteration");
	gsigma_iter_ptr->setXAxisLabel("Iteration");
	gsigma_iter_ptr->setYAxisLabel("Sigma");

	// Fill the objects with data
	float mutVal = 0.;
	float mutValOld=0;
	for (boost::uint32_t i = 0; i < maxIter; i++) {
		mutValOld = mutVal;

		gdga->adapt(mutVal);

		(*gmutpar_iter_ptr) & boost::tuple<double, double>((double)i, (double)mutVal); // The new value of sigma
		(*gmutpardiff_iter_ptr) & boost::tuple<double, double>((double)i, (double)(mutVal - mutValOld)); // Difference between last known value and current value
		(*gsigma_iter_ptr) & boost::tuple<double, double>((double)i, (double)(gdga->getSigma())); // Value of sigma after sigma-adaption
	}

	// Register the plots (and projections) with the plot designer
	gpd.registerPlotter(gmutpar_iter_ptr);
	gpd.registerPlotter(gmutpar_iter_ptr->projectY());
	gpd.registerPlotter(gmutpardiff_iter_ptr);
	gpd.registerPlotter(gmutpardiff_iter_ptr->projectY());
	gpd.registerPlotter(gsigma_iter_ptr);
	gpd.registerPlotter(gsigma_iter_ptr->projectY());

	// Write the result to disk
	gpd.writeToFile("result.C");

	return 0;
}
