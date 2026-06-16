/**
 * @file GMultiCriterionParabolaIndividual.hpp
 */

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
#include <iostream>

// Boost header files go here

// Geneva header files go here
#include <common/GCommonHelperFunctions.hpp>
#include <common/GParserBuilder.hpp>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include <geneva/ind/GFlatGenome.hpp>
#include <geneva/ind/GFlatIndividualFactory.hpp>
#include <geneva/ind/GGenomeBuilder.hpp>

namespace Gem::Geneva {

namespace OptimizationAlgorithms {
class GAdaptionConfigBase;
} // namespace OptimizationAlgorithms

// The number of parameters
constexpr std::size_t NPAR_MC = 3;

/******************************************************************************/
/**
 * This individual implements several, possibly conflicting evaluation
 * criteria, each implemented as a parabola with its own minimum
 */
class GMultiCriterionParabolaIndividual : public gen::GFlatGenome {
    /***************************************************************************/
    /**
	  * This function triggers serialization of this class and its
	  * base classes.
	  */
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(gen::GFlatGenome);
    }

    /** @brief Make the class accessible to Boost.Serialization */
    friend class boost::serialization::access;
    /***************************************************************************/

public:
    /** @brief The default constructor -- the genome is installed by the factory */
    GMultiCriterionParabolaIndividual() = default;
    /** @brief A standard copy constructor */
    GMultiCriterionParabolaIndividual(const GMultiCriterionParabolaIndividual &) = default;
    /** @brief The destructor */
    ~GMultiCriterionParabolaIndividual() override = default;

    /** @brief Assigns a number of minima to this object */
    void setMinima(const std::vector<double> &);

    /** @brief The OA-owned adaption config authoring this genome's per-parameter Gauss groups. */
    std::shared_ptr<OptimizationAlgorithms::GAdaptionConfigBase> getAdaptionConfig() const;

    //---------------------------------------------------------------------------
    // GFlatIndividualFactory<GMultiCriterionParabolaIndividual> hooks. Instead of a bespoke factory the
    // individual supplies the static hooks the generic factory needs: describeConfig (the configurable
    // values), buildGenome (one constrained-double Gauss group per minimum) and applyConfig (the number
    // of evaluation criteria and the per-criterion minima -- per-object, non-genome settings).

    /** @brief All values formerly parsed by the bespoke GMultiCriterionParabolaIndividualFactory. */
    struct Config {
        double par_min = -10.;              ///< The lower boundary of the parabola
        double par_max = 10.;               ///< The upper boundary of the parabola
        std::string minima = "-1., 0., 1."; ///< A list of optima, encoded as a string
    };

    /** @brief Registers the config-file options, binding them to the passed Config */
    static void describeConfig(Gem::Common::GParserBuilder &gpb, Config &c);
    /** @brief Builds the flat genome's structure: one constrained double per minimum, in [par_min, par_max] */
    static gen::Genome buildGenome(const Config &c);
    /** @brief Per-object post-config hook: sets the number of evaluation criteria and the minima */
    static void applyConfig(GMultiCriterionParabolaIndividual &ind, const Config &c);

protected:
    /** @brief Loads the data of another GMultiCriterionParabolaIndividual */
    void load_(const gen::GOptimizableEntity *) final;

    /** @brief The actual fitness calculation takes place here. */
    double fitnessCalculation() final;

private:
    /** @brief Creates a deep clone of this object */
    gen::GFlatGenome *clone_() const final;

    /** @brief Holds the minima needed for multi-criterion optimization */
    std::vector<double> minima_{};
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory for GMultiCriterionParabolaIndividual objects. The bespoke factory has been replaced by the
 * generic, config-driven GFlatIndividualFactory; GMultiCriterionParabolaIndividual supplies the static
 * describeConfig / buildGenome / applyConfig hooks. The alias keeps existing call sites (ctor(path),
 * get(), registerContentCreator()) compiling unchanged.
 */
using GMultiCriterionParabolaIndividualFactory =
    Gem::Geneva::Genome::GFlatIndividualFactory<GMultiCriterionParabolaIndividual>;

/******************************************************************************/
/**
 * Provide an easy way to print the individual's content
 */
std::ostream &operator<<(std::ostream &, const Gem::Geneva::GMultiCriterionParabolaIndividual &);
std::ostream &
operator<<(std::ostream &, const std::shared_ptr<Gem::Geneva::GMultiCriterionParabolaIndividual> &);

/******************************************************************************/

}

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GMultiCriterionParabolaIndividual) // NOLINT
