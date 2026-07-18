/**
 * @file GParaboloidIndividual2D.hpp
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
#include <memory>

#include <common/GParserBuilder.hpp>
#include <geneva/ind/GGenome.hpp>
#include <geneva/ind/GGenomeBuilder.hpp>

namespace Gem {
namespace Geneva {

namespace OptimizationAlgorithms {
class GAdaptionConfigBase;
} // namespace OptimizationAlgorithms

/******************************************************************/
/**
 * This individual searches for the minimum of a 2-dimensional parabola.
 * It is part of an introductory example, used in the Geneva manual.
 */
class GParaboloidIndividual2D : public gen::GGenome {
    /** @brief Make the class accessible to Boost.Serialization */
    friend class boost::serialization::access;

    /**************************************************************/
    /**
	  * This function triggers serialization of this class and its
	  * base classes.
	  */
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;
        // Serialize the base class
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(gen::GGenome);
        // Add other variables here like this:
        // ar & BOOST_SERIALIZATION_NVP(sampleVariable);
    }
    /**************************************************************/
public:
    /** @brief The default constructor */
    GParaboloidIndividual2D();
    /** @brief A standard copy constructor */
    GParaboloidIndividual2D(const GParaboloidIndividual2D &);
    /** @brief The standard destructor */
    virtual ~GParaboloidIndividual2D();

    /** @brief The OA-owned adaption config authoring this genome's two Gauss groups, built from the genome
     *  layout. Static (no adaptor data resides on the individual). Used by the compile-in driver. */
    static std::shared_ptr<OptimizationAlgorithms::GAdaptionConfigBase>
    buildAdaptionConfig(const gen::GGenome &sample);

    //---------------------------------------------------------------------------
    // Tier-2 (config-driven) hooks enabling GIndividualFactory<GParaboloidIndividual2D>, so the SAME
    // individual can also be packaged as a runtime-loadable module (the GParaboloid2D-module artifact) and
    // optimized by the generic optimizer. The compiled-in driver (the GParaboloid2D-fixed artifact) keeps
    // using the constructor and the one-argument buildAdaptionConfig() above; the example builds both.

    /** @brief The configurable values parsed from the config file: each parameter's value range. */
    struct Config {
        double par_min = -10.; ///< Lower bound of each parameter's value range
        double par_max = 10.;  ///< Upper bound of each parameter's value range
    };
    /** @brief Registers the config-file options (the parameter bounds), binding them to the passed Config. */
    static void describeConfig(Gem::Common::GParserBuilder &gpb, Config &c);
    /** @brief Builds the flat genome structure: two constrained doubles in [par_min, par_max]. */
    static gen::GenomeData buildGenome(const Config &c);
    /** @brief Factory hook: the OA-owned adaption config for a produced genome (delegates to the one-argument
     *  form; the Config carries no adaptor settings). */
    static std::shared_ptr<OptimizationAlgorithms::GAdaptionConfigBase>
    buildAdaptionConfig(const gen::GGenome &sample, const Config &c);

protected:
    /** @brief Loads the data of another GParaboloidIndividual2D */
    virtual void load_(const gen::GOptimizableEntity *) final;

    /** @brief The evaluation hook: the sum of squares of the genome's (external) parameters, returned as a
     *  one-element vector (single-criterion). @return The raw fitness vector */
    std::vector<double> evaluate() final;

private:
    /** @brief Creates a deep clone of this object */
    virtual gen::GGenome *clone_() const final;

    const double M_PAR_MIN;
    const double M_PAR_MAX;
};

/******************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GParaboloidIndividual2D) // NOLINT
