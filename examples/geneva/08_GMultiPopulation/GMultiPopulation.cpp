/**
 * @file GBrokerOverhead.cpp
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
#include <sstream>

// Boost header files go here
#include <boost/lexical_cast.hpp>

// Geneva header files go here
#include "geneva/Go2.hpp"
#include "geneva/GMultiPopulationEAT.hpp"

// The individual that should be optimized
#include "geneva-individuals/GFunctionIndividual.hpp"

// Declares a function to parse the command line
#include "GArgumentParser.hpp"

using namespace Gem::Geneva;
using namespace Gem::Courtier;
using namespace Gem::Hap;

/******************************************************************************/
/**
 * The main function.
 */
int main(int argc, char **argv){
	std::string configFile;
	std::string ip;
	boost::uint16_t nEvaluationThreads;
	std::size_t populationSizeSuper;
	std::size_t nParentsSuper;
	boost::uint32_t maxIterationsSuper;
	long maxMinutesSuper;
	boost::uint32_t reportIterationSuper;
	duplicationScheme rSchemeSuper;
	sortingModeMP smodeSuper;
	std::size_t populationSizeSub;
	std::size_t nParentsSub;
	boost::uint32_t maxIterationsSub;
	long maxMinutesSub;
	boost::uint32_t reportIterationSub;
	duplicationScheme rSchemeSub;
	sortingMode smodeSub;

	/****************************************************************************/
	// Parse the command line

	if(!parseCommandLine(
		argc, argv
		, nEvaluationThreads
		, populationSizeSuper
		, nParentsSuper
		, maxIterationsSuper
		, maxMinutesSuper
		, reportIterationSuper
		, rSchemeSuper
		, smodeSuper
		, populationSizeSub
		, nParentsSub
		, maxIterationsSub
		, maxMinutesSub
		, reportIterationSub
		, rSchemeSub
		, smodeSub
	)) { exit(1); } // Leave if help was requested.

	/****************************************************************************/
	// This EA population can hold derivatives of GBaseEA
	GMultiPopulationEAT<GBaseEA> gmp(nEvaluationThreads);

	/****************************************************************************/
	// Create a factory for GFunctionIndividual objects and perform
	// any necessary initial work.
	GFunctionIndividualFactory gfi("./config/GFunctionIndividual.json");

	// Create the first set of parent populations.
	for(std::size_t psuper = 0; psuper<nParentsSuper; psuper++) {
		// This smart pointer holds a parent population.
		std::shared_ptr<GSerialEA> sub_pop_ptr
			= std::shared_ptr<GSerialEA>(new GSerialEA());

		// Create the first set of parent individuals. Initialization of parameters is done randomly.
		for(std::size_t psub = 0 ; psub<nParentsSub; psub++) {
			sub_pop_ptr->push_back(gfi());
		}

		// Specify some general population settings
		sub_pop_ptr->setPopulationSizes(populationSizeSub,nParentsSub);
		sub_pop_ptr->setMaxIteration(maxIterationsSub);
		sub_pop_ptr->setMaxTime(boost::posix_time::minutes(maxMinutesSub));
		sub_pop_ptr->setReportIteration(reportIterationSub);
		sub_pop_ptr->setRecombinationMethod(rSchemeSub);
		sub_pop_ptr->setSortingScheme(smodeSub);
		sub_pop_ptr->setEmitTerminationReason(false);

		// Add the sub population to the vector
		gmp.push_back(sub_pop_ptr);
	}

	/****************************************************************************/
	// Specify some general population settings
	gmp.setPopulationSizes(populationSizeSuper,nParentsSuper);
	gmp.setMaxIteration(maxIterationsSuper);
	gmp.setMaxTime(boost::posix_time::minutes(maxMinutesSuper));
	gmp.setReportIteration(reportIterationSuper);
	gmp.setRecombinationMethod(rSchemeSuper);
	gmp.setSortingScheme(smodeSuper);
	gmp.setEmitTerminationReason(true);

	// Do the actual optimization
	gmp.optimize();

	// Extract the best individual
	std::shared_ptr<GFunctionIndividual> p = gmp.getBestIndividual<GFunctionIndividual>();

	// Do something with the best result found here

	/****************************************************************************/
	// Terminate Geneva
	return 0;
}
