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
#include "GLogger.hpp"
#include "GLogTargets.hpp"
#include "GRandom.hpp"
#include "GParameterT.hpp"
#include "GBoolean.hpp"
#include "GBooleanAdaptor.hpp"
#include "GChar.hpp"
#include "GCharFlipAdaptor.hpp"
#include "GInt32.hpp"
#include "GInt32FlipAdaptor.hpp"
#include "GInt32GaussAdaptor.hpp"
#include "GDouble.hpp"
#include "GDoubleGaussAdaptor.hpp"

using namespace Gem;
using namespace Gem::Util;
using namespace Gem::GenEvA;
using namespace Gem::GLogFramework;

/***********************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GParameterT class. Tests include features of the parent class
// GParameterBaseWithAdaptorsT, as it cannot be instantiated itself.
BOOST_AUTO_TEST_SUITE(GParameterTSuite)

typedef boost::mpl::list<bool, char, boost::int32_t, double> test_types;

/***********************************************************************************/
// Test features that are expected to work
BOOST_AUTO_TEST_CASE_TEMPLATE( GParameterT_no_failure_expected, T, test_types )
{
	GRandom gr;

	// Test default construction
	BOOST_CHECK_NO_THROW(GParameterT<T> gpt);

	// Test construction with a value
	GParameterT<T> gpt0(T(NULL)), gpt1(T(1));
	BOOST_CHECK(gpt0 != gpt1);

	// Test copy construction
	GParameterT<T> gpt2(gpt1);
	BOOST_CHECK(gpt2 == gpt1);
	BOOST_CHECK(gpt2 != gpt0);

	// Test assignment
	GParameterT<T> gpt3;
	gpt3 = gpt1;
	BOOST_CHECK(gpt3 == gpt1);
	BOOST_CHECK(gpt3 != gpt0);

	// Test cloning
	GObject *gpt3_clone = gpt3.clone();

	// Test loading
	BOOST_CHECK_NO_THROW(gpt0.load(gpt3_clone));
	BOOST_CHECK_NO_THROW(delete gpt3_clone);
	BOOST_CHECK(gpt0 == gpt3);

	// Re-assign the original value
	gpt0 = T(NULL);
	BOOST_CHECK(gpt0 != gpt3);

	// Test (de-)serialization in differnet modes
	{ // plain text format
	   GParameterT<T> gpt4(gpt0);
	   BOOST_CHECK(gpt4.isEqualTo(gpt0));
	   gpt4.fromString(gpt1.toString(TEXTSERIALIZATION), TEXTSERIALIZATION);
	   BOOST_CHECK(!gpt4.isEqualTo(gpt0));
	   BOOST_CHECK(gpt4.isSimilarTo(gpt1, exp(-10)));
	}
	{ // XML format
	   GParameterT<T> gpt4(gpt0);
	   BOOST_CHECK(gpt4.isEqualTo(gpt0));
	   gpt4.fromString(gpt1.toString(XMLSERIALIZATION), XMLSERIALIZATION);
	   BOOST_CHECK(!gpt4.isEqualTo(gpt0));
	   BOOST_CHECK(gpt4.isSimilarTo(gpt1, exp(-10)));
	}
	{ // binary test format
	   GParameterT<T> gpt4(gpt0);
	   BOOST_CHECK(gpt4 == gpt0);
	   gpt4.fromString(gpt1.toString(BINARYSERIALIZATION), BINARYSERIALIZATION);
	   BOOST_CHECK(gpt4 != gpt0);
	   BOOST_CHECK(gpt4 == gpt1);
	}
}

