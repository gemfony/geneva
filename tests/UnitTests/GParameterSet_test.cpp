/**
 * @file GParameterSet_test.cpp
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

// Boost header files go here

#include <boost/test/unit_test.hpp>

#include <boost/shared_ptr.hpp>

// Geneva header files go here
#include "GenevaExceptions.hpp"
#include "GLogger.hpp"
#include "GLogTargets.hpp"
#include "GRandom.hpp"
#include "GParameterSet.hpp"
#include "GParabolaIndividual.hpp"
#include "GDoubleCollection.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GStdVectorInterface_test.hpp"

using namespace Gem;
using namespace Gem::Util;
using namespace Gem::GenEvA;
using namespace Gem::GLogFramework;

/***********************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GParameterSet class, using the GParabolaIndividual class. It also
// checks the functionality of the GMutableSetT and the GIndividual classes,
// as far as possible.

BOOST_AUTO_TEST_SUITE(GParameterSetSuite)

/***********************************************************************************/
// Test features that are expected to work
BOOST_AUTO_TEST_CASE( GParameterSet_no_failure_expected )
{
	GRandom gr;

	// Default construction
	GParabolaIndividual gpi;

	// Test the vector interface of GMutableSetT
	GDoubleCollection tempIItem(100, -10., 10.);
	boost::shared_ptr<GDoubleGaussAdaptor> gdga1(new GDoubleGaussAdaptor(1.,0.001,0.,1.));
	tempIItem.addAdaptor(gdga1);

	GDoubleCollection findItem(100, -10., 10.);
	boost::shared_ptr<GDoubleGaussAdaptor> gdga2(new GDoubleGaussAdaptor(2.,0.001,0.,2.));
	findItem.addAdaptor(gdga2);

	stdvectorinterfacetestSP(gpi, tempIItem, findItem);
	// At this point gpi should have a number of items attached to it

	BOOST_CHECK(!gpi.empty());

	// Copy construction
	GParabolaIndividual gpi_cc(gpi);
	BOOST_CHECK(gpi_cc.isEqualTo(gpi));

	// Assignment
	GParabolaIndividual gpi_as;
	gpi_as = gpi;
	BOOST_CHECK(gpi_as.isEqualTo(gpi));

	// Test cloning and loading
	GObject *gpi_clone;
	BOOST_CHECK_NO_THROW(gpi_clone = gpi.clone());
	GParabolaIndividual gpi_load;
	BOOST_CHECK_NO_THROW(gpi_load.load(gpi_clone));
	BOOST_CHECK(gpi_load.isEqualTo(gpi));
	delete gpi_clone;

	// Test that the copied, cloned, ... objects become inequal to the
	// original when they are modified

	// Test registering and using an evaluator

	// Test mutation

	// Test retrieval of the GDoubleCollection object. Can it be modified ?

	// Test serialization and loading in different serialization modes
	{ // plain text format
		// Copy construction of a new object
		GParabolaIndividual gpi_ser;
		boost::shared_ptr<GDoubleCollection> gdc_ser1(new GDoubleCollection(100, -10., 10.));
		boost::shared_ptr<GDoubleGaussAdaptor> gdga_ser1(new GDoubleGaussAdaptor(1.,0.001,0.,1.));
		gdc_ser1->addAdaptor(gdga_ser1);
		gpi_ser.push_back(gdc_ser1);
		GParabolaIndividual gpi_ser_cp(gpi_ser);

		// Check equalities and inequalities
		BOOST_CHECK(gpi_ser_cp == gpi_ser);
		// Add a new collection to gpi_ser_cp
		boost::shared_ptr<GDoubleCollection> gdc_ser2(new GDoubleCollection(100, -10., 10.));
		boost::shared_ptr<GDoubleGaussAdaptor> gdga_ser2(new GDoubleGaussAdaptor(1.,0.001,0.,1.));
		gdc_ser2->addAdaptor(gdga_ser2);
		gpi_ser_cp.push_back(gdc_ser2);
		BOOST_CHECK(gpi_ser_cp != gpi_ser);

		// Serialize gpi_ser and load into gpi_ser_cp, check equalities and similarities
		BOOST_REQUIRE_NO_THROW(gpi_ser_cp.fromString(gpi_ser.toString(TEXTSERIALIZATION), TEXTSERIALIZATION));
		BOOST_CHECK(gpi_ser_cp.isSimilarTo(gpi_ser, exp(-10)));
	}

	{ // XML format
		// Copy construction of a new object
		GParabolaIndividual gpi_ser;
		boost::shared_ptr<GDoubleCollection> gdc_ser1(new GDoubleCollection(100, -10., 10.));
		boost::shared_ptr<GDoubleGaussAdaptor> gdga_ser1(new GDoubleGaussAdaptor(1.,0.001,0.,1.));
		gdc_ser1->addAdaptor(gdga_ser1);
		gpi_ser.push_back(gdc_ser1);
		GParabolaIndividual gpi_ser_cp(gpi_ser);

		// Check equalities and inequalities
		BOOST_CHECK(gpi_ser_cp == gpi_ser);
		// Add a new collection to gpi_ser_cp
		boost::shared_ptr<GDoubleCollection> gdc_ser2(new GDoubleCollection(100, -10., 10.));
		boost::shared_ptr<GDoubleGaussAdaptor> gdga_ser2(new GDoubleGaussAdaptor(1.,0.001,0.,1.));
		gdc_ser2->addAdaptor(gdga_ser2);
		gpi_ser_cp.push_back(gdc_ser2);
		BOOST_CHECK(gpi_ser_cp != gpi_ser);

		// Serialize gpi_ser and load into gpi_ser_cp, check equalities and similarities
		BOOST_REQUIRE_NO_THROW(gpi_ser_cp.fromString(gpi_ser.toString(XMLSERIALIZATION), XMLSERIALIZATION));
		BOOST_CHECK(gpi_ser_cp.isSimilarTo(gpi_ser, exp(-10)));
	}

	{ // binary test format
		// Copy construction of a new object
		// Copy construction of a new object
		GParabolaIndividual gpi_ser;
		boost::shared_ptr<GDoubleCollection> gdc_ser1(new GDoubleCollection(100, -10., 10.));
		boost::shared_ptr<GDoubleGaussAdaptor> gdga_ser1(new GDoubleGaussAdaptor(1.,0.001,0.,1.));
		gdc_ser1->addAdaptor(gdga_ser1);
		gpi_ser.push_back(gdc_ser1);
		GParabolaIndividual gpi_ser_cp(gpi_ser);

		// Check equalities and inequalities
		BOOST_CHECK(gpi_ser_cp == gpi_ser);
		// Add a new collection to gpi_ser_cp
		boost::shared_ptr<GDoubleCollection> gdc_ser2(new GDoubleCollection(100, -10., 10.));
		boost::shared_ptr<GDoubleGaussAdaptor> gdga_ser2(new GDoubleGaussAdaptor(1.,0.001,0.,1.));
		gdc_ser2->addAdaptor(gdga_ser2);
		gpi_ser_cp.push_back(gdc_ser2);
		BOOST_CHECK(gpi_ser_cp != gpi_ser);

		// Serialize gpi_ser and load into gpi_ser_cp, check equalities and similarities
		BOOST_REQUIRE_NO_THROW(gpi_ser_cp.fromString(gpi_ser.toString(BINARYSERIALIZATION), BINARYSERIALIZATION));
		BOOST_CHECK(gpi_ser_cp.isEqualTo(gpi_ser));
	}
}

/***********************************************************************************/
// Test features that are expected to fail
BOOST_AUTO_TEST_CASE( GParameterSet_failures_expected )
{
	GRandom gr;

	// Default construction
	GParabolaIndividual gpi;

	// Self-assignment should throw
	BOOST_CHECK_THROW(gpi.load(&gpi), Gem::GenEvA::geneva_error_condition);
}
/***********************************************************************************/

BOOST_AUTO_TEST_SUITE_END()
