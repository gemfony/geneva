/**
 * @file GStarterIndividual.cpp
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

#include "GStarterIndividual.hpp"

#include "geneva/oa/GAdaption.hpp"
#include "geneva/oa/GAdaptionConfig.hpp"

#include <any>

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#endif /* GEM_TESTING */

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GStarterIndividual) // NOLINT
namespace Gem::Geneva {

/******************************************************************************/
/**
 * Puts a Gem::Geneva::targetFunction item into a stream
 *
 * @param o The ostream the item should be added to
 * @param tF the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::targetFunction &tF) {
    Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(tF);
    o << tmp;
    return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::targetFunction item from a stream
 *
 * @param i The stream the item should be read from
 * @param tF The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::targetFunction &tF) {
    Gem::Common::ENUMBASETYPE tmp;
    i >> tmp;

#ifdef DEBUG
    tF = Gem::Common::narrow<Gem::Geneva::targetFunction>(tmp);
#else
    tF = static_cast<Gem::Geneva::targetFunction>(tmp);
#endif /* DEBUG */

    return i;
}

/******************************************************************************/
/**
 * The default constructor -- intentionally private. Note that some data members
 * may be initialized in the class body.
 */
GStarterIndividual::GStarterIndividual() { /* nothing */
}

/******************************************************************************/
/**
 * The standard constructor. The number of parameters is determined using the number of
 * entries in the startValues vector. Note that all vector arguments need to have the
 * same dimension.
 */
GStarterIndividual::GStarterIndividual(
    const std::size_t &prod_id,
    const std::vector<double> &startValues,
    const std::vector<double> &lowerBoundaries,
    const std::vector<double> &upperBoundaries,
    const double &sigma,
    const double &sigmaSigma,
    const double &minSigma,
    const double &maxSigma,
    const double &adProb
)
  : targetFunction_(targetFunction::PARABOLA) {
    try {
        // The following is a static function used both here
        // and in the factory, so setup code cannot diverge
        GStarterIndividual::addContent(
            *this,
            prod_id,
            startValues,
            lowerBoundaries,
            upperBoundaries,
            sigma,
            sigmaSigma,
            minSigma,
            maxSigma,
            adProb
        );
    }
    catch(const geneva_exception &e) {
        glogger << e.what() << GTERMINATION;
    }
    catch(...) {
        glogger << "Unknown exception caught" << '\n' << GTERMINATION;
    }
}

/******************************************************************************/
/**
 * A standard copy constructor
 *
 * @param cp A copy of another GFunctionIndidivual
 */
GStarterIndividual::GStarterIndividual(const GStarterIndividual &cp)
  : gen::GFlatGenome(cp)
  , targetFunction_(cp.targetFunction_)
  , seed_sigma_(cp.seed_sigma_)
  , seed_sigma_sigma_(cp.seed_sigma_sigma_)
  , seed_min_sigma_(cp.seed_min_sigma_)
  , seed_max_sigma_(cp.seed_max_sigma_)
  , seed_ad_prob_(cp.seed_ad_prob_) { /* nothing */
}

/******************************************************************************/
/**
 * The standard destructor
 */
