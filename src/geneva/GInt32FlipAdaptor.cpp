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

#include "geneva/GInt32FlipAdaptor.hpp"

// Make sure the class gets instantiated at least once
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GInt32FlipAdaptor)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * Initialization with a adaption probability
 *
 * @param adProb The adaption probability
 */
GInt32FlipAdaptor::GInt32FlipAdaptor(const double &adProb)
	: GIntFlipAdaptorT<std::int32_t>(adProb) { /* nothing */ }

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject *GInt32FlipAdaptor::clone_() const {
	return new GInt32FlipAdaptor(*this);
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
void GInt32FlipAdaptor::compare_(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GInt32FlipAdaptor reference independent of this object and convert the pointer
	const GInt32FlipAdaptor *p_load = Gem::Common::g_convert_and_compare<GObject, GInt32FlipAdaptor>(cp, this);

	GToken token("GInt32FlipAdaptor", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<GIntFlipAdaptorT<std::int32_t>>(*this, *p_load, token);

	// ... no local data

	// React on deviations from the expectation
	token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GInt32FlipAdaptor::name_() const {
	return std::string("GInt32FlipAdaptor");
}

/******************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GInt32FlipAdaptor object, camouflaged as a GObject
 */
void GInt32FlipAdaptor::load_(const GObject *cp) {
	// Convert the pointer to our target type and check for self-assignment
	const GInt32FlipAdaptor * p_load = Gem::Common::g_convert_and_compare<GObject, GInt32FlipAdaptor>(cp, this);

	// Load our parent class'es data ...
	GIntFlipAdaptorT<std::int32_t>::load_(cp);

	// ... no local data
}

/******************************************************************************/
/**
 * Retrieves the id of this adaptor
 *
 * @return The id of this adaptor
 */
Gem::Geneva::adaptorId GInt32FlipAdaptor::getAdaptorId_() const {
	return Gem::Geneva::adaptorId::GINT32FLIPADAPTOR;
}

/* ----------------------------------------------------------------------------------
 * Tested in GInt32FlipAdaptor::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GInt32FlipAdaptor::modify_GUnitTests_() {
#ifdef GEM_TESTING

	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	bool result = false;

	// Call the parent class'es function
	if (GIntFlipAdaptorT<std::int32_t>::modify_GUnitTests_()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   Gem::Common::condnotset("GInt32FlipAdaptor::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GInt32FlipAdaptor::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GIntFlipAdaptorT<std::int32_t>::specificTestsNoFailureExpected_GUnitTests_();

	// --------------------------------------------------------------------------

	{ // Check that the adaptor returns the correct adaptor id
		std::shared_ptr <GInt32FlipAdaptor> p_test = this->clone<GInt32FlipAdaptor>();

		BOOST_CHECK_MESSAGE(
			p_test->getAdaptorId() == adaptorId::GINT32FLIPADAPTOR, "\n"
																		<< "p_test->getAdaptorId() = " << p_test->getAdaptorId()
																		<< "GINT32FLIPADAPTOR      = " << adaptorId::GINT32FLIPADAPTOR << "\n"
		);
	}

	// --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   Gem::Common::condnotset("GInt32FlipAdaptor::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GInt32FlipAdaptor::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GIntFlipAdaptorT<std::int32_t>::specificTestsFailuresExpected_GUnitTests_();

	// no local data - nothing to test

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   Gem::Common::condnotset("GInt32FlipAdaptor::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
