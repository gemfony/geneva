/**
 * @file GStarterIndividual.hpp
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
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GParserBuilder.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GFlatIndividualFactory.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
#include <filesystem>
#include <memory>
#include <string>

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
    PARABOLA = 0,
    NOISYPARABOLA = 1
};

// Make sure targetFunction can be streamed
/** @brief Puts a Gem::Geneva::targetFunction into a stream. Needed for streaming / Gem::Common::fromString<> */
std::ostream &operator<<(std::ostream &, const Gem::Geneva::targetFunction &);

/** @brief Reads a Gem::Geneva::targetFunction from a stream. Needed for streaming / Gem::Common::fromString<> */
std::istream &operator>>(std::istream &, Gem::Geneva::targetFunction &);

/******************************************************************************/
// A number of default settings for the factory
const double GSI_DEF_ADPROB = 1.0;
const double GSI_DEF_SIGMA = 0.025;
const double GSI_DEF_SIGMASIGMA = 0.2;
const double GSI_DEF_MINSIGMA = 0.001;
const double GSI_DEF_MAXSIGMA = 1;
const targetFunction GO_DEF_TARGETFUNCTION = targetFunction::PARABOLA;

/******************************************************************************/
/**
 * This individual searches for a minimum of a number of predefined functions, each capable
 * of processing their input in multiple dimensions.
 */
class GStarterIndividual : public gen::GFlatGenome {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /**
     * @brief Single declaration of this class'es local data members
     * @return A tuple of named member references driving serialize(), load_() and compare_()
     */
    template <typename Self>
    auto localMembers_(this Self &self) {
        return std::make_tuple(Gem::Common::make_member("targetFunction_", self.targetFunction_));
    }

    template <class Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        // The member list is derived from the single localMembers() declaration
        // so serialize()/load_()/compare_() stay in sync (no silently-dropped member).
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(gen::GFlatGenome);
        Gem::Common::serialize_members(ar, this->localMembers_());
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GStarterIndividual();
    /** @brief A constructor that receives the genome's start values and bounds */
    GStarterIndividual(
        const std::size_t &,
        const std::vector<double> &,
        const std::vector<double> &,
        const std::vector<double> &
    );
    /** @brief A standard copy constructor */
    GStarterIndividual(const GStarterIndividual &);
    /** @brief The standard destructor */
    virtual ~GStarterIndividual();

    /** @brief Allows to set the demo function */
    void setTargetFunction(targetFunction);
    /** @brief Allows to retrieve the current demo function */
    targetFunction getTargetFunction() const;

    /** @brief Emit information about this individual */
    std::string print();

    //---------------------------------------------------------------------------
    // GFlatIndividualFactory<GStarterIndividual> hooks. The individual supplies the static hooks the
    // generic factory needs: describeConfig (the configurable values,
    // including the target function), buildGenome (one constrained double per start value), applyConfig
    // (the per-object, non-genome target function) and buildAdaptionConfig (the OA-owned Gauss adaption
    // config, authored from the Config -- NOT stored on the individual). The full ctor + addContent()
    // below remain as a standalone (factory-less) construction path, used by the unit tests.

    /** @brief The configurable values parsed from the config file. */
    struct Config {
        double ad_prob = GSI_DEF_ADPROB;
        double sigma = GSI_DEF_SIGMA;
        double sigma_sigma = GSI_DEF_SIGMASIGMA;
        double min_sigma = GSI_DEF_MINSIGMA;
        double max_sigma = GSI_DEF_MAXSIGMA;
        std::vector<double> start_values{1., 1., 1.};
        std::vector<double> lower_boundaries{0., 0., 0.};
        std::vector<double> upper_boundaries{2., 2., 2.};
        targetFunction target_function = GO_DEF_TARGETFUNCTION;
    };

    /** @brief Registers the config-file options, binding them to the passed Config */
    static void describeConfig(Gem::Common::GParserBuilder &gpb, Config &c);
    /** @brief Builds the flat genome's structure: one constrained double per start value */
    static gen::GenomeData buildGenome(const Config &c);
    /** @brief Per-object post-config hook: stamps the (non-genome) target function */
    static void applyConfig(GStarterIndividual &ind, const Config &c);
    /** @brief The OA-owned Gauss adaption config: every parameter group gets the configured adaptor.
     *  Authored from the Config -- no adaptor data resides on the individual. */
    static std::shared_ptr<OptimizationAlgorithms::GAdaptionConfigBase>
    buildAdaptionConfig(const gen::GFlatGenome &sample, const Config &c);

