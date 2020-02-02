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

#include "geneva/G_OptimizationAlgorithm_EvolutionaryAlgorithm_PersonalityTraits.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GEvolutionaryAlgorithm_PersonalityTraits)  // NOLINT

namespace Gem {
namespace Geneva {

/******************************************************************************/
/** A short identifier suitable for storage in a std::map */
G_API_GENEVA const std::string GEvolutionaryAlgorithm_PersonalityTraits::nickname = "ea"; // NOLINT

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GEvolutionaryAlgorithm_PersonalityTraits::compare_(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GEvolutionaryAlgorithm_PersonalityTraits reference independent of this object and convert the pointer
	const GEvolutionaryAlgorithm_PersonalityTraits *p_load = Gem::Common::g_convert_and_compare<GObject, GEvolutionaryAlgorithm_PersonalityTraits>(cp, this);

	GToken token("GEvolutionaryAlgorithm_PersonalityTraits", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<GBaseParChildPersonalityTraits>(*this, *p_load, token);

	// ... and then the local data
	compare_t(IDENTITY(m_isOnParetoFront, p_load->m_isOnParetoFront), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GEvolutionaryAlgorithm_PersonalityTraits::name_() const {
	return std::string("GEvolutionaryAlgorithm_PersonalityTraits");
}

/******************************************************************************/
/**
 * Retrieves the mnemonic of the optimization algorithm
 */
std::string GEvolutionaryAlgorithm_PersonalityTraits::getMnemonic() const {
	return GEvolutionaryAlgorithm_PersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A clone of this object, camouflaged as a GObject
 */
GObject *GEvolutionaryAlgorithm_PersonalityTraits::clone_() const {
	return new GEvolutionaryAlgorithm_PersonalityTraits(*this);
}

/******************************************************************************/
/**
 * Loads the data of another GEAPersonalityTraits object
 *
 * @param cp A copy of another GEAPersonalityTraits object, camouflaged as a GObject
 */
void GEvolutionaryAlgorithm_PersonalityTraits::load_(const GObject *cp) {
	// Check that we are dealing with a GEvolutionaryAlgorithm_PersonalityTraits reference independent of this object and convert the pointer
	const GEvolutionaryAlgorithm_PersonalityTraits *p_load = Gem::Common::g_convert_and_compare<GObject, GEvolutionaryAlgorithm_PersonalityTraits>(cp, this);

	// Load the parent class'es data
	GBaseParChildPersonalityTraits::load_(cp);

	// Then load our local data
	m_isOnParetoFront = p_load->m_isOnParetoFront;
}

/******************************************************************************/
/**
 * Allows to check whether this individual lies on the pareto front (only yields
 * useful results after pareto-sorting in EA)
 *
 * @return A boolean indicating whether this object lies on the current pareto front
 */
bool GEvolutionaryAlgorithm_PersonalityTraits::isOnParetoFront() const {
	return m_isOnParetoFront;
}

/******************************************************************************/
/**
 * Allows to reset the pareto tag to "true"
 */
void GEvolutionaryAlgorithm_PersonalityTraits::resetParetoTag() {
	m_isOnParetoFront = true;
}

/******************************************************************************/
/**
 * Allows to specify that this individual does not lie on the pareto front
 * of the current iteration
 */
void GEvolutionaryAlgorithm_PersonalityTraits::setIsNotOnParetoFront() {
	m_isOnParetoFront = false;
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GEvolutionaryAlgorithm_PersonalityTraits::modify_GUnitTests_() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if (GBaseParChildPersonalityTraits::modify_GUnitTests_()) result = true;

	return result;
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GEvolutionaryAlgorithm_PersonalityTraits::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GEvolutionaryAlgorithm_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GBaseParChildPersonalityTraits::specificTestsNoFailureExpected_GUnitTests_();

	// --------------------------------------------------------------------------
	// --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GEvolutionaryAlgorithm_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GEvolutionaryAlgorithm_PersonalityTraits::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GBaseParChildPersonalityTraits::specificTestsFailuresExpected_GUnitTests_();

	// --------------------------------------------------------------------------
	// --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GEvolutionaryAlgorithm_PersonalityTraits::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

