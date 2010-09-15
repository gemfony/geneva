/**
 * @file GDoubleObjectCollection.cpp
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
#include "geneva/GDoubleObjectCollection.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Geneva::GDoubleObjectCollection)

namespace Gem {
namespace Geneva {

/*******************************************************************************************/
/**
 * The default constructor
 */
GDoubleObjectCollection::GDoubleObjectCollection()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GDoubleObjectCollection object
 */
GDoubleObjectCollection::GDoubleObjectCollection(const GDoubleObjectCollection& cp)
	: GParameterTCollectionT<GDoubleObject>(cp)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The destructor
 */
GDoubleObjectCollection::~GDoubleObjectCollection()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GDoubleObjectCollection object
 * @return A constant reference to this object
 */
const GDoubleObjectCollection& GDoubleObjectCollection::operator=(const GDoubleObjectCollection& cp){
	GDoubleObjectCollection::load_(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GDoubleObjectCollection::clone_() const {
	return new GDoubleObjectCollection(*this);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GDoubleObjectCollection object
 *
 * @param  cp A constant reference to another GDoubleObjectCollection object
 * @return A boolean indicating whether both objects are equal
 */
bool GDoubleObjectCollection::operator==(const GDoubleObjectCollection& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GDoubleObjectCollection::operator==","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GDoubleObjectCollection object
 *
 * @param  cp A constant reference to another GDoubleObjectCollection object
 * @return A boolean indicating whether both objects are inequal
 */
bool GDoubleObjectCollection::operator!=(const GDoubleObjectCollection& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GDoubleObjectCollection::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GDoubleObjectCollection::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GDoubleObjectCollection>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GParameterTCollectionT<GDoubleObject>::checkRelationshipWith(cp, e, limit, "GDoubleObjectCollection", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GDoubleObjectCollection", caller, deviations, e);
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GDoubleObjectCollection object, camouflaged as a GObject
 */
void GDoubleObjectCollection::load_(const GObject* cp){
    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GDoubleObjectCollection>(cp);

	// Load our parent class'es data ...
	GParameterTCollectionT<GDoubleObject>::load_(cp);

	// ... no local data
}

#ifdef GENEVATESTING
/*******************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GDoubleObjectCollection::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GParameterTCollectionT<GDoubleObject>::modify_GUnitTests()) result = true;

	return result;
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GDoubleObjectCollection::specificTestsNoFailureExpected_GUnitTests() {
	// Some settings
	const std::size_t nAddedObjects = 10;

	// Call the parent class'es function
	GParameterTCollectionT<GDoubleObject>::specificTestsNoFailureExpected_GUnitTests();

	//------------------------------------------------------------------------------

	{ // Test the GParameterTCollectionT<T>::adaptImpl() implementation
		boost::shared_ptr<GDoubleObjectCollection> p_test1 = this->clone<GDoubleObjectCollection>();
		boost::shared_ptr<GDoubleObjectCollection> p_test2 = this->clone<GDoubleObjectCollection>();

		// Clear p_test1, so we can start fresh
		BOOST_CHECK_NO_THROW(p_test1->clear());

		// Add GDoubleObject items with adaptors to p_test1
		for(std::size_t i=0; i<nAddedObjects; i++) {
			// Create a suitable adaptor
			boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(0.5, 0.8, 0., 2., 1.0));
			gdga_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
			gdga_ptr->setAdaptionMode(true); // Always adapt

			// Create a suitable GDoubleObject object
			boost::shared_ptr<GDoubleObject> gdo_ptr(new GDoubleObject(-100., 100.)); // Initialization in the range -100, 100

			// Add the adaptor
			gdo_ptr->addAdaptor(gdga_ptr);

			// Randomly initialize the GDoubleObject object, so it is unique
			gdo_ptr->randomInit();

			// Add the object to the collection
			p_test1->push_back(gdo_ptr);
		}

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

	//------------------------------------------------------------------------------
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GDoubleObjectCollection::specificTestsFailuresExpected_GUnitTests() {
	// Some settings
	const std::size_t nAddedObjects = 10;

	// Call the parent class'es function
	GParameterTCollectionT<GDoubleObject>::specificTestsFailuresExpected_GUnitTests();

	//------------------------------------------------------------------------------

	{ // Test that fpAdd throws if an item of invalid size is added (Test of GParameterTCollectionT<T>::fpAdd() )
		boost::shared_ptr<GDoubleObjectCollection> p_test1 = this->clone<GDoubleObjectCollection>();
		boost::shared_ptr<GDoubleObjectCollection> p_test2 = this->clone<GDoubleObjectCollection>();

		// Clear p_test1 and p_test2, so we can start fresh
		BOOST_CHECK_NO_THROW(p_test1->clear());
		BOOST_CHECK_NO_THROW(p_test2->clear());

		// Add GDoubleObject items with adaptors to p_test1
		for(std::size_t i=0; i<nAddedObjects; i++) {
			// Create a suitable adaptor
			boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(0.5, 0.8, 0., 2., 1.0));
			gdga_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
			gdga_ptr->setAdaptionMode(true); // Always adapt

			// Create a suitable GDoubleObject object
			boost::shared_ptr<GDoubleObject> gdo_ptr(new GDoubleObject(-100., 100.)); // Initialization in the range -100, 100

			// Add the adaptor
			gdo_ptr->addAdaptor(gdga_ptr);

			// Randomly initialize the GDoubleObject object, so it is unique
			gdo_ptr->randomInit();

			// Add the object to the collection
			p_test1->push_back(gdo_ptr);
		}

		// Check that both objects are in-equal
		BOOST_CHECK(*p_test1 != *p_test2);

		// Check that the sizes differ
		BOOST_CHECK(p_test1->size() != p_test2->size() && p_test2->size() == 0);

		// Adding p_test2 to p_test1 should throw
		BOOST_CHECK_THROW(p_test1->fpAdd(p_test1), Gem::Common::gemfony_error_condition);
	}

	//------------------------------------------------------------------------------

	{ // Test that fpSubtract throws if an item of invalid size is added (Test of GParameterTCollectionT<T>::fpSubtract() )
		boost::shared_ptr<GDoubleObjectCollection> p_test1 = this->clone<GDoubleObjectCollection>();
		boost::shared_ptr<GDoubleObjectCollection> p_test2 = this->clone<GDoubleObjectCollection>();

		// Clear p_test1 and p_test2, so we can start fresh
		BOOST_CHECK_NO_THROW(p_test1->clear());
		BOOST_CHECK_NO_THROW(p_test2->clear());

		// Add GDoubleObject items with adaptors to p_test1
		for(std::size_t i=0; i<nAddedObjects; i++) {
			// Create a suitable adaptor
			boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(0.5, 0.8, 0., 2., 1.0));
			gdga_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
			gdga_ptr->setAdaptionMode(true); // Always adapt

			// Create a suitable GDoubleObject object
			boost::shared_ptr<GDoubleObject> gdo_ptr(new GDoubleObject(-100., 100.)); // Initialization in the range -100, 100

			// Add the adaptor
			gdo_ptr->addAdaptor(gdga_ptr);

			// Randomly initialize the GDoubleObject object, so it is unique
			gdo_ptr->randomInit();

			// Add the object to the collection
			p_test1->push_back(gdo_ptr);
		}

		// Check that both objects are in-equal
		BOOST_CHECK(*p_test1 != *p_test2);

		// Check that the sizes differ
		BOOST_CHECK(p_test1->size() != p_test2->size() && p_test2->size() == 0);

		// Adding p_test2 to p_test1 should throw
		BOOST_CHECK_THROW(p_test1->fpSubtract(p_test1), Gem::Common::gemfony_error_condition);
	}

	//------------------------------------------------------------------------------
}

/*******************************************************************************************/
#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