    /***************************************************************************/
    /**
	  * This function is used to unify the setup from within the constructor
	  * and factory.
	  */
    static void addContent(
        GStarterIndividual &p,
        const std::size_t &prod_id,
        const std::vector<double> &startValues,
        const std::vector<double> &lowerBoundaries,
        const std::vector<double> &upperBoundaries
    ) {
        // Some error checking
#ifdef DEBUG
        // Check whether values have been provided
        if(startValues.empty()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GStarterIndividual::addContent(): Error!" << '\n'
                << "No parameters given" << '\n'
            );
        }

        // Check whether all sizes match
        if(startValues.size() != lowerBoundaries.size() ||
           startValues.size() != upperBoundaries.size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GStarterIndividual::addContent(): Error!" << '\n'
                << "Invalid sizes" << startValues.size() << " / " << lowerBoundaries.size()
                << " / " << upperBoundaries.size() << '\n'
            );
        }

        // Check that start values and boundaries have valid values
        for(std::size_t i = 0; i < startValues.size(); i++) {
            Gem::Common::checkValueRange( // We expect the start value to be in the range [lower, upper[
				 startValues.at(i)
				 , lowerBoundaries.at(i)
				 , upperBoundaries.at(i)
				 , false // closed lower boundary
				 , true  // open upper boundary
			 );
        }

#endif /* DEBUG */

        // Build the flat genome's STRUCTURE: one constrained double per parameter. The Gauss adaptor lives
        // solely on the OA-owned config (buildAdaptionConfig()); none of its parameters reside here.
        gen::GGenomeBuilder b;
        for(std::size_t i = 0; i < startValues.size(); i++) {
            b.addDouble(startValues.at(i), lowerBoundaries.at(i), upperBoundaries.at(i));
        }
        p.setGenome(b.build());

        // The first individual (prod_id == 0) keeps the supplied start values; all others start
        // randomly within bounds.
        if(prod_id != 0) {
            p.randomInit(Gem::Geneva::activityMode::ALLPARAMETERS);
        }
    }

protected:
    /***************************************************************************/
    /** @brief Loads the data of another GStarterIndividual */
    virtual void load_(const gen::GOptimizableEntity *) final;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GStarterIndividual>(
        GStarterIndividual const &,
        GStarterIndividual const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    virtual void compare_(
        const gen::GOptimizableEntity & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const final;

    /** @brief The evaluation hook: the selected target function on the genome (single criterion). */
    std::vector<double> evaluate() final;

    /** @brief Applies modifications to this object. */
    virtual bool modify_GUnitTests_();
    /** @brief Performs self tests that are expected to succeed. */
    virtual void specificTestsNoFailureExpected_GUnitTests_();
    /** @brief Performs self tests that are expected to fail. */
    virtual void specificTestsFailuresExpected_GUnitTests_();

private:
    /***************************************************************************/

    targetFunction targetFunction_ =
        GO_DEF_TARGETFUNCTION; ///< Specifies which demo function should be used

    /***************************************************************************/
    /** @brief Creates a deep clone of this object */
    virtual gen::GFlatGenome *clone_() const final;

    /***************************************************************************/
    /** @brief A simple n-dimensional parabola */
    double parabola(const std::vector<double> &parVec) const;
    /** @brief A "noisy" parabola */
    double noisyParabola(const std::vector<double> &parVec) const;

    /***************************************************************************/
};

/** @brief Allows to output a GStarterIndividual (or convert it to a string) via its operator<< */
std::ostream &operator<<(std::ostream &, std::shared_ptr<GStarterIndividual>);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory for GStarterIndividual objects: an alias for the generic, config-driven
 * GFlatIndividualFactory, for which GStarterIndividual supplies the static describeConfig /
 * buildGenome / applyConfig / buildAdaptionConfig hooks. Call sites use ctor(path), get_as<>(),
 * registerContentCreator() and the factory's getAdaptionConfig(sample).
 */
using GStarterIndividualFactory = Gem::Geneva::Genome::GFlatIndividualFactory<GStarterIndividual>;

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GStarterIndividual) // NOLINT
