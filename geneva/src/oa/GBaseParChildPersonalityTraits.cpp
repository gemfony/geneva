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

#include "geneva/oa/GBaseParChildPersonalityTraits.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::OptimizationAlgorithms::GBaseParChildPersonalityTraits) // NOLINT
namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/**
 * @brief Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GPersonalityTraits object to compare against
 * @param e The expected outcome of the comparison (e.g. equality or inequality)
 * @param limit The maximum acceptable deviation for floating-point comparisons (unused here)
 */
void GBaseParChildPersonalityTraits::compare_(
    const GPersonalityTraits &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GBasePS::GBaseParChildPersonalityTraits reference independent of this object and convert the pointer
    const GBaseParChildPersonalityTraits *p_load =
        Gem::Common::g_convert_and_compare<GPersonalityTraits, GBaseParChildPersonalityTraits>(cp, this);

    GToken token("GBaseParChildPersonalityTraits", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GPositionPersonalityTraits>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    g_compare_members(this->localMembers_(), p_load->localMembers_(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * @brief Emits a name for this class / object
 *
 * @return The class name as a string ("GBaseParChildPersonalityTraits")
 */
std::string GBaseParChildPersonalityTraits::name_() const {
    return std::string("GBaseParChildPersonalityTraits");
}

/******************************************************************************/
/**
 * @brief Retrieves the mnemonic of the optimization algorithm
 *
 * This base class has no associated optimization algorithm of its own, so the
 * function always throws and never returns normally.
 *
 * @return Never returns normally; always throws a geneva_exception
 */
std::string GBaseParChildPersonalityTraits::getMnemonic() const {
    throw geneva_exception(
        g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
        << "In GBaseParChildPersonalityTraits::getMnemonic(): Error!" << '\n'
        << "This function should never have been called" << '\n'
    );

    return "none";
}

/******************************************************************************/
/**
 * @brief Creates a deep clone of this object
 *
 * @return A heap-allocated clone of this object, camouflaged as a GPersonalityTraits pointer (caller owns it)
 */
GPersonalityTraits *GBaseParChildPersonalityTraits::clone_() const {
    return new GBaseParChildPersonalityTraits(*this);
}

/******************************************************************************/
/**
 * @brief Loads the data of another GBaseParChildPersonalityTraits object
 *
 * @param cp A pointer to another GBaseParChildPersonalityTraits object, camouflaged as a GPersonalityTraits (not taken over)
 */
void GBaseParChildPersonalityTraits::load_(const GPersonalityTraits *cp) {
    // Check that we are dealing with a GBasePS::GBaseParChildPersonalityTraits reference independent of this object and convert the pointer
    const GBaseParChildPersonalityTraits *p_load =
        Gem::Common::g_convert_and_compare<GPersonalityTraits, GBaseParChildPersonalityTraits>(cp, this);

    // Load the parent class'es data
    GPositionPersonalityTraits::load_(cp);

    // Then load our local data, derived from the single localMembers() declaration
    Gem::Common::g_load_members(this->localMembers_(), p_load->localMembers_());
}

/******************************************************************************/
/**
 * @brief Checks whether this is a parent individual
 *
 * @return true if this object is currently a parent (parent_counter_ > 0), false otherwise
 */
bool GBaseParChildPersonalityTraits::isParent() const {
    return parent_counter_ > 0;
}

/* ----------------------------------------------------------------------------------
 * Tested in GBaseParChildPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * @brief Retrieves the current value of the parent_counter_ variable
 *
 * @return The current value of the parent_counter_ variable (the number of generations spent as a parent)
 */
std::uint32_t GBaseParChildPersonalityTraits::getParentCounter() const {
    return parent_counter_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GBaseParChildPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * @brief Marks an individual as a parent
 *
 * Increments the parent counter, so repeated calls track how long the individual has been a parent.
 *
 * @return true if this individual was already a parent before the call, false if it was a child
 */
bool GBaseParChildPersonalityTraits::setIsParent() {
    bool previous = parent_counter_ > 0;
    parent_counter_++;
    return previous;
}

/* ----------------------------------------------------------------------------------
 * Tested in GBaseParChildPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * @brief Marks an individual as a child
 *
 * Resets the parent counter to zero.
 *
 * @return true if this individual was previously a parent, false if it was already a child
 */
bool GBaseParChildPersonalityTraits::setIsChild() {
    bool previous = parent_counter_ > 0;
    parent_counter_ = 0;
    return previous;
}

/* ----------------------------------------------------------------------------------
 * Tested in GBaseParChildPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * @brief Stores the parent's id with this object.
 *
 * @param parent_id The id (population position) of the individual's parent; stored internally as a std::int16_t
 */
void GBaseParChildPersonalityTraits::setParentId(const std::size_t &parent_id) {
    parent_id_ = static_cast<std::int16_t>(parent_id);
}

/* ----------------------------------------------------------------------------------
 * Tested in GBaseParChildPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * @brief Retrieves the parent id's value. Note that this function will throw if
 * no parent id has been set.
 *
 * @return The parent's id; throws a geneva_exception if the parent id is unset
 */
std::size_t GBaseParChildPersonalityTraits::getParentId() const {
    if(parent_id_ >= 0) {
        return parent_id_;
    }
            throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GBaseParChildPersonalityTraits::getParentId():" << '\n'
            << "parent_id_ is unset" << '\n'
        );
   

    // Make the compiler happy
    return static_cast<std::size_t>(0);
}

/* ----------------------------------------------------------------------------------
 * Tested in GBaseParChildPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * Tested in GBaseParChildPersonalityTraits::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * @brief Checks whether a parent id has been set
 *
 * @return true if a parent id has been set (parent_id_ >= 0), false otherwise
 */
bool GBaseParChildPersonalityTraits::parentIdSet() const {
    return parent_id_ >= 0;
   
}

/* ----------------------------------------------------------------------------------
 * Tested in GBaseParChildPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * @brief Marks the parent id as unset (sets parent_id_ to -1)
 */
void GBaseParChildPersonalityTraits::unsetParentId() {
    parent_id_ = -1;
}

/* ----------------------------------------------------------------------------------
 * Tested in GBaseParChildPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * Tested in GBaseParChildPersonalityTraits::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * @brief Applies modifications to this object. This is needed for testing purposes
 *
 * @return true if any modifications were made, false otherwise
 */
bool GBaseParChildPersonalityTraits::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    // Call the parent class'es function
    if(GPositionPersonalityTraits::modify_GUnitTests_()) {
        result = true;
    }

    // A relatively harmless modification is a change of the parentCounter variable
    parent_counter_++;
    result = true;

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GBaseParChildPersonalityTraits::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBaseParChildPersonalityTraits::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent class'es function
    GPositionPersonalityTraits::specificTestsNoFailureExpected_GUnitTests_();

    // --------------------------------------------------------------------------

    { // Check that it is possible to mark this as a parent or child-entity
        std::shared_ptr<GBaseParChildPersonalityTraits> p_test =
            this->clone<GBaseParChildPersonalityTraits>();

        // Mark this object as belonging to a parent and check the correct setting
        CHECK_NOTHROW(p_test->setIsParent());
        CHECK(p_test->isParent() == true);

        // Mark this object as belonging to a child and check the correct setting
        CHECK_NOTHROW(p_test->setIsChild());
        CHECK(p_test->isParent() == false);
    }

    // --------------------------------------------------------------------------

    { // Check that the parent counter is incremented or reset correctly
        std::shared_ptr<GBaseParChildPersonalityTraits> p_test =
            this->clone<GBaseParChildPersonalityTraits>();

        // Mark this object as belonging to a child and check the correct setting
        CHECK_NOTHROW(p_test->setIsChild());
        CHECK(p_test->isParent() == false);

        // Check that the parent counter is now 0
        CHECK(p_test->getParentCounter() == 0);

        // Mark the individual as a parent a number of times and check the parent counter
        for(std::uint32_t i = 1; i <= 10; i++) {
            CHECK_NOTHROW(p_test->setIsParent());
            CHECK(p_test->getParentCounter() == i);
        }

        // Mark the individual as a child and check the parent counter again
        CHECK_NOTHROW(p_test->setIsChild());
        CHECK(p_test->isParent() == false);

        // Check that the parent counter is now 0
        CHECK(p_test->getParentCounter() == 0);
    }

    // --------------------------------------------------------------------------

    { // Check setting and retrieval of the individual's position in the population
        std::shared_ptr<GBaseParChildPersonalityTraits> p_test =
            this->clone<GBaseParChildPersonalityTraits>();

        for(std::size_t i = 0; i < 10; i++) {
            CHECK_NOTHROW(p_test->setPopulationPosition(i));
            CHECK(p_test->getPopulationPosition() == i);
        }
    }

    // --------------------------------------------------------------------------

    { // Test setting and retrieval of valid parent ids
        std::shared_ptr<GBaseParChildPersonalityTraits> p_test =
            this->clone<GBaseParChildPersonalityTraits>();

        for(std::size_t i = 0; i < 10; i++) {
            CHECK_NOTHROW(p_test->setParentId(i));
            CHECK(p_test->getParentId() == i);
            CHECK(p_test->parentIdSet() == true);
            CHECK_NOTHROW(p_test->unsetParentId());
            CHECK(p_test->parentIdSet() == false);
        }
    }

    // --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GBaseParChildPersonalityTraits::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBaseParChildPersonalityTraits::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent class'es function
    GPositionPersonalityTraits::specificTestsFailuresExpected_GUnitTests_();

    // --------------------------------------------------------------------------

    { // Test that retrieval of the parent id throws, if the id isn't set
        std::shared_ptr<GBaseParChildPersonalityTraits> p_test =
            this->clone<GBaseParChildPersonalityTraits>();

        CHECK_NOTHROW(p_test->unsetParentId());
        CHECK_THROWS_AS((p_test->getParentId()), geneva_exception);
    }

    // --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GBaseParChildPersonalityTraits::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
