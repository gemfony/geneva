/**
 * @file
 */

/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/
#include "geneva/GConstrainedInt32ObjectCollection.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GConstrainedInt32ObjectCollection)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * Initialization with a number of identical GConstrainedDoubleObject objects
 */
GConstrainedInt32ObjectCollection::GConstrainedInt32ObjectCollection(
	const std::size_t &nCp, std::shared_ptr <GConstrainedInt32Object> tmpl_ptr
)
	: GParameterTCollectionT<GConstrainedInt32Object>(nCp, tmpl_ptr)
{ /* nothing */ }

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject *GConstrainedInt32ObjectCollection::clone_() const {
	return new GConstrainedInt32ObjectCollection(*this);
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
void GConstrainedInt32ObjectCollection::compare_(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GConstrainedInt32ObjectCollection reference independent of this object and convert the pointer
	const GConstrainedInt32ObjectCollection *p_load = Gem::Common::g_convert_and_compare<GObject, GConstrainedInt32ObjectCollection>(cp, this);

	GToken token("GConstrainedInt32ObjectCollection", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<GParameterTCollectionT<GConstrainedInt32Object>>(*this, *p_load, token);

	// ... no local data

	// React on deviations from the expectation
	token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GConstrainedInt32ObjectCollection::name_() const {
	return std::string("GConstrainedInt32ObjectCollection");
}

/******************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GConstrainedInt32ObjectCollection object, camouflaged as a GObject
 */
void GConstrainedInt32ObjectCollection::load_(const GObject *cp) {
	// Convert the pointer to our target type and check for self-assignment
	const GConstrainedInt32ObjectCollection * p_load = Gem::Common::g_convert_and_compare<GObject, GConstrainedInt32ObjectCollection>(cp, this);

	// Load our parent class'es data ...
	GParameterTCollectionT<GConstrainedInt32Object>::load_(cp);

	// ... no local data
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GConstrainedInt32ObjectCollection::modify_GUnitTests_() {
#ifdef GEM_TESTING
	this->fillWithObjects_(10);

	// Call the parent class'es function
	GParameterTCollectionT<GConstrainedInt32Object>::modify_GUnitTests_();

	return true;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   Gem::Common::condnotset("GConstrainedInt32ObjectCollection::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Fills the collection with GConstrainedInt32Object objects
 */
void GConstrainedInt32ObjectCollection::fillWithObjects_(const std::size_t &nAddedObjects) {
#ifdef GEM_TESTING
	// A random generator
	Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

	// Clear the collection, so we can start fresh
	BOOST_CHECK_NO_THROW(this->clear());

	// Add GConstrainedInt32Object items with adaptors to p_test1
	for (std::size_t i = 0; i < nAddedObjects; i++) {
		// Create a suitable adaptor
		std::shared_ptr <GInt32GaussAdaptor> giga_ptr;

		BOOST_CHECK_NO_THROW(
			giga_ptr = std::shared_ptr<GInt32GaussAdaptor>(new GInt32GaussAdaptor(0.025, 0.1, 0, 1, 1.0)));
		BOOST_CHECK_NO_THROW(giga_ptr->setAdaptionThreshold(
			0)); // Make sure the adaptor's internal parameters don't change through the adaption
		BOOST_CHECK_NO_THROW(giga_ptr->setAdaptionMode(adaptionMode::ALWAYS)); // Always adapt

		// Create a suitable GConstrainedInt32Object object
		std::shared_ptr <GConstrainedInt32Object> gcio_ptr;

		BOOST_CHECK_NO_THROW(gcio_ptr = std::shared_ptr<GConstrainedInt32Object>(
			new GConstrainedInt32Object(-100, 100))); // Initialization in the range -100, 100

		// Add the adaptor
		BOOST_CHECK_NO_THROW(gcio_ptr->addAdaptor(giga_ptr));

		// Randomly initialize the GConstrainedInt32Object object, so it is unique
		BOOST_CHECK_NO_THROW(gcio_ptr->randomInit(activityMode::ALLPARAMETERS, gr));

		// Add the object to the collection
		BOOST_CHECK_NO_THROW(this->push_back(gcio_ptr));
	}

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   Gem::Common::condnotset("GConstrainedInt32ObjectCollection::fillWithObjects", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GConstrainedInt32ObjectCollection::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
	// Some settings
	const std::size_t nAddedObjects = 10;
	const std::size_t nTests = 100;
	const double LOWERINITBOUNDARY = -10;
	const double UPPERINITBOUNDARY = 10;
	const double FIXEDVALUEINIT = 1.;
	const double MULTVALUE = 3.;
	const double RANDLOWERBOUNDARY = 0.;
	const double RANDUPPERBOUNDARY = 10.;

	// A random generator
	Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

	// --------------------------------------------------------------------------

	{ // Call the parent class'es function
		std::shared_ptr <GConstrainedInt32ObjectCollection> p_test = this->clone<GConstrainedInt32ObjectCollection>();

		// Fill p_test with objects
		p_test->fillWithObjects_(nAddedObjects);

		// Execute the parent class'es tests
		p_test->GParameterTCollectionT<GConstrainedInt32Object>::specificTestsNoFailureExpected_GUnitTests_();
	}

	// --------------------------------------------------------------------------

	{ // Test that the fp-family of functions has no effect on this object (and contained objects)
		std::shared_ptr <GConstrainedInt32ObjectCollection> p_test1 = this->clone<GConstrainedInt32ObjectCollection>();
		std::shared_ptr <GConstrainedInt32ObjectCollection> p_test2 = this->clone<GConstrainedInt32ObjectCollection>();

		// Fill p_test1 with objects
		BOOST_CHECK_NO_THROW(p_test1->fillWithObjects_(nAddedObjects));

		// Make sure it has the expected size
		BOOST_CHECK(p_test1->size() == nAddedObjects);

		// Load the data into p_test2
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

		// Check that both items are identical
		BOOST_CHECK(*p_test1 == *p_test2);

		// Try to add a fixed fp value to p_test1 and check whether it has changed
		BOOST_CHECK_NO_THROW(p_test1->fixedValueInit<double>(FIXEDVALUEINIT, activityMode::ALLPARAMETERS));
		BOOST_CHECK(*p_test1 == *p_test2);

		// Try to multiply p_test1 with a fixed fp value and check whether it has changed
		BOOST_CHECK_NO_THROW(p_test1->multiplyBy<double>(MULTVALUE, activityMode::ALLPARAMETERS));
		BOOST_CHECK(*p_test1 == *p_test2);

		// Try to multiply p_test1 with a random fp value in a given range and check whether it has changed
		BOOST_CHECK_NO_THROW(p_test1->multiplyByRandom<double>(RANDLOWERBOUNDARY, RANDUPPERBOUNDARY, activityMode::ALLPARAMETERS, gr));
		BOOST_CHECK(*p_test1 == *p_test2);

		// Try to multiply p_test1 with a random fp value in the range [0,1[ and check whether it has changed
		BOOST_CHECK_NO_THROW(p_test1->multiplyByRandom<double>(activityMode::ALLPARAMETERS, gr));
		BOOST_CHECK_NO_THROW(p_test1->multiplyByRandom<double>(activityMode::ALLPARAMETERS, gr));
		BOOST_CHECK(*p_test1 == *p_test2);

		// Try to add p_test2 to p_test1 and see whether it has changed
		BOOST_CHECK_NO_THROW(p_test1->add<double>(p_test2, activityMode::ALLPARAMETERS));
		BOOST_CHECK(*p_test1 == *p_test2);

		// Try to subtract p_test2 from p_test1 and see whether it has changed
		BOOST_CHECK_NO_THROW(p_test1->subtract<double>(p_test2, activityMode::ALLPARAMETERS));
		BOOST_CHECK(*p_test1 == *p_test2);
	}

	// --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   Gem::Common::condnotset("GConstrainedInt32ObjectCollection::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GConstrainedInt32ObjectCollection::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GParameterTCollectionT<GConstrainedInt32Object>::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   Gem::Common::condnotset("GConstrainedInt32ObjectCollection::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
