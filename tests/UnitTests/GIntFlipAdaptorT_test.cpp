/**
 * @file GIntFlipAdaptorT_test.cpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 */

// Standard header files go here
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

// Boost header files go here

#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>

// Geneva header files go here
#include "GenevaExceptions.hpp"
#include "GLogger.hpp"
#include "GLogTargets.hpp"
#include "GRandom.hpp"
#include "GIntFlipAdaptorT.hpp"

using namespace Gem;
using namespace Gem::Util;
using namespace Gem::GenEvA;
using namespace Gem::GLogFramework;

/***********************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GIntFlipAdaptorT class. The template is instantiated for all types
// defined in the above mpl::list . Note that a lot of functionality of this class has
// already been covered as GBooleanAdaptor has been used as a vehicle to
// test GObject and GAdaotorT.
BOOST_AUTO_TEST_SUITE(GIntFlipAdaptorTSuite)

typedef boost::mpl::list<boost::int32_t, bool,  char> test_types;

/***********************************************************************************/
// Test features that are expected to work
BOOST_AUTO_TEST_CASE_TEMPLATE( GIntFlipAdaptorT_no_failure_expected, T, test_types )
{
	GRandom gr;

	// Test simple instantiation
	GIntFlipAdaptorT<T> gifat0;

	// A name should have been set automatically
	BOOST_CHECK(gifat0.adaptorName() == GINTFLIPADAPTORSTANDARDNAME);

	// Test instantiation with a probability mutation
	GIntFlipAdaptorT<T> gifat1(0.2);

	BOOST_CHECK(gifat1.isNotEqualTo(gifat0));

	// Test copy construction
	GIntFlipAdaptorT<T> gifat2(gifat1);

	BOOST_CHECK(gifat2.isEqualTo(gifat1));
	BOOST_CHECK(gifat2.isNotEqualTo(gifat0));

	// Test assignment
	GIntFlipAdaptorT<T> gifat3;
	gifat3 = gifat1;

	BOOST_CHECK(gifat3.isEqualTo(gifat1));
	BOOST_CHECK(gifat3.isNotEqualTo(gifat0));

	// Retrieve the mutation probablilty and modify it slightly. Then check similariy and equality.
	double mutProb = gifat3.getMutationProbability();
	mutProb -= exp(-10);
	gifat3.setMutationProbability(mutProb);
	BOOST_CHECK(gifat3.isNotEqualTo(gifat1)); // May no longer be equal
	BOOST_CHECK(gifat3.isSimilarTo(gifat1, exp(-9))); // but should be "close"

	// Check mutations
	const std::size_t NMUTATIONS = 10000;
	T mutationTarget=T(0);
	gifat3.setAdaptionThreshold(10);
	gifat3.setMutationProbability(0.1);
	std::vector<T> mutatedValues(NMUTATIONS+1);
	mutatedValues[0] = mutationTarget;
	for(std::size_t m=0; m<NMUTATIONS; m++) {  // mutation counter
		gifat3.mutate(mutationTarget);
		mutatedValues[m+1] = mutationTarget;
	}

	// Check that values do not stay the same for a larger number of mutations
	std::size_t nOriginalValues = std::count(mutatedValues.begin()+1, mutatedValues.end(), mutatedValues[0]);
	BOOST_CHECK(nOriginalValues < NMUTATIONS);

	// Check that no mutations occur if mutProb == 0
	mutationTarget=T(0);
	gifat3.setAdaptionThreshold(0);
	gifat3.setMutationProbability(0.);
	for(std::size_t m=0; m<NMUTATIONS; m++) {  // mutation counter
		gifat3.mutate(mutationTarget);
		BOOST_CHECK(mutationTarget == T(0));
	}

	// Check that mutations always occur  if mutProb == 1
	mutationTarget=T(0);
	gifat3.setAdaptionThreshold(0);
	gifat3.setMutationProbability(1.);
	T oldMutationTarget = T(0);
	for(std::size_t m=0; m<NMUTATIONS; m++) {  // mutation counter
		oldMutationTarget = mutationTarget;
		gifat3.mutate(mutationTarget);
		BOOST_CHECK(mutationTarget != oldMutationTarget);
	}

	// Do some more mutation with varying mutation parameters, just for kicks
	gifat3.setMutationProbability(1.);
	gifat3.setAdaptionThreshold(2);
	for(std::size_t p=0; p<10; p++) {
		BOOST_CHECK_NO_THROW(gifat3.setMutationParameters(gr.evenRandom(0.,0.01),0.00001,0.,0.01));
		for(std::size_t m=0; m<NMUTATIONS; m++)
			BOOST_CHECK_NO_THROW(gifat3.mutate(mutationTarget));
	}
}

/***********************************************************************************/
// Test features that are expected to fail
BOOST_AUTO_TEST_CASE_TEMPLATE( GIntFlipAdaptorT_failures_expected, T, test_types )
{
	GRandom gr;

	// Simple instantiation
	GIntFlipAdaptorT<T> gifat0;
	// Assignment of an invalid mutation probability
	BOOST_CHECK_THROW(gifat0.setMutationProbability(-0.1), Gem::GenEvA::geneva_error_condition);
	BOOST_CHECK_THROW(gifat0.setMutationProbability(1.1), Gem::GenEvA::geneva_error_condition);
}
/***********************************************************************************/

BOOST_AUTO_TEST_SUITE_END()
