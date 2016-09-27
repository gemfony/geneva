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
#include <chrono>

// Boost header files go here
#include <boost/lexical_cast.hpp>

// Geneva header files go here
#include "geneva/Go2.hpp"
#include "geneva/GMultiPopulationEAT.hpp"

// The individual that should be optimized
#include "geneva-individuals/GFunctionIndividual.hpp"

using namespace Gem::Geneva;
using namespace Gem::Courtier;
using namespace Gem::Hap;
using namespace Gem::Common;

namespace po = boost::program_options;

/******************************************************************************/
// Default settings
const std::uint16_t DEFAULTNEVALUATIONTHREADS=4;
const std::size_t DEFAULTPOPULATIONSIZESUPER=5;
const std::size_t DEFAULTNPARENTSSUPER=1;
const std::uint32_t DEFAULTMAXITERATIONSSUPER=10;
const long DEFAULTMAXMINUTESSUPER=0;
const std::uint32_t DEFAULTREPORTITERATIONSUPER=1;
const sortingModeMP DEFAULTSORTINGSCHEMESUPER=sortingModeMP::MUPLUSNU_SINGLEEVAL_MP;
const duplicationScheme DEFAULTRSCHEMESUPER=duplicationScheme::VALUEDUPLICATIONSCHEME;
const std::size_t DEFAULTPOPULATIONSIZESUB=22;
const std::size_t DEFAULTNPARENTSSUB=2; // Allow to explore the parameter space from many starting points
const std::uint32_t DEFAULTMAXITERATIONSSUB=100;
const long DEFAULTMAXMINUTESSUB=0;
const std::uint32_t DEFAULTREPORTITERATIONSUB=0;
const sortingMode DEFAULTSORTINGSCHEMESUB=sortingMode::MUCOMMANU_SINGLEEVAL;
const duplicationScheme DEFAULTRSCHEMESUB=duplicationScheme::VALUEDUPLICATIONSCHEME;

/******************************************************************************/
/**
 * Parses the command line
 */
