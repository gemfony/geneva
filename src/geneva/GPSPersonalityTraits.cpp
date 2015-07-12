/**
 * @file GPSPersonalityTraits.cpp
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
#include "geneva/GPSPersonalityTraits.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GPSPersonalityTraits)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GPSPersonalityTraits::GPSPersonalityTraits()
	: GPersonalityTraits(), popPos_(0) { /* nothing */ }

/******************************************************************************/
/**
 * The copy contructor
 *
 * @param cp A copy of another GPSPersonalityTraits object
 */
GPSPersonalityTraits::GPSPersonalityTraits(const GPSPersonalityTraits &cp)
	: GPersonalityTraits(cp), popPos_(cp.popPos_) { /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
GPSPersonalityTraits::~GPSPersonalityTraits() { /* nothing */ }

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GPSPersonalityTraits &GPSPersonalityTraits::operator=(const GPSPersonalityTraits &cp) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GPSPersonalityTraits object
 *
 * @param  cp A constant reference to another GPSPersonalityTraits object
 * @return A boolean indicating whether both objects are equal
 */
bool GPSPersonalityTraits::operator==(const GPSPersonalityTraits &cp) const {
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
 * Checks for inequality with another GPSPersonalityTraits object
 *
 * @param  cp A constant reference to another GPSPersonalityTraits object
 * @return A boolean indicating whether both objects are inequal
 */
bool GPSPersonalityTraits::operator!=(const GPSPersonalityTraits &cp) const {
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
void GPSPersonalityTraits::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GPSPersonalityTraits reference independent of this object and convert the pointer
	const GPSPersonalityTraits *p_load = Gem::Common::g_convert_and_compare<GObject, GPSPersonalityTraits>(cp, this);

	GToken token("GPSPersonalityTraits", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GPersonalityTraits>(IDENTITY(*this, *p_load), token);

	// ... and then the local data
	compare_t(IDENTITY(popPos_, p_load->popPos_), token);

	// React on deviations from the expectation
	token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GPSPersonalityTraits::name() const {
	return std::string("GPSPersonalityTraits");
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A clone of this object, camouflaged as a GObject
 */
GObject *GPSPersonalityTraits::clone_() const {
	return new GPSPersonalityTraits(*this);
}

/******************************************************************************/
/**
 * Loads the data of another GPSPersonalityTraits object
 *
 * @param cp A copy of another GPSPersonalityTraits object, camouflaged as a GObject
 */
void GPSPersonalityTraits::load_(const GObject *cp) {
	// Check that we are dealing with a GPSPersonalityTraits reference independent of this object and convert the pointer
	const GPSPersonalityTraits *p_load = Gem::Common::g_convert_and_compare<GObject, GPSPersonalityTraits>(cp, this);

	// Load the parent class'es data
	GPersonalityTraits::load_(cp);

	// and then the local data
	popPos_ = p_load->popPos_;
}

/******************************************************************************/
/**
 * Sets the position of the individual in the population
 *
 * @param popPos The new position of this individual in the population
 */
void GPSPersonalityTraits::setPopulationPosition(const std::size_t &popPos) {
	popPos_ = popPos;
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
std::size_t GPSPersonalityTraits::getPopulationPosition(void) const {
	return popPos_;
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
bool GPSPersonalityTraits::modify_GUnitTests() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if (GPersonalityTraits::modify_GUnitTests()) result = true;

	this->setPopulationPosition(this->getPopulationPosition() + 1);
	result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GPSPersonalityTraits::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GPSPersonalityTraits::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GPersonalityTraits::specificTestsNoFailureExpected_GUnitTests();


	// --------------------------------------------------------------------------

	// --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GPSPersonalityTraits::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GPSPersonalityTraits::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GPersonalityTraits::specificTestsFailuresExpected_GUnitTests();

	// --------------------------------------------------------------------------

	// --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GPSPersonalityTraits::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
