/**
 * @file GPlotDesignerTest.cpp
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

// Standard headers go here
#include <cmath>
#include <iostream>

// Boost headers go here
#include <boost/math/constants/constants.hpp>

// Geneva headers go here
#include "common/GPlotDesigner.hpp"

using namespace Gem::Common;

int main(int argc, char** argv) {
	std::tuple<double,double> minMaxX(-boost::math::constants::pi<double>(),boost::math::constants::pi<double>());
	std::tuple<double,double> minMaxY(-boost::math::constants::pi<double>(),boost::math::constants::pi<double>());

	std::shared_ptr<GGraph2D> gsin_ptr(new GGraph2D());
	gsin_ptr->setPlotMode(Gem::Common::graphPlotMode::SCATTER);
	gsin_ptr->setPlotLabel("Sine and cosine functions, plotted through TGraph");
	gsin_ptr->setXAxisLabel("x");
	gsin_ptr->setYAxisLabel("sin(x) vs. cos(x)");

	std::shared_ptr<GGraph2D> gcos_ptr(new GGraph2D());
	gcos_ptr->setPlotMode(Gem::Common::graphPlotMode::SCATTER);
	gcos_ptr->setPlotLabel("A cosine function, plotted through TGraph");
	gcos_ptr->setXAxisLabel("x");
	gcos_ptr->setYAxisLabel("cos(x)");

	std::shared_ptr<GGraph2D> gcos_ptr_2(new GGraph2D());
	gcos_ptr_2->setPlotMode(Gem::Common::graphPlotMode::SCATTER);
	gsin_ptr->registerSecondaryPlotter(gcos_ptr_2);

	for(std::size_t i=0; i<1000; i++) {
		double x = 2*boost::math::constants::pi<double>()*double(i)/1000. - boost::math::constants::pi<double>();

		(*gsin_ptr) & std::tuple<double, double>(x, sin(x));
		(*gcos_ptr) & std::tuple<double, double>(x, cos(x));
		(*gcos_ptr_2) & std::tuple<double, double>(x, cos(x));
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
