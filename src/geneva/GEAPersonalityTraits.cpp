/**
 * @file GEAPersonalityTraits.cpp
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

#include "geneva/GEAPersonalityTraits.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GEAPersonalityTraits)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GEAPersonalityTraits::GEAPersonalityTraits()
	: GBaseParChildPersonalityTraits()
	, isOnParetoFront_(true)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy contructor
 *
 * @param cp A copy of another GEAPersonalityTraits object
 */
GEAPersonalityTraits::GEAPersonalityTraits(const GEAPersonalityTraits& cp)
	: GBaseParChildPersonalityTraits(cp)
	, isOnParetoFront_(cp.isOnParetoFront_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
GEAPersonalityTraits::~GEAPersonalityTraits()
{ /* nothing */ }

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GEAPersonalityTraits& GEAPersonalityTraits::operator=(const GEAPersonalityTraits& cp) {
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
bool GEAPersonalityTraits::operator==(const GEAPersonalityTraits& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GEAPersonalityTraits::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GEAPersonalityTraits object
 *
 * @param  cp A constant reference to another GEAPersonalityTraits object
 * @return A boolean indicating whether both objects are inequal
 */
bool GEAPersonalityTraits::operator!=(const GEAPersonalityTraits& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GEAPersonalityTraits::operator!=","cp", CE_SILENT);
}

/******************************************************************************/
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
boost::optional<std::string> GEAPersonalityTraits::checkRelationshipWith(
   const GObject& cp
   , const Gem::Common::expectation& e
   , const double& limit
   , const std::string& caller
   , const std::string& y_name
   , const bool& withMessages
) const {
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GEAPersonalityTraits *p_load = GObject::gobject_conversion<GEAPersonalityTraits>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GBaseParChildPersonalityTraits::checkRelationshipWith(cp, e, limit, "GEAPersonalityTraits", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GEAPersonalityTraits", isOnParetoFront_, p_load->isOnParetoFront_, "isOnParetoFront_", "p_load->isOnParetoFront_", e , limit));

	return evaluateDiscrepancies("GEAPersonalityTraits", caller, deviations, e);
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
void GEAPersonalityTraits::compare(
   const GObject& cp
   , const Gem::Common::expectation& e
   , const double& limit
) const {
   using namespace Gem::Common;

   // Check that we are indeed dealing with a GBaseEA reference
   const GEAPersonalityTraits *p_load = GObject::gobject_conversion<GEAPersonalityTraits>(&cp);

   try {
      // Check our parent class'es data ...
      GBaseParChildPersonalityTraits::compare(cp, e, limit);

      // ... and then our local data
      COMPARE(isOnParetoFront_, p_load->isOnParetoFront_, e, limit);

   } catch(g_expectation_violation& g) { // Create a suitable stack-trace
      g.add("g_expectation_violation caught by GEAPersonalityTraits");
      throw g;
   }
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GEAPersonalityTraits::name() const {
   return std::string("GEAPersonalityTraits");
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A clone of this object, camouflaged as a GObject
 */
GObject* GEAPersonalityTraits::clone_() const {
	return new GEAPersonalityTraits(*this);
}

/******************************************************************************/
/**
 * Loads the data of another GEAPersonalityTraits object
 *
 * @param cp A copy of another GEAPersonalityTraits object, camouflaged as a GObject
 */
void GEAPersonalityTraits::load_(const GObject* cp) {
	const GEAPersonalityTraits *p_load = gobject_conversion<GEAPersonalityTraits>(cp);

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
bool GEAPersonalityTraits::isOnParetoFront() const {
	return isOnParetoFront_;
}

/******************************************************************************/
/**
 * Allows to reset the pareto tag to "true"
 */
void GEAPersonalityTraits::resetParetoTag() {
	isOnParetoFront_ = true;
}

/******************************************************************************/
/**
 * Allows to specify that this individual does not lie on the pareto front
 * of the current iteration
 */
void GEAPersonalityTraits::setIsNotOnParetoFront() {
	isOnParetoFront_ = false;
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GEAPersonalityTraits::modify_GUnitTests() {
#ifdef GEM_TESTING
   bool result = false;

	// Call the parent class'es function
	if(GBaseParChildPersonalityTraits::modify_GUnitTests()) result = true;

	return result;
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GEAPersonalityTraits::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GEAPersonalityTraits::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GBaseParChildPersonalityTraits::specificTestsNoFailureExpected_GUnitTests();

   // --------------------------------------------------------------------------
	// --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GEAPersonalityTraits::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GEAPersonalityTraits::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GBaseParChildPersonalityTraits::specificTestsFailuresExpected_GUnitTests();

	// --------------------------------------------------------------------------
	// --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GEAPersonalityTraits::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
