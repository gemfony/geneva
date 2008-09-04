/**
 * @file GBitFlipAdaptor_test.cpp
 *
 * This test checks all public member functions of the GBitFlipAdaptor adaptor
 * class. In addition, it attempts to check parent classes, in particular the
 * GObject class. You should run this test both in DEBUG and in RELEASE mode,
 * as some functions work differently in this case.
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
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

// Boost header files go here

#define BOOST_TEST_MODULE GObject_test
#include <boost/test/unit_test.hpp>

#include <boost/shared_ptr.hpp>

// Geneva header files go here
#include "GObject.hpp"
#include "GBitFlipAdaptor.hpp"
#include "GenevaExceptions.hpp"
#include "GLogger.hpp"
#include "GLogTargets.hpp"

using namespace Gem;
using namespace Gem::GenEvA;
using namespace Gem::GLogFramework;

const std::string ADAPTORNAME="GBitFlipAdaptor";
const std::string ADAPTORNAME2="GBitFlipAdaptor2";
const std::string ADAPTORNAME3="GBitFlipAdaptor3";

/***********************************************************************************/
// This test checks as much as possible of the functionality
// provided by the parent class GObject, plus some base functionality
// of the GBitFlipAdaptor class, such as the "named" and the copy
// constructors.
BOOST_AUTO_TEST_CASE( gobject_test_no_failure_expected )
{
	// Add log targets to the system
	LOGGER.addTarget(boost::shared_ptr<GBaseLogTarget>(new GDiskLogger("GBitFlipAdaptor_test.log")));

	GBitFlipAdaptor *gbfa=new GBitFlipAdaptor(ADAPTORNAME); // Construction by name
	GBitFlipAdaptor *gbfa2=new GBitFlipAdaptor(*gbfa); // Copy construction

	// Getting and setting the name
	BOOST_CHECK(gbfa->name() == ADAPTORNAME);
	BOOST_CHECK(gbfa->name() == gbfa2->name());

	gbfa2->setName(ADAPTORNAME2);
	BOOST_CHECK(gbfa->name() == ADAPTORNAME);
	BOOST_CHECK(gbfa->name() != gbfa2->name());
	BOOST_CHECK(gbfa2->name() == ADAPTORNAME2);

	// Assigning the object
	*gbfa2 = *gbfa;
	BOOST_CHECK(gbfa->name() == ADAPTORNAME);
	BOOST_CHECK(gbfa->name() == gbfa2->name());

	// Changing the aspects again
	gbfa->setName(ADAPTORNAME2);
	BOOST_CHECK(gbfa->name() != gbfa2->name());

	// Cloning should create an independent object
	gbfa2->setName(ADAPTORNAME);
	GBitFlipAdaptor *gbfa3 = dynamic_cast<GBitFlipAdaptor *>(gbfa2->clone());
	if(!gbfa3) throw;
	BOOST_CHECK(gbfa3->name() == gbfa2->name());

	// We should be able to directly create an instance of the target type
	GBitFlipAdaptor *gbfa4 = gbfa2->clone_ptr_cast<GBitFlipAdaptor>();

	// And we should be able to create that target type wrapped into a boost::shared_ptr<> .
	boost::shared_ptr<GBitFlipAdaptor> gbfa5 = gbfa2->clone_bptr_cast<GBitFlipAdaptor>();

	// All should now have the same name
	BOOST_CHECK(gbfa3->name() == gbfa2->name());
	BOOST_CHECK(gbfa4->name() == gbfa2->name());
	BOOST_CHECK(gbfa4->name() == gbfa2->name());

	// Check that we have independent objects
	gbfa2->setName(ADAPTORNAME2);
	BOOST_CHECK(gbfa3->name() != gbfa2->name());
	BOOST_CHECK(gbfa4->name() != gbfa2->name());
	BOOST_CHECK(gbfa4->name() != gbfa2->name());

	// Loading the individual should again create an identical copy of the origin
	gbfa2->load(gbfa3);
	BOOST_CHECK(gbfa2->name() == gbfa3->name());

	// Create reports for both objects and check that they are the same
	BOOST_CHECK((gbfa2->report()).size() != 0 && gbfa2->report() == gbfa3->report());

	// Save to a string and load from the string in different modes, repeat a few times
	for(std::size_t i=0; i<10; i++){
		// First in XML mode
		gbfa->setName("ADAPTORNAME");
		gbfa2->setName("ADAPTORNAME2");
		BOOST_CHECK(gbfa->name() != gbfa2->name());
		gbfa->fromString(gbfa2->toString(XMLSERIALIZATION),XMLSERIALIZATION);
		BOOST_CHECK(gbfa->name() == gbfa2->name());

		// Then in text mode
		gbfa->setName("ADAPTORNAME");
		gbfa2->setName("ADAPTORNAME2");
		BOOST_CHECK(gbfa->name() != gbfa2->name());
		gbfa->fromString(gbfa2->toString(TEXTSERIALIZATION),TEXTSERIALIZATION);
		BOOST_CHECK(gbfa->name() == gbfa2->name());

		// And finally in binary mode
		gbfa->setName("ADAPTORNAME");
		gbfa2->setName("ADAPTORNAME2");
		BOOST_CHECK(gbfa->name() != gbfa2->name());
		gbfa->fromString(gbfa2->toString(BINARYSERIALIZATION),BINARYSERIALIZATION);
		BOOST_CHECK(gbfa->name() == gbfa2->name());
	}

	delete gbfa;
	delete gbfa2;
	delete gbfa3;
	delete gbfa4;
	// Note that gbfa5 will be deleted automatically by the boost::shared_ptr<>
}

