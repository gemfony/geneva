/**
 * @file GSerialPS.cpp
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

#include "geneva/GSerialPS.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GSerialPS)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GSerialPS::GSerialPS() : GBasePS()
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard copy constructor
 */
GSerialPS::GSerialPS(const GSerialPS& cp) : GBasePS(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor.
 */
GSerialPS::~GSerialPS()
{ /* nothing */ }

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GSerialPS& GSerialPS::operator=(const GSerialPS& cp) {
   this->load_(&cp);
   return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GSerialPS object
 *
 * @param  cp A constant reference to another GSerialPS object
 * @return A boolean indicating whether both objects are equal
 */
bool GSerialPS::operator==(const GSerialPS& cp) const {
   using namespace Gem::Common;
   try {
      this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
      return true;
   } catch(g_expectation_violation&) {
      return false;
   }
}

/******************************************************************************/
/**
 * Checks for inequality with another GSerialPS object
 *
 * @param  cp A constant reference to another GSerialPS object
 * @return A boolean indicating whether both objects are inequal
 */
bool GSerialPS::operator!=(const GSerialPS& cp) const {
   using namespace Gem::Common;
   try {
      this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
      return true;
   } catch(g_expectation_violation&) {
      return false;
   }
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
boost::optional<std::string> GSerialPS::checkRelationshipWith(
      const GObject& cp,
      const Gem::Common::expectation& e,
      const double& limit,
      const std::string& caller,
      const std::string& y_name,
      const bool& withMessages
) const {
   using namespace Gem::Common;

   // Check that we are indeed dealing with a GParamterBase reference
   // const GSerialPS *p_load = GObject::gobject_conversion<GSerialPS>(&cp);
   // Uncomment the previous line and comment the following line if you wish to use local data
   GObject::selfAssignmentCheck<GSerialPS>(&cp);

   // Will hold possible deviations from the expectation, including explanations
   std::vector<boost::optional<std::string> > deviations;

   // Check our parent class'es data ...
   deviations.push_back(GBasePS::checkRelationshipWith(cp, e, limit, "GSerialPS", y_name, withMessages));

   // ... no local data

   return evaluateDiscrepancies("GSerialPS", caller, deviations, e);
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
void GSerialPS::compare(
   const GObject& cp
   , const Gem::Common::expectation& e
   , const double& limit
) const {
   using namespace Gem::Common;

   // Check that we are indeed dealing with a GBaseEA reference
   const GSerialPS *p_load = GObject::gobject_conversion<GSerialPS>(&cp);

   try {
      BEGIN_COMPARE;

      // Check our parent class'es data ...
      COMPARE_PARENT(GBasePS, cp, e, limit);

      // ... no local data

      END_COMPARE;

   } catch(g_expectation_violation& g) { // Create a suitable stack-trace
      throw g("g_expectation_violation caught by GSerialPS");
   }
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GSerialPS::name() const {
   return std::string("GSerialPS");
}

/******************************************************************************/
/**
 * Loads the data from another GSerialPS object.
 *
 * @param vp Pointer to another GSerialPS object, camouflaged as a GObject
 */
void GSerialPS::load_(const GObject *cp) {
   // Convert GObject pointer to local format
   // const GSerialPS *p_load = this->gobject_conversion<GSerialPS>(cp);
   // Uncomment the previous line and comment the following line if you wish to use local data
   GObject::selfAssignmentCheck<GSerialPS>(cp);

   // First load our parent class'es data ...
   GBasePS::load_(cp);

   // ... no local data
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, camouflaged as a GObject
 */
GObject *GSerialPS::clone_() const  {
   return new GSerialPS(*this);
}

/******************************************************************************/
/**
 * Necessary initialization work before the start of the optimization
 */
void GSerialPS::init() {
   // GSerialPS sees exactly the environment it would when called from its own class
   GBasePS::init();

   // Add local configuration code here
}

/******************************************************************************/
/**
 * Necessary clean-up work after the optimization has finished
 */
void GSerialPS::finalize() {
   // Add local finalization code here

   // GSerialPS sees exactly the environment it would when called from its own class
   GBasePS::finalize();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void GSerialPS::addConfigurationOptions (
   Gem::Common::GParserBuilder& gpb
) {
   // Call our parent class'es function
   GBasePS::addConfigurationOptions(gpb);

   // no local data
}

/******************************************************************************/
/**
 * Allows to assign a name to the role of this individual(-derivative). This is mostly important for the
 * GBrokerEA class which should prevent objects of its type from being stored as an individual in its population.
 * All other objects do not need to re-implement this function (unless they rely on the name for some reason).
 */
std::string GSerialPS::getIndividualCharacteristic() const {
   return std::string("GENEVA_SERIALOPTALG");
}

/******************************************************************************/
/**
 * Triggers fitness calculation of a number of individuals. This function can be overloaded to perform
 * fitness calculation in parallel, using threads or network communication.
 *
 * @param finalPos The position in the vector up to which the fitness calculation should be performed
 * @return The best fitness found amongst all parents
 */
void GSerialPS::runFitnessCalculation() {
   // Trigger value calculation for all individuals
   GSerialPS::iterator it;
   for(it=this->begin(); it!=this->end(); ++it) {
#ifdef DEBUG
      // Make sure the evaluated individuals have the dirty flag set
      if(!(*it)->isDirty()) {
         glogger
         << "In GSerialPS::runFitnessCalculation():" << std::endl
         << "In iteration " << this->getIteration() << ": Found individual in position " << std::distance(this->begin(), it) << " whose dirty flag isn't set" << std::endl
         << GEXCEPTION;
      }
#endif /* DEBUG */

      // Perform the actual evaluation
      (*it)->fitness(0, Gem::Geneva::ALLOWREEVALUATION, Gem::Geneva::USETRANSFORMEDFITNESS);
   }
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 */
bool GSerialPS::modify_GUnitTests() {
#ifdef GEM_TESTING
   bool result = false;

   // Call the parent class'es function
   if(GBasePS::modify_GUnitTests()) result = true;

   return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GSerialPS::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GSerialPS::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
   // Call the parent class'es function
   GBasePS::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GSerialPS::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GSerialPS::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
   // Call the parent class'es function
   GBasePS::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GSerialPS::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */


/******************************************************************************/

