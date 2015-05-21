/**
 * @file GMPEAPersonalityTraits.cpp
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

#include "geneva/GMPEAPersonalityTraits.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GMPEAPersonalityTraits)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GMPEAPersonalityTraits::GMPEAPersonalityTraits()
	: GBaseParChildPersonalityTraits() { /* nothing */ }

/******************************************************************************/
/**
 * The copy contructor
 *
 * @param cp A copy of another GMPEAPersonalityTraits object
 */
GMPEAPersonalityTraits::GMPEAPersonalityTraits(const GMPEAPersonalityTraits &cp)
	: GBaseParChildPersonalityTraits(cp) { /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
GMPEAPersonalityTraits::~GMPEAPersonalityTraits() { /* nothing */ }

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GMPEAPersonalityTraits &GMPEAPersonalityTraits::operator=(const GMPEAPersonalityTraits &cp) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GMPEAPersonalityTraits object
 *
 * @param  cp A constant reference to another GMPEAPersonalityTraits object
 * @return A boolean indicating whether both objects are equal
 */
bool GMPEAPersonalityTraits::operator==(const GMPEAPersonalityTraits &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Checks for inequality with another GMPEAPersonalityTraits object
 *
 * @param  cp A constant reference to another GMPEAPersonalityTraits object
 * @return A boolean indicating whether both objects are inequal
 */
bool GMPEAPersonalityTraits::operator!=(const GMPEAPersonalityTraits &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
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
void GMPEAPersonalityTraits::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are indeed dealing with a GBaseEA reference
	const GMPEAPersonalityTraits *p_load = GObject::gobject_conversion<GMPEAPersonalityTraits>(&cp);

	GToken token("GMPEAPersonalityTraits", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GBaseParChildPersonalityTraits>(IDENTITY(*this, *p_load), token);

	// ... no local data

	// React on deviations from the expectation
	token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GMPEAPersonalityTraits::name() const {
	return std::string("GMPEAPersonalityTraits");
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A clone of this object, camouflaged as a GObject
 */
GObject *GMPEAPersonalityTraits::clone_() const {
	return new GMPEAPersonalityTraits(*this);
}

/******************************************************************************/
/**
 * Loads the data of another GMPEAPersonalityTraits object
 *
 * @param cp A copy of another GMPEAPersonalityTraits object, camouflaged as a GObject
 */
void GMPEAPersonalityTraits::load_(const GObject *cp) {
	const GMPEAPersonalityTraits *p_load = gobject_conversion<GMPEAPersonalityTraits>(cp);

	// Load the parent class'es data
	GBaseParChildPersonalityTraits::load_(cp);

	// Then load our local data
	// no local data ...
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GMPEAPersonalityTraits::modify_GUnitTests() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if (GBaseParChildPersonalityTraits::modify_GUnitTests()) result = true;

	return result;
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GMPEAPersonalityTraits::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GMPEAPersonalityTraits::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GBaseParChildPersonalityTraits::specificTestsNoFailureExpected_GUnitTests();

	// --------------------------------------------------------------------------
	// --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GMPEAPersonalityTraits::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GMPEAPersonalityTraits::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GBaseParChildPersonalityTraits::specificTestsFailuresExpected_GUnitTests();

	// --------------------------------------------------------------------------
	// --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GMPEAPersonalityTraits::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
