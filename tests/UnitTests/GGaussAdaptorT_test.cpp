/**
 * @file GGaussAdaptorT_test.cpp
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
#include "GRandom.hpp"
#include "GGaussAdaptorT.hpp"

using namespace Gem;
using namespace Gem::Util;
using namespace Gem::GenEvA;

/***********************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GGaussAdaptorT class. The template is instantiated for all types
// defined in the above mpl::list . Note that a lot of functionality of this class has
// already been covered as GBooleanAdaptor has been used as a vehicle to
// test GObject and GAdaotorT.
BOOST_AUTO_TEST_SUITE(GGaussAdaptorTSuite)

typedef boost::mpl::list<boost::int32_t, double> test_types;

/***********************************************************************************/
// Test features that are expected to work
BOOST_AUTO_TEST_CASE_TEMPLATE( GGaussAdaptorT_no_failure_expected, T, test_types )
{
	GRandom gr;

	// Test simple instantiation
	GGaussAdaptorT<T> ggat0;
	// An id should have been set automatically
	BOOST_CHECK(ggat0.getAdaptorId() == Gem::GenEvA::GDOUBLEGAUSSADAPTOR || ggat0.getAdaptorId() == Gem::GenEvA::GINT32GAUSSADAPTOR);

	// Instantiation with an intentionally long sigma
	GGaussAdaptorT<T> ggat1(0.202030405060708,0.001, 0., 1.); // intentionally long

	// Instantiation with sigma, sigmaSigma, minSigma and maxSigma
	GGaussAdaptorT<T> ggat2(0.1, 0.001, 0., 1.);
	BOOST_CHECK(ggat2 != ggat1);

	// Copy construction
	GGaussAdaptorT<T> ggat3(ggat2);
	BOOST_CHECK(ggat3 == ggat2);

	// Assignment
	boost::shared_ptr<GGaussAdaptorT<T> > ggat4_ptr(ggat1.GObject::clone_ptr_cast<GGaussAdaptorT<T> >());
	*ggat4_ptr = ggat3;
	BOOST_CHECK(*ggat4_ptr == ggat3 && *ggat4_ptr == ggat2);

	// ... and loading
	GGaussAdaptorT<T> ggat5;
	ggat5.load(ggat4_ptr.get());
	BOOST_CHECK(ggat5 == ggat3 && *ggat4_ptr == ggat2);

	// Check (de-)serialization in different modes
	ggat5.fromString(ggat1.toString(TEXTSERIALIZATION),TEXTSERIALIZATION);
	BOOST_CHECK(ggat5.isSimilarTo(ggat1, exp(-10)));
	ggat5 = ggat3; // reset
	BOOST_CHECK(!ggat5.isEqualTo(ggat1));
	ggat5.fromString(ggat1.toString(XMLSERIALIZATION),XMLSERIALIZATION);
	BOOST_CHECK(ggat5.isSimilarTo(ggat1, exp(-10)));
	ggat5 = ggat3; // reset
	ggat5.fromString(ggat1.toString(BINARYSERIALIZATION),BINARYSERIALIZATION);
	BOOST_CHECK(ggat5.isEqualTo(ggat1)); // no longer just similar in binary mode!

	// Check that we can set and retrieve sigma
	BOOST_CHECK_NO_THROW(ggat5.setSigma(0.5));
	BOOST_CHECK(ggat5.getSigma() == 0.5);

	// Check that we can set and retrieve the sigma range
	BOOST_CHECK_NO_THROW(ggat5.setSigmaRange(0.,2.));
	BOOST_CHECK(ggat5.getSigma() == 0.5);

	// Check that sigma actually gets adapted, if we move the lower boundary
	BOOST_CHECK_NO_THROW(ggat5.setSigmaRange(0.6,2.));
	BOOST_CHECK(ggat5.getSigma() == 0.6);

	// Check the range
	BOOST_CHECK(ggat5.getSigmaRange().first == 0.6 && ggat5.getSigmaRange().second == 2.);

	// Set and retrieve the adaption rate
	BOOST_CHECK_NO_THROW(ggat5.setSigmaAdaptionRate(0.001));
	BOOST_CHECK(ggat5.getSigmaAdaptionRate() == 0.001);

	// Finally set all parameters in one go
	BOOST_CHECK_NO_THROW(ggat5.setAll(0.1,0.001,0.,2.));
	BOOST_CHECK(ggat5.getSigma() == 0.1);
	BOOST_CHECK(ggat5.getSigmaAdaptionRate() == 0.001);
	BOOST_CHECK(ggat5.getSigmaRange().first == DEFAULTMINSIGMA && ggat5.getSigmaRange().second == 2.);

	// Perform mutations with varying mutation parameters
	T mutationTarget = T(0);
	std::size_t NMUTATIONS=10000;
	ggat5.setAdaptionThreshold(1);
	for(std::size_t p=0; p<20; p++) {
		BOOST_CHECK_NO_THROW(ggat5.setAll(gr.evenRandom(DEFAULTMINSIGMA,2.), 0.001, 0., 2.));
		for(std::size_t m=0; m<NMUTATIONS; m++)
			BOOST_CHECK_NO_THROW(ggat5.mutate(mutationTarget));
	}
}

/***********************************************************************************/
// Test features that are expected to fail
BOOST_AUTO_TEST_CASE_TEMPLATE( GGaussAdaptorT_failures_expected, T, test_types )
{
	GRandom gr;

	{
		GGaussAdaptorT<T> ggat0(0.1, 0.001, 0., 1.);
		BOOST_CHECK_THROW(ggat0.setSigma(1.1), Gem::GenEvA::geneva_error_condition); // outside of the allowed range
	}

	{ // not sure what state ggat0 is in after it has thrown. Hence we recreate it for the next test
		GGaussAdaptorT<T> ggat0;
		BOOST_CHECK_THROW(ggat0.setSigmaRange(-1.,1.), Gem::GenEvA::geneva_error_condition); // outside of the allowed range
	}

	{ // not sure what state ggat0 is in after it has thrown. Hence we recreate it for the next test
		GGaussAdaptorT<T> ggat0;
		BOOST_CHECK_THROW(ggat0.setSigmaAdaptionRate(0.), Gem::GenEvA::geneva_error_condition); // 0. is not an allowed value
	}

	{
		// Self assignment should throw in DEBUG mode
#ifdef DEBUG
		GGaussAdaptorT<T> ggat0;
		BOOST_CHECK_THROW(ggat0.load(&ggat0), Gem::GenEvA::geneva_error_condition);
#endif /* DEBUG */
	}
}
/***********************************************************************************/

BOOST_AUTO_TEST_SUITE_END()
