/**
 * @file GInt32Object.cpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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

#include "geneva/GInt32Object.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Geneva::GInt32Object)

namespace Gem {
namespace Geneva {

/*******************************************************************************************/
/**
 * The default constructor
 */
GInt32Object::GInt32Object()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GInt32Object object
 */
GInt32Object::GInt32Object(const GInt32Object& cp)
	: GNumT<boost::int32_t>(cp)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization by contained value
 *
 * @param val A value used for the initialization
 */
GInt32Object::GInt32Object(const boost::int32_t& val)
	: GNumT<boost::int32_t>(val)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization by random number in a given range
 *
 * @param lowerBoundary The lower boundary for the random number used in the initialization
 * @param upperBoundary The upper boundary for the random number used in the initialization
 */
GInt32Object::GInt32Object(const boost::int32_t& lowerBoundary, const boost::int32_t& upperBoundary)
	: GNumT<boost::int32_t>(lowerBoundary, upperBoundary)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The destructor
 */
GInt32Object::~GInt32Object()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * An assignment operator
 *
 * @param val The value to be assigned to this object
 * @return The value that was just assigned to this object
 */
boost::int32_t GInt32Object::operator=(const boost::int32_t& val) {
	return GNumT<boost::int32_t>::operator=(val);
}

/*******************************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GInt32Object object
 * @return A constant reference to this object
 */