/***********************************************************************************/
// Test functions for the following tests
bool testGObjectSelfAssignment(){
	try{
		GBitFlipAdaptor *gbfa=new GBitFlipAdaptor(ADAPTORNAME);
		gbfa->load(gbfa); // This must fail!!!
		delete gbfa;

		std::cerr << "Error: An exception should have been thrown prior to reaching this point" << std::endl;
		return false;
	}
	catch(geneva_error_condition& gec){
		return true; // An exception of this type was expected here
	}
	catch(...){
		std::cerr << "Error: Unknown exception caught" << std::endl;
		return false;
	}

	std::cerr << "Error: This point should never have been reached" << std::endl;
	return false;
}

/***********************************************************************************/
// This test checks for things that are expected to fail in GObject
BOOST_AUTO_TEST_CASE( gobject_test_failures_expected )
{
	BOOST_CHECK(testGObjectSelfAssignment());
}

/***********************************************************************************/
// Tests of the GAdaptor<T> and GBitFlipAdaptor functionality
BOOST_AUTO_TEST_CASE( gbitflipadaptor_no_failure_expected )
{
	boost::shared_ptr<GBitFlipAdaptor> gbfa(new GBitFlipAdaptor(ADAPTORNAME));

	BOOST_CHECK(gbfa); // Automatic conversion to bool. shared_ptr will be empty in case of a failure

	// Set and get the mutation probability
	gbfa->setMutationProbability(0.9);
	BOOST_CHECK(gbfa->getMutationProbability() == 0.9);

	// The value of a bit should never change, if the mutation probability is set to 0
	Gem::GenEvA::bit testBit = G_FALSE;
	gbfa->setMutationProbability(0.);
	for(boost::uint32_t i=0; i<10000; i++){
		gbfa->mutate(testBit);
		BOOST_CHECK(testBit == G_TRUE);
	}

	// The value of a bit should always change, if the mutation probability is set to 1
	gbfa->setMutationProbability(1.);
	testBit = G_FALSE;
	for(boost::uint32_t i=0; i<10000; i++){
		Gem::GenEvA::bit previousBit = testBit;
		gbfa->mutate(testBit);
		BOOST_CHECK(testBit == previousBit);
	}
}

/***********************************************************************************/
// Test functions for expected failures
bool testProbabilityUnsuitable(const double& value){
	try{
		boost::shared_ptr<GBitFlipAdaptor> gbfa(new GBitFlipAdaptor(ADAPTORNAME));
		if(!gbfa) {
			std::cerr << "Error: Construction of smart pointer failed" << std::endl;
			return false;
		}

		gbfa->setMutationProbability(value);
	}
	catch(geneva_error_condition& gec){
		return true; // An exception of this type was expected here
	}
	catch(...){
		std::cerr << "Error: Unknown exception caught" << std::endl;
		return false;
	}

	std::cerr << "Error: This point should never have been reached" << std::endl;
	return false;
}

/***********************************************************************************/
// Tests of the GAdaptor<T> and GBitFlipAdaptor functionality that should trigger
// exceptions or failures
BOOST_AUTO_TEST_CASE( gbitflipadaptor_failures_expected )
{
	BOOST_CHECK(testProbabilityUnsuitable(1.001)); // Probability > 100%
	BOOST_CHECK(testProbabilityUnsuitable(-0.001)); // Probability < 0%
}

/***********************************************************************************/

// EOF
