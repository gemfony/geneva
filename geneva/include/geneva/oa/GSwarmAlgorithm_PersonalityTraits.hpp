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

// Boost headers go here

// Geneva headers go here
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/GPersonalityTraits.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/**
 * This class adds variables and functions to GPersonalityTraits that are specific
 * to swarm optimization.
 */
class GSwarmAlgorithm_PersonalityTraits // NOLINT(cppcoreguidelines-special-member-functions)
  : public GPersonalityTraits {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /***************************************************************************/
    /**
     * Single declaration of this class'es unconditionally-handled local data
     * members. This drives serialize(), load_() and compare_() from one place.
     *
     * Handled manually (NOT in this tuple): personal_best_, a
     * std::shared_ptr<gen::GOptimizableEntity>. It is deep-cloned on load, but the
     * load (and the copy constructor) additionally call resetPersonality() on
     * the clone to avoid building a "chain" of individuals. That extra
     * post-clone step is asymmetric to a plain make_cloneable_member() deep
     * clone, so personal_best_ stays in the documented manual tail of
     * serialize()/load_()/compare_().
     */
    auto localMembers() { // NOLINT -- intentionally hides the base localMembers() (each class is its own single source; the base members are handled via the base-class serialize/load_/compare_ call)
        return std::make_tuple(
            Gem::Common::make_member("neighborhood_", neighborhood_),
            Gem::Common::make_member("no_position_update_", no_position_update_),
            Gem::Common::make_member("personal_best_quality_", personal_best_quality_)
        );
    }
    auto localMembers() const { // NOLINT -- intentionally hides the base localMembers() (each class is its own single source; the base members are handled via the base-class serialize/load_/compare_ call)
        return std::make_tuple(
            Gem::Common::make_member("neighborhood_", neighborhood_),
            Gem::Common::make_member("no_position_update_", no_position_update_),
            Gem::Common::make_member("personal_best_quality_", personal_best_quality_)
        );
    }

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GPersonalityTraits);

        // The unconditionally-handled local members, derived from the single
        // localMembers() declaration.
        Gem::Common::serialize_members(ar, this->localMembers());

        // Manual tail: personal_best_ (deep-cloned + personality-reset on load).
        ar & BOOST_SERIALIZATION_NVP(personal_best_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief An easy identifier for the class */
    static const std::string nickname; // Initialized in the .cpp definition file

    /** @brief The default constructor */
    GSwarmAlgorithm_PersonalityTraits() = default;
    /**
     * @brief The copy constructor
     *
     * @param cp A constant reference to another GSwarmAlgorithm_PersonalityTraits object to be copied
     */
    GSwarmAlgorithm_PersonalityTraits(const GSwarmAlgorithm_PersonalityTraits & cp);
    /** @brief The standard destructor */
    ~GSwarmAlgorithm_PersonalityTraits() override = default;

    /**
     * @brief Specifies in which neighborhood the individual is at present
     *
     * @param neighborhood The id of the neighborhood the individual is to be assigned to
     */
    void setNeighborhood(const std::size_t & neighborhood);
    /**
     * @brief Retrieves the id of the neighborhood the individual is in at present
     *
     * @return The id of the neighborhood the individual currently belongs to
     */
    std::size_t getNeighborhood() const;

    /** @brief Sets the no_position_update_ flag */
    void setNoPositionUpdate();
    /**
     * @brief Retrieves the current value of the no_position_update_ flag
     *
     * @return true if the individual's position is not to be updated, false otherwise
     */
    bool noPositionUpdate() const;
    /**
     * @brief Retrieves and resets the current value of the no_position_update_ flag
     *
     * @return The value of the no_position_update_ flag prior to being reset to false
     */
    bool checkNoPositionUpdateAndReset();

    /**
     * @brief Allows to add a new personal best to the individual
     *
     * @param p A shared pointer to the GOptimizableEntity representing the individual's new personal best
     */
    void registerPersonalBest(std::shared_ptr<gen::GOptimizableEntity> p);
    /**
     * @brief Allows to retrieve the personal best individual
     *
     * @return A shared pointer to the GOptimizableEntity holding the individual's personal best
     */
    std::shared_ptr<gen::GOptimizableEntity> getPersonalBest() const;
    /** @brief Resets the personal best individual */
    void resetPersonalBest();
    /**
     * @brief Retrieve quality of personally best individual
     *
     * @return A tuple holding the raw and transformed fitness of the personally best individual
     */
    std::tuple<double, double> getPersonalBestQuality() const;

    /**
     * @brief Retrieves the mnemonic of the optimization algorithm
     *
     * @return The mnemonic (short identifier) associated with the swarm optimization algorithm
     */
    std::string getMnemonic() const override;

protected:
    /***************************************************************************/
    // Virtual or overridden protected functions

    /**
     * @brief Loads the data of another GSwarmAlgorithm_PersonalityTraits object
     *
     * @param cp A pointer to another GPersonalityTraits object (expected to be a GSwarmAlgorithm_PersonalityTraits), camouflaged as a base-class pointer
     */
    void load_(const GPersonalityTraits * cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GSwarmAlgorithm_PersonalityTraits>(
        GSwarmAlgorithm_PersonalityTraits const &,
        GSwarmAlgorithm_PersonalityTraits const &,
        Gem::Common::GToken &
    );

    /**
     * @brief Searches for compliance with expectations with respect to another object of the same type
     *
     * @param cp A constant reference to another GPersonalityTraits object (the object to be compared against)
     * @param e The expectation for this comparison, e.g. equality or inequality
     * @param limit The maximum allowed deviation of floating point types still considered equal
     */
    void compare_(
        const GPersonalityTraits & cp // the other object
        ,
        const Gem::Common::expectation & e // the expectation for this object, e.g. equality
        ,
        const double & limit // the limit for allowed deviations of floating point types
    ) const override;

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

    /***************************************************************************/

private:
    /**
     * @brief Emits a name for this class / object
     *
     * @return A string holding the name of this class
     */
    std::string name_() const override;
    /**
     * @brief Creates a deep clone of this object
     *
     * @return A deep clone of this object, returned as a pointer to the GPersonalityTraits base class
     */
    GPersonalityTraits *clone_() const override;

    /** @brief Stores the current position in the population */
    std::size_t neighborhood_ = 0;

    /** @brief Determines whether the individual has been randomly initialized */
    bool no_position_update_ = false;

    /** @brief Holds the personally best GOptimizableEntity */
    std::shared_ptr<gen::GOptimizableEntity> personal_best_;
    /** @brief The quality of the personally best individual */
    std::tuple<double, double> personal_best_quality_{0., 0.};
};

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::OptimizationAlgorithms::GSwarmAlgorithm_PersonalityTraits) // NOLINT

