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
#include "geneva/oa/GBaseParChildPersonalityTraits.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class adds variables and functions to GPersonalityTraits that are specific
 * to evolutionary algorithms.
 */
class GEvolutionaryAlgorithm_PersonalityTraits // NOLINT(cppcoreguidelines-special-member-functions)
  : public GBaseParChildPersonalityTraits {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /** @brief Single declaration of this class'es local data members */
    auto localMembers() {
        return std::make_tuple(Gem::Common::make_member("is_on_pareto_front_", is_on_pareto_front_));
    }
    auto localMembers() const {
        return std::make_tuple(Gem::Common::make_member("is_on_pareto_front_", is_on_pareto_front_));
    }

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBaseParChildPersonalityTraits);
        // ... and then our own data, derived from the single localMembers() declaration
        Gem::Common::serialize_members(ar, this->localMembers());
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief An easy identifier for the class */
    static const std::string nickname; // Initialized in the .cpp definition file

    /** @brief The default constructor */
    GEvolutionaryAlgorithm_PersonalityTraits() = default;
    /**
     * @brief The copy contructor.
     * @param The object to be copied
     */
    GEvolutionaryAlgorithm_PersonalityTraits(
        const GEvolutionaryAlgorithm_PersonalityTraits &
    ) = default;
    /** @brief The standard destructor */
    ~GEvolutionaryAlgorithm_PersonalityTraits() override = default;

    /**
     * @brief Allows to check whether this individual lies on the pareto front (only yields useful results after pareto-sorting in EA).
     * @return true if the individual is currently tagged as lying on the pareto front, false otherwise
     */
    bool isOnParetoFront() const;
    /** @brief Allows to reset the pareto tag to "true" */
    void resetParetoTag();
    /** @brief Allows to specify that this individual does not lie on the pareto front of the current iteration */
    void setIsNotOnParetoFront();

    /**
     * @brief Retrieves the mnemonic of the optimization algorithm.
     * @return The short mnemonic string identifying the evolutionary-algorithm personality
     */
    std::string getMnemonic() const override;

protected:
    /***************************************************************************/
    // Virtual or overridden protected functions

    /**
     * @brief Loads the data of another GEvolutionaryAlgorithm_PersonalityTraits object.
     * @param The other object whose data is loaded into this one (downcast from GPersonalityTraits)
     */
    void load_(const GPersonalityTraits *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GEvolutionaryAlgorithm_PersonalityTraits>(
        GEvolutionaryAlgorithm_PersonalityTraits const &,
        GEvolutionaryAlgorithm_PersonalityTraits const &,
        Gem::Common::GToken &
    );

    /**
     * @brief Searches for compliance with expectations with respect to another object of the same type.
     * @param The other object to compare against (downcast from GPersonalityTraits)
     * @param The expectation for this object, e.g. equality
     * @param The limit for allowed deviations of floating point types
     */
    void compare_(
        const GPersonalityTraits & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const override;

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

    /***************************************************************************/

private:
    /** @brief Emits a name for this class / object */
    std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    GPersonalityTraits *clone_() const override;

    /** @brief Determines whether the individual lies on the pareto front */
    bool is_on_pareto_front_ = true;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::OptimizationAlgorithms::GEvolutionaryAlgorithm_PersonalityTraits) // NOLINT

