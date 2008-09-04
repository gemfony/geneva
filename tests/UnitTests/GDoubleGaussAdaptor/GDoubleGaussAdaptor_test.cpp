/**
 * @file GDoubleGaussAdaptor_test.cpp
 *
 * This test checks all public member functions of the GDoubleGaussAdaptor adaptor
 * class. In addition, it attempts to check parent classes, in particular the
 * GObject class. You should run this test both in DEBUG and in RELEASE mode,
 * as some functions may work differently in this case.
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
#include <vector>

// Boost header files go here

#define BOOST_TEST_MODULE GDoubleGaussAdaptor_test
#include <boost/test/unit_test.hpp>

#include <boost/shared_ptr.hpp>

// Geneva header files go here
#include "GObject.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GenevaExceptions.hpp"
#include "GLogger.hpp"
#include "GLogTargets.hpp"
#include "GRandom.hpp"

using namespace Gem;
using namespace Gem::GenEvA;
using namespace Gem::GLogFramework;

const std::string ADAPTORNAME="GDoubleGaussAdaptor";
const std::string ADAPTORNAME2="GDoubleGaussAdaptor2";
const std::string ADAPTORNAME3="GDoubleGaussAdaptor3";

/***********************************************************************************/
// This test checks as much as possible of the functionality
// provided by the parent class GObject, plus some base functionality
// of the GDoubleGaussAdaptor class, such as the "named" and the copy
// constructors.
BOOST_AUTO_TEST_CASE( gdga_gobject_test_no_failure_expected )
{
	// Add log targets to the system
	LOGGER.addTarget(boost::shared_ptr<GBaseLogTarget>(new GDiskLogger("GDoubleGaussAdaptor_test.log")));

	GDoubleGaussAdaptor *gdga=new GDoubleGaussAdaptor(ADAPTORNAME); // Construction by name
	GDoubleGaussAdaptor *gdga2=new GDoubleGaussAdaptor(*gdga); // Copy construction

	// Getting and setting the name
	BOOST_CHECK(gdga->name() == ADAPTORNAME);
	BOOST_CHECK(gdga->name() == gdga2->name());

	gdga2->setName(ADAPTORNAME2);
	BOOST_CHECK(gdga->name() == ADAPTORNAME);
	BOOST_CHECK(gdga->name() != gdga2->name());
	BOOST_CHECK(gdga2->name() == ADAPTORNAME2);

	// Assigning the object
	*gdga2 = *gdga;
	BOOST_CHECK(gdga->name() == ADAPTORNAME);
	BOOST_CHECK(gdga->name() == gdga2->name());

	// Changing the aspects again
	gdga->setName(ADAPTORNAME2);
	BOOST_CHECK(gdga->name() != gdga2->name());

	// Cloning should create an independent object
	gdga2->setName(ADAPTORNAME);
	GDoubleGaussAdaptor *gdga3 = dynamic_cast<GDoubleGaussAdaptor *>(gdga2->clone());
	if(!gdga3) throw;
	BOOST_CHECK(gdga3->name() == gdga2->name());

	// We should be able to directly create an instance of the target type
	GDoubleGaussAdaptor *gdga4 = gdga2->clone_ptr_cast<GDoubleGaussAdaptor>();

	// And we should be able to create that target type wrapped into a boost::shared_ptr<> .
	boost::shared_ptr<GDoubleGaussAdaptor> gdga5 = gdga2->clone_bptr_cast<GDoubleGaussAdaptor>();

	// All should now have the same name
	BOOST_CHECK(gdga3->name() == gdga2->name());
	BOOST_CHECK(gdga4->name() == gdga2->name());
	BOOST_CHECK(gdga4->name() == gdga2->name());

	// Check that we have independent objects
	gdga2->setName(ADAPTORNAME2);
	BOOST_CHECK(gdga3->name() != gdga2->name());
	BOOST_CHECK(gdga4->name() != gdga2->name());
	BOOST_CHECK(gdga4->name() != gdga2->name());

	// Loading the individual should again create an identical copy of the origin
	gdga2->load(gdga3);
	BOOST_CHECK(gdga2->name() == gdga3->name());

	// Create reports for both objects and check that they are the same
	BOOST_CHECK((gdga2->report()).size() != 0 && gdga2->report() == gdga3->report());

	// Save to a string and load from the string in different modes, repeat a few times
	for(std::size_t i=0; i<10; i++){
		// First in XML mode
		gdga->setName("ADAPTORNAME");
		gdga2->setName("ADAPTORNAME2");
		BOOST_CHECK(gdga->name() != gdga2->name());
		gdga->fromString(gdga2->toString(XMLSERIALIZATION),XMLSERIALIZATION);
		BOOST_CHECK(gdga->name() == gdga2->name());

		// Then in text mode
		gdga->setName("ADAPTORNAME");
		gdga2->setName("ADAPTORNAME2");
		BOOST_CHECK(gdga->name() != gdga2->name());
		gdga->fromString(gdga2->toString(TEXTSERIALIZATION),TEXTSERIALIZATION);
		BOOST_CHECK(gdga->name() == gdga2->name());

		// And finally in binary mode
		gdga->setName("ADAPTORNAME");
		gdga2->setName("ADAPTORNAME2");
		BOOST_CHECK(gdga->name() != gdga2->name());
		gdga->fromString(gdga2->toString(BINARYSERIALIZATION),BINARYSERIALIZATION);
		BOOST_CHECK(gdga->name() == gdga2->name());
	}

	delete gdga;
	delete gdga2;
	delete gdga3;
	delete gdga4;
	// Note that gdga5 will be deleted automatically by the boost::shared_ptr<>
}

