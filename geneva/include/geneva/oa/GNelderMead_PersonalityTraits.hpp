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

// Geneva headers go here
#include "geneva/oa/GPositionPersonalityTraits.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/**
 * The Nelder-Mead downhill simplex's personality traits: every individual only needs to know its
 * position in the population (its simplex-vertex / trial-slot index), which the
 * GPositionPersonalityTraits base provides; this class contributes only the algorithm's identity.
 */
class GNelderMead_PersonalityTraits // NOLINT(cppcoreguidelines-special-member-functions)
  : public GPositionPersonalityTraits {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GPositionPersonalityTraits);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief An easy identifier for the class */
    static const std::string nickname; // Initialized in the .cpp definition file

    /** @brief The default constructor */
    GNelderMead_PersonalityTraits() = default;
    /** @brief The copy constructor
     *  @param cp Another GNelderMead_PersonalityTraits object whose state is copied */
    GNelderMead_PersonalityTraits(const GNelderMead_PersonalityTraits &cp) = default;
    /** @brief The standard destructor */
    ~GNelderMead_PersonalityTraits() override = default;

    /**
     * @brief Retrieves the mnemonic of the optimization algorithm.
     * @return The short mnemonic string identifying the Nelder-Mead algorithm
     */
    std::string getMnemonic() const override;

private:
    /**
     * @brief Emits a name for this class / object.
     * @return The class name of this personality-traits object
     */
    std::string name_() const override;
    /**
     * @brief Creates a deep clone of this object.
     * @return A newly allocated deep copy of this object, as a GPersonalityTraits pointer
     */
    GPersonalityTraits *clone_() const override;
};

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::OptimizationAlgorithms::GNelderMead_PersonalityTraits) // NOLINT
