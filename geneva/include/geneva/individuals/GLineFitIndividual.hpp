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

// Standard header files go here
#include <algorithm> // for std::sort
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <list>
#include <sstream>
#include <tuple>
#include <vector>

// Boost header files go here

// Geneva header files go here
#include "common/GCommonEnums.hpp"
#include "common/GExceptions.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {
class GAdaptionConfigBase;
} // namespace Gem::Geneva::OptimizationAlgorithms

namespace Gem::Geneva::Individuals {

/******************************************************************************/
/**
 * This individual takes a vector of 2D double-tuples and calculates the
 * root-square deviation from the line defined by its two parameters
 */
class GLineFitIndividual // NOLINT(cppcoreguidelines-special-member-functions)
  : public gen::GFlatGenome {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(gen::GFlatGenome) &
            BOOST_SERIALIZATION_NVP(data_points_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /**
     * @brief The default constructor.
     * @param data_points The (x,y) data points the fitted line is evaluated against
     */
    GLineFitIndividual(const std::vector<std::tuple<double, double>> & data_points);
    /**
     * @brief The copy constructor.
     * @param cp A constant reference to another GLineFitIndividual object
     */
    GLineFitIndividual(const GLineFitIndividual & cp);

    /** @brief The standard destructor */
    ~GLineFitIndividual() override;

    /**
     * @brief Retrieves the tuple (a,b) of the line represented by this object.
     * @return The line's (offset a, slope b) as a tuple
     */
    std::tuple<double, double> getLine() const;

    /**
     * @brief The OA-owned adaption config authoring this genome's two Gauss groups (offset + slope).
     * @return A shared pointer to the populated adaption config
     */
    std::shared_ptr<OptimizationAlgorithms::GAdaptionConfigBase> getAdaptionConfig() const;

protected:
    /** @brief Single declaration of this class'es local data members */
    template <typename Self>
    auto localMembers_(this Self &self) {
        return std::make_tuple(Gem::Common::make_member("data_points_", self.data_points_));
    }

    /**
     * @brief Loads the data of another GLineFitIndividual.
     * @param cp A pointer to another GLineFitIndividual, camouflaged as a GOptimizableEntity
     */
    void load_(const gen::GOptimizableEntity *cp) final;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GLineFitIndividual>(
        GLineFitIndividual const &,
        GLineFitIndividual const &,
        Gem::Common::GToken &
    );

    /**
     * @brief Searches for compliance with expectations with respect to another object of the same type.
     * @param cp The other object to compare against
     * @param e The expectation for this object, e.g. equality
     * @param limit The limit for allowed deviations of floating point types
     */
    void compare_(
        const gen::GOptimizableEntity & cp
        ,
        const Gem::Common::expectation & e
        ,
        const double & limit
    ) const final;

    /**
     * @brief The actual fitness calculation takes place here.
     * @return The root-square deviation of the data points from the represented line
     */
    double fitnessCalculation() final;

    /**
     * @brief Applies modifications to this object.
     * @return A boolean indicating whether any modifications were made
     */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. */
    void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /**
     * @brief Creates a deep clone of this object.
     * @return A deep clone of this object, camouflaged as a GFlatGenome
     */
    gen::GFlatGenome *clone_() const final;

    /** @brief The default constructor -- private, as it is only needed for (de-)serialization purposes */
    GLineFitIndividual();

    std::vector<std::tuple<double, double>>
        data_points_; ///< Holds the data points used for the fit procedure
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::Individuals */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::Individuals::GLineFitIndividual) // NOLINT
