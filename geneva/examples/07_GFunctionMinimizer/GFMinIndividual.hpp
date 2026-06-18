/**
 * @file GFMinIndividual.hpp
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
#include <cmath>
#include <iostream>
#include <sstream>
#include <tuple>
#include <vector>

// Boost header files go here

// Geneva header files go here
#include "common/GParserBuilder.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GFlatIndividualFactory.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
#include <filesystem>
#include <memory>

namespace Gem {
namespace Geneva {

namespace OptimizationAlgorithms {
class GAdaptionConfigBase;
} // namespace OptimizationAlgorithms

/******************************************************************************/
/**
 * This enum denotes the possible demo function types
 */
enum class targetFunction : Gem::Common::ENUMBASETYPE {
    GFM_PARABOLA = 0,
    GFM_NOISYPARABOLA = 1
};

// Make sure targetFunction can be streamed
/** @brief Puts a Gem::Geneva::targetFunction into a stream. Needed for streaming / Gem::Common::fromString<> */
std::ostream &operator<<(std::ostream &, const Gem::Geneva::targetFunction &);

/** @brief Reads a Gem::Geneva::targetFunction from a stream. Needed for streaming / Gem::Common::fromString<> */
std::istream &operator>>(std::istream &, Gem::Geneva::targetFunction &);

/******************************************************************************/
// A number of default settings for the factory
const double GFI_DEF_ADPROB = 1.0;
const double GFI_DEF_SIGMA = 0.025;
const double GFI_DEF_SIGMASIGMA = 0.2;
const double GFI_DEF_MINSIGMA = 0.001;
const double GFI_DEF_MAXSIGMA = 1;
const std::size_t GFI_DEF_PARDIM = 2;
const double GFI_DEF_MINVAR = -10.;
const double GFI_DEF_MAXVAR = 10.;
const targetFunction GO_DEF_TARGETFUNCTION = targetFunction::GFM_PARABOLA;

/******************************************************************************/
/**
 * This individual searches for a minimum of a number of predefined functions, each capable
 * of processing their input in multiple dimensions.
 */
class GFMinIndividual : public gen::GFlatGenome {
    /////////////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive &ar, const unsigned int) {
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(gen::GFlatGenome) &
            BOOST_SERIALIZATION_NVP(targetFunction_) & BOOST_SERIALIZATION_NVP(seed_sigma_);
    }

    /////////////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GFMinIndividual();
    /** @brief A standard copy constructor */
    GFMinIndividual(const GFMinIndividual &);
    /** @brief The standard destructor */
    virtual ~GFMinIndividual();

    /** @brief Allows to set the demo function */
    void setTargetFunction(targetFunction);
    /** @brief Allows to retrieve the current demo function */
    targetFunction getTargetFunction() const;

    /** @brief Retrieves the average value of the sigma used in Gauss adaptors */
    double getAverageSigma() const;

    //---------------------------------------------------------------------------
    // GFlatIndividualFactory<GFMinIndividual> hooks. The individual supplies the static hooks the generic
    // factory needs: describeConfig (the configurable values, including the
    // target function), buildGenome (one shared constrained-double group), buildAdaptionConfig (the
    // OA-owned Gauss adaptor for that group) and applyConfig (the target function + the seed sigma stamped
    // for the getAverageSigma() telemetry hook -- per-object, non-genome settings).

    /** @brief The configurable values parsed from the config file. */
    struct Config {
        double ad_prob = GFI_DEF_ADPROB;
        double sigma = GFI_DEF_SIGMA;
        double sigma_sigma = GFI_DEF_SIGMASIGMA;
        double min_sigma = GFI_DEF_MINSIGMA;
        double max_sigma = GFI_DEF_MAXSIGMA;
        std::size_t par_dim = GFI_DEF_PARDIM;
        double min_var = GFI_DEF_MINVAR;
        double max_var = GFI_DEF_MAXVAR;
        targetFunction target_function = GO_DEF_TARGETFUNCTION;
    };

    /** @brief Registers the config-file options, binding them to the passed Config */
    static void describeConfig(Gem::Common::GParserBuilder &gpb, Config &c);
    /** @brief Builds the flat genome's structure: one shared constrained-double group of par_dim values */
    static gen::GenomeData buildGenome(const Config &c);
    /** @brief The OA-owned adaption config: the shared double group gets the configured Gauss adaptor */
    static std::shared_ptr<OptimizationAlgorithms::GAdaptionConfigBase>
    buildAdaptionConfig(const gen::GFlatGenome &sample, const Config &c);
    /** @brief Per-object post-config hook: the target function and the seed sigma for getAverageSigma() */
    static void applyConfig(GFMinIndividual &ind, const Config &c);

protected:
    /***************************************************************************/
    /** @brief Loads the data of another GFMinIndividual */
    virtual void load_(const gen::GOptimizableEntity *) final;

    /** @brief The actual value calculation takes place here */
    virtual double fitnessCalculation() final;

    /***************************************************************************/

private:
    /***************************************************************************/
    /** @brief Creates a deep clone of this object */
    virtual gen::GFlatGenome *clone_() const final;

    /***************************************************************************/
    targetFunction targetFunction_ =
        GO_DEF_TARGETFUNCTION; ///< Specifies which demo function should be used

    double seed_sigma_ = GFI_DEF_SIGMA; ///< The configured seed sigma, stamped by the factory for getAverageSigma()

    /***************************************************************************/
    /** @brief A simple n-dimensional parabola */
    double parabola(const std::vector<double> &parVec) const;
    /** @brief A "noisy" parabola */
    double noisyParabola(const std::vector<double> &parVec) const;

    /***************************************************************************/
};

/******************************************************************************/
/**
 * Provides an easy way to print the individual's content
 */
std::ostream &operator<<(std::ostream &, const Gem::Geneva::GFMinIndividual &);
std::ostream &operator<<(std::ostream &, std::shared_ptr<Gem::Geneva::GFMinIndividual>);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory for GFMinIndividual objects: an alias for the generic, config-driven GFlatIndividualFactory,
 * for which GFMinIndividual supplies the static describeConfig / buildGenome / buildAdaptionConfig /
 * applyConfig hooks. Call sites use ctor(path), operator(), get_as<>() and getAdaptionConfig().
 */
using GFMinIndividualFactory = Gem::Geneva::Genome::GFlatIndividualFactory<GFMinIndividual>;

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GFMinIndividual) // NOLINT
