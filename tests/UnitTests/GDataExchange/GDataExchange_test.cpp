/**
 * @file GDataExchange_test.cpp
 *
 * This test checks all public member functions of the GDataExchange class plus
 * dependent classes. You should run this test both in DEBUG and in RELEASE mode,
 * as some functions may work differently in this case.
 */

/* Copyright (C)2009 Dr. Ruediger Berlich
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
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <cmath>

// Boost header files go here

#define BOOST_TEST_MODULE GDataExchange_test
#include <boost/test/unit_test.hpp>

#include <boost/shared_ptr.hpp>

// Geneva header files go here
#include "GRandom.hpp"
#include "GDataExchange.hpp"

using namespace Gem;
using namespace Gem::GenEvA;
using namespace Gem::Util;

const std::size_t NPARAMETERSETS=100;
const std::size_t NDATASETS=10;

/***********************************************************************************/
/**
 * Tests the various GNumericParameterT-class derivatives as a means
 * of storing individual parameters including boundaries.
 */
BOOST_AUTO_TEST_CASE(gnumericparametert_no_failure_expected)
{
	// Get access to a random number generator
	GRandom gr;

	// Test basic construction
	boost::shared_ptr<GDoubleParameter> d0(new GDoubleParameter());
	boost::shared_ptr<GLongParameter> l0(new GLongParameter());
	boost::shared_ptr<GBoolParameter> b0(new GBoolParameter());
	boost::shared_ptr<GCharParameter> c0(new GCharParameter());

	// Reset the object and fill with values
	d0->reset();
	l0->reset();
	b0->reset();
	c0->reset();

	d0->setParameter(1);
	l0->setParameter(2);
	b0->setParameter(false);
	c0->setParameter('x');

	// Test construction with value assignment
	boost::shared_ptr<GDoubleParameter> d1(new GDoubleParameter(gr.evenRandom(0., 10.)));
	boost::shared_ptr<GLongParameter> l1(new GLongParameter(gr.discreteRandom(0,10)));
	boost::shared_ptr<GBoolParameter> b1(new GBoolParameter(gr.boolRandom()));
	boost::shared_ptr<GCharParameter> c1(new GCharParameter(gr.charRandom()));

	// Test construction with value assignment and boundaries
	boost::shared_ptr<GDoubleParameter> d2(new GDoubleParameter(gr.evenRandom(0.,2.),0.,2.));
	boost::shared_ptr<GLongParameter> l2(new GLongParameter(gr.discreteRandom(0,10),0,10));
	boost::shared_ptr<GBoolParameter> b2(new GBoolParameter(gr.boolRandom(),false,true));
	boost::shared_ptr<GCharParameter> c2(new GCharParameter(gr.charRandom(),0,127));

	// Test copy construction
	boost::shared_ptr<GDoubleParameter> d3(new GDoubleParameter(*d2));
	boost::shared_ptr<GLongParameter> l3(new GLongParameter(*l2));
	boost::shared_ptr<GBoolParameter> b3(new GBoolParameter(*b2));
	boost::shared_ptr<GCharParameter> c3(new GCharParameter(*c2));

	// Test assignment operators
	*d3 = *d0;
	*l3 = *l0;
	*b3 = *b0;
	*c3 = *c0;

	// Check that the objects are now identical
	BOOST_REQUIRE(*d3==*d0);
	BOOST_REQUIRE(*l3==*l0);
	BOOST_REQUIRE(*b3==*b0);
	BOOST_REQUIRE(*c3==*c0);

	// Check that d3 etc. have the correct values. Note that they
	// had different values before the assignment.
	BOOST_REQUIRE(d3->getParameter() == 1.);
	BOOST_REQUIRE(l3->getParameter() == 2);
	BOOST_REQUIRE(b3->getParameter() == false);
	BOOST_REQUIRE(c3->getParameter() == 'x');

	// Check that no boundaries have been assigned
	BOOST_REQUIRE(!d3->hasBoundaries());
	BOOST_REQUIRE(!l3->hasBoundaries());
	BOOST_REQUIRE(!b3->hasBoundaries());
	BOOST_REQUIRE(!c3->hasBoundaries());

	// Assign new values
	d3->setParameter(3.);
	l3->setParameter(4);
	b3->setParameter(true);
	c3->setParameter('z');

	// Check again values and boundaries
	BOOST_REQUIRE(d3->getParameter() == 3.);
	BOOST_REQUIRE(l3->getParameter() == 4);
	BOOST_REQUIRE(b3->getParameter() == true);
	BOOST_REQUIRE(c3->getParameter() == 'z');

	// Check that no boundaries have been assigned
	BOOST_REQUIRE(!d3->hasBoundaries());
	BOOST_REQUIRE(!l3->hasBoundaries());
	BOOST_REQUIRE(!b3->hasBoundaries());
	BOOST_REQUIRE(!c3->hasBoundaries());

	// Assign new values, this time with boundaries
	d3->setParameter(4.,0.,4.);
	l3->setParameter(5,0,5);
	b3->setParameter(false,false,true);
	c3->setParameter('a',0,127);

	// Check again values and boundaries
	BOOST_REQUIRE(d3->getParameter() == 4.);
	BOOST_REQUIRE(l3->getParameter() == 5);
	BOOST_REQUIRE(b3->getParameter() == false);
	BOOST_REQUIRE(c3->getParameter() == 'a');

	// Check that boundaries have been assigned this time
	BOOST_REQUIRE(d3->hasBoundaries());
	BOOST_REQUIRE(l3->hasBoundaries());
	BOOST_REQUIRE(b3->hasBoundaries());
	BOOST_REQUIRE(c3->hasBoundaries());

	// Check the value of the lower boundaries ...
	BOOST_REQUIRE(d3->getLowerBoundary() == 0.);
	BOOST_REQUIRE(l3->getLowerBoundary() == 0);
	BOOST_REQUIRE(b3->getLowerBoundary() == false);
	BOOST_REQUIRE(c3->getLowerBoundary() == 0);

	// and the value of the upper boundaries
	BOOST_REQUIRE(d3->getUpperBoundary() == 4.);
	BOOST_REQUIRE(l3->getUpperBoundary() == 5);
	BOOST_REQUIRE(b3->getUpperBoundary() == true);
	BOOST_REQUIRE(c3->getUpperBoundary() == 127);

	// Write objects to file in binary and text mode repeatedly (so we can write
	// out different, random numbers), read them back in and check equality

	for(std::size_t i=0; i<100; i++) {
		//***********************************************************
		// double objects:
		boost::shared_ptr<GDoubleParameter> d4(new GDoubleParameter());
		d3->setParameter(gr.evenRandom(0.,4.));

		std::ofstream doutputbin("ddata.bin");
		d3->binaryWriteToStream(doutputbin);
		doutputbin.close();
		std::ifstream dinputbin("ddata.bin");
		d4->binaryReadFromStream(dinputbin);
		dinputbin.close();

		BOOST_REQUIRE(*d3==*d4);
		d4->reset();

		d3->setPrecision(11);

		std::ofstream doutputtxt("ddata.txt");
		d3->writeToStream(doutputtxt);
		doutputtxt.close();
		std::ifstream dinputtxt("ddata.txt");
		d4->readFromStream(dinputtxt);
		dinputtxt.close();

		// We cannot simply check for equality of d3 and d4 in the case of data exchange
		// in text format, as this exchange format implies a loss in precision.
		BOOST_CHECK(d3->isSimilarTo(*d4, exp(-10)));

		d4->reset();

		//***********************************************************
		// long objects
		boost::shared_ptr<GLongParameter> l4(new GLongParameter());
		l3->setParameter(gr.discreteRandom(0,5));

		std::ofstream loutputbin("ldata.bin");
		l3->binaryWriteToStream(loutputbin);
		loutputbin.close();
		std::ifstream linputbin("ldata.bin");
		l4->binaryReadFromStream(linputbin);
		linputbin.close();

		BOOST_REQUIRE(*l3==*l4);
		l4->reset();

		std::ofstream loutputtxt("ldata.txt");
		l3->writeToStream(loutputtxt);
		loutputtxt.close();
		std::ifstream linputtxt("ldata.txt");
		l4->readFromStream(linputtxt);
		linputtxt.close();

		BOOST_REQUIRE(*l3==*l4);
		l4->reset();

		//***********************************************************
		// bool objects
		boost::shared_ptr<GBoolParameter> b4(new GBoolParameter());
		b3->setParameter(gr.boolRandom());

		std::ofstream boutputbin("bdata.bin");
		b3->binaryWriteToStream(boutputbin);
		boutputbin.close();
		std::ifstream binputbin("bdata.bin");
		b4->binaryReadFromStream(binputbin);
		binputbin.close();

		BOOST_REQUIRE(*b3==*b4);
		b4->reset();

		std::ofstream boutputtxt("bdata.txt");
		b3->writeToStream(boutputtxt);
		boutputtxt.close();
		std::ifstream binputtxt("bdata.txt");
		b4->readFromStream(binputtxt);
		binputtxt.close();

		BOOST_REQUIRE(*b3==*b4);
		b4->reset();

		//***********************************************************
		// char objects
		boost::shared_ptr<GCharParameter> c4(new GCharParameter());
		b3->setParameter(gr.charRandom());

		std::ofstream coutputbin("cdata.bin");
		c3->binaryWriteToStream(coutputbin);
		coutputbin.close();
		std::ifstream cinputbin("cdata.bin");
		c4->binaryReadFromStream(cinputbin);
		cinputbin.close();

		BOOST_REQUIRE(*c3==*c4);
		c4->reset();

		std::ofstream coutputtxt("cdata.txt");
		c3->writeToStream(coutputtxt);
		coutputtxt.close();
		std::ifstream cinputtxt("cdata.txt");
		c4->readFromStream(cinputtxt);
		cinputtxt.close();

		BOOST_REQUIRE(*c3==*c4);
		c4->reset();
		//***********************************************************
	}
}

