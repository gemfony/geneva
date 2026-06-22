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
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

// Standard headers go here
#include <cmath>
#include <iostream>
#include <numbers>

// Geneva headers go here
#include "common/GPlotDesigner.hpp"

using namespace Gem::Common;

int main(int argc, char **argv) {
    std::tuple<double, double> minMaxX(
        -std::numbers::pi,
        std::numbers::pi
    );
    std::tuple<double, double> minMaxY(
        -std::numbers::pi,
        std::numbers::pi
    );

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

    for(std::size_t i = 0; i < 1000; i++) {
        double x = 2 * std::numbers::pi * static_cast<double>(i) / 1000. -
                   std::numbers::pi;

        (*gsin_ptr) & std::tuple<double, double>(x, sin(x));
        (*gcos_ptr) & std::tuple<double, double>(x, cos(x));
        (*gcos_ptr_2) & std::tuple<double, double>(x, cos(x));
    }

    std::shared_ptr<GFunctionPlotter1D> gsin_plotter_1D_ptr(
        new GFunctionPlotter1D("sin(x)", minMaxX)
    );
    gsin_plotter_1D_ptr->setPlotLabel("A sine function, plotted through TF1");
    gsin_plotter_1D_ptr->setXAxisLabel("x");
    gsin_plotter_1D_ptr->setYAxisLabel("sin(x)");

    std::shared_ptr<GFunctionPlotter1D> gcos_plotter_1D_ptr(
        new GFunctionPlotter1D("cos(x)", minMaxX)
    );
    gcos_plotter_1D_ptr->setPlotLabel("A cosine function, plotted through TF1");
    gcos_plotter_1D_ptr->setXAxisLabel("x");
    gcos_plotter_1D_ptr->setYAxisLabel("cos(x)");

    std::shared_ptr<GFunctionPlotter2D> schwefel_plotter_2D_ptr(
        new GFunctionPlotter2D("-0.5*(x*sin(sqrt(abs(x))) + y*sin(sqrt(abs(y))))", minMaxX, minMaxY)
    );
    schwefel_plotter_2D_ptr->setPlotLabel("The Schwefel function");
    schwefel_plotter_2D_ptr->setXAxisLabel("x");
    schwefel_plotter_2D_ptr->setYAxisLabel("y");
    schwefel_plotter_2D_ptr->setYAxisLabel("Schwefel function");
    schwefel_plotter_2D_ptr->setDrawingArguments("surf1");

    std::shared_ptr<GFunctionPlotter2D> noisyParabola_plotter_2D_ptr(
        new GFunctionPlotter2D("(cos(x^2+y^2) + 2)*(x^2+y^2)", minMaxX, minMaxY)
    );
    noisyParabola_plotter_2D_ptr->setPlotLabel("The noisy parabola");
    noisyParabola_plotter_2D_ptr->setXAxisLabel("x");
    noisyParabola_plotter_2D_ptr->setYAxisLabel("y");
    noisyParabola_plotter_2D_ptr->setYAxisLabel("Noisy parabola");
    noisyParabola_plotter_2D_ptr->setDrawingArguments("surf1");

    GPlotDesigner gpd("Sine and cosine and 2D-functions", 2, 3);

    gpd.setCanvasDimensions(1200, 1400);
    gpd.registerPlotter(gsin_ptr);
    gpd.registerPlotter(gcos_ptr);
    gpd.registerPlotter(gsin_plotter_1D_ptr);
    gpd.registerPlotter(gcos_plotter_1D_ptr);
    gpd.registerPlotter(schwefel_plotter_2D_ptr);
    gpd.registerPlotter(noisyParabola_plotter_2D_ptr);

    gpd.writeToFile("result.C");

    // -------------------------------------------------------------------------
    // A graphs-only designer emitted through the gnuplot backend. The gnuplot
    // backend supports only the graph plotters (GGraph2D/2ED/3D/4D), so this uses
    // a separate designer that registers only graphs (no functions / histograms).
    std::shared_ptr<GGraph2D> gp_sin_ptr(new GGraph2D());
    gp_sin_ptr->setPlotMode(Gem::Common::graphPlotMode::SCATTER);
    gp_sin_ptr->setPlotLabel("sin(x), gnuplot");
    gp_sin_ptr->setXAxisLabel("x");
    gp_sin_ptr->setYAxisLabel("sin(x)");

    // A secondary plotter sharing the first pad (a second inline dataset).
    std::shared_ptr<GGraph2D> gp_cos_ptr(new GGraph2D());
    gp_cos_ptr->setPlotMode(Gem::Common::graphPlotMode::SCATTER);
    gp_cos_ptr->setPlotLabel("cos(x), gnuplot");
    gp_sin_ptr->registerSecondaryPlotter(gp_cos_ptr);

    std::shared_ptr<GGraph3D> gp_helix_ptr(new GGraph3D());
    gp_helix_ptr->setPlotLabel("a helix, gnuplot");
    gp_helix_ptr->setXAxisLabel("x");
    gp_helix_ptr->setYAxisLabel("y");
    gp_helix_ptr->setZAxisLabel("z");

    for(std::size_t i = 0; i < 200; i++) {
        double x = 2 * std::numbers::pi * static_cast<double>(i) / 200. - std::numbers::pi;
        (*gp_sin_ptr) & std::tuple<double, double>(x, std::sin(x));
        (*gp_cos_ptr) & std::tuple<double, double>(x, std::cos(x));

        double t = 4 * std::numbers::pi * static_cast<double>(i) / 200.;
        (*gp_helix_ptr) &
            std::tuple<double, double, double>(std::cos(t), std::sin(t), t);
    }

    GPlotDesigner gpd_gnuplot("Graphs through gnuplot", 1, 2);
    gpd_gnuplot.setPlotBackend(Gem::Common::plotBackend::GNUPLOT);
    gpd_gnuplot.registerPlotter(gp_sin_ptr);
    gpd_gnuplot.registerPlotter(gp_helix_ptr);
    gpd_gnuplot.writeToFile("result.gp");
}
