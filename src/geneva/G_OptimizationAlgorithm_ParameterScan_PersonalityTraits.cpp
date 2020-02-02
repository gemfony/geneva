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

#include "geneva/G_OptimizationAlgorithm_ParameterScan_PersonalityTraits.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GParameterScan_PersonalityTraits)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/** A short identifier suitable for storage in a std::map */
G_API_GENEVA const std::string GParameterScan_PersonalityTraits::nickname = "ps";

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GParameterScan_PersonalityTraits::compare_(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GParameterScan_PersonalityTraits reference independent of this object and convert the pointer
	const GParameterScan_PersonalityTraits *p_load = Gem::Common::g_convert_and_compare<GObject, GParameterScan_PersonalityTraits>(cp, this);

	GToken token("GParameterScan_PersonalityTraits", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<GPersonalityTraits>(*this, *p_load, token);

	// ... and then the local data
	compare_t(IDENTITY(m_popPos, p_load->m_popPos), token);

	// React on deviations from the expectation
	token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GParameterScan_PersonalityTraits::name_() const {
	return std::string("GParameterScan_PersonalityTraits");
}

/******************************************************************************/
/**
 * Retrieves the mnemonic of the optimization algorithm
 */
std::string GParameterScan_PersonalityTraits::getMnemonic() const {
	return GParameterScan_PersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A clone of this object, camouflaged as a GObject
 */
GObject *GParameterScan_PersonalityTraits::clone_() const {
	return new GParameterScan_PersonalityTraits(*this);
}

/******************************************************************************/
/**
 * Loads the data of another GPSPersonalityTraits object
 *
 * @param cp A copy of another GPSPersonalityTraits object, camouflaged as a GObject
 */
void GParameterScan_PersonalityTraits::load_(const GObject *cp) {
	// Check that we are dealing with a GParameterScan_PersonalityTraits reference independent of this object and convert the pointer
	const GParameterScan_PersonalityTraits *p_load = Gem::Common::g_convert_and_compare<GObject, GParameterScan_PersonalityTraits>(cp, this);

	// Load the parent class'es data
	GPersonalityTraits::load_(cp);

	// and then the local data
	m_popPos = p_load->m_popPos;
}

/******************************************************************************/
/**
 * Sets the position of the individual in the population
 *
 * @param popPos The new position of this individual in the population
 */
void GParameterScan_PersonalityTraits::setPopulationPosition(const std::size_t &popPos) {
	m_popPos = popPos;
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Retrieves the position of the individual in the population
 *
 * @return The current position of this individual in the population
 */
std::size_t GParameterScan_PersonalityTraits::getPopulationPosition(void) const {
	return m_popPos;
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GParameterScan_PersonalityTraits::modify_GUnitTests_() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if (GPersonalityTraits::modify_GUnitTests_()) result = true;

	this->setPopulationPosition(this->getPopulationPosition() + 1);
	result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   Gem::Common::condnotset("GParameterScan_PersonalityTraits::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GParameterScan_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GPersonalityTraits::specificTestsNoFailureExpected_GUnitTests_();


	// --------------------------------------------------------------------------

	// --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   Gem::Common::condnotset("GParameterScan_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GParameterScan_PersonalityTraits::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GPersonalityTraits::specificTestsFailuresExpected_GUnitTests_();

	// --------------------------------------------------------------------------

	// --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   Gem::Common::condnotset("GParameterScan_PersonalityTraits::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
