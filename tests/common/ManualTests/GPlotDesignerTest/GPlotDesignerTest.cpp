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

// Boost headers go here
#include <boost/math/constants/constants.hpp>

// Geneva headers go here
#include "common/GPlotDesigner.hpp"

using namespace Gem::Common;

int main(int argc, char** argv) {
	boost::tuple<double,double> minMaxX(-boost::math::constants::pi<double>(),boost::math::constants::pi<double>());
	boost::tuple<double,double> minMaxY(-boost::math::constants::pi<double>(),boost::math::constants::pi<double>());

	std::shared_ptr<GGraph2D> gsin_ptr(new GGraph2D());
	gsin_ptr->setPlotMode(Gem::Common::SCATTER);
	gsin_ptr->setPlotLabel("Sine and cosine functions, plotted through TGraph");
	gsin_ptr->setXAxisLabel("x");
	gsin_ptr->setYAxisLabel("sin(x) vs. cos(x)");

	std::shared_ptr<GGraph2D> gcos_ptr(new GGraph2D());
	gcos_ptr->setPlotMode(Gem::Common::SCATTER);
	gcos_ptr->setPlotLabel("A cosine function, plotted through TGraph");
	gcos_ptr->setXAxisLabel("x");
	gcos_ptr->setYAxisLabel("cos(x)");

	std::shared_ptr<GGraph2D> gcos_ptr_2(new GGraph2D());
	gcos_ptr_2->setPlotMode(Gem::Common::SCATTER);
	gsin_ptr->registerSecondaryPlotter(gcos_ptr_2);

	for(std::size_t i=0; i<1000; i++) {
		double x = 2*boost::math::constants::pi<double>()*double(i)/1000. - boost::math::constants::pi<double>();

		(*gsin_ptr) & boost::tuple<double, double>(x, sin(x));
		(*gcos_ptr) & boost::tuple<double, double>(x, cos(x));
		(*gcos_ptr_2) & boost::tuple<double, double>(x, cos(x));
	}

	std::shared_ptr<GFunctionPlotter1D> gsin_plotter_1D_ptr(new GFunctionPlotter1D("sin(x)", minMaxX));
	gsin_plotter_1D_ptr->setPlotLabel("A sine function, plotted through TF1");
	gsin_plotter_1D_ptr->setXAxisLabel("x");
	gsin_plotter_1D_ptr->setYAxisLabel("sin(x)");

	std::shared_ptr<GFunctionPlotter1D> gcos_plotter_1D_ptr(new GFunctionPlotter1D("cos(x)", minMaxX));
	gcos_plotter_1D_ptr->setPlotLabel("A cosine function, plotted through TF1");
	gcos_plotter_1D_ptr->setXAxisLabel("x");
	gcos_plotter_1D_ptr->setYAxisLabel("cos(x)");

	std::shared_ptr<GFunctionPlotter2D> schwefel_plotter_2D_ptr(new GFunctionPlotter2D("-0.5*(x*sin(sqrt(abs(x))) + y*sin(sqrt(abs(y))))", minMaxX, minMaxY));
	schwefel_plotter_2D_ptr->setPlotLabel("The Schwefel function");
	schwefel_plotter_2D_ptr->setXAxisLabel("x");
	schwefel_plotter_2D_ptr->setYAxisLabel("y");
	schwefel_plotter_2D_ptr->setYAxisLabel("Schwefel function");
	schwefel_plotter_2D_ptr->setDrawingArguments("surf1");

	std::shared_ptr<GFunctionPlotter2D> noisyParabola_plotter_2D_ptr(new GFunctionPlotter2D("(cos(x^2+y^2) + 2)*(x^2+y^2)", minMaxX, minMaxY));
	noisyParabola_plotter_2D_ptr->setPlotLabel("The noisy parabola");
	noisyParabola_plotter_2D_ptr->setXAxisLabel("x");
	noisyParabola_plotter_2D_ptr->setYAxisLabel("y");
	noisyParabola_plotter_2D_ptr->setYAxisLabel("Noisy parabola");
	noisyParabola_plotter_2D_ptr->setDrawingArguments("surf1");

	GPlotDesigner gpd("Sine and cosine and 2D-functions", 2,3);

	gpd.setCanvasDimensions(1200,1400);
	gpd.registerPlotter(gsin_ptr);
	gpd.registerPlotter(gcos_ptr);
	gpd.registerPlotter(gsin_plotter_1D_ptr);
	gpd.registerPlotter(gcos_plotter_1D_ptr);
	gpd.registerPlotter(schwefel_plotter_2D_ptr);
	gpd.registerPlotter(noisyParabola_plotter_2D_ptr);

	gpd.writeToFile("result.C");
}
