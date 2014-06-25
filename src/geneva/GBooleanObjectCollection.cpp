/**
 * @file GBooleanObjectCollection.cpp
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

#include "geneva/GBooleanObjectCollection.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GBooleanObjectCollection)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GBooleanObjectCollection::GBooleanObjectCollection()
{ /* nothing */ }

// Tested in this file

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GBooleanObjectCollection object
 */
GBooleanObjectCollection::GBooleanObjectCollection(const GBooleanObjectCollection& cp)
	: GParameterTCollectionT<GBooleanObject>(cp)
{ /* nothing */ }

// Tested in this file

/******************************************************************************/
/**
 * Initialization with a number of identical GBooleanObject objects
 */
GBooleanObjectCollection::GBooleanObjectCollection(
	const std::size_t& nVals
	, boost::shared_ptr<GBooleanObject> tmpl_ptr
)
	:GParameterTCollectionT<GBooleanObject>(nVals, tmpl_ptr)
{ /* nothing */ }

// Tested in this file

/******************************************************************************/
/**
 * Initialization with a number of GBoolean objects with a given probability for the value "true"
 */
GBooleanObjectCollection::GBooleanObjectCollection(
		const std::size_t& nVals
		, const double& probability
) {
	for(std::size_t i=0; i<nVals; i++) {
		this->push_back(boost::shared_ptr<GBooleanObject>(new GBooleanObject(probability)));
	}
}

// Tested in this file

/******************************************************************************/
/**
 * The destructor
 */
GBooleanObjectCollection::~GBooleanObjectCollection()
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GBooleanObjectCollection object
 * @return A constant reference to this object
 */
