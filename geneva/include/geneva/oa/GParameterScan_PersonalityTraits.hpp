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
 * @brief Adds variables and functions to GPersonalityTraits that are specific
 * to parameter scans.
 */
class GParameterScan_PersonalityTraits // NOLINT(cppcoreguidelines-special-member-functions)
  : public GPersonalityTraits {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /** @brief Single declaration of this class'es local data members */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(Gem::Common::make_member("pop_pos_", self.pop_pos_));
    }

    /** @brief Serializes this object via Boost.Serialization
     *  @tparam Archive The archive type used for (de-)serialization
     *  @param ar The archive to serialize to / from
     *  @param version The class version supplied by Boost.Serialization */
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GPersonalityTraits);
        // ... and then our own data, derived from the single localMembers() declaration
        Gem::Common::serialize_members(ar, localMembers_(*this));
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief An easy identifier for the class */
    static const std::string nickname; // Initialized in the .cpp definition file

    /** @brief The default constructor */
    GParameterScan_PersonalityTraits() = default;
    /** @brief The copy constructor */
    GParameterScan_PersonalityTraits(const GParameterScan_PersonalityTraits &) = default;

    /** @brief The standard destructor */
    ~GParameterScan_PersonalityTraits() override = default;

    /** @brief Sets the position of the individual in the population
     *  @param pop_pos The position the individual should be assigned within the population */
    void setPopulationPosition(const std::size_t &pop_pos);
    /** @brief Retrieves the position of the individual in the population
     *  @return The stored position of the individual within the population */
    std::size_t getPopulationPosition() const;

    /** @brief Retrieves the mnemonic of the optimization algorithm
     *  @return The short mnemonic identifying the parameter-scan algorithm */
    std::string getMnemonic() const override;

protected:
    /***************************************************************************/
    // Virtual or overridden protected functions

    /** @brief Loads the data of another GParameterScan_PersonalityTraits object
     *  @param cp A pointer to another object of this type, camouflaged as a GPersonalityTraits */
    void load_(const GPersonalityTraits *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GParameterScan_PersonalityTraits>(
        GParameterScan_PersonalityTraits const &,
        GParameterScan_PersonalityTraits const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type
     *  @param cp The other object to be compared against (as a GPersonalityTraits)
     *  @param e The expectation for this object, e.g. equality
     *  @param limit The limit for allowed deviations of floating point types */
    void compare_(
        const GPersonalityTraits &cp // the other object
        ,
        const Gem::Common::expectation &e // the expectation for this object, e.g. equality
        ,
        const double &limit // the limit for allowed deviations of floating point types
    ) const override;

    /** @brief Applies modifications to this object. This is needed for testing purposes
     *  @return true if the object was modified, false otherwise */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

    /***************************************************************************/

private:
    /** @brief Emits a name for this class / object
     *  @return The class name of this object */
    std::string name_() const override;
    /** @brief Creates a deep clone of this object
     *  @return A pointer to a freshly allocated deep copy of this object */
    GPersonalityTraits *clone_() const override;

    /** @brief Stores the current position in the population */
    std::size_t pop_pos_ = 0;
};

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::OptimizationAlgorithms::GParameterScan_PersonalityTraits) // NOLINT