GStarterIndividual::~GStarterIndividual() { /* nothing */
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GStarterIndividual object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GStarterIndividual::compare_(
    const gen::GOptimizableEntity &cp,
    const Gem::Common::expectation &e,
    const double &limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GStarterIndividual reference independent of this object and convert the pointer
    const GStarterIndividual *p_load =
        Gem::Common::g_convert_and_compare<gen::GOptimizableEntity, GStarterIndividual>(&cp, this);

    Gem::Common::GToken token("GStarterIndividual", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<gen::GFlatGenome>(*this, *p_load, token);

    // ... and then the local data
    Gem::Common::compare_t(IDENTITY(targetFunction_, p_load->targetFunction_), token);
    Gem::Common::compare_t(IDENTITY(seed_sigma_, p_load->seed_sigma_), token);
    Gem::Common::compare_t(IDENTITY(seed_sigma_sigma_, p_load->seed_sigma_sigma_), token);
    Gem::Common::compare_t(IDENTITY(seed_min_sigma_, p_load->seed_min_sigma_), token);
    Gem::Common::compare_t(IDENTITY(seed_max_sigma_, p_load->seed_max_sigma_), token);
    Gem::Common::compare_t(IDENTITY(seed_ad_prob_, p_load->seed_ad_prob_), token);

    // React on deviations from the expectation
    token.evaluate();
}

/*******************************************************************************************/
/**
 * Allows to set the demo function
 *
 * @param tF The id if the demo function
 */
void GStarterIndividual::setTargetFunction(targetFunction tF) {
    targetFunction_ = tF;
}

/*******************************************************************************************/
/**
 * Allows to retrieve the demo function
 *
 * @return The id of the currently selected demo function
 */
targetFunction GStarterIndividual::getTargetFunction() const {
    return targetFunction_;
}

/*******************************************************************************************/
/**
 * Retrieves the average value of all sigmas used in Gauss adaptors.
 *
 * @return The average value of sigma used in Gauss adaptors
 */
double GStarterIndividual::getAverageSigma() const {
    // The Gauss adaptor configuration now lives on the OA-owned config (the genome is structure-only), and
    // the live evolving per-group sigmas are OA-owned scratch on the GIndividualSlot, not on the
    // individual. An individual queried in isolation (as here) is detached from its slot, so this reports
    // the configured SEED sigma stamped at construction (every parameter group shares one configuration).
    return seed_sigma_;
}

/******************************************************************************/
/**
 * Builds the OA-owned adaption configuration: every parameter group receives a Gauss adaptor with this
 * individual's stamped (configured) parameters.
 */
std::shared_ptr<OptimizationAlgorithms::GAdaptionConfigBase> GStarterIndividual::getAdaptionConfig() const {
    auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(*this);
    for(std::size_t i = 0; i < cfg->doubleGroups().size(); i++) {
        cfg->groupDouble(i).gauss(seed_sigma_, seed_sigma_sigma_, seed_min_sigma_, seed_max_sigma_, seed_ad_prob_);
    }
    return cfg;
}

/******************************************************************************/
/**
 * Emit information about this individual
 */
std::string GStarterIndividual::print() {
    std::ostringstream result;

    // Retrieve the parameters
    std::vector<double> parVec;
    this->streamline(parVec);

    result << "GStarterIndividual with target function "
           << (targetFunction_ == targetFunction::PARABOLA ? " PARABOLA" : " NOISY PARABOLA")
           << '\n'
           << "and raw fitness " << this->raw_fitness(0)
           << " has the following parameter values:" << '\n';

    for(std::size_t i = 0; i < parVec.size(); i++) {
        result << i << ": " << parVec.at(i) << '\n';
    }
    result << "The average sigma of this individual is " << this->getAverageSigma() << '\n';

    return result.str();
}

/******************************************************************************/
/**
 * Loads the data of another GStarterIndividual, camouflaged as a GFlatGenome
 *
 * @param cp A copy of another GStarterIndividual, camouflaged as a GFlatGenome
 */
void GStarterIndividual::load_(const gen::GOptimizableEntity *cp) {
    // Check that we are dealing with a GStarterIndividual reference independent of this object and convert the pointer
    const GStarterIndividual *p_load =
        Gem::Common::g_convert_and_compare<gen::GOptimizableEntity, GStarterIndividual>(cp, this);

    // Load our parent class'es data ...
    gen::GFlatGenome::load_(cp);

    // ... and then our local data
    targetFunction_ = p_load->targetFunction_;
    seed_sigma_ = p_load->seed_sigma_;
    seed_sigma_sigma_ = p_load->seed_sigma_sigma_;
    seed_min_sigma_ = p_load->seed_min_sigma_;
    seed_max_sigma_ = p_load->seed_max_sigma_;
    seed_ad_prob_ = p_load->seed_ad_prob_;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GFlatGenome
 */
gen::GFlatGenome *GStarterIndividual::clone_() const {
    return new GStarterIndividual(*this);
}

/******************************************************************************/
/**
 * The actual value calculation takes place here
 *
 * @param The id of the target function (ignored here)
 * @return The value of this object, as calculated with the evaluation function
 */
double GStarterIndividual::fitnessCalculation() {
    // Retrieve the parameters
    std::vector<double> parVec;
    this->streamline(parVec);

    // Perform the actual calculation
    switch(targetFunction_) {
    //-----------------------------------------------------------
    // A simple, multi-dimensional parabola
    case targetFunction::PARABOLA:
        return parabola(parVec);
        break;

    //-----------------------------------------------------------
    // A "noisy" parabola, i.e. a parabola with a very large
    // number of overlaid local optima
    case targetFunction::NOISYPARABOLA:
        return noisyParabola(parVec);
        break;
        //-----------------------------------------------------------
    };

    // Make the compiler happy
    return 0.;
}

/******************************************************************************/
/**
 * A simple n-dimensional parabola
 */
double GStarterIndividual::parabola(const std::vector<double> &parVec) const {
    double result = 0.;

    std::vector<double>::const_iterator cit;
    for(cit = parVec.begin(); cit != parVec.end(); ++cit) {
        result += (*cit) * (*cit);
    }

    return result;
}

/******************************************************************************/
/**
 * A "noisy" parabola
 */
double GStarterIndividual::noisyParabola(const std::vector<double> &parVec) const {
    double xsquared = 0.;

    std::vector<double>::const_iterator cit;
    for(cit = parVec.begin(); cit != parVec.end(); ++cit) {
        xsquared += (*cit) * (*cit);
    }

    return (cos(xsquared) + 2.) * xsquared;
}

/******************************************************************************/
/**
 * Applies modifications to this object. This function is only useful
 * if you wish to run unit tests with your individual.
 *
 * @return A boolean indicating whether
 */
bool GStarterIndividual::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    // Call the parent classes' functions
    if(gen::GFlatGenome::modify_GUnitTests_()) {
        result = true;
    }

    // Change the parameter settings (only when the genome has actually been built). The adaption state +
    // logic are OA-owned; a standalone individual drives them via a self-owned scratch +
    // config (StandaloneAdapter).
    if(this->countParameters<double>() > 0) {
        // The genome is structure-only; drive the self-owned adaption via the individual's authored config.
        oa::StandaloneAdapter(*this, getAdaptionConfig()).adapt(*this);
        result = true;
    }

    // Let the audience know whether we have changed the content
    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GStarterIndividual::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This function is only useful
 * if you wish to run unit tests with your individual.
 */
void GStarterIndividual::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    using namespace Gem::Geneva;

    // Call the parent classes' functions
    gen::GFlatGenome::specificTestsNoFailureExpected_GUnitTests_();

    //------------------------------------------------------------------------------

    {
        const std::size_t NENTRIES = 100;
        double DEFAULTSIGMA = 0.025;

        // Check standard construction and whether calculation of the average sigma works
        std::vector<double> startValues;
        std::vector<double> lowerBoundaries;
        std::vector<double> upperBoundaries;

        for(std::size_t n = 0; n < NENTRIES; n++) {
            startValues.push_back(1.);
            lowerBoundaries.push_back(0.);
            upperBoundaries.push_back(2.);
        }

        std::shared_ptr<GStarterIndividual> p_test;
        CHECK_NOTHROW(
            p_test = std::shared_ptr<GStarterIndividual>(new GStarterIndividual(
                0 // indicates the first individual
                ,
                startValues,
                lowerBoundaries,
                upperBoundaries,
                DEFAULTSIGMA,
                0.6,
                0.001,
                2.,
                0.05
            ))
        );

        CHECK_THAT(
            p_test->getAverageSigma(),
            Catch::Matchers::WithinRel(DEFAULTSIGMA, 0.001 / 100.0)
        ); // Should be similar
    }

    //------------------------------------------------------------------------------

    { // Test setting and retrieval of the target function valie
        std::shared_ptr<GStarterIndividual> p_test = this->clone<GStarterIndividual>();

        CHECK_NOTHROW(p_test->setTargetFunction(targetFunction::PARABOLA));
        CHECK(targetFunction::PARABOLA == p_test->getTargetFunction());

        CHECK_NOTHROW(p_test->setTargetFunction(targetFunction::NOISYPARABOLA));
        CHECK(targetFunction::NOISYPARABOLA == p_test->getTargetFunction());
    }

    //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GStarterIndividual::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This function is only useful
 * if you wish to run unit tests with your individual.
 */
void GStarterIndividual::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    using namespace Gem::Geneva;

    // Call the parent classes' functions
    gen::GFlatGenome::specificTestsFailuresExpected_GUnitTests_();

    //------------------------------------------------------------------------------

    {
        /* Nothing. Add test cases here that are expected to fail.
			Enclose with a BOOST_CHECK_THROW, using the expected
			exception type as an additional argument. See the
			documentation for the Boost.Test library for further
			information */
    }

    //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GStarterIndividual::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Allows to output a GStarterIndividual (or convert it to a string) via its operator<<
 */
std::ostream &operator<<(std::ostream &stream, std::shared_ptr<GStarterIndividual> gsi_ptr) {
    stream << gsi_ptr->print();
    return stream;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Registers the config-file options, binding them to the passed Config. This is the body of the former
 * GStarterIndividualFactory::describeLocalOptions_ (now binding plain Config fields) plus the
 * target_function option the individual formerly registered in its own addConfigurationOptions.
 */
void GStarterIndividual::describeConfig(Gem::Common::GParserBuilder &gpb, Config &c) {
    gpb.registerFileParameter<double>("ad_prob", c.ad_prob, GSI_DEF_ADPROB)
        << "The probability for random adaptions of values in evolutionary algorithms";

    gpb.registerFileParameter<double>("sigma", c.sigma, GSI_DEF_SIGMA)
        << "The sigma for gauss-adaption in ES";

    gpb.registerFileParameter<double>("sigma_sigma", c.sigma_sigma, GSI_DEF_SIGMASIGMA)
        << "Influences the self-adaption of gauss-mutation in ES";

    gpb.registerFileParameter<double>("min_sigma", c.min_sigma, GSI_DEF_MINSIGMA)
        << "The minimum amount value of sigma";

    gpb.registerFileParameter<double>("max_sigma", c.max_sigma, GSI_DEF_MAXSIGMA)
        << "The maximum amount value of sigma";

    std::vector<double> defStartValues{1., 1., 1.};
    gpb.registerFileParameter<double>("start_values", c.start_values, defStartValues)
        << "The start values for all parameters" << '\n'
        << "Note that the number of entries also determines" << '\n'
        << "The number of parameter used in the optimization" << '\n'
        << "The number of entries in the vector may be changed" << '\n'
        << "in the configuration file.";

    std::vector<double> defLowerBoundaries{0., 0., 0.};
    gpb.registerFileParameter<double>("lower_boundaries", c.lower_boundaries, defLowerBoundaries)
        << "The lower boundaries for all parameters" << '\n'
        << "Note that as many entries are needed as" << '\n'
        << "There are entries in the startValues vector";

    std::vector<double> defUpperBoundaries{2., 2., 2.};
    gpb.registerFileParameter<double>("upper_boundaries", c.upper_boundaries, defUpperBoundaries)
        << "The upper boundaries for all parameters" << '\n'
        << "Note that as many entries are needed as" << '\n'
        << "There are entries in the startValues vector";

    gpb.registerFileParameter<targetFunction>(
        "target_function", c.target_function, GO_DEF_TARGETFUNCTION
    ) << "Specifies which target function should be used:" << '\n'
      << "0: Parabola" << '\n'
      << "1: Berlich";
}

/******************************************************************************/
/**
 * Builds the flat genome's STRUCTURE only (the structure-building part of addContent()): one constrained
 * double per start value, in [lower, upper). The start value is the genome's initial value; the
 * optimization algorithm random-initialises within bounds. The Gauss adaptor settings live on the
 * OA-owned config (see getAdaptionConfig()), stamped onto the individual by applyConfig().
 */
gen::GenomeData GStarterIndividual::buildGenome(const Config &c) {
#ifdef DEBUG
    if(c.start_values.empty() || c.start_values.size() != c.lower_boundaries.size() ||
       c.start_values.size() != c.upper_boundaries.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GStarterIndividual::buildGenome(): Error!" << '\n'
            << "Invalid sizes " << c.start_values.size() << " / " << c.lower_boundaries.size()
            << " / " << c.upper_boundaries.size() << '\n'
        );
    }
#endif /* DEBUG */

    gen::GGenomeBuilder b;
    for(std::size_t i = 0; i < c.start_values.size(); i++) {
        b.addDouble(c.start_values.at(i), c.lower_boundaries.at(i), c.upper_boundaries.at(i));
    }
    return b.build();
}

/******************************************************************************/
/**
 * Per-object post-config hook (the per-object tail of addContent()): the target function and the Gauss
 * adaptor parameters the individual stamps so its getAdaptionConfig() can author the OA-owned config and
 * report its configured seed sigma.
 */
void GStarterIndividual::applyConfig(GStarterIndividual &ind, const Config &c) {
    ind.setTargetFunction(c.target_function);
    ind.seed_sigma_ = c.sigma;
    ind.seed_sigma_sigma_ = c.sigma_sigma;
    ind.seed_min_sigma_ = c.min_sigma;
    ind.seed_max_sigma_ = c.max_sigma;
    ind.seed_ad_prob_ = c.ad_prob;
}

/******************************************************************************/

}