/***********************************************************************************/
/**
 * Tests the various functions of the GParameterValuePair class
 */
BOOST_AUTO_TEST_CASE(gparametervaluepair_no_failure_expected)
{
	GRandom gr;


}

/***********************************************************************************/
/**
 * Tests the GDataExchange functionality
 */
BOOST_AUTO_TEST_CASE( gdataexchange_no_failure_expected )
{
	GRandom gr;
	boost::shared_ptr<GDataExchange> gde(new GDataExchange());

	// Fill with individual value items
	for(std::size_t gde_counter=0; gde_counter < NDATASETS; gde_counter++) {
		for(std::size_t i=0; i<NPARAMETERSETS; i++) gde->append<double>(gr.evenRandom(-10.,10.));
		for(std::size_t i=0; i<NPARAMETERSETS; i++) gde->append<boost::int32_t>(gr.discreteRandom(-10,10));
		for(std::size_t i=0; i<NPARAMETERSETS; i++) gde->append<bool>(gr.boolRandom());
		for(std::size_t i=0; i<NPARAMETERSETS; i++) gde->append<char>(gr.charRandom());

		BOOST_REQUIRE(gde->numberOfParameterSets<double>() == NPARAMETERSETS);
		BOOST_REQUIRE(gde->numberOfParameterSets<boost::int32_t>() == NPARAMETERSETS);
		BOOST_REQUIRE(gde->numberOfParameterSets<bool>() == NPARAMETERSETS);
		BOOST_REQUIRE(gde->numberOfParameterSets<char>() == NPARAMETERSETS);

		if(gde_counter < NDATASETS-1) gde->newDataSet(); // Prevent an empty data set at the end
	}

	// GDataExchange fills itself with a single data set upon creation. Since we
	// added another 10 data sets, there should now be 11 of them.
	BOOST_REQUIRE(gde->nDataSets() == NDATASETS);

	gde->gotoStart();
	gde->resetAll(); // There should now only be one data set remaining
	BOOST_REQUIRE(gde->nDataSets() == 1);

	// Fill with values including boundaries
	for(std::size_t gde_counter=0; gde_counter < 10; gde_counter++) {
		for(std::size_t i=0; i<NPARAMETERSETS; i++) gde->append<double>(gr.evenRandom(-10.,10.),-11.,11.);
		for(std::size_t i=0; i<NPARAMETERSETS; i++) gde->append<boost::int32_t>(gr.discreteRandom(-10,10),-11,11);
		for(std::size_t i=0; i<NPARAMETERSETS; i++) gde->append<bool>(gr.boolRandom(), false, true);
		for(std::size_t i=0; i<NPARAMETERSETS; i++) gde->append<char>(gr.charRandom(false), 0, 127);

		BOOST_REQUIRE(gde->numberOfParameterSets<double>() == NPARAMETERSETS);
		BOOST_REQUIRE(gde->numberOfParameterSets<boost::int32_t>() == NPARAMETERSETS);
		BOOST_REQUIRE(gde->numberOfParameterSets<bool>() == NPARAMETERSETS);
		BOOST_REQUIRE(gde->numberOfParameterSets<char>() == NPARAMETERSETS);

		if(gde_counter < NDATASETS-1) gde->newDataSet(); // Prevent an empty data set at the end
	}
	BOOST_REQUIRE(gde->nDataSets() == NDATASETS);

	gde->gotoStart();
	gde->resetAll(); // There should now only be one data set remaining
	BOOST_REQUIRE(gde->nDataSets() == 1);

	// Fill directly with GParameter objects
	for(std::size_t gde_counter=0; gde_counter < NDATASETS; gde_counter++) {
		for(std::size_t i=0; i<NPARAMETERSETS; i++) {
			boost::shared_ptr<GDoubleParameter> d(new GDoubleParameter(gr.evenRandom(-10.,10.)));
			gde->append(d);
		}

		for(std::size_t i=0; i<NPARAMETERSETS; i++) {
			boost::shared_ptr<GLongParameter> l(new GLongParameter(gr.discreteRandom(-10,10)));
			gde->append(l);
		}

		for(std::size_t i=0; i<NPARAMETERSETS; i++) {
			boost::shared_ptr<GBoolParameter> b(new GBoolParameter(gr.boolRandom()));
			gde->append(b);
		}
		for(std::size_t i=0; i<NPARAMETERSETS; i++) {
			boost::shared_ptr<GCharParameter> c(new GCharParameter(gr.charRandom(false)));
			gde->append(c);
		}

		BOOST_REQUIRE(gde->numberOfParameterSets<double>() == NPARAMETERSETS);
		BOOST_REQUIRE(gde->numberOfParameterSets<boost::int32_t>() == NPARAMETERSETS);
		BOOST_REQUIRE(gde->numberOfParameterSets<bool>() == NPARAMETERSETS);
		BOOST_REQUIRE(gde->numberOfParameterSets<char>() == NPARAMETERSETS);

		if(gde_counter < NDATASETS-1) gde->newDataSet(); // Otherwise we will have an empty data set at the end
	}
	BOOST_REQUIRE(gde->nDataSets() == NDATASETS);

	// Check that we can assign values to the data sets and iterate through them
	gde->gotoStart();
	do {
		BOOST_REQUIRE(!gde->hasValue()); // No value has been assigned so far
		double value = gr.evenRandom(0.,10.);
		gde->setValue(value);
		BOOST_REQUIRE(gde->hasValue()); // Value should have been set at this point
		BOOST_REQUIRE(value == gde->value());
	}
	while(gde->nextDataSet());

	// Switch to the best data set, with the lowest value being "best"
	gde->switchToBestDataSet(true);
	BOOST_REQUIRE(gde->nDataSets() == NDATASETS);

	// Check that the container is indeed sorted in ascending order
	double bestValue=-1.; // Note that we have initialized the values with random numbers in the range [0.,10.[
	double tmpBestValue = -1.;
	do {
		tmpBestValue = gde->value();
		BOOST_REQUIRE(bestValue <= tmpBestValue);
		bestValue = tmpBestValue;
	}
	while(gde->nextDataSet());

	// Switch to the best data set, with the highest value being "best"
	gde->switchToBestDataSet(false);
	BOOST_REQUIRE(gde->nDataSets() == NDATASETS);

	// Check that the container is indeed sorted in descending order
	bestValue=11.; // Note that we have initialized the values with random numbers in the range [0.,10.[
	do {
		tmpBestValue = gde->value();
		BOOST_REQUIRE(bestValue >= tmpBestValue);
		bestValue = tmpBestValue;
	}
	while(gde->nextDataSet());
	BOOST_REQUIRE(gde->nDataSets() == NDATASETS);

	/*
	// Test whether data can be written to file and read back in again.
	// In text mode
	// gde->writeToFile("testFile.txt",false);
	boost::shared_ptr<GDataExchange> gde2(new GDataExchange()); // Create second, empty object
	// gde2->readFromFile("testFile.txt",false);
	// BOOST_REQUIRE(*gde == *gde2);

	 // Put gde2 in pristine condition so we can start over with the binary mode
	gde2->resetAll();

	// In binary mode
	gde->writeToFile("testFile.bin",true);
	gde2->readFromFile("testFile.bin",true);
	BOOST_REQUIRE(*gde == *gde2);

	gde->resetAll(); // There should now only be one data set remaining
	BOOST_REQUIRE(gde->nDataSets() == 1);


	// Need test about file creation and scanning. In this context:
	// Improve fp accuracy of serialized numbers.

	// Test precision setting
	 *
	 */
}

/***********************************************************************************/

// EOF
