/**
 * @file GDataExchange_test.cpp
 *
 * This test checks all public member functions of the GDataExchange class plus
 * dependent classes. You should run this test both in DEBUG and in RELEASE mode,
 * as some functions may work differently in this case.
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
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <cmath>

// Boost header files go here

#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GDATAEXCHANGE_TEST_HPP_
#define GDATAEXCHANGE_TEST_HPP_

// Geneva header files go here
#include "hap/GRandom.hpp"
#include "dataexchange/GDataExchange.hpp"

using namespace Gem;
using namespace Gem::GenEvA;
using namespace Gem::Dataexchange;

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

/********************************************************************************************/
// The actual unit tests for this class
class GDataExchange_test {
public:
	/***********************************************************************************/
	// The default constructor
	GDataExchange_test():
		NPARAMETERSETS(100),
		NDATASETS(10)
	{ /* empty */ }

	/***********************************************************************************/
	// Tests the various GNumericParameterT-class derivatives as a means
	// of storing individual parameters including boundaries.
	void gnumericparametert_no_failure_expected() {
		// Test basic construction
		boost::shared_ptr<GDoubleParameter> d0(new GDoubleParameter());
		boost::shared_ptr<GLongParameter> l0(new GLongParameter());
		boost::shared_ptr<GBoolParameter> b0(new GBoolParameter());

		// Reset the object and fill with values
		d0->reset();
		l0->reset();
		b0->reset();

		d0->setParameter(1);
		l0->setParameter(2);
		b0->setParameter(false);

		// Test construction with value assignment
		boost::shared_ptr<GDoubleParameter> d1(new GDoubleParameter(gr.evenRandom(0., 10.)));
		boost::shared_ptr<GLongParameter> l1(new GLongParameter(gr.discreteRandom(0,10)));
		boost::shared_ptr<GBoolParameter> b1(new GBoolParameter(gr.boolRandom()));

		// Test construction with value assignment and boundaries
		boost::shared_ptr<GDoubleParameter> d2(new GDoubleParameter(gr.evenRandom(0.,2.),0.,2.));
		boost::shared_ptr<GLongParameter> l2(new GLongParameter(gr.discreteRandom(0,10),0,10));
		boost::shared_ptr<GBoolParameter> b2(new GBoolParameter(gr.boolRandom(),false,true));

		// Test copy construction
		boost::shared_ptr<GDoubleParameter> d3(new GDoubleParameter(*d2));
		boost::shared_ptr<GLongParameter> l3(new GLongParameter(*l2));
		boost::shared_ptr<GBoolParameter> b3(new GBoolParameter(*b2));

		// Test assignment operators
		*d3 = *d0;
		*l3 = *l0;
		*b3 = *b0;

		// Check that the objects are now identical
		BOOST_CHECK(*d3==*d0);
		BOOST_CHECK(*l3==*l0);
		BOOST_CHECK(*b3==*b0);

		// Check that d3 etc. have the correct values. Note that they
		// had different values before the assignment.
		BOOST_CHECK(d3->getParameter() == 1.);
		BOOST_CHECK(l3->getParameter() == 2);
		BOOST_CHECK(b3->getParameter() == false);

		// Check that no boundaries have been assigned
		BOOST_CHECK(!d3->hasBoundaries());
		BOOST_CHECK(!l3->hasBoundaries());
		BOOST_CHECK(!b3->hasBoundaries());

		// Assign new values
		d3->setParameter(3.);
		l3->setParameter(4);
		b3->setParameter(true);

		// Check again values and boundaries
		BOOST_CHECK(d3->getParameter() == 3.);
		BOOST_CHECK(l3->getParameter() == 4);
		BOOST_CHECK(b3->getParameter() == true);

		// Check that no boundaries have been assigned
		BOOST_CHECK(!d3->hasBoundaries());
		BOOST_CHECK(!l3->hasBoundaries());
		BOOST_CHECK(!b3->hasBoundaries());

		// Assign new values, this time with boundaries
		d3->setParameter(4.,0.,4.);
		l3->setParameter(5,0,5);
		b3->setParameter(false,false,true);

		// Check again values and boundaries
		BOOST_CHECK(d3->getParameter() == 4.);
		BOOST_CHECK(l3->getParameter() == 5);
		BOOST_CHECK(b3->getParameter() == false);

		// Check that boundaries have been assigned this time
		BOOST_CHECK(d3->hasBoundaries());
		BOOST_CHECK(l3->hasBoundaries());
		BOOST_CHECK(b3->hasBoundaries());

		// Check the value of the lower boundaries ...
		BOOST_CHECK(d3->getLowerBoundary() == 0.);
		BOOST_CHECK(l3->getLowerBoundary() == 0);
		BOOST_CHECK(b3->getLowerBoundary() == false);

		// and the value of the upper boundaries
		BOOST_CHECK(d3->getUpperBoundary() == 4.);
		BOOST_CHECK(l3->getUpperBoundary() == 5);
		BOOST_CHECK(b3->getUpperBoundary() == true);

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

			BOOST_CHECK(*d3==*d4);
			d4->reset();

			std::ofstream doutputtxt("ddata.txt");
			doutputtxt.precision(15); // This normally happens in  the GDataExchange object
			d3->writeToStream(doutputtxt);
			doutputtxt.close();
			std::ifstream dinputtxt("ddata.txt");
			dinputtxt.precision(15);  // This normally happens in  the GDataExchange object
			d4->readFromStream(dinputtxt);
			dinputtxt.close();

			// We cannot simply check for equality of d3 and d4 in the case of data exchange
			// in text format, as this exchange format implies a loss in precision.
			BOOST_CHECK(d3->isSimilarTo(*d4, pow(10,-10)));

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

			BOOST_CHECK(*l3==*l4);
			l4->reset();

			std::ofstream loutputtxt("ldata.txt");
			l3->writeToStream(loutputtxt);
			loutputtxt.close();
			std::ifstream linputtxt("ldata.txt");
			l4->readFromStream(linputtxt);
			linputtxt.close();

			BOOST_CHECK(*l3==*l4);
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

			BOOST_CHECK(*b3==*b4);
			b4->reset();

			std::ofstream boutputtxt("bdata.txt");
			b3->writeToStream(boutputtxt);
			boutputtxt.close();
			std::ifstream binputtxt("bdata.txt");
			b4->readFromStream(binputtxt);
			binputtxt.close();

			BOOST_CHECK(*b3==*b4);
			b4->reset();

			//***********************************************************
		}

