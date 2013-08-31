/**
 * @file GFloatObjectCollection.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
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
 * http://www.gemfony.eu .
 */
#include "geneva/GFloatObjectCollection.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GFloatObjectCollection)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GFloatObjectCollection::GFloatObjectCollection()
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with a number of identical GFloatObject objects
 */
GFloatObjectCollection::GFloatObjectCollection(
	const std::size_t& nCp
	, boost::shared_ptr<GFloatObject> tmpl_ptr
)
	:GParameterTCollectionT<GFloatObject>(nCp, tmpl_ptr)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GFloatObjectCollection object
 */
GFloatObjectCollection::GFloatObjectCollection(const GFloatObjectCollection& cp)
	: GParameterTCollectionT<GFloatObject>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GFloatObjectCollection::~GFloatObjectCollection()
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GFloatObjectCollection object
 * @return A constant reference to this object
 */
const GFloatObjectCollection& GFloatObjectCollection::operator=(const GFloatObjectCollection& cp){
	GFloatObjectCollection::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GFloatObjectCollection::clone_() const {
	return new GFloatObjectCollection(*this);
}

/******************************************************************************/
/**
 * Checks for equality with another GFloatObjectCollection object
 *
 * @param  cp A constant reference to another GFloatObjectCollection object
 * @return A boolean indicating whether both objects are equal
 */
bool GFloatObjectCollection::operator==(const GFloatObjectCollection& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GFloatObjectCollection::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GFloatObjectCollection object
 *
 * @param  cp A constant reference to another GFloatObjectCollection object
 * @return A boolean indicating whether both objects are inequal
 */
bool GFloatObjectCollection::operator!=(const GFloatObjectCollection& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GFloatObjectCollection::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GFloatObjectCollection::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GFloatObjectCollection>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GParameterTCollectionT<GFloatObject>::checkRelationshipWith(cp, e, limit, "GFloatObjectCollection", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GFloatObjectCollection", caller, deviations, e);
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GFloatObjectCollection::name() const {
   return std::string("GFloatObjectCollection");
}

/******************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GFloatObjectCollection object, camouflaged as a GObject
 */
void GFloatObjectCollection::load_(const GObject* cp){
    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GFloatObjectCollection>(cp);

	// Load our parent class'es data ...
	GParameterTCollectionT<GFloatObject>::load_(cp);

	// ... no local data
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GFloatObjectCollection::modify_GUnitTests() {
#ifdef GEM_TESTING
	this->fillWithObjects(10);

	// Call the parent class'es function
	GParameterTCollectionT<GFloatObject>::modify_GUnitTests();

	return true;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GFloatObjectCollection::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Fills the collection with GFloatObject objects
 */
void GFloatObjectCollection::fillWithObjects(const std::size_t& nAddedObjects) {
#ifdef GEM_TESTING
	// Clear the collection, so we can start fresh
	BOOST_CHECK_NO_THROW(this->clear());

	// Add GFloatObject items with adaptors to p_test1
	for(std::size_t i=0; i<nAddedObjects; i++) {
		// Create a suitable adaptor
		boost::shared_ptr<GFloatGaussAdaptor> gdga_ptr;

		BOOST_CHECK_NO_THROW(gdga_ptr = boost::shared_ptr<GFloatGaussAdaptor>(new GFloatGaussAdaptor(0.5, 0.8, 0., 2., 1.0)));
		BOOST_CHECK_NO_THROW(gdga_ptr->setAdaptionThreshold(0)); // Make sure the adaptor's internal parameters don't change through the adaption
		BOOST_CHECK_NO_THROW(gdga_ptr->setAdaptionMode(true)); // Always adapt

		// Create a suitable GFloatObject object
		boost::shared_ptr<GFloatObject> gdo_ptr;

		BOOST_CHECK_NO_THROW(gdo_ptr = boost::shared_ptr<GFloatObject>(new GFloatObject(-100., 100.))); // Initialization in the range -100, 100

		// Add the adaptor
		BOOST_CHECK_NO_THROW(gdo_ptr->addAdaptor(gdga_ptr));

		// Randomly initialize the GFloatObject object, so it is unique
		BOOST_CHECK_NO_THROW(gdo_ptr->randomInit());

		// Add the object to the collection
		BOOST_CHECK_NO_THROW(this->push_back(gdo_ptr));
	}

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GFloatObjectCollection::fillWithObjects", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GFloatObjectCollection::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Some settings
	const std::size_t nAddedObjects = 10;
	const std::size_t nTests = 100;
	const float LOWERINITBOUNDARY = -10.1;
	const float UPPERINITBOUNDARY =  10.1;
	const float FIXEDVALUEINIT = 1.;
	const float MULTVALUE = 3.;
	const float RANDLOWERBOUNDARY = 0.;
	const float RANDUPPERBOUNDARY = 10.;

	// --------------------------------------------------------------------------

	{ // Call the parent class'es function
		boost::shared_ptr<GFloatObjectCollection> p_test = this->clone<GFloatObjectCollection>();

		// Fill p_test with objects
		p_test->fillWithObjects(nAddedObjects);

		// Execute the parent class'es tests
		p_test->GParameterTCollectionT<GFloatObject>::specificTestsNoFailureExpected_GUnitTests();
	}

	// --------------------------------------------------------------------------

	{ // Test the GParameterTCollectionT<T>::adaptImpl() implementation
		boost::shared_ptr<GFloatObjectCollection> p_test1 = this->clone<GFloatObjectCollection>();
		boost::shared_ptr<GFloatObjectCollection> p_test2 = this->clone<GFloatObjectCollection>();

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

	{ // Test initialization of GFloat objects with a fixed floating point value
		boost::shared_ptr<GFloatObjectCollection> p_test1 = this->clone<GFloatObjectCollection>();

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
		boost::shared_ptr<GFloatObjectCollection> p_test1 = this->clone<GFloatObjectCollection>();

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
		boost::shared_ptr<GFloatObjectCollection> p_test1 = this->clone<GFloatObjectCollection>();
		boost::shared_ptr<GFloatObjectCollection> p_test2 = this->clone<GFloatObjectCollection>();

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
		boost::shared_ptr<GFloatObjectCollection> p_test1 = this->clone<GFloatObjectCollection>();
		boost::shared_ptr<GFloatObjectCollection> p_test2 = this->clone<GFloatObjectCollection>();

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
		boost::shared_ptr<GFloatObjectCollection> p_test1 = this->clone<GFloatObjectCollection>();
		boost::shared_ptr<GFloatObjectCollection> p_test2 = this->clone<GFloatObjectCollection>();

		// Fill p_test1 with objects
		BOOST_CHECK_NO_THROW(p_test1->fillWithObjects(nAddedObjects));

		// Make sure p_test2 is empty
		BOOST_CHECK_NO_THROW(p_test2->clear());

		// Load p_test1 into p_test2
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

		// Initialize p_test1 with a fixed value (1)
		BOOST_CHECK_NO_THROW(p_test1->fpFixedValueInit(float(1.)));
		// Initialize p_test2 with a fixed value (2)
		BOOST_CHECK_NO_THROW(p_test2->fpFixedValueInit(float(2.)));

		// Add p_test1 to p_test2
		BOOST_CHECK_NO_THROW(p_test2->fpAdd(p_test1));

		// Check each position of p_test2 individually
		for(std::size_t i=0; i<nAddedObjects; i++) {
			BOOST_CHECK(p_test2->at(i)->value() == float(2.) + float(1.));
		}
	}

	// --------------------------------------------------------------------------

	{ // Test subtraction of another object
		boost::shared_ptr<GFloatObjectCollection> p_test1 = this->clone<GFloatObjectCollection>();
		boost::shared_ptr<GFloatObjectCollection> p_test2 = this->clone<GFloatObjectCollection>();

		// Fill p_test1 with objects
		BOOST_CHECK_NO_THROW(p_test1->fillWithObjects(nAddedObjects));

		// Make sure p_test2 is empty
		BOOST_CHECK_NO_THROW(p_test2->clear());

		// Load p_test1 into p_test2
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

		// Initialize p_test1 with a fixed value (1)
		BOOST_CHECK_NO_THROW(p_test1->fpFixedValueInit(float(1.)));
		// Initialize p_test2 with a fixed value (2)
		BOOST_CHECK_NO_THROW(p_test2->fpFixedValueInit(float(2.)));

		// Subtract p_test1 from p_test2
		BOOST_CHECK_NO_THROW(p_test2->fpSubtract(p_test1));

		// Check each position of p_test2 individually
		for(std::size_t i=0; i<nAddedObjects; i++) {
			BOOST_CHECK(p_test2->at(i)->value() == float(2.) - float(1.));
		}
	}

	// --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GFloatObjectCollection::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GFloatObjectCollection::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Some settings
	const std::size_t nAddedObjects = 10;

	// Call the parent class'es function
	GParameterTCollectionT<GFloatObject>::specificTestsFailuresExpected_GUnitTests();

	// --------------------------------------------------------------------------

	{ // Test that fpAdd throws if an item of invalid size is added (Test of GParameterTCollectionT<T>::fpAdd() )
		boost::shared_ptr<GFloatObjectCollection> p_test1 = this->clone<GFloatObjectCollection>();
		boost::shared_ptr<GFloatObjectCollection> p_test2 = this->clone<GFloatObjectCollection>();

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
		boost::shared_ptr<GFloatObjectCollection> p_test1 = this->clone<GFloatObjectCollection>();
		boost::shared_ptr<GFloatObjectCollection> p_test2 = this->clone<GFloatObjectCollection>();

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

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GFloatObjectCollection::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