/***********************************************************************************/
// Test features that are expected to work - bool case
BOOST_AUTO_TEST_CASE( GParameterT_bool_no_failure_expected)
{
	GRandom gr;

	// Default construction
	GBoolean gpt0;

	// Adding a single adaptor
	BOOST_CHECK_NO_THROW(gpt0.addAdaptor(boost::shared_ptr<GBooleanAdaptor>(new GBooleanAdaptor())));
	BOOST_CHECK(gpt0.numberOfAdaptors() == 1);

	// Retrieve the adaptor again, as a GAdaptorT
	BOOST_CHECK_NO_THROW(boost::shared_ptr<GAdaptorT<bool> > gadb0_ptr = gpt0.getAdaptor(GBooleanAdaptor::adaptorName()));

	// Retrieve the adaptor in its original form
	BOOST_CHECK_NO_THROW(boost::shared_ptr<GBooleanAdaptor> gba0_ptr = gpt0.adaptor_cast<GBooleanAdaptor>(GBooleanAdaptor::adaptorName()));

	// Check mutations
	const std::size_t NMUTATIONS=10000;
	std::vector<bool> mutvals(NMUTATIONS);
	bool originalValue = gpt0.value();
	for(std::size_t i=0; i<NMUTATIONS; i++) {
		BOOST_CHECK_NO_THROW(gpt0.mutate());
		mutvals[i] = gpt0.value();
	}

	// Check that values do not stay the same for a larger number of mutations
	std::size_t nOriginalValues = std::count(mutvals.begin(), mutvals.end(), originalValue);
	BOOST_CHECK(nOriginalValues < NMUTATIONS);
}

/***********************************************************************************/
// Test features that are expected to work - char case
BOOST_AUTO_TEST_CASE( GParameterT_char_no_failure_expected)
{
	GRandom gr;

	// Default construction
	GChar gpt0;

	// Adding a single adaptor
	BOOST_CHECK_NO_THROW(gpt0.addAdaptor(boost::shared_ptr<GCharFlipAdaptor>(new GCharFlipAdaptor())));
	BOOST_CHECK(gpt0.numberOfAdaptors() == 1);

	// Retrieve the adaptor again, as a GAdaptorT
	BOOST_CHECK_NO_THROW(boost::shared_ptr<GAdaptorT<char> > gadb0_ptr = gpt0.getAdaptor(GCharFlipAdaptor::adaptorName()));

	// Retrieve the adaptor in its original form
	BOOST_CHECK_NO_THROW(boost::shared_ptr<GCharFlipAdaptor> gpt0_ptr = gpt0.adaptor_cast<GCharFlipAdaptor>(GCharFlipAdaptor::adaptorName()));

	// Check mutations
	const std::size_t NMUTATIONS=10000;
	std::vector<char> mutvals(NMUTATIONS);
	char originalValue = gpt0.value();
	for(std::size_t i=0; i<NMUTATIONS; i++) {
		BOOST_CHECK_NO_THROW(gpt0.mutate());
		mutvals[i] = gpt0.value();
	}

	// Check that values do not stay the same for a larger number of mutations
	std::size_t nOriginalValues = std::count(mutvals.begin(), mutvals.end(), originalValue);
	BOOST_CHECK(nOriginalValues < NMUTATIONS);
}

