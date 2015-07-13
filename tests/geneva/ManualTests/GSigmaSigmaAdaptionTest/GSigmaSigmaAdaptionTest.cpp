/**
 * @file GSigmaSigmaAdaptionTest.cpp
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
#include "common/GHelperFunctions.hpp"
#include "common/GMathHelperFunctionsT.hpp"
#include "hap/GRandomT.hpp"

using namespace Gem::Common;
using namespace Gem::Hap;

const std::size_t NPOINTS = 10000;
const std::size_t NBINS = 100;

const double sigmaStart = 0.025; // The start value
const double minSigma = 0.;
const double maxSigma = 1.;

/******************************************************************************/
/**
 * First distribution:
 * gexp(gr_ptr->normal_distribution(gfabs(sigmaSigma_))*(gr_ptr->uniform_bool()?fp_type(1):fp_type(-1)))
 */
double dist1(std::shared_ptr<Gem::Hap::GRandomBase> gr_ptr, const double& sigmaSigma) {
	return gexp(gr_ptr->normal_distribution(gfabs(sigmaSigma))*(gr_ptr->uniform_bool()?1.:-1.));
}

/**
 * Second distribution:
 * gexp(gr_ptr->normal_distribution(gfabs(sigmaSigma_)))
 */
double dist2(std::shared_ptr<Gem::Hap::GRandomBase> gr_ptr, const double& sigmaSigma) {
	return gexp(gr_ptr->normal_distribution(gfabs(sigmaSigma)));
}

/******************************************************************************/
/**
 * This program plots a number of random distributions used to adapt sigma
 * in gauss mutation. It also shows the development of a fixed value over
 * time, as it is repeatedly mutated without selection pressure.
 */
