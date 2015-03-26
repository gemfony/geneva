/**
 * @file GSwarmPersonalityTraits.cpp
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

#include "geneva/GSwarmPersonalityTraits.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GSwarmPersonalityTraits)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GSwarmPersonalityTraits::GSwarmPersonalityTraits()
   : GPersonalityTraits()
   , neighborhood_(0)
   , noPositionUpdate_(false)
   , personal_best_() // empty
   , personal_best_quality_(boost::make_tuple(0.,0.))
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy contructor
 *
 * @param cp A copy of another GSwarmPersonalityTraits object
 */
GSwarmPersonalityTraits::GSwarmPersonalityTraits(const GSwarmPersonalityTraits& cp)
	: GPersonalityTraits(cp)
	, neighborhood_(cp.neighborhood_)
	, noPositionUpdate_(cp.noPositionUpdate_)
	, personal_best_() // empty at this point
	, personal_best_quality_(cp.personal_best_quality_)
{
   // Copy the personal_best_ vector over
   copyGenevaSmartPointer(cp.personal_best_, personal_best_);
   // Make sure we do not get a "chain" of individuals
   if(personal_best_) {
      personal_best_->resetPersonality();
   }
}

/******************************************************************************/
/**
 * The standard destructor
 */
GSwarmPersonalityTraits::~GSwarmPersonalityTraits()
{ /* nothing */ }

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GSwarmPersonalityTraits& GSwarmPersonalityTraits::operator=(const GSwarmPersonalityTraits& cp) {
   this->load_(&cp);
   return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GSwarmPersonalityTraits object
 *
 * @param  cp A constant reference to another GSwarmPersonalityTraits object
 * @return A boolean indicating whether both objects are equal
 */
bool GSwarmPersonalityTraits::operator==(const GSwarmPersonalityTraits& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GSwarmPersonalityTraits::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GSwarmPersonalityTraits object
 *
 * @param  cp A constant reference to another GSwarmPersonalityTraits object
 * @return A boolean indicating whether both objects are inequal
 */
bool GSwarmPersonalityTraits::operator!=(const GSwarmPersonalityTraits& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GSwarmPersonalityTraits::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GSwarmPersonalityTraits::checkRelationshipWith(const GObject& cp,
   const Gem::Common::expectation& e,
   const double& limit,
   const std::string& caller,
   const std::string& y_name,
   const bool& withMessages
) const {
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GSwarmPersonalityTraits *p_load = GObject::gobject_conversion<GSwarmPersonalityTraits>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GPersonalityTraits::checkRelationshipWith(cp, e, limit, "GSwarmPersonalityTraits", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GSwarmPersonalityTraits", neighborhood_, p_load->neighborhood_, "neighborhood_", "p_load->neighborhood_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GSwarmPersonalityTraits", noPositionUpdate_, p_load->noPositionUpdate_, "noPositionUpdate_", "p_load->noPositionUpdate_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GSwarmPersonalityTraits", personal_best_, p_load->personal_best_, "personal_best_", "p_load->personal_best_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GSwarmPersonalityTraits", personal_best_quality_, p_load->personal_best_quality_, "personal_best_quality_", "p_load->personal_best_quality_", e , limit));

	return evaluateDiscrepancies("GEAPersonalityTraits", caller, deviations, e);
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GSwarmPersonalityTraits::name() const {
   return std::string("GSwarmPersonalityTraits");
}

/******************************************************************************/
/**
 * Sets the noPositionUpdate_ flag
 */
void GSwarmPersonalityTraits::setNoPositionUpdate() {
	noPositionUpdate_ = true;
}

/* ----------------------------------------------------------------------------------
 * Tested in GSwarmPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Retrieves the current value of the noPositionUpdate_ flag
 *
 * @return The current value of the noPositionUpdate_ flag
 */
bool GSwarmPersonalityTraits::noPositionUpdate() const {
	return noPositionUpdate_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GSwarmPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Retrieves and resets the current value of the noPositionUpdate_ flag
 *
 * @return The value of the noPositionUpdate_ flag when the function was called
 */
bool GSwarmPersonalityTraits::checkNoPositionUpdateAndReset() {
	bool current = noPositionUpdate_;
	if(noPositionUpdate_) noPositionUpdate_ = false;
	return current;
}

/* ----------------------------------------------------------------------------------
 * Tested in GSwarmPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Allows to add a new personal best to the individual. Note that this function
 * will internally clone the argument and extract the GParameterBase objects
 * and p's fitness, as this is all we need.
 *
 * @param p A pointer to the personally best parameter set
 */
void GSwarmPersonalityTraits::registerPersonalBest(boost::shared_ptr<GParameterSet> p) {
	// Some error checking
#ifdef DEBUG
	// Does it point anywhere ?
	if(!p) {
	   glogger
	   << "In GSwarmPersonalityTraits::registerPersonalBest():" << std::endl
      << "Got empty smart pointer." << std::endl
      << GEXCEPTION;
	}

	// Is the dirty flag set ?
	if(p->isDirty()) {
	   glogger
	   << "In GSwarmPersonalityTraits::registerPersonalBest():" << std::endl
      << "Got individual whose dirty flag is set." << std::endl
      << GEXCEPTION;
	}
#endif

   // Copy the personal_best_ vector over and make sure we do not get a "chain" of individuals
   copyGenevaSmartPointer(p, personal_best_);
   if(personal_best_) {
      personal_best_->resetPersonality();
   }

	personal_best_quality_ = p->getFitnessTuple();
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Allows to retrieve the personally best individual
 *
 * @return The personally best individual
 */
boost::shared_ptr<GParameterSet> GSwarmPersonalityTraits::getPersonalBest() const {
#ifdef DEBUG
   if(!personal_best_) {
      glogger
      << "In GSwarmPersonalityTraits::getPersonalBest(): Error!" << std::endl
      << "Tried to retrieve personal_best_ while pointer is empty" << std::endl
      << GEXCEPTION;
   }
#endif

	return personal_best_;
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Resets the personally best individual by assigning a default-constructed
 * parameter set.
 */
void GSwarmPersonalityTraits::resetPersonalBest() {
	personal_best_ = boost::shared_ptr<GParameterSet>(); // empty
	personal_best_quality_ = boost::make_tuple(0.,0.);
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Retrieve quality of personally best individual
 *
 * @return The fitness of the personally best individual
 */
boost::tuple<double, double> GSwarmPersonalityTraits::getPersonalBestQuality() const {
	return personal_best_quality_;
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A clone of this object, camouflaged as a GObject
 */
GObject* GSwarmPersonalityTraits::clone_() const {
	return new GSwarmPersonalityTraits(*this);
}

/******************************************************************************/
/**
 * Loads the data of another GSwarmPersonalityTraits object
 *
 * @param cp A copy of another GSwarmPersonalityTraits object, camouflaged as a GObject
 */
void GSwarmPersonalityTraits::load_(const GObject* cp) {
	const GSwarmPersonalityTraits *p_load = this->gobject_conversion<GSwarmPersonalityTraits>(cp);

	// Load the parent class'es data
	GPersonalityTraits::load_(cp);

	// and then the local data
	neighborhood_ = p_load->neighborhood_;
	noPositionUpdate_ = p_load->noPositionUpdate_;

   // Copy the personal_best_ vector over and make sure we do not get a "chain" of individuals
	copyGenevaSmartPointer(p_load->personal_best_, personal_best_);
	if(personal_best_) {
      personal_best_->resetPersonality();
	}

	personal_best_quality_ = p_load->personal_best_quality_;
}

/******************************************************************************/
/**
 * Specifies in which of the populations neighborhood the individual lives
 *
 * @param neighborhood The current neighborhood of this individual
 */
void GSwarmPersonalityTraits::setNeighborhood(const std::size_t& neighborhood) {
	neighborhood_ = neighborhood;
}

/******************************************************************************/
/**
 * Retrieves the position of the individual in the population
 *
 * @return The current position of this individual in the population
 */
std::size_t GSwarmPersonalityTraits::getNeighborhood(void) const {
	return neighborhood_;
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GSwarmPersonalityTraits::modify_GUnitTests() {
#ifdef GEM_TESTING
   bool result = false;

	// Call the parent class'es function
	if(GPersonalityTraits::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GSwarmPersonalityTraits::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GSwarmPersonalityTraits::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GPersonalityTraits::specificTestsNoFailureExpected_GUnitTests();

	//---------------------------------------------------------------------------

	{ // Test setting and retrieval of the noPositionUpdate_ flag
		boost::shared_ptr<GSwarmPersonalityTraits> p_test = this->clone<GSwarmPersonalityTraits>();

		// Check setting and retrieval
		BOOST_CHECK_NO_THROW(p_test->setNoPositionUpdate());
		BOOST_CHECK(p_test->noPositionUpdate() == true);

		// Check retrieval and reset
		bool noPositionUpdate = false; // This value should be changed by the following call
		BOOST_CHECK_NO_THROW(noPositionUpdate = p_test->checkNoPositionUpdateAndReset());
		BOOST_CHECK(noPositionUpdate == true);
		BOOST_CHECK(p_test->noPositionUpdate() == false);

		// Try again -- the value "false" should not change
		noPositionUpdate = true; // This value should be changed by the following call
		BOOST_CHECK_NO_THROW(noPositionUpdate = p_test->checkNoPositionUpdateAndReset());
		BOOST_CHECK(noPositionUpdate == false);
		BOOST_CHECK(p_test->noPositionUpdate() == false);
	}

	//---------------------------------------------------------------------------

	{ // Test setting and retrieval of the neighborhood
		boost::shared_ptr<GSwarmPersonalityTraits> p_test = this->clone<GSwarmPersonalityTraits>();

		// Setting and retrieval of the neighborhood
		for(std::size_t i=0; i<10; i++) {
			BOOST_CHECK_NO_THROW(p_test->setNeighborhood(i));
			BOOST_CHECK(p_test->getNeighborhood() == i);
		}
	}

	//---------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GSwarmPersonalityTraits::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GSwarmPersonalityTraits::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GPersonalityTraits::specificTestsFailuresExpected_GUnitTests();


	//---------------------------------------------------------------------------
	//---------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GSwarmPersonalityTraits::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