/***********************************************************************************/
// Test features that are expected to work - boost::int32 case
BOOST_AUTO_TEST_CASE( GParameterT_int32_no_failure_expected)
{
	GRandom gr;

	{   // First try with just one adaptor
		// Default construction
		GInt32 gpt0;

		// Adding a single adaptor
		BOOST_CHECK_NO_THROW(gpt0.addAdaptor(boost::shared_ptr<GInt32FlipAdaptor>(new GInt32FlipAdaptor())));
		BOOST_CHECK(gpt0.numberOfAdaptors() == 1);

		// Retrieve the adaptor again, as a GAdaptorT
		BOOST_CHECK_NO_THROW(boost::shared_ptr<GAdaptorT<boost::int32_t> > gadb0_ptr = gpt0.getAdaptor(GInt32FlipAdaptor::adaptorName()));

		// Retrieve the adaptor in its original form
		BOOST_CHECK_NO_THROW(boost::shared_ptr<GInt32FlipAdaptor> gpt0_ptr = gpt0.adaptor_cast<GInt32FlipAdaptor>(GInt32FlipAdaptor::adaptorName()));

		// Check mutations
		const std::size_t NMUTATIONS=10000;
		std::vector<boost::int32_t> mutvals(NMUTATIONS);
		boost::int32_t originalValue = gpt0.value();
		for(std::size_t i=0; i<NMUTATIONS; i++) {
			BOOST_CHECK_NO_THROW(gpt0.mutate());
			mutvals[i] = gpt0.value();
		}

		// Check that values do not stay the same for a larger number of mutations
		std::size_t nOriginalValues = std::count(mutvals.begin(), mutvals.end(), originalValue);
		BOOST_CHECK(nOriginalValues < NMUTATIONS);
	}

	{   // Now we do the same again, with two adaptors
		// Default construction
		GInt32 gpt0;

		// Adding two adaptors
		BOOST_CHECK_NO_THROW(gpt0.addAdaptor(boost::shared_ptr<GInt32FlipAdaptor>(new GInt32FlipAdaptor())));
		BOOST_CHECK_NO_THROW(gpt0.addAdaptor(boost::shared_ptr<GInt32GaussAdaptor>(new GInt32GaussAdaptor())));
		BOOST_CHECK(gpt0.numberOfAdaptors() == 2);

		// Retrieve the adaptors again, camouflaged as a GAdaptorT
		BOOST_CHECK_NO_THROW(boost::shared_ptr<GAdaptorT<boost::int32_t> > gadb0_ptr = gpt0.getAdaptor(GInt32FlipAdaptor::adaptorName()));
		BOOST_CHECK_NO_THROW(boost::shared_ptr<GAdaptorT<boost::int32_t> > gadb1_ptr = gpt0.getAdaptor(GInt32GaussAdaptor::adaptorName()));

		// Retrieve the adaptors in their original form
		boost::shared_ptr<GInt32GaussAdaptor> gpt1_ptr;
		BOOST_CHECK_NO_THROW(boost::shared_ptr<GInt32FlipAdaptor> gpt0_ptr = gpt0.adaptor_cast<GInt32FlipAdaptor>(GInt32FlipAdaptor::adaptorName()));
		BOOST_CHECK_NO_THROW(gpt1_ptr = gpt0.adaptor_cast<GInt32GaussAdaptor>(GInt32GaussAdaptor::adaptorName()));

		// Check mutations
		const std::size_t NMUTATIONS=10000;
		std::vector<boost::int32_t> mutvals(NMUTATIONS);
		boost::int32_t originalValue = gpt0.value();
		for(std::size_t i=0; i<NMUTATIONS; i++) {
			BOOST_CHECK_NO_THROW(gpt0.mutate());
			mutvals[i] = gpt0.value();
		}

		// Check that values do not stay the same for a larger number of mutations
		std::size_t nOriginalValues = std::count(mutvals.begin(), mutvals.end(), originalValue);
		BOOST_CHECK(nOriginalValues < NMUTATIONS);

		// Delete one adaptor
		BOOST_CHECK_NO_THROW(gpt0.deleteAdaptor(GInt32FlipAdaptor::adaptorName()));
		BOOST_CHECK(gpt0.numberOfAdaptors() == 1);

		// again perform mutations. Make sure the adaptor has useful values so that
		// mutations actually have an effect ...
		gpt1_ptr->setAll(10., 1., 0., 100);
		originalValue = gpt0.value();
		for(std::size_t i=0; i<NMUTATIONS; i++) {
			BOOST_CHECK_NO_THROW(gpt0.mutate());
			mutvals[i] = gpt0.value();
		}

		// and check that values do not stay the same for a larger number of mutations
		nOriginalValues = std::count(mutvals.begin(), mutvals.end(), originalValue);
		BOOST_CHECK(nOriginalValues < NMUTATIONS);
	}
}

