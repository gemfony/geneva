/**
 * @file GRandomWalk.cpp
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
	gdo_adapt_ptr->setPlotMode(Gem::Common::CURVE);
	gdo_adapt_ptr->setPlotLabel("GDoubleObject");
	gdo_adapt_ptr->setXAxisLabel("x");
	gdo_adapt_ptr->setYAxisLabel("y");
	// gdo_adapt_ptr->setDrawArrows();

	std::shared_ptr<GGraph2D> gcdo_adapt_ptr(new GGraph2D());
	gcdo_adapt_ptr->setPlotMode(Gem::Common::CURVE);
	gcdo_adapt_ptr->setPlotLabel("GConstrainedDoubleObject");
	gcdo_adapt_ptr->setXAxisLabel("x");
	gcdo_adapt_ptr->setYAxisLabel("y");
	// gcdo_adapt_ptr->setDrawArrows();

	std::shared_ptr<GGraph2D> gcdoc_adapt_ptr(new GGraph2D());
	gcdoc_adapt_ptr->setPlotMode(Gem::Common::CURVE);
	gcdoc_adapt_ptr->setPlotLabel("GConstrainedDoubleObjectCollection");
	gcdoc_adapt_ptr->setXAxisLabel("x");
	gcdoc_adapt_ptr->setYAxisLabel("y");
	// gcdoc_adapt_ptr->setDrawArrows();

	std::shared_ptr<GGraph2D> gdc_adapt_ptr(new GGraph2D());
	gdc_adapt_ptr->setPlotMode(Gem::Common::CURVE);
	gdc_adapt_ptr->setPlotLabel("GDoubleCollection");
	gdc_adapt_ptr->setXAxisLabel("x");
	gdc_adapt_ptr->setYAxisLabel("y");
	// gdc_adapt_ptr->setDrawArrows();

	std::shared_ptr<GGraph2D> gcdc_adapt_ptr(new GGraph2D());
	gcdc_adapt_ptr->setPlotMode(Gem::Common::CURVE);
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
				(*gdo_adapt_ptr) & boost::tuple<double,double>(par[0], par[1]);
				break;

			case 1:
				(*gcdo_adapt_ptr) & boost::tuple<double,double>(par[0], par[1]);
				break;

			case 2:
				(*gcdoc_adapt_ptr) & boost::tuple<double,double>(par[0], par[1]);
				break;

			case 3:
				(*gdc_adapt_ptr) & boost::tuple<double,double>(par[0], par[1]);
				break;

			case 4:
				(*gcdc_adapt_ptr) & boost::tuple<double,double>(par[0], par[1]);
				break;

			default:
				std::cerr << "Error in main(): Incorrect object type requested: " << o << std::endl;
				exit(1);
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
