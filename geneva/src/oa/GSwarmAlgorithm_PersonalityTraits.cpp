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
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include "geneva/oa/GSwarmAlgorithm_PersonalityTraits.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include <cstddef>
#include <memory>
#include <tuple>

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::OptimizationAlgorithms::GSwarmAlgorithm_PersonalityTraits) // NOLINT

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/** A short identifier suitable for storage in a std::map */
const std::string GSwarmAlgorithm_PersonalityTraits::nickname = "swarm"; // NOLINT

/******************************************************************************/
/**
 * @brief The copy constructor
 *
 * @param cp A constant reference to another GSwarmAlgorithm_PersonalityTraits object to be copied
 */
GSwarmAlgorithm_PersonalityTraits::GSwarmAlgorithm_PersonalityTraits(
    const GSwarmAlgorithm_PersonalityTraits &cp
)
  : GPersonalityTraits(cp)
  , neighborhood_(cp.neighborhood_)
  , no_position_update_(cp.no_position_update_)
  , personal_best_quality_(cp.personal_best_quality_) {
    // Copy the personal_best_ vector over. The stored best is a bare individual carrying no personality
    // (the personality lives on the population slot), so there is no "chain" of individuals to break.
    Gem::Common::copyCloneableSmartPointer(cp.personal_best_, personal_best_);
}

/******************************************************************************/
/**
 * @brief Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GPersonalityTraits object to compare against
 * @param e The expected outcome of the comparison (e.g. equality or inequality)
 * @param limit The maximum allowed deviation for floating point comparisons (unused here)
 */
