/**
 * @file GConstrainedDoubleObjectCollection.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include "geneva/GConstrainedDoubleObjectCollection.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GConstrainedDoubleObjectCollection)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GConstrainedDoubleObjectCollection::GConstrainedDoubleObjectCollection()
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with a number of identical GConstrainedDoubleObject objects
 */
GConstrainedDoubleObjectCollection::GConstrainedDoubleObjectCollection(
	const std::size_t& nCp
	, boost::shared_ptr<GConstrainedDoubleObject> tmpl_ptr
)
	:GParameterTCollectionT<GConstrainedDoubleObject>(nCp, tmpl_ptr)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GConstrainedDoubleObjectCollection object
 */
GConstrainedDoubleObjectCollection::GConstrainedDoubleObjectCollection(const GConstrainedDoubleObjectCollection& cp)
	: GParameterTCollectionT<GConstrainedDoubleObject>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GConstrainedDoubleObjectCollection::~GConstrainedDoubleObjectCollection()
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GConstrainedDoubleObjectCollection object
 * @return A constant reference to this object
 */
const GConstrainedDoubleObjectCollection& GConstrainedDoubleObjectCollection::operator=(const GConstrainedDoubleObjectCollection& cp){
	GConstrainedDoubleObjectCollection::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GConstrainedDoubleObjectCollection::clone_() const {
	return new GConstrainedDoubleObjectCollection(*this);
}

/******************************************************************************/
/**
 * Checks for equality with another GConstrainedDoubleObjectCollection object
 *
 * @param  cp A constant reference to another GConstrainedDoubleObjectCollection object
 * @return A boolean indicating whether both objects are equal
 */
bool GConstrainedDoubleObjectCollection::operator==(const GConstrainedDoubleObjectCollection& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GConstrainedDoubleObjectCollection::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GConstrainedDoubleObjectCollection object
 *
 * @param  cp A constant reference to another GConstrainedDoubleObjectCollection object
 * @return A boolean indicating whether both objects are inequal
 */
bool GConstrainedDoubleObjectCollection::operator!=(const GConstrainedDoubleObjectCollection& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GConstrainedDoubleObjectCollection::operator!=","cp", CE_SILENT);
}

/******************************************************************************/
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
boost::optional<std::string> GConstrainedDoubleObjectCollection::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

	// Check for a possible self-assignment
    GObject::selfAssignmentCheck<GConstrainedDoubleObjectCollection>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GParameterTCollectionT<GConstrainedDoubleObject>::checkRelationshipWith(cp, e, limit, "GConstrainedDoubleObjectCollection", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GConstrainedDoubleObjectCollection", caller, deviations, e);
}

/******************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GConstrainedDoubleObjectCollection object, camouflaged as a GObject
 */
void GConstrainedDoubleObjectCollection::load_(const GObject* cp){
	// Check for a possible self-assignment
    GObject::selfAssignmentCheck<GConstrainedDoubleObjectCollection>(cp);

	// Load our parent class'es data ...
	GParameterTCollectionT<GConstrainedDoubleObject>::load_(cp);

	// ... no local data
}

#ifdef GEM_TESTING

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GConstrainedDoubleObjectCollection::modify_GUnitTests() {
	this->fillWithObjects(10);

	// Call the parent class'es function
	GParameterTCollectionT<GConstrainedDoubleObject>::modify_GUnitTests();

	return true;
}

/******************************************************************************/
/**
 * Fills the collection with GConstrainedDoubleObject objects
 */
void GConstrainedDoubleObjectCollection::fillWithObjects(const std::size_t& nAddedObjects) {
	// Clear the collection, so we can start fresh
	BOOST_CHECK_NO_THROW(this->clear());

	// Add GConstrainedDoubleObject items with adaptors to p_test1
	for(std::size_t i=0; i<nAddedObjects; i++) {
		// Create a suitable adaptor
		boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr;

		BOOST_CHECK_NO_THROW(gdga_ptr = boost::shared_ptr<GDoubleGaussAdaptor>(new GDoubleGaussAdaptor(0.5, 0.8, 0., 2., 1.0)));
		BOOST_CHECK_NO_THROW(gdga_ptr->setAdaptionThreshold(0)); // Make sure the adaptor's internal parameters don't change through the adaption
		BOOST_CHECK_NO_THROW(gdga_ptr->setAdaptionMode(true)); // Always adapt

		// Create a suitable GConstrainedDoubleObject object
		boost::shared_ptr<GConstrainedDoubleObject> gcdo_ptr;

		BOOST_CHECK_NO_THROW(gcdo_ptr = boost::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject(-100., 100.))); // Boundaries in the range [-100., 100.[

		// Add the adaptor
		BOOST_CHECK_NO_THROW(gcdo_ptr->addAdaptor(gdga_ptr));

		// Randomly initialize the GConstrainedDoubleObject object, so it is unique
		BOOST_CHECK_NO_THROW(gcdo_ptr->randomInit());

		// Add the object to the collection
		BOOST_CHECK_NO_THROW(this->push_back(gcdo_ptr));
	}
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GConstrainedDoubleObjectCollection::specificTestsNoFailureExpected_GUnitTests() {
	// Some settings
	const std::size_t nAddedObjects = 10;
	const std::size_t nTests = 100;
	const double LOWERINITBOUNDARY = -10.1;
	const double UPPERINITBOUNDARY =  10.1;
	const double FIXEDVALUEINIT = 1.;
	const double MULTVALUE = 3.;
	const double RANDLOWERBOUNDARY = 0.;
	const double RANDUPPERBOUNDARY = 10.;

	// --------------------------------------------------------------------------

	{ // Call the parent class'es function
		boost::shared_ptr<GConstrainedDoubleObjectCollection> p_test = this->clone<GConstrainedDoubleObjectCollection>();

		// Fill p_test with objects
		p_test->fillWithObjects(nAddedObjects);

		// Execute the parent class'es tests
		p_test->GParameterTCollectionT<GConstrainedDoubleObject>::specificTestsNoFailureExpected_GUnitTests();
	}

	// --------------------------------------------------------------------------

	{ // Test the GParameterTCollectionT<T>::adaptImpl() implementation
		boost::shared_ptr<GConstrainedDoubleObjectCollection> p_test1 = this->clone<GConstrainedDoubleObjectCollection>();
		boost::shared_ptr<GConstrainedDoubleObjectCollection> p_test2 = this->clone<GConstrainedDoubleObjectCollection>();

		// Fill p_test1 with objects
		BOOST_CHECK_NO_THROW(p_test1->fillWithObjects(nAddedObjects));

	    // Load the p_test1 data into p_test2
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

		// Check that both objects are identical
		BOOST_CHECK(*p_test1 == *p_test2);

		// Modify p_test2 using its adaptImpl function
		BOOST_CHECK_NO_THROW(p_test2->adaptImpl());

		// Check that both objects differ
		// Check that both objects are identical
		BOOST_CHECK(*p_test1 != *p_test2);

		// All items in the collection must have been modified individually
		for(std::size_t i=0; i<nAddedObjects; i++) {
			BOOST_CHECK(*(p_test1->at(i)) != *(p_test2->at(i)));
		}
	}

	// --------------------------------------------------------------------------

	{ // Test initialization of GConstrainedDouble objects with a fixed floating point value
		boost::shared_ptr<GConstrainedDoubleObjectCollection> p_test1 = this->clone<GConstrainedDoubleObjectCollection>();

		// Fill p_test1 with objects
		BOOST_CHECK_NO_THROW(p_test1->fillWithObjects(nAddedObjects));

		// Cross check the amount of items in the collection
		BOOST_CHECK(p_test1->size() == nAddedObjects);

		// Initialize with a fixed value
		BOOST_CHECK_NO_THROW(p_test1->fpFixedValueInit(FIXEDVALUEINIT));

		// Check that all items have the expected value
		for(std::size_t i=0; i<nAddedObjects; i++) {
			BOOST_CHECK(p_test1->at(i)->value() == FIXEDVALUEINIT);
		}
	}

	// --------------------------------------------------------------------------

	{ // Test multiplication with a fixed value
		boost::shared_ptr<GConstrainedDoubleObjectCollection> p_test1 = this->clone<GConstrainedDoubleObjectCollection>();

		// Fill p_test1 with objects
		BOOST_CHECK_NO_THROW(p_test1->fillWithObjects(nAddedObjects));

		// Cross check the amount of items in the collection
		BOOST_CHECK(p_test1->size() == nAddedObjects);

		// Initialize with a fixed value (1), so we have a defined start value for the multiplication
		BOOST_CHECK_NO_THROW(p_test1->fpFixedValueInit(FIXEDVALUEINIT));

		// Multiply all items with a defined value
		BOOST_CHECK_NO_THROW(p_test1->fpMultiplyBy(MULTVALUE));

		// Check the values of all items
		for(std::size_t i=0; i<nAddedObjects; i++) {
			BOOST_CHECK(p_test1->at(i)->value() == MULTVALUE);
		}
	}

	// --------------------------------------------------------------------------

	{ // Test multiplication with a random number in a given range
		boost::shared_ptr<GConstrainedDoubleObjectCollection> p_test1 = this->clone<GConstrainedDoubleObjectCollection>();
		boost::shared_ptr<GConstrainedDoubleObjectCollection> p_test2 = this->clone<GConstrainedDoubleObjectCollection>();

		// Fill p_test1 with objects
		BOOST_CHECK_NO_THROW(p_test1->fillWithObjects(nAddedObjects));

		// Make sure p_test2 is empty
		BOOST_CHECK_NO_THROW(p_test2->clear());

		// Cross check the amount of items in the collection
		BOOST_CHECK(p_test1->size() == nAddedObjects);

		// Initialize with a fixed value (1), so we have a defined start value for the multiplication
		BOOST_CHECK_NO_THROW(p_test1->fpFixedValueInit(FIXEDVALUEINIT));

		// Load p_test1 into p_test2
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

		// Make sure both objects are the same
		BOOST_CHECK(*p_test1 == *p_test2);

		// Multiply p_test1 with a random value
		BOOST_CHECK_NO_THROW(p_test1->fpMultiplyByRandom(LOWERINITBOUNDARY, UPPERINITBOUNDARY));

		// Check that p_test1 and p_test2 differ
		BOOST_CHECK(*p_test1 != *p_test2);

		// Check that each item individually differs
		for(std::size_t i=0; i<nAddedObjects; i++) {
			BOOST_CHECK(p_test1->at(i)->value() != p_test2->at(i)->value());
		}
	}

	// --------------------------------------------------------------------------

	{ // Test multiplication with a random number in a the range [0,1[
		boost::shared_ptr<GConstrainedDoubleObjectCollection> p_test1 = this->clone<GConstrainedDoubleObjectCollection>();
		boost::shared_ptr<GConstrainedDoubleObjectCollection> p_test2 = this->clone<GConstrainedDoubleObjectCollection>();

		// Fill p_test1 with objects
		BOOST_CHECK_NO_THROW(p_test1->fillWithObjects(nAddedObjects));

		// Make sure p_test2 is empty
		BOOST_CHECK_NO_THROW(p_test2->clear());

		// Cross check the amount of items in the collection
		BOOST_CHECK(p_test1->size() == nAddedObjects);

		// Initialize with a fixed value (1), so we have a defined start value for the multiplication
		BOOST_CHECK_NO_THROW(p_test1->fpFixedValueInit(FIXEDVALUEINIT));

		// Load p_test1 into p_test2
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

		// Make sure both objects are the same
		BOOST_CHECK(*p_test1 == *p_test2);

		// Multiply p_test1 with a random value
		BOOST_CHECK_NO_THROW(p_test1->fpMultiplyByRandom());

		// Check that p_test1 and p_test2 differ
		BOOST_CHECK(*p_test1 != *p_test2);

		// Check that each item individually differs
		for(std::size_t i=0; i<nAddedObjects; i++) {
			BOOST_CHECK(p_test1->at(i)->value() != p_test2->at(i)->value());
		}
	}

	// --------------------------------------------------------------------------

	{ // Test addition of another object
		boost::shared_ptr<GConstrainedDoubleObjectCollection> p_test1 = this->clone<GConstrainedDoubleObjectCollection>();
		boost::shared_ptr<GConstrainedDoubleObjectCollection> p_test2 = this->clone<GConstrainedDoubleObjectCollection>();

		// Fill p_test1 with objects
		BOOST_CHECK_NO_THROW(p_test1->fillWithObjects(nAddedObjects));

		// Make sure p_test2 is empty
		BOOST_CHECK_NO_THROW(p_test2->clear());

		// Load p_test1 into p_test2
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

		// Initialize p_test1 with a fixed value (1)
		BOOST_CHECK_NO_THROW(p_test1->fpFixedValueInit(double(1.)));
		// Initialize p_test2 with a fixed value (2)
		BOOST_CHECK_NO_THROW(p_test2->fpFixedValueInit(double(2.)));

		// Add p_test1 to p_test2
		BOOST_CHECK_NO_THROW(p_test2->fpAdd(p_test1));

		// Check each position of p_test2 individually
		for(std::size_t i=0; i<nAddedObjects; i++) {
			BOOST_CHECK(p_test2->at(i)->value() == double(2.) + double(1.));
		}
	}

	// --------------------------------------------------------------------------

	{ // Test subtraction of another object
		boost::shared_ptr<GConstrainedDoubleObjectCollection> p_test1 = this->clone<GConstrainedDoubleObjectCollection>();
		boost::shared_ptr<GConstrainedDoubleObjectCollection> p_test2 = this->clone<GConstrainedDoubleObjectCollection>();

		// Fill p_test1 with objects
		BOOST_CHECK_NO_THROW(p_test1->fillWithObjects(nAddedObjects));

		// Make sure p_test2 is empty
		BOOST_CHECK_NO_THROW(p_test2->clear());

		// Load p_test1 into p_test2
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

		// Initialize p_test1 with a fixed value (1)
		BOOST_CHECK_NO_THROW(p_test1->fpFixedValueInit(double(1.)));
		// Initialize p_test2 with a fixed value (2)
		BOOST_CHECK_NO_THROW(p_test2->fpFixedValueInit(double(2.)));

		// Subtract p_test1 from p_test2
		BOOST_CHECK_NO_THROW(p_test2->fpSubtract(p_test1));

		// Check each position of p_test2 individually
		for(std::size_t i=0; i<nAddedObjects; i++) {
			BOOST_CHECK(p_test2->at(i)->value() == double(2.) - double(1.));
		}
	}

	// --------------------------------------------------------------------------
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GConstrainedDoubleObjectCollection::specificTestsFailuresExpected_GUnitTests() {
	// Some settings
	const std::size_t nAddedObjects = 10;

	// Call the parent class'es function
	GParameterTCollectionT<GConstrainedDoubleObject>::specificTestsFailuresExpected_GUnitTests();

	// --------------------------------------------------------------------------

	{ // Test that fpAdd throws if an item of invalid size is added (Test of GParameterTCollectionT<T>::fpAdd() )
		boost::shared_ptr<GConstrainedDoubleObjectCollection> p_test1 = this->clone<GConstrainedDoubleObjectCollection>();
		boost::shared_ptr<GConstrainedDoubleObjectCollection> p_test2 = this->clone<GConstrainedDoubleObjectCollection>();

		// Fill p_test1 with objects
		BOOST_CHECK_NO_THROW(p_test1->fillWithObjects(nAddedObjects));

		// Clear p_test2, so we are sure it is empty
		BOOST_CHECK_NO_THROW(p_test2->clear());

		// Check that both objects are in-equal
		BOOST_CHECK(*p_test1 != *p_test2);

		// Check that the sizes differ
		BOOST_CHECK(p_test1->size() != p_test2->size() && p_test2->size() == 0);

		// Adding p_test2 to p_test1 should throw
		BOOST_CHECK_THROW(p_test1->fpAdd(p_test2), Gem::Common::gemfony_error_condition);
	}

	// --------------------------------------------------------------------------

	{ // Test that fpSubtract throws if an item of invalid size is added (Test of GParameterTCollectionT<T>::fpSubtract() )
		boost::shared_ptr<GConstrainedDoubleObjectCollection> p_test1 = this->clone<GConstrainedDoubleObjectCollection>();
		boost::shared_ptr<GConstrainedDoubleObjectCollection> p_test2 = this->clone<GConstrainedDoubleObjectCollection>();

		// Fill p_test1 with objects
		BOOST_CHECK_NO_THROW(p_test1->fillWithObjects(nAddedObjects));

		// Clear p_test2, so we are sure it is empty
		BOOST_CHECK_NO_THROW(p_test2->clear());

		// Check that both objects are in-equal
		BOOST_CHECK(*p_test1 != *p_test2);

		// Check that the sizes differ
		BOOST_CHECK(p_test1->size() != p_test2->size() && p_test2->size() == 0);

		// Subtracting p_test2 from p_test1 should throw
		BOOST_CHECK_THROW(p_test1->fpSubtract(p_test2), Gem::Common::gemfony_error_condition);
	}

	// --------------------------------------------------------------------------
}

/******************************************************************************/

#endif /* GEM_TESTING */

} /* namespace Geneva */
} /* namespace Gem */