/***********************************************************************************/
// Test functions for the following tests
bool gdga_testGObjectSelfAssignment(){
	try{
		GDoubleGaussAdaptor *gdga=new GDoubleGaussAdaptor(ADAPTORNAME);
		gdga->load(gdga); // This must fail!!!
		delete gdga;

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
BOOST_AUTO_TEST_CASE( gdga_gobject_test_failures_expected )
{
	BOOST_CHECK(gdga_testGObjectSelfAssignment());
}

/***********************************************************************************/
// Tests of the GAdaptor<T> and GDoubleGaussAdaptor functionality
BOOST_AUTO_TEST_CASE( gdoublegaussadaptor_no_failure_expected )
{
	{
		// Construction with a given sigma
		boost::shared_ptr<GDoubleGaussAdaptor> gdga(new GDoubleGaussAdaptor(2.0, ADAPTORNAME));
		BOOST_CHECK(gdga); // Automatic conversion to bool. shared_ptr will be empty in case of a failure
		BOOST_CHECK(gdga->getSigma() == 2.0);
		BOOST_CHECK(gdga->name() == ADAPTORNAME);
	} // gdga will cease to exist here

	// Construction with all parameters
	boost::shared_ptr<GDoubleGaussAdaptor> gdga(new GDoubleGaussAdaptor(2.0, 0.2, 0.002, ADAPTORNAME));
	BOOST_CHECK(gdga); // Automatic conversion to bool. shared_ptr will be empty in cas	e of a failure

	// Check that the parameters have been set
	BOOST_CHECK(gdga->name() == ADAPTORNAME);
	BOOST_CHECK(gdga->getSigma() == 2.0);
	BOOST_CHECK(gdga->getSigmaSigma() == 0.2);
	BOOST_CHECK(gdga->getMinSigma() == 0.002);

	// Set and get all parameters
	gdga->setAll(1.0, 0.1, 0.001);
	BOOST_CHECK(gdga->getSigma() == 1.0);
	BOOST_CHECK(gdga->getSigmaSigma() == 0.1);
	BOOST_CHECK(gdga->getMinSigma() == 0.001);

	// Do it again with different functions
	gdga->setSigma(2.0);
	gdga->setSigmaSigma(0.2, 0.002);
	BOOST_CHECK(gdga->getSigma() == 2.0);
	BOOST_CHECK(gdga->getSigmaSigma() == 0.2);
	BOOST_CHECK(gdga->getMinSigma() == 0.002);

	// The value of a double should change during each mutation
	double testValue = 1.;
	double previousValue = testValue;

	gdga->setSigmaSigma(0.,0.); // Prevent changes of sigma
	for(boost::uint32_t i=0; i<100000; i++){
		previousValue = testValue;
		gdga->mutate(testValue);
		BOOST_CHECK(previousValue != testValue);
	}

	gdga->setSigmaSigma(0.2,0.002); // Allow changes of sigma
	for(boost::uint32_t i=0; i<100000; i++){
		previousValue = testValue;
		gdga->mutate(testValue);
		BOOST_CHECK(previousValue != testValue);
	}

	// Start with a new adaptor, this time for a vector
	boost::shared_ptr<GDoubleGaussAdaptor> gdga2(new GDoubleGaussAdaptor(2.0, 0.2, 0.002, ADAPTORNAME));
	std::vector<double> doubleVector, previousVector;
	GRandom gr;
	for(std::size_t i=0; i<1000; i++){
		doubleVector.push_back(gr.evenRandom(-1.,1.));
	}

	gdga2->setAlwaysInit(false);
	// Mutate a couple of times, see what happens
	for(boost::uint32_t i=0; i<1000; i++){
		previousVector = doubleVector;
		gdga2->mutate(doubleVector);
		BOOST_CHECK(previousVector!=doubleVector);
	}

	gdga2->setAlwaysInit(true);
	for(boost::uint32_t i=0; i<1000; i++){
		previousVector = doubleVector;
		gdga2->mutate(doubleVector);
		BOOST_CHECK(previousVector!=doubleVector);
	}
}

/***********************************************************************************/
// Test functions for expected failures
bool gdga_testUnsuitableMutationParameters(const double& sigma, const double& sigmaSigma, const double& minSigma){
	try{
		boost::shared_ptr<GDoubleGaussAdaptor> gdga(new GDoubleGaussAdaptor(ADAPTORNAME));
		if(!gdga) {
			std::cerr << "Error: Construction of smart pointer failed" << std::endl;
			return false;
		}

		gdga->setAll(sigma, sigmaSigma, minSigma);
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
// Tests of the GAdaptor<T> and GDoubleGaussAdaptor functionality that should trigger
// exceptions or failures
BOOST_AUTO_TEST_CASE( gdoublegaussadaptor_failures_expected )
{
	BOOST_CHECK(gdga_testUnsuitableMutationParameters(0.,0.1,0.001)); // Invalid sigma
	BOOST_CHECK(gdga_testUnsuitableMutationParameters(-1.,0.1,0.001)); // Invalid sigma
	BOOST_CHECK(gdga_testUnsuitableMutationParameters(1.,-1.,0.001)); // Invalid sigmaSigma
	BOOST_CHECK(gdga_testUnsuitableMutationParameters(1.,0.1,0.)); // Invalid minSigma
	BOOST_CHECK(gdga_testUnsuitableMutationParameters(1.,0.1,-1)); // Invalid minSigma
}

/***********************************************************************************/

// EOF
