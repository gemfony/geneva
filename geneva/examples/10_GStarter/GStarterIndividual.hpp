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
#include "common/GFactoryT.hpp"
#include "common/GParserBuilder.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"

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
/** @brief Puts a Gem::Geneva::targetFunction into a stream. Needed also for boost::lexical_cast<> */
std::ostream &operator<<(std::ostream &, const Gem::Geneva::targetFunction &);

/** @brief Reads a Gem::Geneva::targetFunction from a stream. Needed also for boost::lexical_cast<> */
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
class GStarterIndividual : public gpar::GFlatGenome {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive &ar, const unsigned int) {
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(gpar::GFlatGenome) &
            BOOST_SERIALIZATION_NVP(targetFunction_) & BOOST_SERIALIZATION_NVP(seed_sigma_) &
            BOOST_SERIALIZATION_NVP(seed_sigma_sigma_) & BOOST_SERIALIZATION_NVP(seed_min_sigma_) &
            BOOST_SERIALIZATION_NVP(seed_max_sigma_) & BOOST_SERIALIZATION_NVP(seed_ad_prob_);
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GStarterIndividual();
    /** @brief A constructor that receives all arguments */
    GStarterIndividual(
        const std::size_t &,
        const std::vector<double> &,
        const std::vector<double> &,
        const std::vector<double> &,
        const double &,
        const double &,
        const double &,
        const double &,
        const double &
    );
    /** @brief A standard copy constructor */
    GStarterIndividual(const GStarterIndividual &);
    /** @brief The standard destructor */
    virtual ~GStarterIndividual();

    /** @brief Adds local configuration options to a GParserBuilder object */
    virtual void addConfigurationOptions(Gem::Common::GParserBuilder &) final;

    /** @brief Allows to set the demo function */
    void setTargetFunction(targetFunction);
    /** @brief Allows to retrieve the current demo function */
    targetFunction getTargetFunction() const;

    /** @brief Retrieves the average value of the sigma used in local Gauss adaptors */
    double getAverageSigma() const;

    /** @brief The OA-owned Gauss adaption config authoring every parameter group with this individual's
     *  configured (stamped) adaptor parameters. */
    std::shared_ptr<OptimizationAlgorithms::GAdaptionConfigBase> getAdaptionConfig() const;

    /** @brief Emit information about this individual */
    std::string print();

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
        const std::vector<double> &upperBoundaries,
        const double &sigma,
        const double &sigmaSigma,
        const double &minSigma,
        const double &maxSigma,
        const double &adProb
    ) {
        // Some error checking
#ifdef DEBUG
        // Check whether values have been provided
        if(startValues.empty()) {
            glogger << "In GStarterIndividual::addContent(): Error!" << '\n'
                    << "No parameters given" << '\n'
                    << GTERMINATION;
        }

        // Check whether all sizes match
        if(startValues.size() != lowerBoundaries.size() ||
           startValues.size() != upperBoundaries.size()) {
            glogger << "In GStarterIndividual::addContent(): Error!" << '\n'
                    << "Invalid sizes" << startValues.size() << " / " << lowerBoundaries.size()
                    << " / " << upperBoundaries.size() << '\n'
                    << GTERMINATION;
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
        // on the OA-owned config (getAdaptionConfig()); stamp its parameters here so the individual can
        // author that config and report its configured seed sigma.
        gpar::GGenomeBuilder b;
        for(std::size_t i = 0; i < startValues.size(); i++) {
            b.addDouble(startValues.at(i), lowerBoundaries.at(i), upperBoundaries.at(i));
        }
        p.setGenome(b.build());
        p.seed_sigma_ = sigma;
        p.seed_sigma_sigma_ = sigmaSigma;
        p.seed_min_sigma_ = minSigma;
        p.seed_max_sigma_ = maxSigma;
        p.seed_ad_prob_ = adProb;

        // The first individual keeps the supplied start values; all others start randomly within bounds.
        if(Gem::Common::GFACTTORYFIRSTID != prod_id) {
            p.randomInit(Gem::Geneva::activityMode::ALLPARAMETERS);
        }
    }

protected:
    /***************************************************************************/
    /** @brief Loads the data of another GStarterIndividual */
    virtual void load_(const gpar::GOptimizableEntity *) final;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GStarterIndividual>(
        GStarterIndividual const &,
        GStarterIndividual const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    virtual void compare_(
        const gpar::GOptimizableEntity & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const final;

    /** @brief The actual value calculation takes place here */
    virtual double fitnessCalculation() final;

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
    // The configured Gauss adaptor parameters, stamped by addContent(). They live here (not in the
    // structure-only genome layout) so the individual can author its OA-owned adaption config and report
    // its configured seed sigma. All parameter groups share one configuration.
    double seed_sigma_ = 0.025;
    double seed_sigma_sigma_ = 0.6;
    double seed_min_sigma_ = 0.001;
    double seed_max_sigma_ = 2.;
    double seed_ad_prob_ = 0.05;

    /***************************************************************************/
    /** @brief Creates a deep clone of this object */
    virtual gpar::GFlatGenome *clone_() const final;

    /***************************************************************************/
    /** @brief A simple n-dimensional parabola */
    double parabola(const std::vector<double> &parVec) const;
    /** @brief A "noisy" parabola */
    double noisyParabola(const std::vector<double> &parVec) const;

    /***************************************************************************/
};

/** @brief Allows to output a GStarterIndividual or convert it to a string using boost::lexical_cast */
std::ostream &operator<<(std::ostream &, std::shared_ptr<GStarterIndividual>);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory for GStarterIndividual objects
 */
class GStarterIndividualFactory : public Gem::Common::GFactoryT<gpar::GOptimizableEntity> {
public:
    /** @brief The standard constructor */
    explicit GStarterIndividualFactory(std::filesystem::path const &);
    /** @brief The destructor */
    ~GStarterIndividualFactory() override = default;

protected:
    /** @brief Allows to describe local configuration options in derived classes */
    void describeLocalOptions_(Gem::Common::GParserBuilder &) override;
    /** @brief Allows to act on the configuration options received from the configuration file */
    void postProcess_(std::shared_ptr<gpar::GOptimizableEntity> &) override;

private:
    /** @brief The default constructor. Only needed for (de-)serialization purposes */
    GStarterIndividualFactory() = default;
    /** @brief Creates individuals of this type */
    std::shared_ptr<gpar::GOptimizableEntity>
    getObject_(Gem::Common::GParserBuilder &, const std::size_t &) override;

    double adProb_ = GSI_DEF_ADPROB;         ///< Probability for a parameter to be mutated
    double sigma_ = GSI_DEF_SIGMA;           ///< Step-width
    double sigmaSigma_ = GSI_DEF_SIGMASIGMA; ///< Speed of sigma_-adaption
    double minSigma_ = GSI_DEF_MINSIGMA;     ///< Minimum allowed sigma value
    double maxSigma_ = GSI_DEF_MAXSIGMA;     ///< Maximum allowed sigma value

    std::vector<double> startValues_;     ///< Start values for all parameters
    std::vector<double> lowerBoundaries_; ///< Lower boundaries for all parameters
    std::vector<double> upperBoundaries_; ///< Upper boundaroes for all parameters
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GStarterIndividual) // NOLINT
