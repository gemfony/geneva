/**
 * @file GPSPersonalityTraits.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
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
   : GPersonalityTraits()
   , popPos_(0)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy contructor
 *
 * @param cp A copy of another GPSPersonalityTraits object
 */
GPSPersonalityTraits::GPSPersonalityTraits(const GPSPersonalityTraits& cp)
   : GPersonalityTraits(cp)
   , popPos_(cp.popPos_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
GPSPersonalityTraits::~GPSPersonalityTraits()
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard assignment operator for GPSPersonalityTraits objects.
 *
 * @param cp Reference to another GPSPersonalityTraits object
 * @return A constant reference to this object
 */
const GPSPersonalityTraits& GPSPersonalityTraits::operator=(const GPSPersonalityTraits& cp) {
   GPSPersonalityTraits::load_(&cp);
   return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GPSPersonalityTraits object
 *
 * @param  cp A constant reference to another GPSPersonalityTraits object
 * @return A boolean indicating whether both objects are equal
 */
bool GPSPersonalityTraits::operator==(const GPSPersonalityTraits& cp) const {
   using namespace Gem::Common;
   // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
   return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GPSPersonalityTraits::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GPSPersonalityTraits object
 *
 * @param  cp A constant reference to another GPSPersonalityTraits object
 * @return A boolean indicating whether both objects are inequal
 */
bool GPSPersonalityTraits::operator!=(const GPSPersonalityTraits& cp) const {
   using namespace Gem::Common;
   // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
   return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GPSPersonalityTraits::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GPSPersonalityTraits::checkRelationshipWith(const GObject& cp,
      const Gem::Common::expectation& e,
      const double& limit,
      const std::string& caller,
      const std::string& y_name,
      const bool& withMessages) const
{
    using namespace Gem::Common;

   // Check that we are indeed dealing with a GParamterBase reference
   const GPSPersonalityTraits *p_load = GObject::gobject_conversion<GPSPersonalityTraits>(&cp);

   // Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

   // Check our parent class'es data ...
   deviations.push_back(GPersonalityTraits::checkRelationshipWith(cp, e, limit, "GPSPersonalityTraits", y_name, withMessages));

   // ... and then our local data
   deviations.push_back(checkExpectation(withMessages, "GPSPersonalityTraits", popPos_, p_load->popPos_, "popPos_", "p_load->popPos_", e , limit));

   return evaluateDiscrepancies("GPSPersonalityTraits", caller, deviations, e);
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
GObject* GPSPersonalityTraits::clone_() const {
   return new GPSPersonalityTraits(*this);
}

/******************************************************************************/
/**
 * Loads the data of another GPSPersonalityTraits object
 *
 * @param cp A copy of another GPSPersonalityTraits object, camouflaged as a GObject
 */
void GPSPersonalityTraits::load_(const GObject* cp) {
   const GPSPersonalityTraits *p_load = gobject_conversion<GPSPersonalityTraits>(cp);

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
void GPSPersonalityTraits::setPopulationPosition(const std::size_t& popPos) {
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
   if(GPersonalityTraits::modify_GUnitTests()) result = true;

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