		// Still need to test serialization
	}

	/***********************************************************************************/
	// Tests the various functions of the GParameterValuePair class
	void gparametervaluepair_no_failure_expected() {
		// Test default construction
		boost::shared_ptr<GParameterValuePair> p0(new GParameterValuePair());
		boost::shared_ptr<GParameterValuePair> p1(new GParameterValuePair());

		BOOST_CHECK(p0->value_ == 0.);
		BOOST_CHECK(!p0->hasValue_);
		BOOST_CHECK(p0->dArray_.empty());
		BOOST_CHECK(p0->lArray_.empty());
		BOOST_CHECK(p0->bArray_.empty());

		// Attach data to the vectors
		for(std::size_t i=0; i<NPARAMETERSETS; i++) {
			// Deal with p0
			boost::shared_ptr<GDoubleParameter> d0(new GDoubleParameter(gr.evenRandom(0.,10.)));
			p0->dArray_.push_back(d0);

			boost::shared_ptr<GLongParameter> l0(new GLongParameter(gr.discreteRandom(0,10)));
			p0->lArray_.push_back(l0);

			boost::shared_ptr<GBoolParameter> b0(new GBoolParameter(gr.boolRandom()));
			p0->bArray_.push_back(b0);

			// And now p1
			boost::shared_ptr<GDoubleParameter> d1(new GDoubleParameter(gr.evenRandom(0.,10.)));
			p1->dArray_.push_back(d1);

			boost::shared_ptr<GLongParameter> l1(new GLongParameter(gr.discreteRandom(0,10)));
			p1->lArray_.push_back(l1);

			boost::shared_ptr<GBoolParameter> b1(new GBoolParameter(gr.boolRandom()));
			p1->bArray_.push_back(b1);
		}

		// Assign A value and check for its existence
		p0->value_ = 1.234;
		p0->hasValue_ = true;
		BOOST_CHECK(p0->value_ == p0->value());
		BOOST_CHECK(p0->hasValue_ == p0->hasValue());

		// Check copy construction and the correct copying of data. Also checks the operator==
		boost::shared_ptr<GParameterValuePair> p2(new GParameterValuePair(*p0));
		BOOST_CHECK(*p2 == *p0);

		// Check that two very different objects are indeed not similar to each other
		BOOST_CHECK(!p2->isSimilarTo(*p1));

		// Reset p2 and check that it is no different from p0 and empty
		p2->reset();
		BOOST_CHECK(*p2 != *p0);
		BOOST_CHECK(p2->value_ == 0.);
		BOOST_CHECK(!p2->hasValue_);
		BOOST_CHECK(p2->dArray_.empty());
		BOOST_CHECK(p2->lArray_.empty());
		BOOST_CHECK(p2->bArray_.empty());

		// Check the assignment operator and check again equality
		*p2 = *p0;
		BOOST_CHECK(*p2 == *p0);

		// Write the object out in binary mode and load it back in. Then check equality.
		std::ofstream binaryOutput("pvp.bin");
		BOOST_CHECK(binaryOutput.good());
		p0->binaryWriteToStream(binaryOutput);
		binaryOutput.close();
		p2->reset();
		BOOST_CHECK(*p2 !=*p0);
		std::ifstream binaryInput("pvp.bin");
		BOOST_CHECK(binaryInput.good());
		p2->binaryReadFromStream(binaryInput);
		binaryInput.close();
		BOOST_CHECK(*p2 == *p0);

		// Write the object out in text mode and load it back in. Then check similarity
		std::ofstream textOutput("pvp.txt");
		textOutput.precision(15); // This is normally done by the GDataExchange object
		p0->writeToStream(textOutput);
		textOutput.close();
		p2->reset();
		BOOST_CHECK(*p2 != *p0);
		std::ifstream textInput("pvp.txt");
		textInput.precision(15); // This is normally done by the GDataExchange object
		p2->readFromStream(textInput);
		textInput.close();
		BOOST_CHECK(p2->isSimilarTo(*p0, pow(10,-10)));
	}

	/***********************************************************************************/
	// Tests the GDataExchange functionality
	void gdataexchange_no_failure_expected() {
		boost::shared_ptr<GDataExchange> gde(new GDataExchange());
		BOOST_CHECK(!gde->dataIsAvailable());
		BOOST_CHECK(gde->nDataSets() == 1);

		// Fill with individual value items
		for(std::size_t gde_counter=0; gde_counter < NDATASETS; gde_counter++) {
			for(std::size_t i=0; i<NPARAMETERSETS; i++) gde->append<double>(gr.evenRandom(-10.,10.));
			for(std::size_t i=0; i<NPARAMETERSETS; i++) gde->append<boost::int32_t>(gr.discreteRandom(-10,10));
			for(std::size_t i=0; i<NPARAMETERSETS; i++) gde->append<bool>(gr.boolRandom());

			BOOST_CHECK(gde->numberOfParameterSets<double>() == NPARAMETERSETS);
			BOOST_CHECK(gde->numberOfParameterSets<boost::int32_t>() == NPARAMETERSETS);
			BOOST_CHECK(gde->numberOfParameterSets<bool>() == NPARAMETERSETS);

			if(gde_counter < NDATASETS-1) gde->newDataSet(); // Prevent an empty data set at the end
		}

		// GDataExchange fills itself with a single data set upon creation. Since we
		// added another 10 data sets, there should now be 11 of them.
		BOOST_CHECK(gde->nDataSets() == NDATASETS);
		BOOST_CHECK(gde->dataIsAvailable());

		gde->gotoStart();
		gde->resetAll(); // There should now only be one data set remaining
		BOOST_CHECK(gde->nDataSets() == 1);
		BOOST_CHECK(!gde->dataIsAvailable());

		// Fill with values including boundaries
		for(std::size_t gde_counter=0; gde_counter < 10; gde_counter++) {
			for(std::size_t i=0; i<NPARAMETERSETS; i++) gde->append<double>(gr.evenRandom(-10.,10.),-11.,11.);
			for(std::size_t i=0; i<NPARAMETERSETS; i++) gde->append<boost::int32_t>(gr.discreteRandom(-10,10),-11,11);
			for(std::size_t i=0; i<NPARAMETERSETS; i++) gde->append<bool>(gr.boolRandom(), false, true);

			BOOST_CHECK(gde->numberOfParameterSets<double>() == NPARAMETERSETS);
			BOOST_CHECK(gde->numberOfParameterSets<boost::int32_t>() == NPARAMETERSETS);
			BOOST_CHECK(gde->numberOfParameterSets<bool>() == NPARAMETERSETS);

			if(gde_counter < NDATASETS-1) gde->newDataSet(); // Prevent an empty data set at the end
		}
		BOOST_CHECK(gde->nDataSets() == NDATASETS);

		gde->gotoStart();
		gde->resetAll(); // There should now only be one data set remaining
		BOOST_CHECK(gde->nDataSets() == 1);

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

			BOOST_CHECK(gde->numberOfParameterSets<double>() == NPARAMETERSETS);
			BOOST_CHECK(gde->numberOfParameterSets<boost::int32_t>() == NPARAMETERSETS);
			BOOST_CHECK(gde->numberOfParameterSets<bool>() == NPARAMETERSETS);

			if(gde_counter < NDATASETS-1) gde->newDataSet(); // Otherwise we will have an empty data set at the end
		}
		BOOST_CHECK(gde->nDataSets() == NDATASETS);

		// Check that we can assign values to the data sets and iterate through them
		gde->gotoStart();
		do {
			BOOST_CHECK(!gde->hasValue()); // No value has been assigned so far
			double value = gr.evenRandom(0.,10.);
			gde->setValue(value);
			BOOST_CHECK(gde->hasValue()); // Value should have been set at this point
			BOOST_CHECK(value == gde->value());
		}
		while(gde->nextDataSet());

		// Switch to the best data set, with the lowest value being "best"
		gde->switchToBestDataSet(true);
		BOOST_CHECK(gde->nDataSets() == NDATASETS);

		// Check that the container is indeed sorted in ascending order
		double bestValue=-1.; // Note that we have initialized the values with random numbers in the range [0.,10.[
		double tmpBestValue = -1.;
		do {
			tmpBestValue = gde->value();
			BOOST_CHECK(bestValue <= tmpBestValue);
			bestValue = tmpBestValue;
		}
		while(gde->nextDataSet());

		// Switch to the best data set, with the highest value being "best"
		gde->switchToBestDataSet(false);
		BOOST_CHECK(gde->nDataSets() == NDATASETS);

		// Check that the container is indeed sorted in descending order
		bestValue=11.; // Note that we have initialized the values with random numbers in the range [0.,10.[
		do {
			tmpBestValue = gde->value();
			BOOST_CHECK(bestValue >= tmpBestValue);
			bestValue = tmpBestValue;
		}
		while(gde->nextDataSet());
		BOOST_CHECK(gde->nDataSets() == NDATASETS);
		BOOST_CHECK(gde->dataIsAvailable());

		// Test whether data can be written to file and read back in again.
		// In text mode
		gde->writeToFile("testFile.txt",false);
		boost::shared_ptr<GDataExchange> gde2(new GDataExchange()); // Create second, empty object
		gde2->readFromFile("testFile.txt",false);
		BOOST_CHECK(gde2->isSimilarTo(*gde,pow(10,-10)));

		// Put gde2 in pristine condition so we can start over with the binary mode
		gde2->resetAll();

		// In binary mode
		gde->writeToFile("testFile.bin",true);
		gde2->readFromFile("testFile.bin",true);
		BOOST_CHECK(*gde == *gde2);

		gde->resetAll(); // There should now only be one data set remaining
		BOOST_CHECK(gde->nDataSets() == 1);
	}

	/***********************************************************************************/
private:
	GRandom gr;
	const std::size_t NPARAMETERSETS;
	const std::size_t NDATASETS;
};

/********************************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GDataExchange classes and dependent classes.
class GDataExchangeSuite: public test_suite
{
public:
	GDataExchangeSuite() :test_suite("GDataExchangeSuite") {
	  // create an instance of the test cases class
	  boost::shared_ptr<GDataExchange_test> instance(new GDataExchange_test());

	  test_case* gnumericparametert_no_failure_expected_test_case = BOOST_CLASS_TEST_CASE(&GDataExchange_test::gnumericparametert_no_failure_expected, instance);
	  test_case* gparametervaluepair_no_failure_expected_test_case = BOOST_CLASS_TEST_CASE(&GDataExchange_test::gparametervaluepair_no_failure_expected, instance);
	  test_case* gdataexchange_no_failure_expected_test_case = BOOST_CLASS_TEST_CASE(&GDataExchange_test::gdataexchange_no_failure_expected, instance);

	  add(gnumericparametert_no_failure_expected_test_case);
	  add(gparametervaluepair_no_failure_expected_test_case);
	  add(gdataexchange_no_failure_expected_test_case);
	}
};

#endif /* GDATAEXCHANGE_TEST_HPP_ */
