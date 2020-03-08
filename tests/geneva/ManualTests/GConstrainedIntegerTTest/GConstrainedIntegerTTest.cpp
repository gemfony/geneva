/**
 * @file GConstrainedIntTTest.cpp
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
	Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

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
