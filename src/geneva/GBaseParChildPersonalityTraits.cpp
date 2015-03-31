/**
 * @file GBaseParChildPersonalityTraits.cpp
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

#include "geneva/GBaseParChildPersonalityTraits.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GBaseParChildPersonalityTraits)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GBaseParChildPersonalityTraits::GBaseParChildPersonalityTraits()
   : GPersonalityTraits()
   , parentCounter_(0)
   , popPos_(0)
   , parentId_(-1) // means "unset"
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy contructor
 *
 * @param cp A copy of another GBaseParChildPersonalityTraits object
 */
GBaseParChildPersonalityTraits::GBaseParChildPersonalityTraits(const GBaseParChildPersonalityTraits& cp)
   : GPersonalityTraits(cp)
   , parentCounter_(cp.parentCounter_)
   , popPos_(cp.popPos_)
   , parentId_(cp.parentId_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
GBaseParChildPersonalityTraits::~GBaseParChildPersonalityTraits()
{ /* nothing */ }

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GBaseParChildPersonalityTraits& GBaseParChildPersonalityTraits::operator=(
   const GBaseParChildPersonalityTraits& cp
) {
   this->load_(&cp);
   return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GBaseParChildPersonalityTraits object
 *
 * @param  cp A constant reference to another GBaseParChildPersonalityTraits object
 * @return A boolean indicating whether both objects are equal
 */
bool GBaseParChildPersonalityTraits::operator==(const GBaseParChildPersonalityTraits& cp) const {
   using namespace Gem::Common;
   // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
   return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GBaseParChildPersonalityTraits::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GBaseParChildPersonalityTraits object
 *
 * @param  cp A constant reference to another GBaseParChildPersonalityTraits object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBaseParChildPersonalityTraits::operator!=(const GBaseParChildPersonalityTraits& cp) const {
   using namespace Gem::Common;
   // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
   return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GBaseParChildPersonalityTraits::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GBaseParChildPersonalityTraits::checkRelationshipWith(
   const GObject& cp
   , const Gem::Common::expectation& e
   , const double& limit
   , const std::string& caller
   , const std::string& y_name
   , const bool& withMessages
) const {
    using namespace Gem::Common;

   // Check that we are indeed dealing with a GParamterBase reference
   const GBaseParChildPersonalityTraits *p_load = GObject::gobject_conversion<GBaseParChildPersonalityTraits>(&cp);

   // Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

   // Check our parent class'es data ...
   deviations.push_back(GPersonalityTraits::checkRelationshipWith(cp, e, limit, "GBaseParChildPersonalityTraits", y_name, withMessages));

   // ... and then our local data
   deviations.push_back(checkExpectation(withMessages, "GBaseParChildPersonalityTraits", parentCounter_, p_load->parentCounter_, "parentCounter_", "p_load->parentCounter_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GBaseParChildPersonalityTraits", popPos_, p_load->popPos_, "popPos_", "p_load->popPos_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GBaseParChildPersonalityTraits", parentId_, p_load->parentId_, "parentId_", "p_load->parentId_", e , limit));

   return evaluateDiscrepancies("GBaseParChildPersonalityTraits", caller, deviations, e);
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
void GBaseParChildPersonalityTraits::compare(
   const GObject& cp
   , const Gem::Common::expectation& e
   , const double& limit
) const {
   using namespace Gem::Common;

   // Check that we are indeed dealing with a GBaseEA reference
   const GBaseParChildPersonalityTraits *p_load = GObject::gobject_conversion<GBaseParChildPersonalityTraits>(&cp);

   try {
      // Check our parent class'es data ...
      GObject::compare(cp, e, limit);

      // ... and then our local data
      COMPARE(parentCounter_, p_load->parentCounter_, e, limit);
      COMPARE(popPos_, p_load->popPos_, e, limit);
      COMPARE(parentId_, p_load->parentId_, e, limit);

   } catch(g_expectation_violation& g) { // Create a suitable stack-trace
      g.add("g_expectation_violation caught by GBaseParChildPersonalityTraits");
      throw g;
   }
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GBaseParChildPersonalityTraits::name() const {
   return std::string("GBaseParChildPersonalityTraits");
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A clone of this object, camouflaged as a GObject
 */
GObject* GBaseParChildPersonalityTraits::clone_() const {
   return new GBaseParChildPersonalityTraits(*this);
}

/******************************************************************************/
/**
 * Loads the data of another GBaseParChildPersonalityTraits object
 *
 * @param cp A copy of another GBaseParChildPersonalityTraits object, camouflaged as a GObject
 */
void GBaseParChildPersonalityTraits::load_(const GObject* cp) {
   const GBaseParChildPersonalityTraits *p_load = gobject_conversion<GBaseParChildPersonalityTraits>(cp);

   // Load the parent class'es data
   GPersonalityTraits::load_(cp);

   // Then load our local data
   parentCounter_ = p_load->parentCounter_;
   popPos_ = p_load->popPos_;
   parentId_ = p_load->parentId_;
}

/******************************************************************************/
/**
 * Checks whether this is a parent individual
 *
 * @return A boolean indicating whether this object is a parent at this time
 */
bool GBaseParChildPersonalityTraits::isParent() const {
   return (parentCounter_>0)?true:false;
}

/* ----------------------------------------------------------------------------------
 * Tested in GBaseParChildPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Retrieves the current value of the parentCounter_ variable
 *
 * @return The current value of the parentCounter_ variable
 */
boost::uint32_t GBaseParChildPersonalityTraits::getParentCounter() const {
   return parentCounter_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GBaseParChildPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Marks an individual as a parent
 *
 * @return A boolean indicating whether this individual was previously a parent (true) or a child (false)
 */
bool GBaseParChildPersonalityTraits::setIsParent() {
   bool previous=(parentCounter_>0)?true:false;
   parentCounter_++;
   return previous;
}

/* ----------------------------------------------------------------------------------
 * Tested in GBaseParChildPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Marks an individual as a child
 *
 * @return A boolean indicating whether this individual was previously a parent (true) or a child (false)
 */
bool GBaseParChildPersonalityTraits::setIsChild() {
   bool previous=(parentCounter_>0)?true:false;
   parentCounter_=0;
   return previous;
}

/* ----------------------------------------------------------------------------------
 * Tested in GBaseParChildPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Sets the position of the individual in the population
 *
 * @param popPos The new position of this individual in the population
 */
void GBaseParChildPersonalityTraits::setPopulationPosition(const std::size_t& popPos) {
   popPos_ = popPos;
}

/* ----------------------------------------------------------------------------------
 * Tested in GBaseParChildPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Retrieves the position of the individual in the population
 *
 * @return The current position of this individual in the population
 */
std::size_t GBaseParChildPersonalityTraits::getPopulationPosition(void) const {
   return popPos_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GBaseParChildPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Stores the parent's id with this object.
 *
 * @param parentId The id of the individual's parent
 */
void GBaseParChildPersonalityTraits::setParentId(const std::size_t& parentId) {
   parentId_ = (boost::int16_t) parentId;
}

/* ----------------------------------------------------------------------------------
 * Tested in GBaseParChildPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Retrieves the parent id's value. Note that this function will throw if
 * no parent id has been set.
 *
 * @return The parent's id
 */
std::size_t GBaseParChildPersonalityTraits::getParentId() const {
   if(parentId_ >= 0) return parentId_;
   else {
      glogger
      << "In GBaseParChildPersonalityTraits::getParentId():" << std::endl
      << "parentId_ is unset" << std::endl
      << GEXCEPTION;
   }

   // Make the compiler happy
   return std::size_t(0);
}

/* ----------------------------------------------------------------------------------
 * Tested in GBaseParChildPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * Tested in GBaseParChildPersonalityTraits::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Checks whether a parent id has been set
 *
 * @return A boolean which indicates whether the parent id has been set
 */
bool GBaseParChildPersonalityTraits::parentIdSet() const {
   if(parentId_ >= 0) return true;
   else return false;
}

/* ----------------------------------------------------------------------------------
 * Tested in GBaseParChildPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Marks the parent id as unset
 */
void GBaseParChildPersonalityTraits::unsetParentId() {
   parentId_ = -1;
}

/* ----------------------------------------------------------------------------------
 * Tested in GBaseParChildPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * Tested in GBaseParChildPersonalityTraits::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBaseParChildPersonalityTraits::modify_GUnitTests() {
#ifdef GEM_TESTING
   bool result = false;

   // Call the parent class'es function
   if(GPersonalityTraits::modify_GUnitTests()) result = true;

   // A relatively harmless modification is a change of the parentCounter variable
   parentCounter_++;
   result = true;

   return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBaseParChildPersonalityTraits::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBaseParChildPersonalityTraits::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
   using boost::unit_test_framework::test_suite;
   using boost::unit_test_framework::test_case;

   // Call the parent class'es function
   GPersonalityTraits::specificTestsNoFailureExpected_GUnitTests();

   // --------------------------------------------------------------------------

   { // Check that it is possible to mark this as a parent or child-entity
      boost::shared_ptr<GBaseParChildPersonalityTraits> p_test = this->clone<GBaseParChildPersonalityTraits>();

      // Mark this object as belonging to a parent and check the correct setting
      BOOST_CHECK_NO_THROW(p_test->setIsParent());
      BOOST_CHECK(p_test->isParent() == true);

      // Mark this object as belonging to a child and check the correct setting
      BOOST_CHECK_NO_THROW(p_test->setIsChild());
      BOOST_CHECK(p_test->isParent() == false);
   }

   // --------------------------------------------------------------------------


   { // Check that the parent counter is incremented or reset correctly
      boost::shared_ptr<GBaseParChildPersonalityTraits> p_test = this->clone<GBaseParChildPersonalityTraits>();

      // Mark this object as belonging to a child and check the correct setting
      BOOST_CHECK_NO_THROW(p_test->setIsChild());
      BOOST_CHECK(p_test->isParent() == false);

      // Check that the parent counter is now 0
      BOOST_CHECK(p_test->getParentCounter() == 0);

      // Mark the individual as a parent a number of times and check the parent counter
      for(boost::uint32_t i=1; i<=10; i++) {
         BOOST_CHECK_NO_THROW(p_test->setIsParent());
         BOOST_CHECK(p_test->getParentCounter() == i);
      }

      // Mark the individual as a child and check the parent counter again
      BOOST_CHECK_NO_THROW(p_test->setIsChild());
      BOOST_CHECK(p_test->isParent() == false);

      // Check that the parent counter is now 0
      BOOST_CHECK(p_test->getParentCounter() == 0);
   }

   // --------------------------------------------------------------------------

   { // Check setting and retrieval of the individual's position in the population
      boost::shared_ptr<GBaseParChildPersonalityTraits> p_test = this->clone<GBaseParChildPersonalityTraits>();

      for(std::size_t i=0; i<10; i++) {
         BOOST_CHECK_NO_THROW(p_test->setPopulationPosition(i));
         BOOST_CHECK(p_test->getPopulationPosition() == i);
      }
   }

   // --------------------------------------------------------------------------

   { // Test setting and retrieval of valid parent ids
      boost::shared_ptr<GBaseParChildPersonalityTraits> p_test = this->clone<GBaseParChildPersonalityTraits>();

      for(std::size_t i=0; i<10; i++) {
         BOOST_CHECK_NO_THROW(p_test->setParentId(i));
         BOOST_CHECK(p_test->getParentId() == i);
         BOOST_CHECK(p_test->parentIdSet() == true);
         BOOST_CHECK_NO_THROW(p_test->unsetParentId());
         BOOST_CHECK(p_test->parentIdSet() == false);
      }
   }

   // --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBaseParChildPersonalityTraits::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBaseParChildPersonalityTraits::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
   using boost::unit_test_framework::test_suite;
   using boost::unit_test_framework::test_case;

   // Call the parent class'es function
   GPersonalityTraits::specificTestsFailuresExpected_GUnitTests();

   // --------------------------------------------------------------------------

   { // Test that retrieval of the parent id throws, if the id isn't set
      boost::shared_ptr<GBaseParChildPersonalityTraits> p_test = this->clone<GBaseParChildPersonalityTraits>();

      BOOST_CHECK_NO_THROW(p_test->unsetParentId());
      BOOST_CHECK_THROW(p_test->getParentId(), Gem::Common::gemfony_error_condition);
   }

   // --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBaseParChildPersonalityTraits::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
