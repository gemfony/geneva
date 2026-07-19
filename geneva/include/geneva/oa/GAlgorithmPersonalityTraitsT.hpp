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

// Geneva headers go here
#include "geneva/oa/GPositionPersonalityTraits.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/**
 * The one shared implementation behind the position-only personality-traits classes (Nelder-Mead,
 * CGD, parameter scan, GSA, ACO, PSO2011): each of those carries no state beyond the population
 * position its GPositionPersonalityTraits base provides, and used to repeat the identical
 * getMnemonic() / name_() / clone_() / serialize() boilerplate. A concrete traits class now only
 * supplies its identity:
 *
 * @code
 * class GNelderMead_PersonalityTraits
 *   : public GAlgorithmPersonalityTraitsT<GNelderMead_PersonalityTraits> {
 * public:
 *     static const std::string nickname;                       // "nm", defined in the .cpp
 *     static constexpr std::string_view class_name{"GNelderMead_PersonalityTraits"};
 * };
 * @endcode
 *
 * plus the usual BOOST_CLASS_EXPORT pair in its header/source.
 *
 * Serialization note: the inherited serialize() emits the GPositionPersonalityTraits base under the
 * same NVP tag the per-class boilerplate produced, and the base_object<> call is made on the
 * DERIVED type (via the CRTP cast) so Boost registers the derived-to-base void-cast chain --
 * archives are unchanged by this fold.
 *
 * @tparam derived_type The concrete personality-traits class (CRTP)
 */
template <typename derived_type>
class GAlgorithmPersonalityTraitsT // NOLINT(cppcoreguidelines-special-member-functions)
  : public GPositionPersonalityTraits {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        // Cast to the concrete type so the derived->base conversion is registered for
        // pointer-based (de)serialization; the tag matches the former per-class
        // BOOST_SERIALIZATION_BASE_OBJECT_NVP(GPositionPersonalityTraits).
        ar &make_nvp(
            "GPositionPersonalityTraits",
            boost::serialization::base_object<GPositionPersonalityTraits>(
                static_cast<derived_type &>(*this)
            )
        );
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GAlgorithmPersonalityTraitsT() = default;
    /** @brief The copy constructor
     *  @param cp Another object of the same type whose state is copied */
    GAlgorithmPersonalityTraitsT(const GAlgorithmPersonalityTraitsT &cp) = default;
    /** @brief The standard destructor */
    ~GAlgorithmPersonalityTraitsT() override = default;

    /**
     * @brief Retrieves the mnemonic of the optimization algorithm.
     * @return The short mnemonic string (derived_type::nickname) identifying the algorithm
     */
    [[nodiscard]] std::string getMnemonic() const override {
        return derived_type::nickname;
    }

private:
    /**
     * @brief Emits a name for this class / object.
     * @return The concrete class name (derived_type::class_name)
     */
    [[nodiscard]] std::string name_() const override {
        return std::string(derived_type::class_name);
    }

    /**
     * @brief Creates a deep clone of this object.
     * @return A newly allocated deep copy of the concrete object, as a GPersonalityTraits pointer
     */
    [[nodiscard]] GPersonalityTraits *clone_() const override {
        return new derived_type(static_cast<const derived_type &>(*this));
    }
};

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
