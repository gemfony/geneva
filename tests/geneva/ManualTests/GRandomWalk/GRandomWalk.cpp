/**
 * @file GRandomWalk.cpp
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
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>

// Boost header files go here
#include "boost/lexical_cast.hpp"

// Geneva header files go here
#include "common/GPlotDesigner.hpp"
#include "geneva-individuals/GTestIndividual2.hpp"

using namespace Gem::Common;
using namespace Gem::Geneva;
using namespace Gem::Tests;

const std::size_t NPOINTS = 1000;

int main(int argc, char **argv) {
	std::string caption = "Random walks by adaption of different FP-based parameter objects";
	GPlotDesigner gpd(caption, 2, 3);

	std::shared_ptr<GGraph2D> gdo_adapt_ptr(new GGraph2D());
	gdo_adapt_ptr->setPlotMode(Gem::Common::graphPlotMode::CURVE);
	gdo_adapt_ptr->setPlotLabel("GDoubleObject");
	gdo_adapt_ptr->setXAxisLabel("x");
	gdo_adapt_ptr->setYAxisLabel("y");
	// gdo_adapt_ptr->setDrawArrows();

	std::shared_ptr<GGraph2D> gcdo_adapt_ptr(new GGraph2D());
	gcdo_adapt_ptr->setPlotMode(Gem::Common::graphPlotMode::CURVE);
	gcdo_adapt_ptr->setPlotLabel("GConstrainedDoubleObject");
	gcdo_adapt_ptr->setXAxisLabel("x");
	gcdo_adapt_ptr->setYAxisLabel("y");
	// gcdo_adapt_ptr->setDrawArrows();

	std::shared_ptr<GGraph2D> gcdoc_adapt_ptr(new GGraph2D());
	gcdoc_adapt_ptr->setPlotMode(Gem::Common::graphPlotMode::CURVE);
	gcdoc_adapt_ptr->setPlotLabel("GConstrainedDoubleObjectCollection");
	gcdoc_adapt_ptr->setXAxisLabel("x");
	gcdoc_adapt_ptr->setYAxisLabel("y");
	// gcdoc_adapt_ptr->setDrawArrows();

	std::shared_ptr<GGraph2D> gdc_adapt_ptr(new GGraph2D());
	gdc_adapt_ptr->setPlotMode(Gem::Common::graphPlotMode::CURVE);
	gdc_adapt_ptr->setPlotLabel("GDoubleCollection");
	gdc_adapt_ptr->setXAxisLabel("x");
	gdc_adapt_ptr->setYAxisLabel("y");
	// gdc_adapt_ptr->setDrawArrows();

	std::shared_ptr<GGraph2D> gcdc_adapt_ptr(new GGraph2D());
	gcdc_adapt_ptr->setPlotMode(Gem::Common::graphPlotMode::CURVE);
	gcdc_adapt_ptr->setPlotLabel("GConstrainedDoubleCollection");
	gcdc_adapt_ptr->setXAxisLabel("x");
	gcdc_adapt_ptr->setYAxisLabel("y");
	// gcdc_adapt_ptr->setDrawArrows();

	for(std::size_t o=0; o<NPERFOBJECTTYPES; o++) {
		// Create a GTestIndividual2 object of size 2
		std::shared_ptr<GTestIndividual2> gti_ptr(new GTestIndividual2(2,PERFOBJECTTYPE(o)));

		std::vector<double> par;
		for(std::size_t i=0; i<NPOINTS; i++) {
			gti_ptr->streamline(par);

			switch(o) {
				case 0:
					(*gdo_adapt_ptr) & std::tuple<double,double>(par[0], par[1]);
					break;

				case 1:
					(*gcdo_adapt_ptr) & std::tuple<double,double>(par[0], par[1]);
					break;

				case 2:
					(*gcdoc_adapt_ptr) & std::tuple<double,double>(par[0], par[1]);
					break;

				case 3:
					(*gdc_adapt_ptr) & std::tuple<double,double>(par[0], par[1]);
					break;

				case 4:
					(*gcdc_adapt_ptr) & std::tuple<double,double>(par[0], par[1]);
					break;

				default:
					throw gemfony_exception(
						g_error_streamer(DO_LOG,  time_and_place)
							<< "Error in main(): Incorrect object type requested: " << o << std::endl
					);
					break;
			}

			gti_ptr->adapt();
		}
	}

	gpd.registerPlotter(gdo_adapt_ptr);
	gpd.registerPlotter(gcdo_adapt_ptr);
	gpd.registerPlotter(gcdoc_adapt_ptr);
	gpd.registerPlotter(gdc_adapt_ptr);
	gpd.registerPlotter(gcdc_adapt_ptr);

	// Emit the result file
	gpd.writeToFile("result.C");
}