int main() {
	double sigma_1_02 = sigmaStart;
	double sigma_1_04 = sigmaStart;
	double sigma_1_06 = sigmaStart;
	double sigma_1_08 = sigmaStart;
	double sigma_2_02 = sigmaStart;
	double sigma_2_04 = sigmaStart;
	double sigma_2_06 = sigmaStart;
	double sigma_2_08 = sigmaStart;

	double fact_1_02, fact_1_04, fact_1_06, fact_1_08;
	double fact_2_02, fact_2_04, fact_2_06, fact_2_08;

	std::shared_ptr<Gem::Hap::GRandomBase> gr_ptr = std::shared_ptr<GRandomT<RANDFLAVOURS::RANDOMPROXY>>(new GRandomT<RANDFLAVOURS::RANDOMPROXY>());

	std::string caption_dist = "Different random distributions for the adaption of sigma, varying sigmaSigma";
	GPlotDesigner gpd_dist (caption_dist, 2, 4);
	gpd_dist.setCanvasDimensions(1600, 1200);

	std::string caption_devel = "Development of a single sigma upon repeated calls, varying sigmaSigma";
	GPlotDesigner gpd_devel(caption_devel, 2, 4);
	gpd_devel.setCanvasDimensions(1600, 1200);

	std::shared_ptr<GHistogram1D> dist1_02_ptr(new GHistogram1D(NBINS, 0., 3.));
	std::shared_ptr<GHistogram1D> dist1_04_ptr(new GHistogram1D(NBINS, 0., 3.));
	std::shared_ptr<GHistogram1D> dist1_06_ptr(new GHistogram1D(NBINS, 0., 3.));
	std::shared_ptr<GHistogram1D> dist1_08_ptr(new GHistogram1D(NBINS, 0., 3.));

	std::shared_ptr<GHistogram1D> dist2_02_ptr(new GHistogram1D(NBINS, 0., 3.));
	std::shared_ptr<GHistogram1D> dist2_04_ptr(new GHistogram1D(NBINS, 0., 3.));
	std::shared_ptr<GHistogram1D> dist2_06_ptr(new GHistogram1D(NBINS, 0., 3.));
	std::shared_ptr<GHistogram1D> dist2_08_ptr(new GHistogram1D(NBINS, 0., 3.));

	dist1_02_ptr->setXAxisLabel("Distribution 1 (random sign)"); dist1_02_ptr->setYAxisLabel("Number of Entries"); dist1_02_ptr->setPlotLabel("sigmaSigma = 0.2");
	dist1_04_ptr->setXAxisLabel("Distribution 1 (random sign)"); dist1_04_ptr->setYAxisLabel("Number of Entries"); dist1_04_ptr->setPlotLabel("sigmaSigma = 0.4");
	dist1_06_ptr->setXAxisLabel("Distribution 1 (random sign)"); dist1_06_ptr->setYAxisLabel("Number of Entries"); dist1_06_ptr->setPlotLabel("sigmaSigma = 0.6");
	dist1_08_ptr->setXAxisLabel("Distribution 1 (random sign)"); dist1_08_ptr->setYAxisLabel("Number of Entries"); dist1_08_ptr->setPlotLabel("sigmaSigma = 0.8");

	dist2_02_ptr->setXAxisLabel("Distribution 2 (no sign)"); dist2_02_ptr->setYAxisLabel("Number of Entries"); dist2_02_ptr->setPlotLabel("sigmaSigma = 0.2");
	dist2_04_ptr->setXAxisLabel("Distribution 2 (no sign)"); dist2_04_ptr->setYAxisLabel("Number of Entries"); dist2_04_ptr->setPlotLabel("sigmaSigma = 0.4");
	dist2_06_ptr->setXAxisLabel("Distribution 2 (no sign)"); dist2_06_ptr->setYAxisLabel("Number of Entries"); dist2_06_ptr->setPlotLabel("sigmaSigma = 0.6");
	dist2_08_ptr->setXAxisLabel("Distribution 2 (no sign)"); dist2_08_ptr->setYAxisLabel("Number of Entries"); dist2_08_ptr->setPlotLabel("sigmaSigma = 0.8");

	std::shared_ptr<GGraph2D> devel1_02_ptr(new GGraph2D());
	std::shared_ptr<GGraph2D> devel1_04_ptr(new GGraph2D());
	std::shared_ptr<GGraph2D> devel1_06_ptr(new GGraph2D());
	std::shared_ptr<GGraph2D> devel1_08_ptr(new GGraph2D());

	std::shared_ptr<GGraph2D> devel2_02_ptr(new GGraph2D());
	std::shared_ptr<GGraph2D> devel2_04_ptr(new GGraph2D());
	std::shared_ptr<GGraph2D> devel2_06_ptr(new GGraph2D());
	std::shared_ptr<GGraph2D> devel2_08_ptr(new GGraph2D());

	devel1_02_ptr->setXAxisLabel("Call"); devel1_02_ptr->setYAxisLabel("Sigma with Distribution 1 (random sign)"); devel1_02_ptr->setPlotLabel("sigmaSigma = 0.2");
	devel1_04_ptr->setXAxisLabel("Call"); devel1_04_ptr->setYAxisLabel("Sigma with Distribution 1 (random sign)"); devel1_04_ptr->setPlotLabel("sigmaSigma = 0.4");
	devel1_06_ptr->setXAxisLabel("Call"); devel1_06_ptr->setYAxisLabel("Sigma with Distribution 1 (random sign)"); devel1_06_ptr->setPlotLabel("sigmaSigma = 0.6");
	devel1_08_ptr->setXAxisLabel("Call"); devel1_08_ptr->setYAxisLabel("Sigma with Distribution 1 (random sign)"); devel1_08_ptr->setPlotLabel("sigmaSigma = 0.8");

	devel2_02_ptr->setXAxisLabel("Call"); devel2_02_ptr->setYAxisLabel("Sigma with Distribution 2 (no sign)"); devel2_02_ptr->setPlotLabel("sigmaSigma = 0.2");
	devel2_04_ptr->setXAxisLabel("Call"); devel2_04_ptr->setYAxisLabel("Sigma with Distribution 2 (no sign)"); devel2_04_ptr->setPlotLabel("sigmaSigma = 0.4");
	devel2_06_ptr->setXAxisLabel("Call"); devel2_06_ptr->setYAxisLabel("Sigma with Distribution 2 (no sign)"); devel2_06_ptr->setPlotLabel("sigmaSigma = 0.6");
	devel2_08_ptr->setXAxisLabel("Call"); devel2_08_ptr->setYAxisLabel("Sigma with Distribution 2 (no sign)"); devel2_08_ptr->setPlotLabel("sigmaSigma = 0.8");

	// Fill data into plotters
	for(std::size_t p=0; p<NPOINTS; p++) {
		// Obtain the data points
		fact_1_02 = dist1(gr_ptr, 0.2);
		fact_1_04 = dist1(gr_ptr, 0.4);
		fact_1_06 = dist1(gr_ptr, 0.6);
		fact_1_08 = dist1(gr_ptr, 0.8);

		fact_2_02 = dist2(gr_ptr, 0.2);
		fact_2_04 = dist2(gr_ptr, 0.4);
		fact_2_06 = dist2(gr_ptr, 0.6);
		fact_2_08 = dist2(gr_ptr, 0.8);

		// Fill the points into histograms
		dist1_02_ptr->add(fact_1_02);
		dist1_04_ptr->add(fact_1_04);
		dist1_06_ptr->add(fact_1_06);
		dist1_08_ptr->add(fact_1_08);

		dist2_02_ptr->add(fact_2_02);
		dist2_04_ptr->add(fact_2_04);
		dist2_06_ptr->add(fact_2_06);
		dist2_08_ptr->add(fact_2_08);

		// Update our fake sigmas
		sigma_1_02 *= fact_1_02;
		sigma_1_04 *= fact_1_04;
		sigma_1_06 *= fact_1_06;
		sigma_1_08 *= fact_1_08;

		sigma_2_02 *= fact_2_02;
		sigma_2_04 *= fact_2_04;
		sigma_2_06 *= fact_2_06;
		sigma_2_08 *= fact_2_08;

		enforceRangeConstraint(sigma_1_02, 0.,1.);
		enforceRangeConstraint(sigma_1_04, 0.,1.);
		enforceRangeConstraint(sigma_1_06, 0.,1.);
		enforceRangeConstraint(sigma_1_08, 0.,1.);

		enforceRangeConstraint(sigma_2_02, 0.,1.);
		enforceRangeConstraint(sigma_2_04, 0.,1.);
		enforceRangeConstraint(sigma_2_06, 0.,1.);
		enforceRangeConstraint(sigma_2_08, 0.,1.);

		// Record the development of a sigma over time, when no
		// selection pressure exists
		devel1_02_ptr->add(std::tuple<double,double>(double(p), sigma_1_02));
		devel1_04_ptr->add(std::tuple<double,double>(double(p), sigma_1_04));
		devel1_06_ptr->add(std::tuple<double,double>(double(p), sigma_1_06));
		devel1_08_ptr->add(std::tuple<double,double>(double(p), sigma_1_08));

		devel2_02_ptr->add(std::tuple<double,double>(double(p), sigma_2_02));
		devel2_04_ptr->add(std::tuple<double,double>(double(p), sigma_2_04));
		devel2_06_ptr->add(std::tuple<double,double>(double(p), sigma_2_06));
		devel2_08_ptr->add(std::tuple<double,double>(double(p), sigma_2_08));
	}

	// Add the plots to the plot-designers
	gpd_dist.registerPlotter(dist1_02_ptr); gpd_dist.registerPlotter(dist2_02_ptr);
	gpd_dist.registerPlotter(dist1_04_ptr); gpd_dist.registerPlotter(dist2_04_ptr);
	gpd_dist.registerPlotter(dist1_06_ptr); gpd_dist.registerPlotter(dist2_06_ptr);
	gpd_dist.registerPlotter(dist1_08_ptr); gpd_dist.registerPlotter(dist2_08_ptr);

	gpd_devel.registerPlotter(devel1_02_ptr); gpd_devel.registerPlotter(devel2_02_ptr);
	gpd_devel.registerPlotter(devel1_04_ptr); gpd_devel.registerPlotter(devel2_04_ptr);
	gpd_devel.registerPlotter(devel1_06_ptr); gpd_devel.registerPlotter(devel2_06_ptr);
	gpd_devel.registerPlotter(devel1_08_ptr); gpd_devel.registerPlotter(devel2_08_ptr);

	// Write the results out to files
	gpd_dist.writeToFile("multiplierDistributions.C");
	gpd_devel.writeToFile("sigmaDevelopment.C");
}
