/**
 * @file G_OA_EvolutionaryAlgorithm_PersonalityTraits.cpp
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

#include "geneva/G_OptimizationAlgorithm_EvolutionaryAlgorithm_PersonalityTraits.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GEvolutionaryAlgorithm_PersonalityTraits)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/** A short identifier suitable for storage in a std::map */
G_API_GENEVA const std::string GEvolutionaryAlgorithm_PersonalityTraits::nickname = "ea";

/******************************************************************************/
/**
 * The default constructor
 */
GEvolutionaryAlgorithm_PersonalityTraits::GEvolutionaryAlgorithm_PersonalityTraits()
	: GBaseParChildPersonalityTraits(), isOnParetoFront_(true) { /* nothing */ }

/******************************************************************************/
/**
 * The copy contructor
 *
 * @param cp A copy of another GEAPersonalityTraits object
 */
GEvolutionaryAlgorithm_PersonalityTraits::GEvolutionaryAlgorithm_PersonalityTraits(const GEvolutionaryAlgorithm_PersonalityTraits &cp)
	: GBaseParChildPersonalityTraits(cp), isOnParetoFront_(cp.isOnParetoFront_) { /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
GEvolutionaryAlgorithm_PersonalityTraits::~GEvolutionaryAlgorithm_PersonalityTraits() { /* nothing */ }

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GEvolutionaryAlgorithm_PersonalityTraits &GEvolutionaryAlgorithm_PersonalityTraits::operator=(const GEvolutionaryAlgorithm_PersonalityTraits &cp) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GEAPersonalityTraits object
 *
 * @param  cp A constant reference to another GEAPersonalityTraits object
 * @return A boolean indicating whether both objects are equal
 */
bool GEvolutionaryAlgorithm_PersonalityTraits::operator==(const GEvolutionaryAlgorithm_PersonalityTraits &cp) const {
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
 * Checks for inequality with another GEAPersonalityTraits object
 *
 * @param  cp A constant reference to another GEAPersonalityTraits object
 * @return A boolean indicating whether both objects are inequal
 */
bool GEvolutionaryAlgorithm_PersonalityTraits::operator!=(const GEvolutionaryAlgorithm_PersonalityTraits &cp) const {
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
void GEvolutionaryAlgorithm_PersonalityTraits::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GEvolutionaryAlgorithm_PersonalityTraits reference independent of this object and convert the pointer
	const GEvolutionaryAlgorithm_PersonalityTraits *p_load = Gem::Common::g_convert_and_compare<GObject, GEvolutionaryAlgorithm_PersonalityTraits>(cp, this);

	GToken token("GEvolutionaryAlgorithm_PersonalityTraits", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GBaseParChildPersonalityTraits>(IDENTITY(*this, *p_load), token);

	// ... and then the local data
	compare_t(IDENTITY(isOnParetoFront_, p_load->isOnParetoFront_), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GEvolutionaryAlgorithm_PersonalityTraits::name() const {
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
	isOnParetoFront_ = p_load->isOnParetoFront_;
}

/******************************************************************************/
/**
 * Allows to check whether this individual lies on the pareto front (only yields
 * useful results after pareto-sorting in EA)
 *
 * @return A boolean indicating whether this object lies on the current pareto front
 */
bool GEvolutionaryAlgorithm_PersonalityTraits::isOnParetoFront() const {
	return isOnParetoFront_;
}

/******************************************************************************/
/**
 * Allows to reset the pareto tag to "true"
 */
void GEvolutionaryAlgorithm_PersonalityTraits::resetParetoTag() {
	isOnParetoFront_ = true;
}

/******************************************************************************/
/**
 * Allows to specify that this individual does not lie on the pareto front
 * of the current iteration
 */
void GEvolutionaryAlgorithm_PersonalityTraits::setIsNotOnParetoFront() {
	isOnParetoFront_ = false;
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GEvolutionaryAlgorithm_PersonalityTraits::modify_GUnitTests() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if (GBaseParChildPersonalityTraits::modify_GUnitTests()) result = true;

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
void GEvolutionaryAlgorithm_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GBaseParChildPersonalityTraits::specificTestsNoFailureExpected_GUnitTests();

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
void GEvolutionaryAlgorithm_PersonalityTraits::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GBaseParChildPersonalityTraits::specificTestsFailuresExpected_GUnitTests();

	// --------------------------------------------------------------------------
	// --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GEvolutionaryAlgorithm_PersonalityTraits::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

