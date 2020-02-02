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

#include "geneva/G_OptimizationAlgorithm_SwarmAlgorithm_PersonalityTraits.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GSwarmAlgorithm_PersonalityTraits) // NOLINT

namespace Gem {
namespace Geneva {

/******************************************************************************/
/** A short identifier suitable for storage in a std::map */
G_API_GENEVA const std::string GSwarmAlgorithm_PersonalityTraits::nickname = "swarm"; // NOLINT

/******************************************************************************/
/**
 * The copy contructor
 *
 * @param cp A copy of another GSwarmPersonalityTraits object
 */
GSwarmAlgorithm_PersonalityTraits::GSwarmAlgorithm_PersonalityTraits(const GSwarmAlgorithm_PersonalityTraits &cp)
	: GPersonalityTraits(cp)
	  , neighborhood_(cp.neighborhood_)
	  , noPositionUpdate_(cp.noPositionUpdate_)
	  , personal_best_() // empty at this point
	  , personal_best_quality_(cp.personal_best_quality_)
{
	// Copy the personal_best_ vector over
	Gem::Common::copyCloneableSmartPointer(cp.personal_best_, personal_best_);
	// Make sure we do not get a "chain" of individuals
	if (personal_best_) {
		personal_best_->resetPersonality();
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
void GSwarmAlgorithm_PersonalityTraits::compare_(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GSwarmAlgorithm_PersonalityTraits reference independent of this object and convert the pointer
	const GSwarmAlgorithm_PersonalityTraits *p_load = Gem::Common::g_convert_and_compare<GObject, GSwarmAlgorithm_PersonalityTraits>(cp, this);

	GToken token("GSwarmAlgorithm_PersonalityTraits", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<GPersonalityTraits>(*this, *p_load, token);

	// ... and then the local data
	compare_t(IDENTITY(neighborhood_, p_load->neighborhood_), token);
	compare_t(IDENTITY(noPositionUpdate_, p_load->noPositionUpdate_), token);
	compare_t(IDENTITY(personal_best_, p_load->personal_best_), token);
	compare_t(IDENTITY(personal_best_quality_, p_load->personal_best_quality_), token);


	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GSwarmAlgorithm_PersonalityTraits::name_() const {
	return std::string("GSwarmAlgorithm_PersonalityTraits");
}

/******************************************************************************/
/**
 * Retrieves the mnemonic of the optimization algorithm
 */
std::string GSwarmAlgorithm_PersonalityTraits::getMnemonic() const {
	return GSwarmAlgorithm_PersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * Sets the noPositionUpdate_ flag
 */
void GSwarmAlgorithm_PersonalityTraits::setNoPositionUpdate() {
	noPositionUpdate_ = true;
}

/* ----------------------------------------------------------------------------------
 * Tested in GSwarmAlgorithm_PersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Retrieves the current value of the noPositionUpdate_ flag
 *
 * @return The current value of the noPositionUpdate_ flag
 */
bool GSwarmAlgorithm_PersonalityTraits::noPositionUpdate() const {
	return noPositionUpdate_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GSwarmAlgorithm_PersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Retrieves and resets the current value of the noPositionUpdate_ flag
 *
 * @return The value of the noPositionUpdate_ flag when the function was called
 */
bool GSwarmAlgorithm_PersonalityTraits::checkNoPositionUpdateAndReset() {
	bool current = noPositionUpdate_;
	if (noPositionUpdate_) noPositionUpdate_ = false;
	return current;
}

/* ----------------------------------------------------------------------------------
 * Tested in GSwarmAlgorithm_PersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
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
void GSwarmAlgorithm_PersonalityTraits::registerPersonalBest(std::shared_ptr<GParameterSet> p) {
	// Some error checking
#ifdef DEBUG
	// Does it point anywhere ?
	if(not p) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSwarmAlgorithm_PersonalityTraits::registerPersonalBest():" << std::endl
				<< "Got empty smart pointer." << std::endl
		);
	}

	// Is the dirty flag set ?
	if(not p->is_processed()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSwarmAlgorithm_PersonalityTraits::registerPersonalBest():" << std::endl
				<< "Got individual which isn't processed." << std::endl
		);
	}
#endif

	// Copy the personal_best_ vector over and make sure we do not get a "chain" of individuals
	Gem::Common::copyCloneableSmartPointer(p, personal_best_);
	if (personal_best_) {
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
std::shared_ptr <GParameterSet> GSwarmAlgorithm_PersonalityTraits::getPersonalBest() const {
#ifdef DEBUG
	if(not personal_best_) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GSwarmAlgorithm_PersonalityTraits::getPersonalBest(): Error!" << std::endl
				<< "Tried to retrieve personal_best_ while pointer is empty" << std::endl
		);
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
void GSwarmAlgorithm_PersonalityTraits::resetPersonalBest() {
	personal_best_ = std::shared_ptr<GParameterSet>(); // empty
	personal_best_quality_ = std::make_tuple(0., 0.);
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
std::tuple<double, double> GSwarmAlgorithm_PersonalityTraits::getPersonalBestQuality() const {
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
GObject *GSwarmAlgorithm_PersonalityTraits::clone_() const {
	return new GSwarmAlgorithm_PersonalityTraits(*this);
}

/******************************************************************************/
/**
 * Loads the data of another GSwarmPersonalityTraits object
 *
 * @param cp A copy of another GSwarmPersonalityTraits object, camouflaged as a GObject
 */
void GSwarmAlgorithm_PersonalityTraits::load_(const GObject *cp) {
	// Check that we are dealing with a GSwarmAlgorithm_PersonalityTraits reference independent of this object and convert the pointer
	const GSwarmAlgorithm_PersonalityTraits *p_load = Gem::Common::g_convert_and_compare<GObject, GSwarmAlgorithm_PersonalityTraits>(cp, this);

	// Load the parent class'es data
	GPersonalityTraits::load_(cp);

	// and then the local data
	neighborhood_ = p_load->neighborhood_;
	noPositionUpdate_ = p_load->noPositionUpdate_;

	// Copy the personal_best_ vector over and make sure we do not get a "chain" of individuals
	Gem::Common::copyCloneableSmartPointer(p_load->personal_best_, personal_best_);
	if (personal_best_) {
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
void GSwarmAlgorithm_PersonalityTraits::setNeighborhood(const std::size_t &neighborhood) {
	neighborhood_ = neighborhood;
}

/******************************************************************************/
/**
 * Retrieves the position of the individual in the population
 *
 * @return The current position of this individual in the population
 */
std::size_t GSwarmAlgorithm_PersonalityTraits::getNeighborhood() const {
	return neighborhood_;
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GSwarmAlgorithm_PersonalityTraits::modify_GUnitTests_() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if (GPersonalityTraits::modify_GUnitTests_()) result = true;

	this->setNeighborhood(this->getNeighborhood() + 1);
	result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GSwarmAlgorithm_PersonalityTraits::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GSwarmAlgorithm_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GPersonalityTraits::specificTestsNoFailureExpected_GUnitTests_();

	//---------------------------------------------------------------------------

	{ // Test setting and retrieval of the noPositionUpdate_ flag
		std::shared_ptr <GSwarmAlgorithm_PersonalityTraits> p_test = this->clone<GSwarmAlgorithm_PersonalityTraits>();

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
		std::shared_ptr <GSwarmAlgorithm_PersonalityTraits> p_test = this->clone<GSwarmAlgorithm_PersonalityTraits>();

		// Setting and retrieval of the neighborhood
		for (std::size_t i = 0; i < 10; i++) {
			BOOST_CHECK_NO_THROW(p_test->setNeighborhood(i));
			BOOST_CHECK(p_test->getNeighborhood() == i);
		}
	}

	//---------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GSwarmAlgorithm_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GSwarmAlgorithm_PersonalityTraits::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GPersonalityTraits::specificTestsFailuresExpected_GUnitTests_();


	//---------------------------------------------------------------------------
	//---------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GSwarmAlgorithm_PersonalityTraits::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
