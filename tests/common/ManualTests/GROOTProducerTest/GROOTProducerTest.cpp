/**
 * @file GROOTProducerTest.hpp
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

// Standard headers go here
#include <cmath>
#include <iostream>

// Boost headers go here
#include <boost/shared_ptr.hpp>

// Geneva headers go here
#include "common/GPlotDesigner.hpp"

using namespace Gem::Common;

int main(int argc, char** argv) {
	boost::shared_ptr<GGraph2D> gsin_ptr(new GGraph2D());
	boost::shared_ptr<GGraph2D> gcos_ptr(new GGraph2D());

	gsin_ptr->setPlotMode(Gem::Common::SCATTER);
	gsin_ptr->setPlotLabel("A sinus function");
	gsin_ptr->setXAxisLabel("x");
	gsin_ptr->setYAxisLabel("sin(x)");

	gcos_ptr->setPlotMode(Gem::Common::SCATTER);
	gcos_ptr->setPlotLabel("A cosinus function");
	gcos_ptr->setXAxisLabel("x");
	gcos_ptr->setYAxisLabel("cos(x)");

	for(std::size_t i=0; i<1000; i++) {
		(*gsin_ptr) & boost::tuple<std::size_t, double>(i, sin(2*M_PI*double(i)/1000.));
	}

	for(std::size_t i=0; i<1000; i++) {
		(*gcos_ptr) & boost::tuple<std::size_t, double>(i, cos(2*M_PI*double(i)/1000.));
	}

	GPlotDesigner gpd(2,1);
	gpd.setCanvasLabel("Sinus and cosinus");

	gpd.registerPlotter(gsin_ptr);
	gpd.registerPlotter(gcos_ptr);

	std::cout << gpd.plot();
}