void GSwarmAlgorithm_PersonalityTraits::compare_(
    const GPersonalityTraits &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GSwarmAlgorithm_PersonalityTraits reference independent of this object and convert the pointer
    const GSwarmAlgorithm_PersonalityTraits *p_load =
        Gem::Common::g_convert_and_compare<GPersonalityTraits, GSwarmAlgorithm_PersonalityTraits>(cp, this);

    GToken token("GSwarmAlgorithm_PersonalityTraits", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GPersonalityTraits>(*this, *p_load, token);

    // ... then the unconditionally-handled local data, derived from the single
    // localMembers() declaration ...
    Gem::Common::g_compare_members(localMembers_(*this), localMembers_(*p_load), token);

    // ... and finally the manual tail: personal_best_ (see localMembers() docs).
    compare_t(Gem::Common::getIdentity(personal_best_, p_load->personal_best_, "personal_best_", "p_load->personal_best_"), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * @brief Emits a name for this class / object
 *
 * @return The name of this class ("GSwarmAlgorithm_PersonalityTraits")
 */
std::string GSwarmAlgorithm_PersonalityTraits::name_() const {
    return std::string("GSwarmAlgorithm_PersonalityTraits");
}

/******************************************************************************/
/**
 * @brief Retrieves the mnemonic of the optimization algorithm
 *
 * @return The mnemonic ("swarm") associated with this personality
 */
std::string GSwarmAlgorithm_PersonalityTraits::getMnemonic() const {
    return GSwarmAlgorithm_PersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * @brief Sets the no_position_update_ flag to true
 */
void GSwarmAlgorithm_PersonalityTraits::setNoPositionUpdate() {
    no_position_update_ = true;
}

/* ----------------------------------------------------------------------------------
 * Tested in GSwarmAlgorithm_PersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * @brief Retrieves the current value of the no_position_update_ flag
 *
 * @return The current value of the no_position_update_ flag
 */
bool GSwarmAlgorithm_PersonalityTraits::noPositionUpdate() const {
    return no_position_update_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GSwarmAlgorithm_PersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * @brief Retrieves and resets the current value of the no_position_update_ flag
 *
 * @return The value of the no_position_update_ flag at the time the function was called
 */
bool GSwarmAlgorithm_PersonalityTraits::checkNoPositionUpdateAndReset() {
    bool current = no_position_update_;
    if(no_position_update_) {
        no_position_update_ = false;
    }
    return current;
}

/* ----------------------------------------------------------------------------------
 * Tested in GSwarmAlgorithm_PersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * @brief Allows to add a new personal best to the individual. Note that this function
 * will internally clone the argument and store it together with p's fitness, as this is
 * all we need.
 *
 * @param p A shared pointer to the personally best individual to be registered
 */
void GSwarmAlgorithm_PersonalityTraits::registerPersonalBest(std::shared_ptr<gen::GOptimizableEntity> p) {
    // Some error checking
#ifdef DEBUG
    // Does it point anywhere ?
    if(not p) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GSwarmAlgorithm_PersonalityTraits::registerPersonalBest():" << '\n'
            << "Got empty smart pointer." << '\n'
        );
    }

    // Is the fitness current ?
    if(not p->fitnessIsCurrent()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GSwarmAlgorithm_PersonalityTraits::registerPersonalBest():" << '\n'
            << "Got individual whose fitness is not current." << '\n'
        );
    }
#endif

    // Copy the personal_best_ vector over. The stored best is a bare individual carrying no personality
    // (it lives on the population slot), so there is no "chain" of individuals to break.
    Gem::Common::copyCloneableSmartPointer(p, personal_best_);

    personal_best_quality_ = p->getFitnessTuple();
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * @brief Allows to retrieve the personally best individual
 *
 * @return A shared pointer to the personally best individual
 */
std::shared_ptr<gen::GOptimizableEntity> GSwarmAlgorithm_PersonalityTraits::getPersonalBest() const {
#ifdef DEBUG
    if(not personal_best_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GSwarmAlgorithm_PersonalityTraits::getPersonalBest(): Error!" << '\n'
            << "Tried to retrieve personal_best_ while pointer is empty" << '\n'
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
 * @brief Resets the personally best individual by clearing the stored pointer and
 * resetting the stored quality tuple to (0., 0.).
 */
void GSwarmAlgorithm_PersonalityTraits::resetPersonalBest() {
    personal_best_ = std::shared_ptr<gen::GOptimizableEntity>(); // empty
    personal_best_quality_ = std::make_tuple(0., 0.);
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * @brief Retrieve quality of personally best individual
 *
 * @return A tuple holding the raw and transformed fitness of the personally best individual
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
 * @brief Creates a deep clone of this object
 *
 * @return A clone of this object, camouflaged as a GPersonalityTraits
 */
GPersonalityTraits *GSwarmAlgorithm_PersonalityTraits::clone_() const {
    return new GSwarmAlgorithm_PersonalityTraits(*this);
}

/******************************************************************************/
/**
 * @brief Loads the data of another GSwarmAlgorithm_PersonalityTraits object
 *
 * @param cp A pointer to another GSwarmAlgorithm_PersonalityTraits object, camouflaged as a GPersonalityTraits
 */
void GSwarmAlgorithm_PersonalityTraits::load_(const GPersonalityTraits *cp) {
    // Check that we are dealing with a GSwarmAlgorithm_PersonalityTraits reference independent of this object and convert the pointer
    const GSwarmAlgorithm_PersonalityTraits *p_load =
        Gem::Common::g_convert_and_compare<GPersonalityTraits, GSwarmAlgorithm_PersonalityTraits>(cp, this);

    // Load the parent class'es data
    GPersonalityTraits::load_(cp);

    // and then the unconditionally-handled local data, derived from the single
    // localMembers() declaration
    Gem::Common::g_load_members(localMembers_(*this), localMembers_(*p_load));

    // Manual tail: copy the personal_best_ over (see localMembers() docs). The stored best is a bare
    // individual carrying no personality (it lives on the population slot), so there is no "chain".
    Gem::Common::copyCloneableSmartPointer(p_load->personal_best_, personal_best_);
}

/******************************************************************************/
/**
 * @brief Specifies in which of the population's neighborhoods the individual lives
 *
 * @param neighborhood The index of the neighborhood this individual is assigned to
 */
void GSwarmAlgorithm_PersonalityTraits::setNeighborhood(const std::size_t &neighborhood) {
    neighborhood_ = neighborhood;
}

/******************************************************************************/
/**
 * @brief Retrieves the neighborhood index of the individual in the population
 *
 * @return The index of the neighborhood this individual is assigned to
 */
std::size_t GSwarmAlgorithm_PersonalityTraits::getNeighborhood() const {
    return neighborhood_;
}

/******************************************************************************/
/**
 * @brief Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GSwarmAlgorithm_PersonalityTraits::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    // Call the parent class'es function
    if(GPersonalityTraits::modify_GUnitTests_()) {
        result = true;
    }

    this->setNeighborhood(this->getNeighborhood() + 1);
    result = true;

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GSwarmAlgorithm_PersonalityTraits::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GSwarmAlgorithm_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent class'es function
    GPersonalityTraits::specificTestsNoFailureExpected_GUnitTests_();

    //---------------------------------------------------------------------------

    { // Test setting and retrieval of the no_position_update_ flag
        std::shared_ptr<GSwarmAlgorithm_PersonalityTraits> p_test =
            this->clone<GSwarmAlgorithm_PersonalityTraits>();

        // Check setting and retrieval
        CHECK_NOTHROW(p_test->setNoPositionUpdate());
        CHECK(p_test->noPositionUpdate() == true);

        // Check retrieval and reset
        bool no_position_update = false; // This value should be changed by the following call
        CHECK_NOTHROW(no_position_update = p_test->checkNoPositionUpdateAndReset());
        CHECK(no_position_update == true);
        CHECK(p_test->noPositionUpdate() == false);

        // Try again -- the value "false" should not change
        no_position_update = true; // This value should be changed by the following call
        CHECK_NOTHROW(no_position_update = p_test->checkNoPositionUpdateAndReset());
        CHECK(no_position_update == false);
        CHECK(p_test->noPositionUpdate() == false);
    }

    //---------------------------------------------------------------------------

    { // Test setting and retrieval of the neighborhood
        std::shared_ptr<GSwarmAlgorithm_PersonalityTraits> p_test =
            this->clone<GSwarmAlgorithm_PersonalityTraits>();

        // Setting and retrieval of the neighborhood
        for(std::size_t i = 0; i < 10; i++) {
            CHECK_NOTHROW(p_test->setNeighborhood(i));
            CHECK(p_test->getNeighborhood() == i);
        }
    }

    //---------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GSwarmAlgorithm_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GSwarmAlgorithm_PersonalityTraits::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent class'es function
    GPersonalityTraits::specificTestsFailuresExpected_GUnitTests_();

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GSwarmAlgorithm_PersonalityTraits::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
