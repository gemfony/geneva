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
#include <string>
#include <tuple>

// Boost headers go here

// Geneva headers go here
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/GPersonalityTraits.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/**
 * This class adds variables and functions to GPersonalityTraits that are specific
 * to the Generalized (Dual) Simulated Annealing (GSA) algorithm. It carries the
 * index of the population slot the individual occupies, mirroring the
 * informational tags the other algorithms' personalities carry.
 */
class GGeneralizedSimulatedAnnealing_PersonalityTraits // NOLINT(cppcoreguidelines-special-member-functions)
  : public GPersonalityTraits {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /** @brief Single declaration of this class'es local data members */
    auto localMembers() { // NOLINT -- intentionally hides the base localMembers() (each class is its own single source; the base members are handled via the base-class serialize/load_/compare_ call)
        return std::make_tuple(
            Gem::Common::make_member("population_position_", population_position_)
        );
    }
    auto localMembers() const { // NOLINT -- intentionally hides the base localMembers() (each class is its own single source; the base members are handled via the base-class serialize/load_/compare_ call)
        return std::make_tuple(
            Gem::Common::make_member("population_position_", population_position_)
        );
    }

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GPersonalityTraits);
        Gem::Common::serialize_members(ar, this->localMembers());
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief An easy identifier for the class */
    static const std::string nickname; // Initialized in the .cpp definition file

    /** @brief The default constructor */
    GGeneralizedSimulatedAnnealing_PersonalityTraits() = default;
    /**
     * @brief The copy constructor.
     * @param The object to be copied
     */
    GGeneralizedSimulatedAnnealing_PersonalityTraits(
        const GGeneralizedSimulatedAnnealing_PersonalityTraits &
    ) = default;
    /** @brief The standard destructor */
    ~GGeneralizedSimulatedAnnealing_PersonalityTraits() override = default;

    /**
     * @brief Sets the index of the population slot this individual occupies.
     * @param population_position The slot index to store
     */
    void setPopulationPosition(std::size_t population_position);
    /**
     * @brief Retrieves the index of the population slot this individual occupies.
     * @return The stored slot index
     */
    std::size_t getPopulationPosition() const;

    /**
     * @brief Retrieves the mnemonic of the optimization algorithm.
     * @return The short mnemonic string identifying this personality
     */
    std::string getMnemonic() const override;

protected:
    /***************************************************************************/
    // Virtual or overridden protected functions

    /**
     * @brief Loads the data of another GGeneralizedSimulatedAnnealing_PersonalityTraits object.
     * @param The other object whose data is loaded into this one (downcast from GPersonalityTraits)
     */
    void load_(const GPersonalityTraits *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GGeneralizedSimulatedAnnealing_PersonalityTraits>(
        GGeneralizedSimulatedAnnealing_PersonalityTraits const &,
        GGeneralizedSimulatedAnnealing_PersonalityTraits const &,
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

    /** @brief The index of the population slot this individual occupies */
    std::size_t population_position_ = 0;
};

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::OptimizationAlgorithms::GGeneralizedSimulatedAnnealing_PersonalityTraits) // NOLINT
