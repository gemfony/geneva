/**
 * @file GIntFlipAdaptorT_test.cpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
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

#ifndef GINTFLIPADAPTORT_TEST_HPP_
#define GINTFLIPADAPTORT_TEST_HPP_

// Geneva header files go here
#include "common/GExceptions.hpp"
#include "random/GRandom.hpp"
#include "optimization/GIntFlipAdaptorT.hpp"
#include "optimization/GInt32FlipAdaptor.hpp"
#include "optimization/GBooleanAdaptor.hpp"
#include "GEqualityPrinter.hpp"

using namespace Gem;
using namespace Gem::Util;
using namespace Gem::GenEvA;

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

/***********************************************************************************/
// Test features that are expected to work
BOOST_TEST_CASE_TEMPLATE_FUNCTION ( GIntFlipAdaptorT_no_failure_expected, T)
{
	// Prepare printing of error messages in object comparisons
	GEqualityPrinter gep("GIntFlipAdaptorT_no_failure_expected",
						 pow(10,-10),
						 Gem::Common::CE_WITH_MESSAGES);

	// A local random number generator
	GRandom gr;

	// Test simple instantiation
	T gifat0;

	// An id should have been set automatically
	BOOST_CHECK(gifat0.getAdaptorId() == Gem::GenEvA::GBOOLEANADAPTOR ||
			gifat0.getAdaptorId() == Gem::GenEvA::GINT32FLIPADAPTOR);

	// Test instantiation with a probability adaption
	T gifat1(0.2);

	BOOST_CHECK(gep.isInEqual(gifat1, gifat0));

	// Test copy construction
	T gifat2(gifat1);

	BOOST_CHECK(gep.isEqual(gifat2, gifat1));
	BOOST_CHECK(gep.isInEqual(gifat2, gifat0));

	// Test assignment
	T gifat3;
	gifat3 = gifat1;

	BOOST_CHECK(gep.isEqual(gifat3, gifat1));
	BOOST_CHECK(gep.isInEqual(gifat3, gifat0));

	// Retrieve the adaption probablilty and modify it slightly. Then check similariy and equality.
	double adProb = gifat3.getAdaptionProbability();
	adProb -= pow(10,-10);
	gifat3.setAdaptionProbability(adProb);

	BOOST_CHECK(gep.isInEqual(gifat3, gifat1)); // May no longer be equal
	BOOST_CHECK(gep.isSimilar(gifat3, gifat1, exp(-9))); // but should be "close"

	// Check adaptions
	const std::size_t NADAPTIONS = 10000;
	typename T::adaption_type adaptionTarget=typename T::adaption_type(0);
	gifat3.setAdaptionThreshold(10);
	gifat3.setAdaptionProbability(0.1);
	std::vector<typename T::adaption_type> adaptedValues(NADAPTIONS+1);
	adaptedValues[0] = adaptionTarget;
	for(std::size_t m=0; m<NADAPTIONS; m++) {  // adaption counter
		gifat3.adapt(adaptionTarget);
		adaptedValues[m+1] = adaptionTarget;
	}

	// Check that values do not stay the same for a larger number of adaptions
	std::size_t nOriginalValues = std::count(adaptedValues.begin()+1, adaptedValues.end(), adaptedValues[0]);
	BOOST_CHECK(nOriginalValues < NADAPTIONS);

	// Check that no adaptions occur if adProb == 0
	adaptionTarget=typename T::adaption_type(0);
	gifat3.setAdaptionThreshold(0);
	gifat3.setAdaptionProbability(0.);
	for(std::size_t m=0; m<NADAPTIONS; m++) {  // adaption counter
		gifat3.adapt(adaptionTarget);
		BOOST_CHECK(adaptionTarget == typename T::adaption_type(0));
	}

	// Check that adaptions always occur  if adProb == 1
	adaptionTarget=typename T::adaption_type(0);
	gifat3.setAdaptionThreshold(0);
	gifat3.setAdaptionProbability(1.);
	typename T::adaption_type oldAdaptionTarget = typename T::adaption_type(0);
	for(std::size_t m=0; m<NADAPTIONS; m++) {  // adaption counter
		oldAdaptionTarget = adaptionTarget;
		gifat3.adapt(adaptionTarget);
		BOOST_CHECK(adaptionTarget != oldAdaptionTarget);
	}

	// Do some more adaption with varying adaption parameters, just for kicks
	gifat3.setAdaptionProbability(1.);
	gifat3.setAdaptionThreshold(2);
	for(std::size_t p=0; p<10; p++) {
		// BOOST_CHECK_NO_THROW(gifat3.setAdaptionParameters(gr.evenRandom(0.,0.01),0.00001,0.,0.01));
		for(std::size_t m=0; m<NADAPTIONS; m++)
			BOOST_CHECK_NO_THROW(gifat3.adapt(adaptionTarget));
	}
}

/***********************************************************************************/
// Test features that are expected to fail
BOOST_TEST_CASE_TEMPLATE_FUNCTION( GIntFlipAdaptorT_failures_expected, T)
{
	GRandom gr;

	{
		// Simple instantiation
		T gifat0;
		// Assignment of an invalid adaption probability
		BOOST_CHECK_THROW(gifat0.setAdaptionProbability(-0.1), Gem::Common::gemfony_error_condition);
	}

	{
		// Simple instantiation
		T gifat0;
		// Assignment of an invalid adaption probability
		BOOST_CHECK_THROW(gifat0.setAdaptionProbability(1.1), Gem::Common::gemfony_error_condition);
	}

	{
		// Self assignment should throw in DEBUG mode
#ifdef DEBUG
		boost::shared_ptr<T> gifat0_ptr(new T());
		BOOST_CHECK_THROW(gifat0_ptr->load(gifat0_ptr), Gem::Common::gemfony_error_condition);
#endif /* DEBUG */
	}
}

/***********************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GIntFlipAdaptorT class. The template is instantiated for all types
// defined in the above mpl::list . Note that a lot of functionality of this class has
// already been covered as GBooleanAdaptor has been used as a vehicle to
// test GObject and GAdaotorT.
class GIntFlipAdaptorTSuite: public test_suite
{
public:
	GIntFlipAdaptorTSuite() :test_suite("GIntFlipAdaptorTSuite") {
		typedef boost::mpl::list<GInt32FlipAdaptor, GBooleanAdaptor> test_types;

		add( BOOST_TEST_CASE_TEMPLATE( GIntFlipAdaptorT_no_failure_expected, test_types ) );
		add( BOOST_TEST_CASE_TEMPLATE( GIntFlipAdaptorT_failures_expected, test_types ) );
	}
};

#endif /* GINTFLIPADAPTORT_TEST_HPP_ */
