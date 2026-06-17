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
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief Personality traits for populations comprising parents and children.
 *
 * Adds variables and functions to GPersonalityTraits that are specific to parent-child
 * populations: a parent re-election counter, the individual's position in the population,
 * and the id of the parent it descended from.
 */
class GBaseParChildPersonalityTraits // NOLINT(cppcoreguidelines-special-member-functions)
  : public GPersonalityTraits {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /**
     * @brief Single declaration of this class's local data members (mutable access, for serialization).
     *
     * @return A tuple of named member bindings for parent_counter_, pop_pos_ and parent_id_
     */
    auto localMembers() {
        return std::make_tuple(
            Gem::Common::make_member("parent_counter_", parent_counter_),
            Gem::Common::make_member("pop_pos_", pop_pos_),
            Gem::Common::make_member("parent_id_", parent_id_)
        );
    }
    /**
     * @brief Single declaration of this class's local data members (const access, for serialization).
     *
     * @return A tuple of named member bindings for parent_counter_, pop_pos_ and parent_id_
     */
    auto localMembers() const {
        return std::make_tuple(
            Gem::Common::make_member("parent_counter_", parent_counter_),
            Gem::Common::make_member("pop_pos_", pop_pos_),
            Gem::Common::make_member("parent_id_", parent_id_)
        );
    }

    /**
     * @brief Serializes this object, including its base class, to or from a Boost archive.
     *
     * @tparam Archive The Boost.Serialization archive type
     * @param ar The archive to read from or write to
     * @param unnamed The serialization version number (unused)
     */
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GPersonalityTraits);
        // ... and then our own data, derived from the single localMembers() declaration
        Gem::Common::serialize_members(ar, this->localMembers());
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GBaseParChildPersonalityTraits() = default;
    /** @brief The copy contructor */
    GBaseParChildPersonalityTraits(const GBaseParChildPersonalityTraits &) = default;
    /** @brief The standard destructor */
    ~GBaseParChildPersonalityTraits() override = default;

    /**
     * @brief Marks the individual as a parent (incrementing its parent re-election counter).
     *
     * @return true if the individual was a child before this call (i.e. the status changed)
     */
    bool setIsParent();
    /**
     * @brief Marks the individual as a child (resetting its parent re-election counter).
     *
     * @return true if the individual was a parent before this call (i.e. the status changed)
     */
    bool setIsChild();

    /**
     * @brief Checks whether this is a parent individual.
     *
     * @return true if the individual is currently a parent, false if it is a child
     */
    bool isParent() const;
    /**
     * @brief Retrieves the current value of the parent_counter_ variable.
     *
     * @return The number of consecutive generations this individual has been re-elected as parent (0 if it is a child)
     */
    std::uint32_t getParentCounter() const;

    /**
     * @brief Sets the position of the individual in the population.
     *
     * @param popPos The zero-based position of the individual within the population
     */
    void setPopulationPosition(const std::size_t &);
    /**
     * @brief Retrieves the position of the individual in the population.
     *
     * @return The individual's zero-based position within the population
     */
    std::size_t getPopulationPosition() const;

    /**
     * @brief Stores the parent's id with this object.
     *
     * @param parentId The population position of the parent this individual descended from
     */
    void setParentId(const std::size_t &);
    /**
     * @brief Retrieves the parent id's value.
     *
     * @return The stored parent id (throws if no parent id has been set)
     */
    std::size_t getParentId() const;
    /**
     * @brief Checks whether a parent id has been set.
     *
     * @return true if a parent id has been recorded, false otherwise
     */
    bool parentIdSet() const;
    /** @brief Marks the parent id as unset. */
    void unsetParentId();

    /**
     * @brief Retrieves the mnemonic of the optimization algorithm.
     *
     * @return The short mnemonic string identifying this personality's optimization algorithm
     */
    std::string getMnemonic() const override;

protected:
    /***************************************************************************/
    // Virtual or overridden protected functions

    /**
     * @brief Loads the data of another GBaseParChildPersonalityTraits object into this one.
     *
     * @param cp A pointer to the GPersonalityTraits object to load from (must be a GBaseParChildPersonalityTraits)
     */
    void load_(const GPersonalityTraits *) override;

    /** @brief Allow access to this class's compare_ function. */
    friend void Gem::Common::compare_base_t<GBaseParChildPersonalityTraits>(
        GBaseParChildPersonalityTraits const &,
        GBaseParChildPersonalityTraits const &,
        Gem::Common::GToken &
    );

    /**
     * @brief Searches for compliance with expectations with respect to another object of the same type.
     *
     * @param cp The other object to compare against
     * @param e The expectation for this comparison (e.g. equality or inequality)
     * @param limit The maximum allowed deviation for floating point comparisons
     */
    void compare_(
        const GPersonalityTraits & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const override;

    /**
     * @brief Applies modifications to this object. This is needed for testing purposes.
     *
     * @return true if at least one modification was made
     */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes. */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes. */
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
     * @return A pointer to a newly allocated, deep copy of this object (caller takes ownership)
     */
    GPersonalityTraits *clone_() const override;

    /** @brief Allows populations to record how often an individual has been reelected as parent (0 if it is a child) */
    std::uint32_t parent_counter_ = 0;
    /** @brief Stores the current position in the population */
    std::size_t pop_pos_ = 0;
    /** @brief The id of the old parent individual. This is intentionally a signed value. A negative value refers to an unset parent id */
    std::int16_t parent_id_ = -1;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::OptimizationAlgorithms::GBaseParChildPersonalityTraits) // NOLINT

