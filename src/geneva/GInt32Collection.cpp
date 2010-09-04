/**
 * @file GInt32Collection.cpp
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
#include "geneva/GInt32Collection.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Geneva::GInt32Collection)


namespace Gem {
namespace Geneva {

/*******************************************************************************************/
/**
 * The default constructor
 */
GInt32Collection::GInt32Collection()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization with a number of random values in a given range
 *
 * @param nval The amount of random values
 * @param min The minimum random value
 * @param max The maximum random value
 */
GInt32Collection::GInt32Collection(const std::size_t& nval, const boost::int32_t& min, const boost::int32_t& max)
	: GNumCollectionT<boost::int32_t>(min, max)
{
	for(std::size_t i= 0; i<nval; i++) this->push_back(gr->uniform_int(min,max));
}

/*******************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GInt32Collection object
 */
GInt32Collection::GInt32Collection(const GInt32Collection& cp)
	: GNumCollectionT<boost::int32_t>(cp)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The destructor
 */
GInt32Collection::~GInt32Collection()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GInt32Collection object
 * @return A constant reference to this object
 */
const GInt32Collection& GInt32Collection::operator=(const GInt32Collection& cp){
	GInt32Collection::load_(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GInt32Collection::clone_() const {
	return new GInt32Collection(*this);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GInt32Collection object
 *
 * @param  cp A constant reference to another GInt32Collection object
 * @return A boolean indicating whether both objects are equal
 */
bool GInt32Collection::operator==(const GInt32Collection& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GInt32Collection::operator==","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GInt32Collection object
 *
 * @param  cp A constant reference to another GInt32Collection object
 * @return A boolean indicating whether both objects are inequal
 */
bool GInt32Collection::operator!=(const GInt32Collection& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GInt32Collection::operator!=","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Triggers random initialization of the parameter collection. Note that this
 * function assumes that the collection has been completely set up. Data
 * that is added later will remain unaffected.
 */
void GInt32Collection::randomInit_() {
	boost::int32_t lowerBoundary = getLowerInitBoundary();
	boost::int32_t upperBoundary = getUpperInitBoundary()+1;

	GInt32Collection::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		(*it)=gr->uniform_int(lowerBoundary, upperBoundary);
	}
}

/* ----------------------------------------------------------------------------------
 * Tested in GInt32Collection::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

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
boost::optional<std::string> GInt32Collection::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GInt32Collection>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GParameterCollectionT<boost::int32_t>::checkRelationshipWith(cp, e, limit, "GInt32Collection", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GInt32Collection", caller, deviations, e);
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GInt32Collection object, camouflaged as a GObject
 */
void GInt32Collection::load_(const GObject* cp){
    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GInt32Collection>(cp);

	// Load our parent class'es data ...
	GNumCollectionT<boost::int32_t>::load_(cp);

	// ... no local data
}

#ifdef GENEVATESTING
/*******************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GInt32Collection::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GNumCollectionT<boost::int32_t>::modify_GUnitTests()) result = true;

	return result;
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GInt32Collection::specificTestsNoFailureExpected_GUnitTests() {
	// A few general settings
	const std::size_t nItems = 100;
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
	GNumCollectionT<boost::int32_t>::specificTestsNoFailureExpected_GUnitTests();

	//------------------------------------------------------------------------------

	{ // Initialize with a fixed value, then check setting and retrieval of boundaries and random initialization
		boost::shared_ptr<GInt32Collection> p_test1 = this->GObject::clone<GInt32Collection>();
		boost::shared_ptr<GInt32Collection> p_test2 = this->GObject::clone<GInt32Collection>();

		// Make sure p_test1 and p_test2 are empty
		BOOST_CHECK_NO_THROW(p_test1->clear());
		BOOST_CHECK_NO_THROW(p_test2->clear());

		// Add a few items
		for(std::size_t i=0; i<nItems; i++) {
			p_test1->push_back(2*UPPERINITBOUNDARY); // Make sure random initialization cannot randomly leave the value unchanged
		}

		// Set initialization boundaries
		BOOST_CHECK_NO_THROW(p_test1->setInitBoundaries(LOWERINITBOUNDARY, UPPERINITBOUNDARY));

		// Check that the boundaries have been set as expected
		BOOST_CHECK(p_test1->getLowerInitBoundary() == LOWERINITBOUNDARY);
		BOOST_CHECK(p_test1->getUpperInitBoundary() == UPPERINITBOUNDARY);

		// Load the data of p_test1 into p_test2
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));
		// Cross check that both are indeed equal
		BOOST_CHECK(*p_test1 == *p_test2);

		// Randomly initialize one of the two objects. Note: we are using the protected function rather than the "global" function
		BOOST_CHECK_NO_THROW(p_test1->randomInit_());

		// Check that the object has indeed changed
		BOOST_CHECK(*p_test1 != *p_test2);

		// Check that the values of p_test1 are inside of the allowed boundaroes
		for(std::size_t i=0; i<nItems; i++) {
			BOOST_CHECK(p_test1->at(i) != p_test2->at(i));
			BOOST_CHECK(p_test1->at(i) >= LOWERINITBOUNDARY);
			BOOST_CHECK(p_test1->at(i) <= UPPERINITBOUNDARY);
		}
	}

	//------------------------------------------------------------------------------

	{ // Check that the fp-family of functions doesn't have an effect on this object
		boost::shared_ptr<GInt32Collection> p_test1 = this->GObject::clone<GInt32Collection>();
		boost::shared_ptr<GInt32Collection> p_test2 = this->GObject::clone<GInt32Collection>();
		boost::shared_ptr<GInt32Collection> p_test3 = this->GObject::clone<GInt32Collection>();

		// Add a few items to p_test1
		for(std::size_t i=0; i<nItems; i++) {
			p_test1->push_back(FIXEDVALUEINIT);
		}

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
void GInt32Collection::specificTestsFailuresExpected_GUnitTests() {
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
	GNumCollectionT<boost::int32_t>::specificTestsFailuresExpected_GUnitTests();

	// Remove the test adaptor
	this->resetAdaptor();

	// Load the old adaptor, if needed
	if(adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}
}

/*******************************************************************************************/
#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