const GBooleanObjectCollection& GBooleanObjectCollection::operator=(const GBooleanObjectCollection& cp){
	GBooleanObjectCollection::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GBooleanObjectCollection::clone_() const {
	return new GBooleanObjectCollection(*this);
}

/******************************************************************************/
/**
 * Checks for equality with another GBooleanObjectCollection object
 *
 * @param  cp A constant reference to another GBooleanObjectCollection object
 * @return A boolean indicating whether both objects are equal
 */
bool GBooleanObjectCollection::operator==(const GBooleanObjectCollection& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GBooleanObjectCollection::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GBooleanObjectCollection object
 *
 * @param  cp A constant reference to another GBooleanObjectCollection object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBooleanObjectCollection::operator!=(const GBooleanObjectCollection& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GBooleanObjectCollection::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GBooleanObjectCollection::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

	// Check for a possible self-assignment
    GObject::selfAssignmentCheck<GBooleanObjectCollection>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GParameterTCollectionT<GBooleanObject>::checkRelationshipWith(cp, e, limit, "GBooleanObjectCollection", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GBooleanObjectCollection", caller, deviations, e);
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GBooleanObjectCollection::name() const {
   return std::string("GBooleanObjectCollection");
}

/******************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GBooleanObjectCollection object, camouflaged as a GObject
 */
void GBooleanObjectCollection::load_(const GObject* cp){
	// Check for a possible self-assignment
    GObject::selfAssignmentCheck<GBooleanObjectCollection>(cp);

	// Load our parent class'es data ...
	GParameterTCollectionT<GBooleanObject>::load_(cp);

	// ... no local data
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBooleanObjectCollection::modify_GUnitTests() {
#ifdef GEM_TESTING
	this->fillWithObjects(10);

	// Call the parent class'es function
	GParameterTCollectionT<GBooleanObject>::modify_GUnitTests();

	return true;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBooleanObjectCollection::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Fills the collection with GBooleanObject objects
 */
void GBooleanObjectCollection::fillWithObjects(const std::size_t& nAddedObjects) {
#ifdef GEM_TESTING
	// Clear the collection, so we can start fresh
	BOOST_CHECK_NO_THROW(this->clear());

	// Add GBooleanObject items with adaptors to p_test1
	for(std::size_t i=0; i<nAddedObjects; i++) {
		// Create a suitable adaptor
		boost::shared_ptr<GBooleanAdaptor> gba_ptr;

		BOOST_CHECK_NO_THROW(gba_ptr = boost::shared_ptr<GBooleanAdaptor>(new GBooleanAdaptor(1.0)));
		BOOST_CHECK_NO_THROW(gba_ptr->setAdaptionThreshold(0)); // Make sure the adaptor's internal parameters don't change through the adaption
		BOOST_CHECK_NO_THROW(gba_ptr->setAdaptionMode(true)); // Always adapt

		// Create a suitable GBooleanObject object
		boost::shared_ptr<GBooleanObject> gbo_ptr;

		BOOST_CHECK_NO_THROW(gbo_ptr = boost::shared_ptr<GBooleanObject>(new GBooleanObject())); // Initialization with standard values

		// Add the adaptor
		BOOST_CHECK_NO_THROW(gbo_ptr->addAdaptor(gba_ptr));

		// Randomly initialize the GBooleanObject object, so it is unique
		BOOST_CHECK_NO_THROW(gbo_ptr->randomInit(ALLPARAMETERS));

		// Add the object to the collection
		BOOST_CHECK_NO_THROW(this->push_back(gbo_ptr));
	}

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBooleanObjectCollection::fillWithObjects", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBooleanObjectCollection::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Some settings
	const std::size_t nAddedObjects = 10;
	const std::size_t nTests = 10000;
	const double LOWERINITBOUNDARY = -10;
	const double UPPERINITBOUNDARY =  10;
	const double FIXEDVALUEINIT = 1.;
	const double MULTVALUE = 3.;
	const double RANDLOWERBOUNDARY = 0.;
	const double RANDUPPERBOUNDARY = 10.;
	const double LOWERBND = 0.8, UPPERBND = 1.2;

	//----------------------------------------------------------------------------

	{ // Call the parent class'es function
		boost::shared_ptr<GBooleanObjectCollection> p_test = this->clone<GBooleanObjectCollection>();

		// Fill p_test with objects
		p_test->fillWithObjects(nAddedObjects);

		// Call the parent's tests
		p_test->GParameterTCollectionT<GBooleanObject>::specificTestsNoFailureExpected_GUnitTests();
	}

	//----------------------------------------------------------------------------

	{ // Check default construction
		GBooleanObjectCollection gboc;
		BOOST_CHECK(gboc.empty());
	}

	//----------------------------------------------------------------------------

	{ // Check copy construction
		GBooleanObjectCollection gboc1;
		gboc1.push_back(boost::shared_ptr<GBooleanObject>(new GBooleanObject(0.5)));
		BOOST_CHECK(gboc1.size() == 1);
		GBooleanObjectCollection gboc2(gboc1);
		BOOST_CHECK(gboc1.size() == gboc2.size());
		BOOST_CHECK_MESSAGE(
				gboc1.at(0)->value() == gboc2.at(0)->value()
				, "\n"
				<< "gboc1.at(0)->value() = " << gboc1.at(0)->value()
				<< "gboc2.at(0)->value() = " << gboc2.at(0)->value()
		);
	}

	//----------------------------------------------------------------------------

	{ // Check construction with a number of object templates
		boost::shared_ptr<GBooleanObject> gbo_ptr(new GBooleanObject(Gem::Common::GDefaultValueT<bool>()));
		GBooleanObjectCollection gboc(nTests, gbo_ptr);

		BOOST_CHECK_MESSAGE(
				gboc.size() == nTests
				, "\n"
				<< "gboc.size() = " << gboc.size()
				<< "nTests = " << nTests
		);

		for(std::size_t i=0; i<nTests; i++) {
			BOOST_CHECK_MESSAGE(
					gboc.at(i)->value() == Gem::Common::GDefaultValueT<bool>()
					, "\n"
					<< "gboc.at(" << i << ")->value() = " << gboc.at(i)->value()
					<< "Gem::Common::GDefaultValueT<bool>() = " << Gem::Common::GDefaultValueT<bool>()
			);
		}
	}

	// --------------------------------------------------------------------------

	{ // Check construction with a number of GBooleanObject with a given probability for "true"
		GBooleanObjectCollection gboc(nTests, 0.5);

		std::size_t nTrue=0, nFalse=0;
		for(std::size_t i=0; i<nTests; i++) {
			gboc.at(i)->value()==true?nTrue++:nFalse++;
		}

		// We allow a slight deviation, as the initialization is a random process
		BOOST_REQUIRE(nFalse != 0); // There should be a few false values
		double ratio = double(nTrue)/double(nFalse);
		BOOST_CHECK_MESSAGE(
				ratio>LOWERBND && ratio<UPPERBND
				,  "\n"
				<< "ratio = " << ratio << "\n"
				<< "nTrue = " << nTrue << "\n"
				<< "nFalse = " << nFalse << "\n"
		);
	}

	// --------------------------------------------------------------------------

	{ // Test that the fp-family of functions has no effect on this object (and contained objects)
		boost::shared_ptr<GBooleanObjectCollection> p_test1 = this->clone<GBooleanObjectCollection>();
		boost::shared_ptr<GBooleanObjectCollection> p_test2 = this->clone<GBooleanObjectCollection>();

		// Fill p_test1 with objects
		BOOST_CHECK_NO_THROW(p_test1->fillWithObjects(nAddedObjects));

		// Make sure it has the expected size
		BOOST_CHECK(p_test1->size() == nAddedObjects);

		// Load the data into p_test2
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

		// Check that both items are identical
		BOOST_CHECK(*p_test1 == *p_test2);

		// Try to add a fixed fp value to p_test1 and check whether it has changed
		BOOST_CHECK_NO_THROW(p_test1->fpFixedValueInit(FIXEDVALUEINIT));
		BOOST_CHECK(*p_test1 == *p_test2);

		// Try to multiply p_test1 with a fixed fp value and check whether it has changed
		BOOST_CHECK_NO_THROW(p_test1->fpMultiplyBy(MULTVALUE));
		BOOST_CHECK(*p_test1 == *p_test2);

		// Try to multiply p_test1 with a random fp value in a given range and check whether it has changed
		BOOST_CHECK_NO_THROW(p_test1->fpMultiplyByRandom(RANDLOWERBOUNDARY, RANDUPPERBOUNDARY));
		BOOST_CHECK(*p_test1 == *p_test2);

		// Try to multiply p_test1 with a random fp value in the range [0,1[ and check whether it has changed
		BOOST_CHECK_NO_THROW(p_test1->fpMultiplyByRandom());
		BOOST_CHECK(*p_test1 == *p_test2);

		// Try to add p_test2 to p_test1 and see whether it has changed
		BOOST_CHECK_NO_THROW(p_test1->fpAdd(p_test2));
		BOOST_CHECK(*p_test1 == *p_test2);

		// Try to subtract p_test2 from p_test1 and see whether it has changed
		BOOST_CHECK_NO_THROW(p_test1->fpSubtract(p_test2));
		BOOST_CHECK(*p_test1 == *p_test2);
	}

	// --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBooleanObjectCollection::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBooleanObjectCollection::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GParameterTCollectionT<GBooleanObject>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBooleanObjectCollection::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
