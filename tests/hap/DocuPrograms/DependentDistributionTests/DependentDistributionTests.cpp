/**
 * @file DependentDistributionTests.cpp
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

/**
 * This directory contains a simple program that documents
 * dependent random distributions. E.g. it answers the queststion:
 * How does a distribution for a variable x_n look like, when it
 * is calculated from a formula "Const - Sum(x_1 ... x_(n-1)) .
 * It currently answers this questions for 1-4 "free" variables.
 */

// Standard header files
#include <vector>
#include <iostream>
#include <fstream>
#include <random>

// Boost header files

// Geneva header files
#include "hap/GRandomT.hpp"
#include "common/GPlotDesigner.hpp"

using namespace Gem::Hap;
using namespace Gem::Common;
using namespace boost;

const std::size_t NENTRIES = 100000;
const std::size_t NBINS = 100;

std::uniform_real_distribution<double> uniform_real_distribution;

int main(int argc, char **argv){
	double x1, x2, x3, x4, sum1, sum2, sum3, sum4;

	std::shared_ptr<GRandomT<RANDFLAVOURS::PROXY>> gr_ptr(new GRandomT<RANDFLAVOURS::PROXY>());

	std::shared_ptr<GHistogram1D> x1_ptr(new GHistogram1D(NBINS, 0., 1.));
	std::shared_ptr<GHistogram1D> x2_ptr(new GHistogram1D(NBINS, 0., 1.));
	std::shared_ptr<GHistogram1D> x3_ptr(new GHistogram1D(NBINS, 0., 1.));
	std::shared_ptr<GHistogram1D> x4_ptr(new GHistogram1D(NBINS, 0., 1.));

	std::shared_ptr<GHistogram1D> sum1_extract_ptr(new GHistogram1D(NBINS, 0., 1.)); // 1 - x1
	std::shared_ptr<GHistogram1D> sum2_extract_ptr(new GHistogram1D(NBINS, 0., 1.)); // 2 - (x1+x2)
	std::shared_ptr<GHistogram1D> sum3_extract_ptr(new GHistogram1D(NBINS, 0., 1.)); // 3 - (x1+x2+x3)
	std::shared_ptr<GHistogram1D> sum4_extract_ptr(new GHistogram1D(NBINS, 0., 1.)); // 4 - (x1+x2+x3+x4)

	std::shared_ptr<GHistogram1D> sum1_all_ptr(new GHistogram1D(NBINS, 0., 1.));  // 1 - x1
	std::shared_ptr<GHistogram1D> sum2_all_ptr(new GHistogram1D(NBINS, -1., 1.)); // 2 - (x1+x2)
	std::shared_ptr<GHistogram1D> sum3_all_ptr(new GHistogram1D(NBINS, -2., 1.)); // 3 - (x1+x2+x3)
	std::shared_ptr<GHistogram1D> sum4_all_ptr(new GHistogram1D(NBINS, -3., 1.)); // 4 - (x1+x2+x3+x4)

	x1_ptr->setXAxisLabel("x1"); x1_ptr->setYAxisLabel("Number of Entries"); x1_ptr->setPlotLabel("x_{1}");
	x2_ptr->setXAxisLabel("x2"); x2_ptr->setYAxisLabel("Number of Entries"); x2_ptr->setPlotLabel("x_{2}");
	x3_ptr->setXAxisLabel("x3"); x3_ptr->setYAxisLabel("Number of Entries"); x3_ptr->setPlotLabel("x_{3}");
	x4_ptr->setXAxisLabel("x4"); x4_ptr->setYAxisLabel("Number of Entries"); x4_ptr->setPlotLabel("x_{4}");

	sum1_extract_ptr->setXAxisLabel("1-x1"); sum1_extract_ptr->setYAxisLabel("Number of Entries"); sum1_extract_ptr->setPlotLabel("1.-x_{1}, extract");
	sum2_extract_ptr->setXAxisLabel("1-(x1+x2)"); sum2_extract_ptr->setYAxisLabel("Number of Entries"); sum2_extract_ptr->setPlotLabel("1.-(x_{1}+x_{2}), extract");
	sum3_extract_ptr->setXAxisLabel("1-(x1+x2+x3)"); sum3_extract_ptr->setYAxisLabel("Number of Entries"); sum3_extract_ptr->setPlotLabel("1.-(x_{1}+x_{2}+x_{3}), extract");
	sum4_extract_ptr->setXAxisLabel("1-(x1+x2+x3+x4)"); sum4_extract_ptr->setYAxisLabel("Number of Entries"); sum4_extract_ptr->setPlotLabel("1.-(x_{1}+x_{2}+x_{3}+x_{4}), extract");

	sum1_all_ptr->setXAxisLabel("1-x1"); sum1_all_ptr->setYAxisLabel("Number of Entries"); sum1_all_ptr->setPlotLabel("1.-x_{1}, all");
	sum2_all_ptr->setXAxisLabel("1-(x1+x2)"); sum2_all_ptr->setYAxisLabel("Number of Entries"); sum2_all_ptr->setPlotLabel("1.-(x_{1}+x_{2}), all");
	sum3_all_ptr->setXAxisLabel("1-(x1+x2+x3)"); sum3_all_ptr->setYAxisLabel("Number of Entries"); sum3_all_ptr->setPlotLabel("1.-(x_{1}+x_{2}+x_{3}), all");
	sum4_all_ptr->setXAxisLabel("1-(x1+x2+x3+x4)"); sum4_all_ptr->setYAxisLabel("Number of Entries"); sum4_all_ptr->setPlotLabel("1.-(x_{1}+x_{2}+x_{3}+x_{4}), all");

	for(std::size_t i=0; i<NENTRIES; i++) {
		x1=uniform_real_distribution(*gr_ptr);
		x2=uniform_real_distribution(*gr_ptr);
		x3=uniform_real_distribution(*gr_ptr);
		x4=uniform_real_distribution(*gr_ptr);

		sum1 = x1;
		sum2 = x1+x2;
		sum3 = x1+x2+x3;
		sum4 = x1+x2+x3+x4;

		x1_ptr->add(x1);
		x2_ptr->add(x2);
		x3_ptr->add(x3);
		x4_ptr->add(x4);

		sum1_extract_ptr->add(1.-sum1);
		sum2_extract_ptr->add(1.-sum2);
		sum3_extract_ptr->add(1.-sum3);
		sum4_extract_ptr->add(1.-sum4);

		sum1_all_ptr->add(1.-sum1);
		sum2_all_ptr->add(1.-sum2);
		sum3_all_ptr->add(1.-sum3);
		sum4_all_ptr->add(1.-sum4);
	}

	GPlotDesigner gpd("Dependent random number distributions", 4,3);

	gpd.setCanvasDimensions(1600,1200);

	gpd.registerPlotter(x1_ptr);
	gpd.registerPlotter(x2_ptr);
	gpd.registerPlotter(x3_ptr);
	gpd.registerPlotter(x4_ptr);

	gpd.registerPlotter(sum1_extract_ptr);
	gpd.registerPlotter(sum2_extract_ptr);
	gpd.registerPlotter(sum3_extract_ptr);
	gpd.registerPlotter(sum4_extract_ptr);

	gpd.registerPlotter(sum1_all_ptr);
	gpd.registerPlotter(sum2_all_ptr);
	gpd.registerPlotter(sum3_all_ptr);
	gpd.registerPlotter(sum4_all_ptr);

	gpd.writeToFile("rootPlotDependentDistributions.C");
}
