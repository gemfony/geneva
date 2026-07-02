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
#include <tuple>

// Boost headers go here

// Geneva headers go here
#include "geneva/GPersonalityTraits.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/**
 * This class adds variables and functions to GPersonalityTraits that are specific
 * to conjugate gradient descents. Structurally it mirrors
 * GGradientDescent_PersonalityTraits: every individual needs to know its
 * position in the (parent + difference-quotient children) population layout.
 */
class GConjugateGradientDescent_PersonalityTraits // NOLINT(cppcoreguidelines-special-member-functions)
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
    /** @brief An easy identifier for the class */
    static const std::string nickname; // Initialized in the .cpp definition file

    /** @brief The default constructor */
    GConjugateGradientDescent_PersonalityTraits() = default;
    /**
     * @brief The copy contructor.
     *
     * @param GConjugateGradientDescent_PersonalityTraits const & The object to copy from
     */
    GConjugateGradientDescent_PersonalityTraits(
        const GConjugateGradientDescent_PersonalityTraits &
    ) = default;

    /** @brief The standard destructor */
    ~GConjugateGradientDescent_PersonalityTraits() override = default;

    /**
     * @brief Sets the position of the individual in the population.
     *
     * @param std::size_t const & The position of the individual within the (parent + children) layout
     */
    void setPopulationPosition(const std::size_t &pop_pos);
    /**
     * @brief Retrieves the position of the individual in the population.
     *
     * @return The position of the individual within the population layout
     */
    std::size_t getPopulationPosition() const;

    /**
     * @brief Retrieves the mnemonic of the optimization algorithm.
     *
     * @return The short identifier (mnemonic) of the conjugate gradient descent algorithm
     */
    std::string getMnemonic() const override;

protected:
    /***************************************************************************/
    // Virtual or overridden protected functions

    /**
     * @brief Single declaration of this class'es local data members (non-const overload).
     *
     * @return A tuple of named members used by the comparison and serialization framework
     */
    template <typename Self>
    auto localMembers_(this Self &self) {
        return std::make_tuple(Gem::Common::make_member("pop_pos_", self.pop_pos_));
    }

    /**
     * @brief Loads the data of another GConjugateGradientDescent_PersonalityTraits object into this one.
     *
     * @param GPersonalityTraits const * Pointer to the object whose data is copied (must be a GConjugateGradientDescent_PersonalityTraits)
     */
    void load_(const GPersonalityTraits *cp) override;

    /**
     * @brief Allow access to this classes compare_ function.
     *
     * @param GConjugateGradientDescent_PersonalityTraits const & The first object to compare
     * @param GConjugateGradientDescent_PersonalityTraits const & The second object to compare
     * @param Gem::Common::GToken & The token accumulating the comparison result
     */
    friend void Gem::Common::compare_base_t<GConjugateGradientDescent_PersonalityTraits>(
        GConjugateGradientDescent_PersonalityTraits const &,
        GConjugateGradientDescent_PersonalityTraits const &,
        Gem::Common::GToken &
    );

    /**
     * @brief Searches for compliance with expectations with respect to another object of the same type.
     *
     * @param GPersonalityTraits const & The other object to compare against
     * @param Gem::Common::expectation const & The expectation for this object, e.g. equality
     * @param double const & The limit for allowed deviations of floating point types
     */
    void compare_(
        const GPersonalityTraits &cp // the other object
        ,
        const Gem::Common::expectation &e // the expectation for this object, e.g. equality
        ,
        [[maybe_unused]] const double &limit // the limit for allowed deviations of floating point types
    ) const override;

    /**
     * @brief Applies modifications to this object. This is needed for testing purposes.
     *
     * @return true if the object was modified, false otherwise
     */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

    /***************************************************************************/

private:
    /**
     * @brief Emits a name for this class / object.
     *
     * @return The class name as a string
     */
    std::string name_() const override;
    /**
     * @brief Creates a deep clone of this object.
     *
     * @return A pointer to a newly allocated deep copy of this object
     */
    GPersonalityTraits *clone_() const override;

    /** @brief Stores the current position in the population */
    std::size_t pop_pos_ = 0;
};

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::OptimizationAlgorithms::GConjugateGradientDescent_PersonalityTraits) // NOLINT