bool parseCommandLine(
	int argc, char **argv
	, std::uint16_t& nEvaluationThreads
	, std::size_t& populationSizeSuper
	, std::size_t& nParentsSuper
	, std::uint32_t& maxIterationsSuper
	, long& maxMinutesSuper
	, std::uint32_t& reportIterationSuper
	, duplicationScheme& rSchemeSuper
	, sortingModeMP& smodeSuper
	, std::size_t& populationSizeSub
	, std::size_t& nParentsSub
	, std::uint32_t& maxIterationsSub
	, long& maxMinutesSub
	, std::uint32_t& reportIterationSub
	, duplicationScheme& rSchemeSub
	, sortingMode& smodeSub
) {
	// Create the parser builder
	Gem::Common::GParserBuilder gpb;

	gpb.registerCLParameter<std::uint16_t>(
		"nEvaluationThreads"
		, nEvaluationThreads
		, DEFAULTNEVALUATIONTHREADS
		, "The amount of threads processing individuals simultaneously"
	);

	gpb.registerCLParameter<std::size_t>(
		"populationSizeSuper"
		, populationSizeSuper
		, DEFAULTPOPULATIONSIZESUPER
		, "The desired size of the super population"
	);

	gpb.registerCLParameter<std::size_t>(
		"nParentsSuper"
		, nParentsSuper
		, DEFAULTNPARENTSSUPER
		, "The number of parents in the super population"
	);

	gpb.registerCLParameter<std::uint32_t>(
		"maxIterationsSuper"
		, maxIterationsSuper
		, DEFAULTMAXITERATIONSSUPER
		, "Maximum number of iterations in the super population"
	);

	gpb.registerCLParameter<long>(
		"maxMinutesSuper"
		, maxMinutesSuper
		, DEFAULTMAXMINUTESSUPER
		, "The maximum number of minutes the optimization of the super population should run"
	);

	gpb.registerCLParameter<std::uint32_t>(
		"reportIterationSuper"
		, reportIterationSuper
		, DEFAULTREPORTITERATIONSUPER
		, "The number of iterations after which information should be emitted in the super population"
	);

	gpb.registerCLParameter<duplicationScheme>(
		"rSchemeSuper"
		, rSchemeSuper
		, DEFAULTRSCHEMESUPER
		, "The recombination scheme of the evolutionary algorithm (super population)"
	);

	gpb.registerCLParameter<sortingModeMP>(
		"smodeSuper"
		, smodeSuper
		, DEFAULTSORTINGSCHEMESUPER
		, "Determines whether sorting is done in MUPLUSNU_SINGLEEVAL (0), MUCOMMANU_SINGLEEVAL (1) or MUNU1PRETAIN (2) mode in the super population"
	);

	gpb.registerCLParameter<std::size_t>(
		"populationSizeSub"
		, populationSizeSub
		, DEFAULTPOPULATIONSIZESUB
		, "The desired size of the sub population"
	);

	gpb.registerCLParameter<std::size_t>(
		"nParentsSub"
		, nParentsSub
		, DEFAULTNPARENTSSUB
		, "The number of parents in the sub population"
	);

	gpb.registerCLParameter<std::uint32_t>(
		"maxIterationsSub"
		, maxIterationsSub
		, DEFAULTMAXITERATIONSSUB
		, "Maximum number of iterations in the sub population"
	);

	gpb.registerCLParameter<long>(
		"maxMinutesSub"
		, maxMinutesSub
		, DEFAULTMAXMINUTESSUB
		, "The maximum number of minutes the optimization of the sub population should run"
	);

	gpb.registerCLParameter<std::uint32_t>(
		"reportIterationSub"
		, reportIterationSub
		, DEFAULTREPORTITERATIONSUB
		, "The number of iterations after which information should be emitted in the sub population"
	);

	gpb.registerCLParameter<duplicationScheme>(
		"rSchemeSub"
		, rSchemeSub
		, DEFAULTRSCHEMESUB
		, "The recombination scheme of the evolutionary algorithm (sub population)"
	);

	gpb.registerCLParameter<sortingMode>(
		"smodeSub"
		, smodeSub
		, DEFAULTSORTINGSCHEMESUB
		, "Determines whether sorting is done in MUPLUSNU_SINGLEEVAL (0), MUCOMMANU_SINGLEEVAL (1) or MUNU1PRETAIN (2) mode in the sub population"
	);


	// Parse the command line and leave if the help flag was given. The parser
	// will emit an appropriate help message by itself
	if(Gem::Common::GCL_HELP_REQUESTED == gpb.parseCommandLine(argc, argv, true /*verbose*/)) {
		return false; // Do not continue
	}

	return true;
}

/******************************************************************************/
/**
 * The main function.
 */
int main(int argc, char **argv){
	std::string configFile;
	std::string ip;
	std::uint16_t nEvaluationThreads;
	std::size_t populationSizeSuper;
	std::size_t nParentsSuper;
	std::uint32_t maxIterationsSuper;
	long maxMinutesSuper;
	std::uint32_t reportIterationSuper;
	duplicationScheme rSchemeSuper;
	sortingModeMP smodeSuper;
	std::size_t populationSizeSub;
	std::size_t nParentsSub;
	std::uint32_t maxIterationsSub;
	long maxMinutesSub;
	std::uint32_t reportIterationSub;
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
		sub_pop_ptr->setMaxTime(std::chrono::minutes(maxMinutesSub));
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
	gmp.setMaxTime(std::chrono::minutes(maxMinutesSuper));
	gmp.setReportIteration(reportIterationSuper);
	gmp.setRecombinationMethod(rSchemeSuper);
	gmp.setSortingScheme(smodeSuper);
	gmp.setEmitTerminationReason(true);

	// Do the actual optimization
	gmp.optimize();

	// Extract the best individual
	std::shared_ptr<GFunctionIndividual> p = gmp.getBestGlobalIndividual<GFunctionIndividual>();

	// Do something with the best result found here

	/****************************************************************************/
	// Terminate Geneva
	return 0;
}
