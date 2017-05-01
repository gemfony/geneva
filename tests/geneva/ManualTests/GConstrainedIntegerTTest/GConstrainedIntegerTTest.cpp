/**
 * @file GConstrainedIntTTest.cpp
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
 * This test takes a GConstrainedIntT<std::int32_t> object and examines the mapping
 * from internal to external representation of its value.
 *
 * In order to see the results of this test, you need the Root toolkit from http://root.cern.ch.
 * Once installed call "root -l result.C" .
 */

// Standard header files go here
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

// Boost header files go here

// Geneva header files go here
#include "common/GPlotDesigner.hpp"
#include "geneva/GConstrainedInt32Object.hpp"
#include "geneva/GInt32GaussAdaptor.hpp"
#include "geneva/GInt32FlipAdaptor.hpp"

using namespace Gem::Geneva;
using namespace Gem::Common;
using namespace boost;

const std::uint32_t NTESTS=2000;

int main(int argc, char **argv){
	// Get a random number generator
	Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::PROXY> gr;

	std::shared_ptr<GHistogram1I> multipleFlipMutation_ptr(new GHistogram1I(50, 0.5,50.5));
	multipleFlipMutation_ptr->setPlotLabel("Occurance of different values when flip-mutating, starting with 1");

	std::shared_ptr<GGraph2D> multipleFlipProgress_ptr(new GGraph2D());
	multipleFlipProgress_ptr->setPlotMode(Gem::Common::graphPlotMode::SCATTER);
	multipleFlipProgress_ptr->setPlotLabel("Current value of a GConstrainedInt32Object after repeated flip mutations");
	multipleFlipProgress_ptr->setXAxisLabel("iteration");
	multipleFlipProgress_ptr->setYAxisLabel("value");

	std::shared_ptr<GHistogram1I> multipleGaussMutation_ptr(new GHistogram1I(50, 0.5,50.5));
	multipleGaussMutation_ptr->setPlotLabel("Occurance of different values when gauss-mutating, starting with 1");

	std::shared_ptr<GGraph2D> multipleGaussProgress_ptr(new GGraph2D());
	multipleGaussProgress_ptr->setPlotMode(Gem::Common::graphPlotMode::SCATTER);
	multipleGaussProgress_ptr->setPlotLabel("Current value of a GConstrainedInt32Object after repeated gauss mutations");
	multipleGaussProgress_ptr->setXAxisLabel("iteration");
	multipleGaussProgress_ptr->setYAxisLabel("value");

	double internalValue = 0., externalValue = 0.;
	std::shared_ptr<GGraph2D> mapping_ptr(new GGraph2D());
	mapping_ptr->setPlotLabel("Mapping from internal to external value");

	GConstrainedInt32Object gMultFlipMut(1, 1, 50);
	std::shared_ptr<GInt32FlipAdaptor> gifa_ptr(new GInt32FlipAdaptor());
	gMultFlipMut.addAdaptor(gifa_ptr);

	GConstrainedInt32Object gMultGaussMut(1, 1, 50);
	std::shared_ptr<GInt32GaussAdaptor> giga_ptr(new GInt32GaussAdaptor(
		0.025 // sigma
		, 0.1 // sigmaSigma
		, 0. // minSigma
		, 1. // maxSigma
		, 1. // adProb
	)
	);
	gMultGaussMut.addAdaptor(giga_ptr);

	GConstrainedInt32Object gint13(-1, 3); // lower boundary -1, upper Boundary 3

	// Mutate and register results
	for(std::uint32_t i=0; i<NTESTS; i++) {
		*multipleFlipMutation_ptr & gMultFlipMut.value();
		*multipleFlipProgress_ptr & std::tuple<double,double>((double)i, (double)gMultFlipMut.value());
		gMultFlipMut.adapt(gr);

		*multipleGaussMutation_ptr & gMultGaussMut.value();
		*multipleGaussProgress_ptr & std::tuple<double,double>((double)i, (double)gMultGaussMut.value());
		gMultGaussMut.adapt(gr);

		internalValue=-30.+50.*double(i)/double(NTESTS);

		externalValue = double(gint13.transfer(std::int32_t(internalValue)));
		*mapping_ptr & std::tuple<double,double>(internalValue, externalValue);
	}

	GPlotDesigner gpd("Manual tests of GConstrainedInt32Object", 2,3);

	gpd.setCanvasDimensions(1200,1200);
	gpd.registerPlotter(multipleFlipMutation_ptr);
	gpd.registerPlotter(multipleFlipProgress_ptr);
	gpd.registerPlotter(multipleGaussMutation_ptr);
	gpd.registerPlotter(multipleGaussProgress_ptr);
	gpd.registerPlotter(mapping_ptr);

	gpd.writeToFile("result.C");

	return 0;
}
