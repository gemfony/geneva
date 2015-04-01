/**
 * @file GSAPersonalityTraits.cpp
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

#include "geneva/GSAPersonalityTraits.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GSAPersonalityTraits)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GSAPersonalityTraits::GSAPersonalityTraits()
   : GBaseParChildPersonalityTraits()
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy contructor
 *
 * @param cp A copy of another GSAPersonalityTraits object
 */
GSAPersonalityTraits::GSAPersonalityTraits(const GSAPersonalityTraits& cp)
   : GBaseParChildPersonalityTraits(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
GSAPersonalityTraits::~GSAPersonalityTraits()
{ /* nothing */ }

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GSAPersonalityTraits& GSAPersonalityTraits::operator=(const GSAPersonalityTraits& cp) {
   this->load_(&cp);
   return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GSAPersonalityTraits object
 *
 * @param  cp A constant reference to another GSAPersonalityTraits object
 * @return A boolean indicating whether both objects are equal
 */
bool GSAPersonalityTraits::operator==(const GSAPersonalityTraits& cp) const {
   using namespace Gem::Common;
   // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
   return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GSAPersonalityTraits::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GSAPersonalityTraits object
 *
 * @param  cp A constant reference to another GSAPersonalityTraits object
 * @return A boolean indicating whether both objects are inequal
 */
bool GSAPersonalityTraits::operator!=(const GSAPersonalityTraits& cp) const {
   using namespace Gem::Common;
   // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
   return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GSAPersonalityTraits::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GSAPersonalityTraits::checkRelationshipWith(const GObject& cp,
      const Gem::Common::expectation& e,
      const double& limit,
      const std::string& caller,
      const std::string& y_name,
      const bool& withMessages) const
{
    using namespace Gem::Common;

   // Check that we are indeed dealing with a GParamterBase reference
   const GSAPersonalityTraits *p_load = GObject::gobject_conversion<GSAPersonalityTraits>(&cp);

   // Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

   // Check our parent class'es data ...
   deviations.push_back(GBaseParChildPersonalityTraits::checkRelationshipWith(cp, e, limit, "GSAPersonalityTraits", y_name, withMessages));

   // ... and then our local data
   // no local data

   return evaluateDiscrepancies("GSAPersonalityTraits", caller, deviations, e);
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
void GSAPersonalityTraits::compare(
   const GObject& cp
   , const Gem::Common::expectation& e
   , const double& limit
) const {
   using namespace Gem::Common;

   // Check that we are indeed dealing with a GBaseEA reference
   const GSAPersonalityTraits *p_load = GObject::gobject_conversion<GSAPersonalityTraits>(&cp);

   try {
      // Check our parent class'es data ...
      GBaseParChildPersonalityTraits::compare(cp, e, limit);

      // ... no local data

   } catch(g_expectation_violation& g) { // Create a suitable stack-trace
      g.add("g_expectation_violation caught by GSAPersonalityTraits");
      throw g;
   }
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GSAPersonalityTraits::name() const {
   return std::string("GSAPersonalityTraits");
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A clone of this object, camouflaged as a GObject
 */
GObject* GSAPersonalityTraits::clone_() const {
   return new GSAPersonalityTraits(*this);
}

/******************************************************************************/
/**
 * Loads the data of another GSAPersonalityTraits object
 *
 * @param cp A copy of another GSAPersonalityTraits object, camouflaged as a GObject
 */
void GSAPersonalityTraits::load_(const GObject* cp) {
   const GSAPersonalityTraits *p_load = gobject_conversion<GSAPersonalityTraits>(cp);

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
bool GSAPersonalityTraits::modify_GUnitTests() {
#ifdef GEM_TESTING
   bool result = false;

   // Call the parent class'es function
   if(GBaseParChildPersonalityTraits::modify_GUnitTests()) result = true;

   return result;
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GSAPersonalityTraits::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GSAPersonalityTraits::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
   using boost::unit_test_framework::test_suite;
   using boost::unit_test_framework::test_case;

   // Call the parent class'es function
   GBaseParChildPersonalityTraits::specificTestsNoFailureExpected_GUnitTests();

   // --------------------------------------------------------------------------
   // --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GSAPersonalityTraits::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GSAPersonalityTraits::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
   using boost::unit_test_framework::test_suite;
   using boost::unit_test_framework::test_case;

   // Call the parent class'es function
   GBaseParChildPersonalityTraits::specificTestsFailuresExpected_GUnitTests();

   // --------------------------------------------------------------------------
   // --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GSAPersonalityTraits::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