const GInt32Object& GInt32Object::operator=(const GInt32Object& cp){
	GInt32Object::load_(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GInt32Object::clone_() const {
	return new GInt32Object(*this);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GInt32Object object
 *
 * @param  cp A constant reference to another GInt32Object object
 * @return A boolean indicating whether both objects are equal
 */
bool GInt32Object::operator==(const GInt32Object& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GInt32Object::operator==","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GInt32Object object
 *
 * @param  cp A constant reference to another GInt32Object object
 * @return A boolean indicating whether both objects are inequal
 */
bool GInt32Object::operator!=(const GInt32Object& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GInt32Object::operator!=","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks whether a given expectation for the relationship between this object and another object
 * is fulfilled.
 *
 * @param cp A constant reference to another object, camouflaged as a GObject
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 * @param caller An identifier for the calling entity
 * @param y_name An identifier for the object that should be compared to this one
 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
 */
boost::optional<std::string> GInt32Object::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GInt32Object>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GNumT<boost::int32_t>::checkRelationshipWith(cp, e, limit, "GInt32Object", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GInt32Object", caller, deviations, e);
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GInt32Object object, camouflaged as a GObject
 */
void GInt32Object::load_(const GObject* cp){
    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GInt32Object>(cp);

	// Load our parent class'es data ...
	GNumT<boost::int32_t>::load_(cp);

	// ... no local data
}

/*******************************************************************************************/
/**
 * Triggers random initialization of the parameter.
 */
void GInt32Object::randomInit_() {
	boost::int32_t lowerBoundary = getLowerInitBoundary();
	boost::int32_t upperBoundary = getUpperInitBoundary()+1;

	setValue(gr->uniform_int(lowerBoundary, upperBoundary));
}

/* ----------------------------------------------------------------------------------
 * Tested in GInt32Object::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

#ifdef GENEVATESTING
/*******************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GInt32Object::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GNumT<boost::int32_t>::modify_GUnitTests()) result = true;

	return result;
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GInt32Object::specificTestsNoFailureExpected_GUnitTests() {
	// A few settings
	const std::size_t nTests = 10000;
	const boost::int32_t LOWERINITBOUNDARY = -10;
	const boost::int32_t UPPERINITBOUNDARY =  10;
	const boost::int32_t FIXEDVALUEINIT = 1;

	// Make sure we have an appropriate adaptor loaded when performing these tests
	bool adaptorStored = false;
	boost::shared_ptr<GAdaptorT<boost::int32_t> > storedAdaptor;

	if(this->hasAdaptor()) {
		storedAdaptor = this->getAdaptor();
		adaptorStored = true;
	}

	boost::shared_ptr<GInt32GaussAdaptor> giga_ptr(new GInt32GaussAdaptor(0.5, 0.8, 0., 2., 1.0));
	giga_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
	giga_ptr->setAdaptionMode(true); // Always adapt
	this->addAdaptor(giga_ptr);

	// Call the parent class'es function
	GNumT<boost::int32_t>::specificTestsNoFailureExpected_GUnitTests();

	//------------------------------------------------------------------------------

	{ // Initialize with a fixed value, then check setting and retrieval of boundaries and random initialization
		boost::shared_ptr<GInt32Object> p_test1 = this->GObject::clone<GInt32Object>();
		boost::shared_ptr<GInt32Object> p_test2 = this->GObject::clone<GInt32Object>();

		// Assign a boolean value true
		BOOST_CHECK_NO_THROW(*p_test1 = 2*UPPERINITBOUNDARY); // Make sure random initialization cannot randomly result in an unchanged value
		// Cross-check
		BOOST_CHECK(p_test1->value() == 2*UPPERINITBOUNDARY);

		// Set initialization boundaries
		BOOST_CHECK_NO_THROW(p_test1->setInitBoundaries(LOWERINITBOUNDARY, UPPERINITBOUNDARY));

		// Check that the boundaries have been set as expected
		BOOST_CHECK(p_test1->getLowerInitBoundary() == LOWERINITBOUNDARY);
		BOOST_CHECK(p_test1->getUpperInitBoundary() == UPPERINITBOUNDARY);

		// Load the data of p_test1 into p_test2
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));
		// Cross check that both are indeed equal
		BOOST_CHECK(*p_test1 == *p_test2);

		// Check that the values of p_test1 are inside of the allowed boundaries
		for(std::size_t i=0; i<nTests; i++) {
			BOOST_CHECK_NO_THROW(p_test1->randomInit_());
			BOOST_CHECK(p_test1->value() >= LOWERINITBOUNDARY);
			BOOST_CHECK(p_test1->value() <= UPPERINITBOUNDARY);
			BOOST_CHECK(p_test1->value() != p_test2->value());
		}
	}

	//------------------------------------------------------------------------------

	{ // Check that the fp-family of functions doesn't have an effect on this object
		boost::shared_ptr<GInt32Object> p_test1 = this->GObject::clone<GInt32Object>();
		boost::shared_ptr<GInt32Object> p_test2 = this->GObject::clone<GInt32Object>();
		boost::shared_ptr<GInt32Object> p_test3 = this->GObject::clone<GInt32Object>();

		// Assign a boolean value true
		BOOST_CHECK_NO_THROW(*p_test1 = FIXEDVALUEINIT); // Make sure random initialization cannot randomly result in an unchanged value
		// Cross-check
		BOOST_CHECK(p_test1->value() == FIXEDVALUEINIT);

		// Load into p_test2 and p_test3 and test equality
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));
		BOOST_CHECK_NO_THROW(p_test3->load(p_test1));
		BOOST_CHECK(*p_test2 == *p_test1);
		BOOST_CHECK(*p_test3 == *p_test1);
		BOOST_CHECK(*p_test3 == *p_test2);

		// Check that initialization with a fixed floating point value has no effect on this object
		BOOST_CHECK_NO_THROW(p_test2->fpFixedValueInit(2.));
		BOOST_CHECK(*p_test2 == *p_test1);

		// Check that multiplication with a fixed floating point value has no effect on this object
		BOOST_CHECK_NO_THROW(p_test2->fpMultiplyBy(2.));
		BOOST_CHECK(*p_test2 == *p_test1);

		// Check that a component-wise multiplication with a random fp value in a given range does not have an effect on this object
		BOOST_CHECK_NO_THROW(p_test2->fpMultiplyByRandom(1., 2.));
		BOOST_CHECK(*p_test2 == *p_test1);

		// Check that a component-wise multiplication with a random fp value in the range [0:1[ does not have an effect on this object
		BOOST_CHECK_NO_THROW(p_test2->fpMultiplyByRandom());
		BOOST_CHECK(*p_test2 == *p_test1);

		// Check that adding p_test1 to p_test3 does not have an effect
		BOOST_CHECK_NO_THROW(p_test3->fpAdd(p_test1));
		BOOST_CHECK(*p_test3 == *p_test2);

		// Check that subtracting p_test1 from p_test3 does not have an effect
		BOOST_CHECK_NO_THROW(p_test3->fpSubtract(p_test1));
		BOOST_CHECK(*p_test3 == *p_test2);
	}

	//------------------------------------------------------------------------------


	// Remove the test adaptor
	this->resetAdaptor();

	// Load the old adaptor, if needed
	if(adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GInt32Object::specificTestsFailuresExpected_GUnitTests() {
	// Make sure we have an appropriate adaptor loaded when performing these tests
	bool adaptorStored = false;
	boost::shared_ptr<GAdaptorT<boost::int32_t> > storedAdaptor;

	if(this->hasAdaptor()) {
		storedAdaptor = this->getAdaptor();
		adaptorStored = true;
	}

	boost::shared_ptr<GInt32GaussAdaptor> giga_ptr(new GInt32GaussAdaptor(0.5, 0.8, 0., 2., 1.0));
	giga_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
	giga_ptr->setAdaptionMode(true); // Always adapt
	this->addAdaptor(giga_ptr);

	// Call the parent class'es function
	GNumT<boost::int32_t>::specificTestsFailuresExpected_GUnitTests();

	// Load the old adaptor, if needed
	if(adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}
}

/*******************************************************************************************/
#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
