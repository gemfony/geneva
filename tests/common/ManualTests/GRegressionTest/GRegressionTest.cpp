/**
 * @file GPlotDesignerTest.cpp
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

// Standard headers go here
#include <cmath>
#include <iostream>
#include <chrono>

// Boost headers go here

// Geneva headers go here
#include "common/GPlotDesigner.hpp"
#include "common/GMathHelperFunctionsT.hpp"

#include "GLineFit.hpp"

using namespace Gem::Common;
using namespace Gem::Geneva;

/******************************************************************************/
/**
 * This is a mixed demo which shows both a "Geneva"- and a "Common"-lib method
 * for determining the parameters of a line which best describes a set of
 * measurements.
 */
int main(int argc, char** argv) {
	// Create a suitable data distribution for our tests. It is possible to
	// describe this distribution by a line f(x) = x, i.e. a=0, b=1
	std::vector<std::tuple<double, double>> dataPoints;
	for(std::size_t i=0; i<100; i++) {
		double x = double(i-50);
		dataPoints.push_back(std::tuple<double, double>(x, x+sin(x)));
	}

	// Determine a line through linear regression
	std::tuple<double,double,double,double> lineWithErrors = getRegressionParameters(dataPoints);

	// Let the audience know
	std::cout
	<< "f(x)=x through linear regression: " << std::endl
	<< "a = " << std::get<0>(lineWithErrors) << " +/- " << std::get<1>(lineWithErrors) << std::endl
	<< "b = " << std::get<2>(lineWithErrors) << " +/- " << std::get<3>(lineWithErrors) << std::endl;

	// Determine a line through a fit procedure
	std::tuple<double, double> line = Gem::Geneva::gLineFit(dataPoints);

	// Let the audience know
	std::cout
	<< "f(x)=x through ea fit: " << std::endl
	<< "a = " << std::get<0>(line) << std::endl
	<< "b = " << std::get<1>(line) << std::endl;
}
