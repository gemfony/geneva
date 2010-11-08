/**
 * @file GConstrainedInt32ObjectCollection.cpp
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
#include "geneva/GConstrainedInt32ObjectCollection.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Geneva::GConstrainedInt32ObjectCollection)


namespace Gem {
namespace Geneva {

/*******************************************************************************************/
/**
 * The default constructor
 */
GConstrainedInt32ObjectCollection::GConstrainedInt32ObjectCollection()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GConstrainedInt32ObjectCollection object
 */
GConstrainedInt32ObjectCollection::GConstrainedInt32ObjectCollection(const GConstrainedInt32ObjectCollection& cp)
	: GParameterTCollectionT<GConstrainedInt32Object>(cp)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The destructor
 */
GConstrainedInt32ObjectCollection::~GConstrainedInt32ObjectCollection()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GConstrainedInt32ObjectCollection object
 * @return A constant reference to this object
 */
const GConstrainedInt32ObjectCollection& GConstrainedInt32ObjectCollection::operator=(const GConstrainedInt32ObjectCollection& cp){
	GConstrainedInt32ObjectCollection::load_(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GConstrainedInt32ObjectCollection::clone_() const {
	return new GConstrainedInt32ObjectCollection(*this);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GConstrainedInt32ObjectCollection object
 *
 * @param  cp A constant reference to another GConstrainedInt32ObjectCollection object
 * @return A boolean indicating whether both objects are equal
 */
bool GConstrainedInt32ObjectCollection::operator==(const GConstrainedInt32ObjectCollection& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GConstrainedInt32ObjectCollection::operator==","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GConstrainedInt32ObjectCollection object
 *
 * @param  cp A constant reference to another GConstrainedInt32ObjectCollection object
 * @return A boolean indicating whether both objects are inequal
 */
bool GConstrainedInt32ObjectCollection::operator!=(const GConstrainedInt32ObjectCollection& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GConstrainedInt32ObjectCollection::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GConstrainedInt32ObjectCollection::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

	// Check for a possible self-assignment
    GObject::selfAssignmentCheck<GConstrainedInt32ObjectCollection>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GParameterTCollectionT<GConstrainedInt32Object>::checkRelationshipWith(cp, e, limit, "GConstrainedInt32ObjectCollection", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GConstrainedInt32ObjectCollection", caller, deviations, e);
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GConstrainedInt32ObjectCollection object, camouflaged as a GObject
 */
void GConstrainedInt32ObjectCollection::load_(const GObject* cp){
	// Check for a possible self-assignment
    GObject::selfAssignmentCheck<GConstrainedInt32ObjectCollection>(cp);

	// Load our parent class'es data ...
	GParameterTCollectionT<GConstrainedInt32Object>::load_(cp);

	// ... no local data
}

#ifdef GENEVATESTING
/*******************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GConstrainedInt32ObjectCollection::modify_GUnitTests() {
	this->fillWithObjects(10);

	// Call the parent class'es function
	GParameterTCollectionT<GConstrainedInt32Object>::modify_GUnitTests();

	return true;
}

/*******************************************************************************************/
/**
 * Fills the collection with GConstrainedInt32Object objects
 */
void GConstrainedInt32ObjectCollection::fillWithObjects(const std::size_t& nAddedObjects) {
	// Clear the collection, so we can start fresh
	BOOST_CHECK_NO_THROW(this->clear());

	// Add GConstrainedInt32Object items with adaptors to p_test1
	for(std::size_t i=0; i<nAddedObjects; i++) {
		// Create a suitable adaptor
		boost::shared_ptr<GInt32GaussAdaptor> giga_ptr;

		BOOST_CHECK_NO_THROW(giga_ptr = boost::shared_ptr<GInt32GaussAdaptor>(new GInt32GaussAdaptor(10, 0.8, 5, 10, 1.0)));
		BOOST_CHECK_NO_THROW(giga_ptr->setAdaptionThreshold(0)); // Make sure the adaptor's internal parameters don't change through the adaption
		BOOST_CHECK_NO_THROW(giga_ptr->setAdaptionMode(true)); // Always adapt

		// Create a suitable GConstrainedInt32Object object
		boost::shared_ptr<GConstrainedInt32Object> gcio_ptr;

		BOOST_CHECK_NO_THROW(gcio_ptr = boost::shared_ptr<GConstrainedInt32Object>(new GConstrainedInt32Object(-100, 100))); // Initialization in the range -100, 100

		// Add the adaptor
		BOOST_CHECK_NO_THROW(gcio_ptr->addAdaptor(giga_ptr));

		// Randomly initialize the GConstrainedInt32Object object, so it is unique
		BOOST_CHECK_NO_THROW(gcio_ptr->randomInit());

		// Add the object to the collection
		BOOST_CHECK_NO_THROW(this->push_back(gcio_ptr));
	}
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GConstrainedInt32ObjectCollection::specificTestsNoFailureExpected_GUnitTests() {
	// Some settings
	const std::size_t nAddedObjects = 10;
	const std::size_t nTests = 100;
	const double LOWERINITBOUNDARY = -10;
	const double UPPERINITBOUNDARY =  10;
	const double FIXEDVALUEINIT = 1.;
	const double MULTVALUE = 3.;
	const double RANDLOWERBOUNDARY = 0.;
	const double RANDUPPERBOUNDARY = 10.;

	//------------------------------------------------------------------------------

	{ // Call the parent class'es function
		boost::shared_ptr<GConstrainedInt32ObjectCollection> p_test = this->clone<GConstrainedInt32ObjectCollection>();

		// Fill p_test with objects
		p_test->fillWithObjects(nAddedObjects);

		// Execute the parent class'es tests
		p_test->GParameterTCollectionT<GConstrainedInt32Object>::specificTestsNoFailureExpected_GUnitTests();
	}

	//------------------------------------------------------------------------------

	{ // Test that the fp-family of functions has no effect on this object (and contained objects)
		boost::shared_ptr<GConstrainedInt32ObjectCollection> p_test1 = this->clone<GConstrainedInt32ObjectCollection>();
		boost::shared_ptr<GConstrainedInt32ObjectCollection> p_test2 = this->clone<GConstrainedInt32ObjectCollection>();

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

	//------------------------------------------------------------------------------
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GConstrainedInt32ObjectCollection::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GParameterTCollectionT<GConstrainedInt32Object>::specificTestsFailuresExpected_GUnitTests();
}

/*******************************************************************************************/

#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
