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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <cstddef>
#include <tuple>

// Geneva headers go here
#include "geneva/GPersonalityTraits.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/**
 * The shared intermediate base of every personality-traits class whose only per-algorithm state is
 * the individual's POSITION in the population (its slot / vertex / particle / ant index). It owns the
 * position member, its accessors, the serialization, the load_()/compare_() machinery and the
 * GEM_TESTING bodies -- stated ONCE; a concrete traits class on top supplies only its identity
 * (nickname / getMnemonic() / name_() / clone_() and its serialization export). Before this base
 * existed, six algorithm traits classes carried near-byte-identical copies of all of it, and the
 * parent-child traits a seventh.
 *
 * The class is abstract (name_()/clone_() stay pure); load_() and compare_() enforce an EXACT
 * dynamic-type match, so two DIFFERENT algorithms' position traits never load from or compare equal
 * to one another merely because they share this base.
 */
class GPositionPersonalityTraits // NOLINT(cppcoreguidelines-special-member-functions)
  : public GPersonalityTraits {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GPersonalityTraits) &
            BOOST_SERIALIZATION_NVP(pop_pos_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GPositionPersonalityTraits() = default;
    /** @brief The copy constructor
     *  @param cp Another GPositionPersonalityTraits object whose state is copied */
    GPositionPersonalityTraits(const GPositionPersonalityTraits &cp) = default;
    /** @brief The standard destructor */
    ~GPositionPersonalityTraits() override = default;

    /**
     * @brief Sets the position of the individual in the population.
     * @param pop_pos The individual's position in the population
     */
    void setPopulationPosition(std::size_t pop_pos) { pop_pos_ = pop_pos; }
    /**
     * @brief Retrieves the position of the individual in the population.
     * @return The individual's stored position in the population
     */
    std::size_t getPopulationPosition() const { return pop_pos_; }

protected:
    /***************************************************************************/
    /** @brief Single declaration of this class'es local data members */
    template <typename Self>
    auto localMembers_(this Self &self) {
        return std::make_tuple(Gem::Common::make_member("pop_pos_", self.pop_pos_));
    }

    /**
     * @brief Loads the data of another GPositionPersonalityTraits object. Enforces an exact
     * dynamic-type match, so loading across two different algorithms' traits types throws.
     * @param cp A pointer to another object of the SAME concrete traits type (as a GPersonalityTraits)
     */
    void load_(const GPersonalityTraits *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GPositionPersonalityTraits>(
        GPositionPersonalityTraits const &,
        GPositionPersonalityTraits const &,
        Gem::Common::GToken &
    );

    /**
     * @brief Searches for compliance with expectations with respect to another object of the SAME
     * concrete traits type (an exact dynamic-type match is enforced).
     * @param cp The other object (a GPersonalityTraits) to compare against
     * @param e The expectation for this comparison, e.g. equality
     * @param limit The limit for allowed deviations of floating point types
     */
    void compare_(
        const GPersonalityTraits &cp,
        const Gem::Common::expectation &e,
        const double &limit
    ) const override;

    /**
     * @brief Applies modifications to this object. This is needed for testing purposes.
     * @return true if the object was modified, false otherwise
     */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

    /***************************************************************************/

private:
    /** @brief Stores the current position in the population */
    std::size_t pop_pos_ = 0;
};

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::OptimizationAlgorithms::GPositionPersonalityTraits) // NOLINT