/***********************************************************************************/
// Test features that are expected to work - double case
BOOST_AUTO_TEST_CASE( GParameterT_double_no_failure_expected)
{
	GRandom gr;

	GDouble gpt0;

	// Adding a single adaptor
	BOOST_CHECK_NO_THROW(gpt0.addAdaptor(boost::shared_ptr<GDoubleGaussAdaptor>(new GDoubleGaussAdaptor())));

	// Retrieve the adaptor again, as a GAdaptorT
	BOOST_CHECK_NO_THROW(boost::shared_ptr<GAdaptorT<double> > gadb0_ptr = gpt0.getAdaptor(GDoubleGaussAdaptor::adaptorName()));

	// Retrieve the adaptor in its original form
	BOOST_CHECK_NO_THROW(boost::shared_ptr<GDoubleGaussAdaptor> gpt0_ptr = gpt0.adaptor_cast<GDoubleGaussAdaptor>(GDoubleGaussAdaptor::adaptorName()));

	// Check mutations
	const std::size_t NMUTATIONS=10000;
	std::vector<double> mutvals(NMUTATIONS);
	double originalValue = gpt0.value();
	for(std::size_t i=0; i<NMUTATIONS; i++) {
		BOOST_CHECK_NO_THROW(gpt0.mutate());
		mutvals[i] = gpt0.value();
	}

	// Check that values do not stay the same for a larger number of mutations
	std::size_t nOriginalValues = std::count(mutvals.begin(), mutvals.end(), originalValue);
	BOOST_CHECK(nOriginalValues < NMUTATIONS);
}

/***********************************************************************************/
// Test features that are expected to fail
BOOST_AUTO_TEST_CASE( GParmeterT_failures_expected)
{
	GRandom gr;

	{
		// Default construction
		GInt32 gpt0;
		// Adding an empty adaptor should throw
		BOOST_CHECK_THROW(gpt0.addAdaptor(boost::shared_ptr<GInt32FlipAdaptor>()), Gem::GenEvA::geneva_error_condition);
	}

	{
		// Default construction
		GInt32 gpt0;
		// Adding the same adaptor twice should throw
		BOOST_CHECK_NO_THROW(gpt0.addAdaptor(boost::shared_ptr<GInt32FlipAdaptor>(new GInt32FlipAdaptor())));
		BOOST_CHECK_THROW(gpt0.addAdaptor(boost::shared_ptr<GInt32FlipAdaptor>(new GInt32FlipAdaptor())), Gem::GenEvA::geneva_error_condition);
	}

	{
		// Default construction
		GInt32 gpt0;
		// Trying to find an adaptor that does not exist should throw
		BOOST_CHECK_NO_THROW(gpt0.addAdaptor(boost::shared_ptr<GInt32FlipAdaptor>(new GInt32FlipAdaptor())));
		BOOST_CHECK_THROW(gpt0.adaptor_cast<GInt32FlipAdaptor>("xyz"), Gem::GenEvA::geneva_error_condition);
	}

#ifdef DEBUG
	{
		// Default construction
		GInt32 gpt0;
		// Extracting an adaptor of wrong type should throw in DEBUG mode
		BOOST_CHECK_NO_THROW(gpt0.addAdaptor(boost::shared_ptr<GInt32FlipAdaptor>(new GInt32FlipAdaptor())));
		BOOST_CHECK_THROW(gpt0.adaptor_cast<GCharFlipAdaptor>(GInt32FlipAdaptor::adaptorName()), Gem::GenEvA::geneva_error_condition);
	}
#endif /* DEBUG */

	{
		// Default construction
		GInt32 gpt0;
		// Self assignment should throw
		BOOST_CHECK_THROW(gpt0.load(&gpt0), Gem::GenEvA::geneva_error_condition);
	}
}
/***********************************************************************************/

BOOST_AUTO_TEST_SUITE_END()
