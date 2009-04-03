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
#include "GGaussAdaptorT.hpp"

using namespace Gem;
using namespace Gem::Util;
using namespace Gem::GenEvA;
using namespace Gem::GLogFramework;

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
	// A name should have been set automatically
	BOOST_CHECK(ggat0.adaptorName() == GGAUSSADAPTORSTANDARDNAME);

	// Instantiation with a suitable sigma value
	GGaussAdaptorT<T> ggat1(0.1);

	// Instantiation with sigma, sigmaSigma, minSigma and maxSigma
	GGaussAdaptorT<T> ggat2(0.1, 0.001, 0., 1.);

	// Copy construction
	GGaussAdaptorT<T> ggat3(ggat2);

	// Assignment
	boost::shared_ptr<GGaussAdaptorT<T> > ggat4_ptr(new GGaussAdaptorT<T>());
	*ggat4_ptr = ggat3;

	// ... and loading
	GGaussAdaptorT<T> ggat5;
	ggat5.load(ggat4_ptr.get());
}

/***********************************************************************************/
// Test features that are expected to fail
BOOST_AUTO_TEST_CASE_TEMPLATE( GGaussAdaptorT_failures_expected, T, test_types )
{
	GRandom gr;

}
/***********************************************************************************/

BOOST_AUTO_TEST_SUITE_END()
