/**
 * @file GParameterObjectCollection.cpp
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
#include "geneva/GParameterObjectCollection.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GParameterObjectCollection)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GParameterObjectCollection::GParameterObjectCollection()
	: GParameterTCollectionT<GParameterBase>()
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with a number of identical GParameterBase objects
 */
GParameterObjectCollection::GParameterObjectCollection(
	const std::size_t &nCp, std::shared_ptr <GParameterBase> tmpl_ptr
)
	: GParameterTCollectionT<GParameterBase>(nCp, tmpl_ptr)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GParameterObjectCollection object
 */
GParameterObjectCollection::GParameterObjectCollection(const GParameterObjectCollection &cp)
	: GParameterTCollectionT<GParameterBase>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GParameterObjectCollection::~GParameterObjectCollection()
{ /* nothing */ }

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject *GParameterObjectCollection::clone_() const {
	return new GParameterObjectCollection(*this);
}

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GParameterObjectCollection &GParameterObjectCollection::operator=(const GParameterObjectCollection &cp) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GParameterObjectCollection object
 *
 * @param  cp A constant reference to another GParameterObjectCollection object
 * @return A boolean indicating whether both objects are equal
 */
bool GParameterObjectCollection::operator==(const GParameterObjectCollection &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Checks for inequality with another GParameterObjectCollection object
 *
 * @param  cp A constant reference to another GParameterObjectCollection object
 * @return A boolean indicating whether both objects are inequal
 */
bool GParameterObjectCollection::operator!=(const GParameterObjectCollection &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GParameterObjectCollection::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GParameterObjectCollection reference independent of this object and convert the pointer
	const GParameterObjectCollection *p_load = Gem::Common::g_convert_and_compare<GObject, GParameterObjectCollection>(cp, this);

	GToken token("GParameterObjectCollection", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GParameterTCollectionT<GParameterBase>>(IDENTITY(*this, *p_load), token);

	// ... no local data

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GParameterObjectCollection::name() const {
	return std::string("GParameterObjectCollection");
}

/******************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GParameterObjectCollection object, camouflaged as a GObject
 */
void GParameterObjectCollection::load_(const GObject *cp) {
	// Convert the pointer to our target type and check for self-assignment
	const GParameterObjectCollection * p_load = Gem::Common::g_convert_and_compare<GObject, GParameterObjectCollection>(cp, this);

	// Load our parent class'es data ...
	GParameterTCollectionT<GParameterBase>::load_(cp);

	// ... no local data
}

/******************************************************************************/
/**
 * Prevent shadowing of std::vector<GParameterBase>::at()
 *
 * @param pos The position for which an item should be returned
 * @return The item at position pos
 */
std::shared_ptr <Gem::Geneva::GParameterBase> GParameterObjectCollection::at(const std::size_t &pos) {
	return data.at(pos);
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GParameterObjectCollection::modify_GUnitTests() {
#ifdef GEM_TESTING
	this->fillWithObjects();

	// Call the parent class'es function
	GParameterTCollectionT<GParameterBase>::modify_GUnitTests();

	return true;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GParameterObjectCollection::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Fills the collection with GParameterBase derivatives
 */
void GParameterObjectCollection::fillWithObjects() {
#ifdef GEM_TESTING
	// A random generator
	Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

	// Clear the collection, so we can start fresh
	BOOST_CHECK_NO_THROW(this->clear());

	// Add a GBooleanObject object
	// Create a suitable adaptor
	std::shared_ptr <GBooleanAdaptor> gba_ptr;
	BOOST_CHECK_NO_THROW(gba_ptr = std::shared_ptr<GBooleanAdaptor>(new GBooleanAdaptor(1.0)));
	BOOST_CHECK_NO_THROW(gba_ptr->setAdaptionThreshold(
		0)); // Make sure the adaptor's internal parameters don't change through the adaption
	BOOST_CHECK_NO_THROW(gba_ptr->setAdaptionMode(true)); // Always adapt

	// Create a suitable GBooleanObject object
	std::shared_ptr <GBooleanObject> gbo_ptr;
	BOOST_CHECK_NO_THROW(
		gbo_ptr = std::shared_ptr<GBooleanObject>(new GBooleanObject())); // Initialization with standard values

	// Add the adaptor
	BOOST_CHECK_NO_THROW(gbo_ptr->addAdaptor(gba_ptr));

	// Randomly initialize the GBooleanObject object, so it is unique
	BOOST_CHECK_NO_THROW(gbo_ptr->randomInit(activityMode::ALLPARAMETERS, gr));

	// Add the object to the collection
	BOOST_CHECK_NO_THROW(this->push_back(gbo_ptr));

	// Add a GInt32 object
	// Create a suitable adaptor
	std::shared_ptr <GInt32GaussAdaptor> giga_ptr;
	BOOST_CHECK_NO_THROW(giga_ptr = std::shared_ptr<GInt32GaussAdaptor>(new GInt32GaussAdaptor(0.025, 0.1, 0, 1, 1.0)));
	BOOST_CHECK_NO_THROW(giga_ptr->setAdaptionThreshold(
		0)); // Make sure the adaptor's internal parameters don't change through the adaption
	BOOST_CHECK_NO_THROW(giga_ptr->setAdaptionMode(true)); // Always adapt

	// Create a suitable GInt32Object object
	std::shared_ptr <GInt32Object> gio_ptr;
	BOOST_CHECK_NO_THROW(
		gio_ptr = std::shared_ptr<GInt32Object>(new GInt32Object(-100, 100))); // Initialization in the range -100, 100

	// Add the adaptor
	BOOST_CHECK_NO_THROW(gio_ptr->addAdaptor(giga_ptr));

	// Randomly initialize the GInt32Object object, so it is unique
	BOOST_CHECK_NO_THROW(gio_ptr->randomInit(activityMode::ALLPARAMETERS, gr));

	// Add the object to the collection
	BOOST_CHECK_NO_THROW(this->push_back(gio_ptr));

	// Add a GDouble object
	// Create a suitable adaptor
	std::shared_ptr <GDoubleGaussAdaptor> gdga_ptr;
	BOOST_CHECK_NO_THROW(
		gdga_ptr = std::shared_ptr<GDoubleGaussAdaptor>(new GDoubleGaussAdaptor(0.025, 0.1, 0, 1, 1.0)));
	BOOST_CHECK_NO_THROW(gdga_ptr->setAdaptionThreshold(
		0)); // Make sure the adaptor's internal parameters don't change through the adaption
	BOOST_CHECK_NO_THROW(gdga_ptr->setAdaptionMode(true)); // Always adapt

	// Create a suitable GDoubleObject object
	std::shared_ptr <GDoubleObject> gdo_ptr;
	BOOST_CHECK_NO_THROW(gdo_ptr = std::shared_ptr<GDoubleObject>(
		new GDoubleObject(-100., 100.))); // Initialization in the range -100, 100

	// Add the adaptor
	BOOST_CHECK_NO_THROW(gdo_ptr->addAdaptor(gdga_ptr));

	// Randomly initialize the GDoubleObject object, so it is unique
	BOOST_CHECK_NO_THROW(gdo_ptr->randomInit(activityMode::ALLPARAMETERS, gr));

	// Add the object to the collection
	BOOST_CHECK_NO_THROW(this->push_back(gdo_ptr));

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GParameterObjectCollection::fillWithObjects", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GParameterObjectCollection::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Some settings
	const double LOWERINITBOUNDARY = -10;
	const double UPPERINITBOUNDARY = 10;
	const double FIXEDVALUEINIT = 1.;
	const double MULTVALUE = 3.;
	const double RANDLOWERBOUNDARY = 2.;
	const double RANDUPPERBOUNDARY = 10.;

	// A random generator
	Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

	//------------------------------------------------------------------------------

	{ // Call the parent class'es function
		std::shared_ptr <GParameterObjectCollection> p_test = this->clone<GParameterObjectCollection>();

		// Fill p_test with parameters
		p_test->fillWithObjects();

		// Run the parent class'es tests
		p_test->GParameterTCollectionT<GParameterBase>::specificTestsNoFailureExpected_GUnitTests();
	}

	//------------------------------------------------------------------------------

	{ // Test that the fpFixedValueInit() function only has an effect on fp parameters
		std::shared_ptr <GParameterObjectCollection> p_test1 = this->clone<GParameterObjectCollection>();
		std::shared_ptr <GParameterObjectCollection> p_test2 = this->clone<GParameterObjectCollection>();

		// Fill p_test1 with parameters
		BOOST_CHECK_NO_THROW(p_test1->fillWithObjects());

		// Load the data intp p_test2
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

		// Check that both individuals are identical
		BOOST_CHECK(*p_test1 == *p_test2);

		// Test initialization of p_test2 with a fixed value
		BOOST_CHECK_NO_THROW(p_test2->fixedValueInit<double>(FIXEDVALUEINIT, activityMode::ALLPARAMETERS));

		// The first two parameters should be unchanged
		BOOST_CHECK(*(p_test1->at(0)) == *(p_test2->at(0)));
		BOOST_CHECK(*(p_test1->at(1)) == *(p_test2->at(1)));

		// Extract the fp parameters
		std::shared_ptr <GDoubleObject> gdo_ptr1, gdo_ptr2;
		BOOST_CHECK_NO_THROW(gdo_ptr1 = p_test1->at<GDoubleObject>(2));
		BOOST_CHECK_NO_THROW(gdo_ptr2 = p_test2->at<GDoubleObject>(2));

		// Check that the value is changed
		BOOST_CHECK(gdo_ptr1->value() != gdo_ptr2->value());

		// Check that the desired value has been assigned
		BOOST_CHECK(gdo_ptr2->value() == FIXEDVALUEINIT);
	}

	//------------------------------------------------------------------------------

	{ // Test that the fpMultiplyBy() function only has an effect on fp parameters
		std::shared_ptr <GParameterObjectCollection> p_test1 = this->clone<GParameterObjectCollection>();
		std::shared_ptr <GParameterObjectCollection> p_test2 = this->clone<GParameterObjectCollection>();

		// Fill p_test1 with parameters
		BOOST_CHECK_NO_THROW(p_test1->fillWithObjects());

		// Load the data into p_test2
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

		// Check that both individuals are identical
		BOOST_CHECK(*p_test1 == *p_test2);

		// Initialize p_test2 with a fixed value
		BOOST_CHECK_NO_THROW(p_test2->fixedValueInit<double>(FIXEDVALUEINIT, activityMode::ALLPARAMETERS));

		// The first two parameters should be unchanged
		BOOST_CHECK(*(p_test1->at(0)) == *(p_test2->at(0)));
		BOOST_CHECK(*(p_test1->at(1)) == *(p_test2->at(1)));

		// Extract the fp parameters
		std::shared_ptr <GDoubleObject> gdo_ptr1, gdo_ptr2;
		BOOST_CHECK_NO_THROW(gdo_ptr1 = p_test1->at<GDoubleObject>(2));
		BOOST_CHECK_NO_THROW(gdo_ptr2 = p_test2->at<GDoubleObject>(2));

		// Check that the value is changed
		BOOST_CHECK(gdo_ptr1->value() != gdo_ptr2->value());

		// Check that the desired value has been assigned
		BOOST_CHECK(gdo_ptr2->value() == FIXEDVALUEINIT);

		// Multiply with a fixed value
		BOOST_CHECK_NO_THROW(p_test2->multiplyBy<double>(MULTVALUE, activityMode::ALLPARAMETERS));

		// The first two parameters should again be unchanged
		BOOST_CHECK(*(p_test1->at(0)) == *(p_test2->at(0)));
		BOOST_CHECK(*(p_test1->at(1)) == *(p_test2->at(1)));

		// The fp value should have changed
		BOOST_CHECK_MESSAGE(
			gdo_ptr2->value() == FIXEDVALUEINIT * MULTVALUE,
			"gdo_ptr2->value() = " << gdo_ptr2->value() << ", FIXEDVALUEINIT*MULTVALUE = " << FIXEDVALUEINIT * MULTVALUE
		);
	}

	//------------------------------------------------------------------------------

	{ // Test that the fpMultiplyByRandom(min,max) function only has an effect on fp parameters
		std::shared_ptr <GParameterObjectCollection> p_test1 = this->clone<GParameterObjectCollection>();
		std::shared_ptr <GParameterObjectCollection> p_test2 = this->clone<GParameterObjectCollection>();

		// Fill p_test1 with parameters
		BOOST_CHECK_NO_THROW(p_test1->fillWithObjects());

		// Initialize p_test1 with a fixed value
		BOOST_CHECK_NO_THROW(p_test1->fixedValueInit<double>(FIXEDVALUEINIT, activityMode::ALLPARAMETERS));

		// Load the data intp p_test2
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

		// Check that both individuals are identical
		BOOST_CHECK(*p_test1 == *p_test2);

		// Multiply p_test2 with a random number in a given range
		BOOST_CHECK_NO_THROW(p_test2->multiplyByRandom<double>(RANDLOWERBOUNDARY, RANDUPPERBOUNDARY, activityMode::ALLPARAMETERS));

		// The first two parameters should again be unchanged
		BOOST_CHECK(*(p_test1->at(0)) == *(p_test2->at(0)));
		BOOST_CHECK(*(p_test1->at(1)) == *(p_test2->at(1)));

		// Extract the fp parameters
		std::shared_ptr <GDoubleObject> gdo_ptr1, gdo_ptr2;
		BOOST_CHECK_NO_THROW(gdo_ptr1 = p_test1->at<GDoubleObject>(2));
		BOOST_CHECK_NO_THROW(gdo_ptr2 = p_test2->at<GDoubleObject>(2));

		// The fp value should have changed
		BOOST_CHECK(gdo_ptr2->value() != gdo_ptr1->value());
	}

	//------------------------------------------------------------------------------

	{ // Test that the fpMultiplyByRandom() function only has an effect on fp parameters
		std::shared_ptr <GParameterObjectCollection> p_test1 = this->clone<GParameterObjectCollection>();
		std::shared_ptr <GParameterObjectCollection> p_test2 = this->clone<GParameterObjectCollection>();

		// Fill p_test1 with parameters
		BOOST_CHECK_NO_THROW(p_test1->fillWithObjects());

		// Initialize p_test1 with a fixed value
		BOOST_CHECK_NO_THROW(p_test1->fixedValueInit<double>(FIXEDVALUEINIT, activityMode::ALLPARAMETERS));

		// Load the data intp p_test2
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

		// Check that both individuals are identical
		BOOST_CHECK(*p_test1 == *p_test2);

		// Multiply p_test2 with a random number in the range [0,1[
		BOOST_CHECK_NO_THROW(p_test2->multiplyByRandom<double>(activityMode::ALLPARAMETERS));

		// The first two parameters should again be unchanged
		BOOST_CHECK(*(p_test1->at(0)) == *(p_test2->at(0)));
		BOOST_CHECK(*(p_test1->at(1)) == *(p_test2->at(1)));

		// Extract the fp parameters
		std::shared_ptr <GDoubleObject> gdo_ptr1, gdo_ptr2;
		BOOST_CHECK_NO_THROW(gdo_ptr1 = p_test1->at<GDoubleObject>(2));
		BOOST_CHECK_NO_THROW(gdo_ptr2 = p_test2->at<GDoubleObject>(2));

		// The fp value should have changed
		BOOST_CHECK(gdo_ptr2->value() != gdo_ptr1->value());
	}

	//------------------------------------------------------------------------------

	{ // Test that the fpAdd() function only has an effect on fp parameters
		std::shared_ptr <GParameterObjectCollection> p_test1 = this->clone<GParameterObjectCollection>();
		std::shared_ptr <GParameterObjectCollection> p_test2 = this->clone<GParameterObjectCollection>();

		// Fill p_test1 with parameters
		BOOST_CHECK_NO_THROW(p_test1->fillWithObjects());

		// Initialize p_test1 with a fixed value
		BOOST_CHECK_NO_THROW(p_test1->fixedValueInit<double>(FIXEDVALUEINIT, activityMode::ALLPARAMETERS));

		// Load the data intp p_test2
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

		// Check that both individuals are identical
		BOOST_CHECK(*p_test1 == *p_test2);

		// Add p_test1 to p_test2
		BOOST_CHECK_NO_THROW(p_test2->add<double>(p_test1, activityMode::ALLPARAMETERS));

		// The first two parameters should again be unchanged
		BOOST_CHECK(*(p_test1->at(0)) == *(p_test2->at(0)));
		BOOST_CHECK(*(p_test1->at(1)) == *(p_test2->at(1)));

		// Extract the fp parameters
		std::shared_ptr <GDoubleObject> gdo_ptr1, gdo_ptr2;
		BOOST_CHECK_NO_THROW(gdo_ptr1 = p_test1->at<GDoubleObject>(2));
		BOOST_CHECK_NO_THROW(gdo_ptr2 = p_test2->at<GDoubleObject>(2));

		// The fp value should have changed, the value should be (FIXEDVALUEINIT+FIXEDVALUEINIT)
		BOOST_CHECK(gdo_ptr2->value() != gdo_ptr1->value());
		BOOST_CHECK(gdo_ptr2->value() == FIXEDVALUEINIT + FIXEDVALUEINIT);
	}

	//------------------------------------------------------------------------------

	{ // Test that the fpSubtract() function only has an effect on fp parameters
		std::shared_ptr <GParameterObjectCollection> p_test1 = this->clone<GParameterObjectCollection>();
		std::shared_ptr <GParameterObjectCollection> p_test2 = this->clone<GParameterObjectCollection>();

		// Fill p_test1 with parameters
		BOOST_CHECK_NO_THROW(p_test1->fillWithObjects());

		// Initialize p_test1 with a fixed value
		BOOST_CHECK_NO_THROW(p_test1->fixedValueInit<double>(FIXEDVALUEINIT, activityMode::ALLPARAMETERS));

		// Load the data intp p_test2
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

		// Check that both individuals are identical
		BOOST_CHECK(*p_test1 == *p_test2);

		// Subtract p_test1 from p_test2
		BOOST_CHECK_NO_THROW(p_test2->subtract<double>(p_test1, activityMode::ALLPARAMETERS));

		// The first two parameters should again be unchanged
		BOOST_CHECK(*(p_test1->at(0)) == *(p_test2->at(0)));
		BOOST_CHECK(*(p_test1->at(1)) == *(p_test2->at(1)));

		// Extract the fp parameters
		std::shared_ptr <GDoubleObject> gdo_ptr1, gdo_ptr2;
		BOOST_CHECK_NO_THROW(gdo_ptr1 = p_test1->at<GDoubleObject>(2));
		BOOST_CHECK_NO_THROW(gdo_ptr2 = p_test2->at<GDoubleObject>(2));

		// The fp value should have changed, the value should be 0
		BOOST_CHECK(gdo_ptr2->value() != gdo_ptr1->value());
		BOOST_CHECK(gdo_ptr2->value() == 0);
	}

	//------------------------------------------------------------------------------

	{ // Test random initialization (test of GParameterTCollectionT<T>::randomInit_() )
		std::shared_ptr<GParameterObjectCollection> p_test1 = this->clone<GParameterObjectCollection>();
		std::shared_ptr<GParameterObjectCollection> p_test2 = this->clone<GParameterObjectCollection>();

		// Fill p_test1 with parameters
		BOOST_CHECK_NO_THROW(p_test1->fillWithObjects());
		BOOST_REQUIRE(!p_test1->empty());

		// Load the data intp p_test2
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));
		BOOST_REQUIRE(!p_test2->empty());

		// Check that both individuals are identical
		BOOST_CHECK(*p_test1 == *p_test2);

		// Randomly initialize p_test2, using GParameterTCollectionT<T>::randomInit()
		BOOST_CHECK_NO_THROW(p_test2->randomInit(activityMode::ALLPARAMETERS, gr));

		// Check that the two objects differ
		BOOST_CHECK(*p_test1 != *p_test2);

		// Note: Checks of changes in the individual parameters do not make sense, as e.g. the boolean type
		// might not have changed.
	}

	//------------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GParameterObjectCollection::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GParameterObjectCollection::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GParameterTCollectionT<GParameterBase>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GParameterObjectCollection::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
