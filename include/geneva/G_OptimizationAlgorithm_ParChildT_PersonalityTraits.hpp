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
#include "geneva/GPersonalityTraits.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class adds variables and functions to GPersonalityTraits that are specific
 * to populations comprising parents and children
 */
class GBaseParChildPersonalityTraits // NOLINT(cppcoreguidelines-special-member-functions)
  : public GPersonalityTraits {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GPersonalityTraits) &
            BOOST_SERIALIZATION_NVP(parentCounter_) & BOOST_SERIALIZATION_NVP(popPos_) &
            BOOST_SERIALIZATION_NVP(parentId_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GBaseParChildPersonalityTraits() = default;
    /** @brief The copy contructor */
    GBaseParChildPersonalityTraits(const GBaseParChildPersonalityTraits &) = default;
    /** @brief The standard destructor */
    ~GBaseParChildPersonalityTraits() override = default;

    /** @brief Marks an individual as a parent*/
    bool setIsParent();
    /** @brief Marks an individual as a child */
    bool setIsChild();

    /** @brief Checks whether this is a parent individual */
    bool isParent() const;
    /** @brief Retrieves the current value of the parentCounter_ variable */
    std::uint32_t getParentCounter() const;

    /** @brief Sets the position of the individual in the population */
    void setPopulationPosition(const std::size_t &);
    /** @brief Retrieves the position of the individual in the population */
    std::size_t getPopulationPosition(void) const;

    /** @brief Stores the parent's id with this object */
    void setParentId(const std::size_t &);
    /** @brief Retrieves the parent id's value */
    std::size_t getParentId() const;
    /** @brief Checks whether a parent id has been set */
    bool parentIdSet() const;
    /** @brief Marks the parent id as unset */
    void unsetParentId();

    /** @brief Retrieves the mnemonic of the optimization algorithm */
    std::string getMnemonic() const override;

protected:
    /***************************************************************************/
    // Virtual or overridden protected functions

    /** @brief Loads the data of another GBaseParChildPersonalityTraits object */
    void load_(const GObject *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GBaseParChildPersonalityTraits>(
        GBaseParChildPersonalityTraits const &,
        GBaseParChildPersonalityTraits const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GObject & // the other object
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
    GObject *clone_() const override;

    /** @brief Allows populations to record how often an individual has been reelected as parent (0 if it is a child) */
    std::uint32_t parentCounter_ = 0;
    /** @brief Stores the current position in the population */
    std::size_t popPos_ = 0;
    /** @brief The id of the old parent individual. This is intentionally a signed value. A negative value refers to an unset parent id */
    std::int16_t parentId_ = -1;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::OptimizationAlgorithms::GBaseParChildPersonalityTraits) // NOLINT

// Phase-2b compile shim — removed in NS Phase 3 (reference migration).
namespace Gem::Geneva {
using OptimizationAlgorithms::GBaseParChildPersonalityTraits;
} // namespace Gem::Geneva
