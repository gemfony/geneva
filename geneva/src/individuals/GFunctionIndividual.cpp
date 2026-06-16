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

#include "geneva/individuals/GFunctionIndividual.hpp"
#include "geneva/individuals/GBenchmarkFunctions.hpp"
#include "common/GCommonEnums.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GFactoryT.hpp"
#include "common/GLogger.hpp"
#include "common/GParserBuilder.hpp"
#include "geneva/GMultiConstraintT.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
#include "geneva/oa/GAdaption.hpp"
#include "geneva/oa/GAdaptionConfig.hpp"
#include "geneva/par/GOptimizableEntityFactory.hpp"
#include "geneva/par/GOptimizableEntityMultiConstraint.hpp"
#include "hap/GRandomT.hpp"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <istream>
#include <memory>
#include <ostream>
#include <tuple>
#include <vector>

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Individuals::GFunctionIndividual)        // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Individuals::GFunctionIndividualFactory) // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Individuals::GDoubleSumConstraint)       // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Individuals::GDoubleSumGapConstraint)    // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Individuals::GSphereConstraint)          // NOLINT

namespace Gem::Geneva::Individuals {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization with the constant
 */
GDoubleSumConstraint::GDoubleSumConstraint(const double &c)
  : c_(c) { /* nothing */
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GPreEvaluationValidityCheckT object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GDoubleSumConstraint::compare_(
    const GPreEvaluationValidityCheckT<gen::GOptimizableEntity> &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GDoubleSumConstraint reference independent of this object and convert the pointer
    const GDoubleSumConstraint *p_load =
        Gem::Common::g_convert_and_compare<
            GPreEvaluationValidityCheckT<gen::GOptimizableEntity>,
            GDoubleSumConstraint>(cp, this);

    Gem::Common::GToken token("GDoubleSumConstraint", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<gen::GOptimizableEntityConstraint>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    Gem::Common::g_compare_members(localMembers(), p_load->localMembers(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 */
void GDoubleSumConstraint::addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) {
    // Call our parent class'es function
    gen::GOptimizableEntityConstraint::addConfigurationOptions_(gpb);
}

/******************************************************************************/
/**
 * Checks whether a given individual is valid
 */
double GDoubleSumConstraint::check_(const gen::GOptimizableEntity *p) const {
    std::vector<double> par_vec;
    p->streamlineFP(par_vec);

    double sum = 0.;
    for(const auto &val : par_vec) {
        sum += val;
    }

    if(sum < c_) {
        return 0.;
    }
            return sum / c_;
   
}

/******************************************************************************/
/**
 * Loads the data of another GDoubleSumConstraint
 */
void GDoubleSumConstraint::load_(const GPreEvaluationValidityCheckT<gen::GOptimizableEntity> *cp) {
    // Check that we are dealing with a GDoubleSumConstraint reference independent of this object and convert the pointer
    const GDoubleSumConstraint *p_load =
        Gem::Common::g_convert_and_compare<
            GPreEvaluationValidityCheckT<gen::GOptimizableEntity>,
            GDoubleSumConstraint>(cp, this);

    // Load our parent class'es data ...
    gen::GOptimizableEntityConstraint::load_(cp);

    // ... and then our local data, derived from the single localMembers() declaration
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GPreEvaluationValidityCheckT<gen::GOptimizableEntity> *GDoubleSumConstraint::clone_() const {
    return new GDoubleSumConstraint(*this);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization with the constant
 */
GDoubleSumGapConstraint::GDoubleSumGapConstraint(const double &c, const double &gap)
  : c_(c)
  , gap_(gap) { /* nothing */
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GPreEvaluationValidityCheckT object
 * @param e The expected outcome of the comparison
 */
void GDoubleSumGapConstraint::compare_(
    const GPreEvaluationValidityCheckT<gen::GOptimizableEntity> &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GDoubleSumGapConstraint reference independent of this object and convert the pointer
    const GDoubleSumGapConstraint *p_load =
        Gem::Common::g_convert_and_compare<
            GPreEvaluationValidityCheckT<gen::GOptimizableEntity>,
            GDoubleSumGapConstraint>(cp, this);

    Gem::Common::GToken token("GDoubleSumGapConstraint", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<gen::GOptimizableEntityConstraint>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    Gem::Common::g_compare_members(localMembers(), p_load->localMembers(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 */
void GDoubleSumGapConstraint::addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) {
    // Call our parent class'es function
    gen::GOptimizableEntityConstraint::addConfigurationOptions_(gpb);
}

/******************************************************************************/
/**
 * Checks whether a given individual is valid
 */
double GDoubleSumGapConstraint::check_(const gen::GOptimizableEntity *p) const {
    std::vector<double> par_vec;
    p->streamlineFP(par_vec);

    double sum = 0.;
    for(const auto &val : par_vec) {
        sum += val;
    }

    // Is the sum in the allowed corridor ?
    if(sum >= (c_ - gap_) && sum <= (c_ + gap_)) {
        return 0.;
    }
            return 1. + fabs(sum - c_) / c_;
   
}

/******************************************************************************/
/**
 * Loads the data of another GDoubleSumGapConstraint
 */
void GDoubleSumGapConstraint::load_(const GPreEvaluationValidityCheckT<gen::GOptimizableEntity> *cp) {
    // Check that we are dealing with a GDoubleSumGapConstraint reference independent of this object and convert the pointer
    const GDoubleSumGapConstraint *p_load =
        Gem::Common::g_convert_and_compare<
            GPreEvaluationValidityCheckT<gen::GOptimizableEntity>,
            GDoubleSumGapConstraint>(cp, this);

    // Load our parent class'es data ...
    gen::GOptimizableEntityConstraint::load_(cp);

    // ... and then our local data, derived from the single localMembers() declaration
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GPreEvaluationValidityCheckT<gen::GOptimizableEntity> *GDoubleSumGapConstraint::clone_() const {
    return new GDoubleSumGapConstraint(*this);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization with the diameter
 */
GSphereConstraint::GSphereConstraint(const double &diameter)
  : diameter_(diameter) { /* nothing */
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GPreEvaluationValidityCheckT object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GSphereConstraint::compare_(
    const GPreEvaluationValidityCheckT<gen::GOptimizableEntity> &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    // Check that we are dealing with a GSphereConstraint reference independent of this object and convert the pointer
    const GSphereConstraint *p_load =
        Gem::Common::g_convert_and_compare<
            GPreEvaluationValidityCheckT<gen::GOptimizableEntity>,
            GSphereConstraint>(cp, this);

    Gem::Common::GToken token("GSphereConstraint", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<gen::GOptimizableEntityConstraint>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    Gem::Common::g_compare_members(localMembers(), p_load->localMembers(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 */
void GSphereConstraint::addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) {
    // Call our parent class'es function
    gen::GOptimizableEntityConstraint::addConfigurationOptions_(gpb);
}

/******************************************************************************/
/**
 * Checks whether a given individual is valid
 */
double GSphereConstraint::check_(const gen::GOptimizableEntity *p) const {
    std::vector<double> par_vec;
    p->streamlineFP(par_vec);

    double sum = 0.;
    for(const auto &val : par_vec) {
        sum += Gem::Common::gsquared(val);
    }
    sum = sqrt(sum);

    if(sum <= diameter_) {
        return 0.;
    }
            return Gem::Common::gsquared(sum / diameter_);
   
}

/******************************************************************************/
/**
 * Loads the data of another GSphereConstraint
 */
void GSphereConstraint::load_(const GPreEvaluationValidityCheckT<gen::GOptimizableEntity> *cp) {
    // Check that we are dealing with a GSphereConstraint reference independent of this object and convert the pointer
    const GSphereConstraint *p_load =
        Gem::Common::g_convert_and_compare<
            GPreEvaluationValidityCheckT<gen::GOptimizableEntity>,
            GSphereConstraint>(cp, this);

    // Load our parent class'es data ...
    gen::GOptimizableEntityConstraint::load_(cp);

    // ... and then our local data, derived from the single localMembers() declaration
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GPreEvaluationValidityCheckT<gen::GOptimizableEntity> *GSphereConstraint::clone_() const {
    return new GSphereConstraint(*this);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Puts a Gem::Geneva::Individuals::solverFunction item into a stream
 *
 * @param o The ostream the item should be added to
 * @param ur the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::Individuals::solverFunction &ur) {
    auto tmp = static_cast<Gem::Common::ENUMBASETYPE>(ur);
    o << tmp;
    return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::Individuals::solverFunction item from a stream
 *
 * @param i The stream the item should be read from
 * @param ur The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::Individuals::solverFunction &ur) {
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;

#ifdef DEBUG
    ur = Gem::Common::narrow<Gem::Geneva::Individuals::solverFunction>(tmp);
#else
    ur = static_cast<Gem::Geneva::Individuals::solverFunction>(tmp);
#endif /* DEBUG */

    return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Geneva::Individuals::parameterType item into a stream
 *
 * @param o The ostream the item should be added to
 * @param ur the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::Individuals::parameterType &ur) {
    auto tmp = static_cast<Gem::Common::ENUMBASETYPE>(ur);
    o << tmp;
    return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::Individuals::parameterType item from a stream
 *
 * @param i The stream the item should be read from
 * @param ur The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::Individuals::parameterType &ur) {
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;

#ifdef DEBUG
    ur = Gem::Common::narrow<Gem::Geneva::Individuals::parameterType>(tmp);
#else
    ur = static_cast<Gem::Geneva::Individuals::parameterType>(tmp);
#endif /* DEBUG */

    return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Geneva::Individuals::initMode item into a stream
 *
 * @param o The ostream the item should be added to
 * @param ur the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::Individuals::initMode &ur) {
    auto tmp = static_cast<Gem::Common::ENUMBASETYPE>(ur);
    o << tmp;
    return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::Individuals::initMode item from a stream
 *
 * @param i The stream the item should be read from
 * @param ur The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::Individuals::initMode &ur) {
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;

#ifdef DEBUG
    ur = Gem::Common::narrow<Gem::Geneva::Individuals::initMode>(tmp);
#else
    ur = static_cast<Gem::Geneva::Individuals::initMode>(tmp);
#endif /* DEBUG */

    return i;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization with the desired demo function
 *
 * @param d_f The id of the demo function
 */
GFunctionIndividual::GFunctionIndividual(const solverFunction &d_f)
  : demo_function_(d_f) { /* nothing */
}

/******************************************************************************/
/**
 * Allows external entities to set the fitness
 */
void GFunctionIndividual::setFitness(std::vector<double> const &result_vec) {
    this->setFitness_(result_vec);
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GPreEvaluationValidityCheckT object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GFunctionIndividual::compare_(
    const gen::GOptimizableEntity &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    // Check that we are dealing with a GFunctionIndividual reference independent of this object and convert the pointer
    const GFunctionIndividual *p_load =
        Gem::Common::g_convert_and_compare<gen::GOptimizableEntity, GFunctionIndividual>(cp, this);

    Gem::Common::GToken token("GFunctionIndividual", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<gen::GFlatGenome>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    Gem::Common::g_compare_members(localMembers(), p_load->localMembers(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void GFunctionIndividual::addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) {
    // Call our parent class'es function. The demo_function option (and all other configurable values)
    // is now registered by the static describeConfig() hook and applied via applyConfig(), so that the
    // generic GFlatIndividualFactory<GFunctionIndividual> is a complete replacement for the former
    // bespoke factory.
    gen::GFlatGenome::addConfigurationOptions_(gpb);
}

/******************************************************************************/
/**
 * Allows to set the demo function
 *
 * @param d_f The id if the demo function
 */
void GFunctionIndividual::setDemoFunction(solverFunction d_f) {
    demo_function_ = d_f;
}

/******************************************************************************/
/**
 * Allows to retrieve the demo function
 *
 * @return The id of the currently selected demo function
 */
solverFunction GFunctionIndividual::getDemoFunction() const {
    return demo_function_;
}

/******************************************************************************/
/**
 * Allows to cross check the parameter size
 *
 * @return The number of doubles stored in this object
 */
std::size_t GFunctionIndividual::getParameterSize() const {
    // Retrieve the parameters
    std::vector<double> par_vec;
    this->streamline(par_vec);
    return par_vec.size();
}

/******************************************************************************/
/**
 * Loads the data of another GFunctionIndividual, camouflaged as a GFlatGenome
 *
 * @param cp A copy of another GFunctionIndividual, camouflaged as a GFlatGenome
 */
void GFunctionIndividual::load_(const gen::GOptimizableEntity *cp) {
    // Check that we are dealing with a GFunctionIndividual reference independent of this object and convert the pointer
    const GFunctionIndividual *p_load =
        Gem::Common::g_convert_and_compare<gen::GOptimizableEntity, GFunctionIndividual>(cp, this);

    // Load our parent class'es data ...
    gen::GFlatGenome::load_(cp);

    // ... and then our local data, derived from the single localMembers() declaration
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GFlatGenome
 */
gen::GFlatGenome *GFunctionIndividual::clone_() const {
    return new GFunctionIndividual(*this);
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GFunctionIndividual::modify_GUnitTests_() {
#ifdef GEM_TESTING

    bool result = false;

    // Call the parent classes' functions
    if(gen::GFlatGenome::modify_GUnitTests_()) {
        result = true;
    }

    // Change the parameter settings
    result = true;

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GFunctionIndividual::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self-tests that are expected to succeed. This is needed for testing purposes
 */
void GFunctionIndividual::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    using namespace Gem::Geneva;

    // Call the parent classes' functions
    gen::GFlatGenome::specificTestsNoFailureExpected_GUnitTests_();

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GFunctionIndividual::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GFunctionIndividual::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    using namespace Gem::Geneva;

    // Call the parent classes' functions
    gen::GFlatGenome::specificTestsFailuresExpected_GUnitTests_();

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GFunctionIndividual::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
	 * @brief Evaluates the individual's parameters against the selected benchmark function.
	 *
	 * Delegates to Gem::Geneva::Benchmarks::eval() in GBenchmarkFunctions.hpp, which provides
	 * the same implementations annotated for both CPU and CUDA device execution.
	 * The function set covers all 15 solverFunction IDs 0–14.
	 *
	 * @return Fitness value (lower is better for minimisation functions)
	 */
double GFunctionIndividual::fitnessCalculation() {
    std::vector<double> par_vec;
    this->streamline(par_vec);

#ifdef DEBUG
    const int id = static_cast<int>(demo_function_);
    if(par_vec.size() < 2 &&
       (demo_function_ == solverFunction::ROSENBROCK || demo_function_ == solverFunction::ACKLEY)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFunctionIndividual::fitnessCalculation(): function " << id
            << " requires at least 2 dimensions, got " << par_vec.size() << '\n'
        );
    }
#endif /* DEBUG */

    return gbm::eval(
        static_cast<int>(demo_function_),
        par_vec.data(),
        static_cast<int>(par_vec.size())
    );
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Registers the config-file options, binding them to the passed Config. This is the body of the
 * former GFunctionIndividualFactory::describeLocalOptions_ (now binding plain Config fields instead of
 * GOneTimeRefParameterT references) plus the demo_function option the individual formerly registered in
 * its own addConfigurationOptions_.
 */
void GFunctionIndividual::describeConfig(Gem::Common::GParserBuilder &gpb, Config &c) {
    std::string comment; // NOLINT(cppcoreguidelines-init-variables)

    comment = "";
    comment += "The probability for random adaption of values in evolutionary algorithms;";
    gpb.registerFileParameter<double>(
        "ad_prob", c.ad_prob, GFI_DEF_ADPROB, Gem::Common::VAR_IS_ESSENTIAL, comment
    );

    comment = "";
    comment +=
        "Determines the rate of adaption of ad_prob. Set to 0, if you do not need this feature;";
    gpb.registerFileParameter<double>(
        "adapt_ad_prob", c.adapt_ad_prob, GFI_DEF_ADAPTADPROB, Gem::Common::VAR_IS_ESSENTIAL, comment
    );

    comment = "";
    comment += "The lower allowed boundary for ad_prob-variation;";
    gpb.registerFileParameter<double>(
        "min_ad_prob", c.min_ad_prob, GFI_DEF_MINADPROB, Gem::Common::VAR_IS_ESSENTIAL, comment
    );

    comment = "";
    comment += "The upper allowed boundary for ad_prob-variation;";
    gpb.registerFileParameter<double>(
        "max_ad_prob", c.max_ad_prob, GFI_DEF_MAXADPROB, Gem::Common::VAR_IS_ESSENTIAL, comment
    );

    comment = "";
    comment += "The number of successful calls to an adaptor after which adaption;";
    comment += "of mutation parameters takes place (e.g sigma-variation in gauss mutation);";
    gpb.registerFileParameter<std::uint32_t>(
        "adaption_threshold", c.adaption_threshold, GFI_DEF_ADAPTIONTHRESHOLD,
        Gem::Common::VAR_IS_ESSENTIAL, comment
    );

    comment = "";
    comment += "Whether to use a double gaussion for the adaption of parmeters in ES;";
    gpb.registerFileParameter<bool>(
        "use_bi_gaussian", c.use_bi_gaussian, GFI_DEF_USEBIGAUSSIAN, Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment +=
        "The sigma for gauss-adaption in ES;(or the sigma of the left peak of a double gaussian);";
    gpb.registerFileParameter<double>(
        "sigma1", c.sigma1, GFI_DEF_SIGMA1, Gem::Common::VAR_IS_ESSENTIAL, comment
    );

    comment = "";
    comment += "Influences the self-adaption of gauss-mutation in ES;";
    gpb.registerFileParameter<double>(
        "sigma_sigma1", c.sigma_sigma1, GFI_DEF_SIGMASIGMA1, Gem::Common::VAR_IS_ESSENTIAL, comment
    );

    comment = "";
    comment += "The minimum value of sigma1;";
    gpb.registerFileParameter<double>(
        "min_sigma1", c.min_sigma1, GFI_DEF_MINSIGMA1, Gem::Common::VAR_IS_ESSENTIAL, comment
    );

    comment = "";
    comment += "The maximum value of sigma1;";
    gpb.registerFileParameter<double>(
        "max_sigma1", c.max_sigma1, GFI_DEF_MAXSIGMA1, Gem::Common::VAR_IS_ESSENTIAL, comment
    );

    comment = "";
    comment += "The sigma of the right peak of a double gaussian (if any);";
    gpb.registerFileParameter<double>(
        "sigma2", c.sigma2, GFI_DEF_SIGMA2, Gem::Common::VAR_IS_ESSENTIAL, comment
    );

    comment = "";
    comment += "Influences the self-adaption of gauss-mutation in ES;";
    gpb.registerFileParameter<double>(
        "sigma_sigma2", c.sigma_sigma2, GFI_DEF_SIGMASIGMA2, Gem::Common::VAR_IS_ESSENTIAL, comment
    );

    comment = "";
    comment += "The minimum value of sigma2;";
    gpb.registerFileParameter<double>(
        "min_sigma2", c.min_sigma2, GFI_DEF_MINSIGMA2, Gem::Common::VAR_IS_ESSENTIAL, comment
    );

    comment = "";
    comment += "The maximum value of sigma2;";
    gpb.registerFileParameter<double>(
        "max_sigma2", c.max_sigma2, GFI_DEF_MAXSIGMA2, Gem::Common::VAR_IS_ESSENTIAL, comment
    );

    comment = "";
    comment += "The start distance between both peaks used for bi-gaussian mutations in ES;";
    gpb.registerFileParameter<double>(
        "delta", c.delta, GFI_DEF_DELTA, Gem::Common::VAR_IS_ESSENTIAL, comment
    );

    comment = "";
    comment += "The width of the gaussian used for mutations of the delta parameter;";
    gpb.registerFileParameter<double>(
        "sigma_delta", c.sigma_delta, GFI_DEF_SIGMADELTA, Gem::Common::VAR_IS_ESSENTIAL, comment
    );

    comment = "";
    comment += "The minimum allowed value of delta;";
    gpb.registerFileParameter<double>(
        "min_delta", c.min_delta, GFI_DEF_MINDELTA, Gem::Common::VAR_IS_ESSENTIAL, comment
    );

    comment = "";
    comment += "The maximum allowed value of delta;";
    gpb.registerFileParameter<double>(
        "max_delta", c.max_delta, GFI_DEF_MAXDELTA, Gem::Common::VAR_IS_ESSENTIAL, comment
    );

    comment = "";
    comment += "The number of dimensions used for the demo function;";
    gpb.registerFileParameter<std::size_t>(
        "par_dim", c.par_dim, GFI_DEF_PARDIM, Gem::Common::VAR_IS_ESSENTIAL, comment
    );

    comment = "";
    comment += "The lower boundary of the initialization range for parameters;";
    gpb.registerFileParameter<double>(
        "min_var", c.min_var, GFI_DEF_MINVAR, Gem::Common::VAR_IS_ESSENTIAL, comment
    );

    comment = "";
    comment += "The upper boundary of the initialization range for parameters;";
    gpb.registerFileParameter<double>(
        "max_var", c.max_var, GFI_DEF_MAXVAR, Gem::Common::VAR_IS_ESSENTIAL, comment
    );

    comment = "";
    comment +=
        "Indicates what type of parameter object should be used;(0) GDoubleCollection;(1) "
        "GConstrainedDoubleCollection;(2) GDoubleObjectCollection; (3) "
        "GConstrainedDoubleObjectCollection; (4) GConstrainedDoubleObjects on the root level;";
    gpb.registerFileParameter<parameterType>(
        "parameter_type", c.p_t, GFI_DEF_PARAMETERTYPE, Gem::Common::VAR_IS_ESSENTIAL, comment
    );

    comment = "";
    comment += "Indicates how the parameters are initialized;(0) randomly;(1) with a value on the "
               "perimeter of the allowed or recommended value range";
    gpb.registerFileParameter<initMode>(
        "init_mode", c.i_m, GFI_DEF_INITMODE, Gem::Common::VAR_IS_ESSENTIAL, comment
    );

    comment = "";
    comment += "Specifies which benchmark function to minimise (maximise for NEGPARABOLA), by integer ID;";
    gpb.registerFileParameter<solverFunction>(
        "demo_function", c.demo_function, GO_DEF_EVALFUNCTION, Gem::Common::VAR_IS_ESSENTIAL, comment
    );
}

/******************************************************************************/
/**
 * Builds the flat genome's STRUCTURE only (the body of the former
 * GFunctionIndividualFactory::postProcess_). The five legacy modes differ in constrained-vs-unbounded and
 * whether the parameters share one adaption group (a *collection*) or each carry their own (a collection
 * of *objects* / individual objects). The configured Gauss / bi-Gauss adaptor settings live on the
 * OA-owned config (see buildAdaptionConfig()), not in the genome layout. The start value is the lower
 * perimeter; the optimization algorithm random-initialises within [min, max].
 */
gen::Genome GFunctionIndividual::buildGenome(const Config &c) {
    const std::size_t n_data = c.par_dim;
    const double min_v = c.min_var;
    const double max_v = c.max_var;

    gen::GGenomeBuilder b;
    switch(c.p_t) {
    case parameterType::USEGDOUBLECOLLECTION: { // unbounded, one shared group
        b.addDoublePlainGroup(n_data, min_v, max_v);
    } break;

    case parameterType::USEGCONSTRAINEDOUBLECOLLECTION: { // constrained, one shared group
        b.addDoubleGroup(n_data, min_v, max_v);
    } break;

    case parameterType::USEGDOUBLEOBJECTCOLLECTION: { // unbounded, a group per parameter
        for(std::size_t i = 0; i < n_data; i++) {
            b.addDouble(min_v).perimeter(min_v, max_v);
        }
    } break;

    case parameterType::USEGCONSTRAINEDDOUBLEOBJECTCOLLECTION:
    case parameterType::USEGCONSTRAINEDDOUBLEOBJECT: { // constrained, a group per parameter
        for(std::size_t i = 0; i < n_data; i++) {
            b.addDouble(min_v, min_v, max_v);
        }
    } break;

    default: {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFunctionIndividual::buildGenome(): Error!"
            << "Found invalid parameter_type: " << c.p_t << '\n'
        );
    }
    }

    return b.build();
}

/******************************************************************************/
/**
 * Builds the OA-owned adaption configuration for a genome produced by this factory (the body of the
 * former GFunctionIndividualFactory::getAdaptionConfig). Every double group (one shared group for the
 * collection modes, one per parameter for the object modes) receives the configured single-Gauss or
 * bi-Gauss adaptor.
 */
std::shared_ptr<OptimizationAlgorithms::GAdaptionConfigBase>
GFunctionIndividual::buildAdaptionConfig(const gen::GFlatGenome &sample, const Config &c) {
    namespace oa = Gem::Geneva::OptimizationAlgorithms;
    auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(sample);
    for(std::size_t i = 0; i < cfg->doubleGroups().size(); i++) {
        if(c.use_bi_gaussian) {
            cfg->groupDouble(i).biGauss(
                c.sigma1, c.sigma_sigma1, c.min_sigma1, c.max_sigma1,
                c.sigma2, c.sigma_sigma2, c.min_sigma2, c.max_sigma2,
                c.delta, c.sigma_delta, c.min_delta, c.max_delta,
                c.ad_prob, /* use_symmetric_sigmas = */ false, c.adapt_ad_prob,
                c.adaption_threshold
            );
        }
        else {
            cfg->groupDouble(i).gauss(
                c.sigma1, c.sigma_sigma1, c.min_sigma1, c.max_sigma1,
                c.ad_prob, c.adapt_ad_prob, c.adaption_threshold,
                Gem::Geneva::adaptionMode::WITHPROBABILITY, c.min_ad_prob, c.max_ad_prob
            );
        }
    }
    return cfg;
}

/******************************************************************************/
/**
 * Per-object post-config hook: applies the (non-genome) demo function to a produced individual. The demo
 * function was formerly registered + applied by the individual's own addConfigurationOptions_; it now
 * lives in the Config and is applied here, the symmetric companion to buildAdaptionConfig().
 */
void GFunctionIndividual::applyConfig(GFunctionIndividual &ind, const Config &c) {
    ind.setDemoFunction(c.demo_function);
}

/******************************************************************************/
/**
 * Reads a GFunctionIndividual config file into a Config. Used by callers that build individuals directly
 * rather than through the factory (e.g. the dimension-sweeping GOptimizationBenchmark / CUDA benchmark,
 * which need a different genome dimension per measurement row).
 */
GFunctionIndividual::Config GFunctionIndividual::readConfig(std::filesystem::path const &configFile) {
    Config c;
    Gem::Common::GParserBuilder gpb;
    describeConfig(gpb, c);

    if(not gpb.parseConfigFile(configFile)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFunctionIndividual::readConfig(): Error!" << '\n'
            << "Could not parse configuration file " << configFile.string() << '\n'
        );
    }

    return c;
}

/******************************************************************************/
/**
 * Builds an individual fully configured the way the factory's get_as<>() would, but with a genome
 * dimension taken from @p c (which the caller may have overridden) rather than from the config file.
 * Mirrors the factory's getObject_ + describeLocalOptions_ + parse + postProcess_ sequence: the base
 * GOptimizableEntity options (eval_policy, maxmode, validity thresholds, ...) are registered and applied
 * from @p configFile, the genome structure comes from buildGenome(c), and the demo function from
 * applyConfig(). The Config-shaping keys are bound to a throwaway Config so they don't trip the
 * unknown-key diagnostic -- the caller's @p c drives the genome instead.
 */
std::shared_ptr<GFunctionIndividual>
GFunctionIndividual::buildConfigured(const Config &c, std::filesystem::path const &configFile) {
    auto ind = std::make_shared<GFunctionIndividual>();

    Gem::Common::GParserBuilder gpb;
    Config sink;
    describeConfig(gpb, sink);          // bind the genome-shaping keys (suppresses unknown-key noise)
    ind->addConfigurationOptions(gpb);  // bind the base GOptimizableEntity options to this individual
    if(not gpb.parseConfigFile(configFile)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFunctionIndividual::buildConfigured(): Error!" << '\n'
            << "Could not parse configuration file " << configFile.string() << '\n'
        );
    }

    ind->setGenome(buildGenome(c));
    applyConfig(*ind, c);
    return ind;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Provide an easy way to print the individual's content
 */
std::ostream &operator<<(std::ostream &s, const Gem::Geneva::Individuals::GFunctionIndividual &f) {
    std::vector<double> par_vec;
    f.streamline(par_vec);

    std::cout << '\n' << "Raw fitness: " << f.raw_fitness(0) << '\n' << '\n';
    std::size_t pos = 0;
    std::cout << "Parameter values of best individual:" << '\n';
    for(const auto &val : par_vec) {
        std::cout << pos++ << ": " << val << '\n';
    }
    std::cout << '\n';

    return s;
}

/******************************************************************************/
/**
 * Provide an easy way to print the individual's content through a smart-pointer
 */
std::ostream &operator<<(std::ostream &s, std::shared_ptr<Gem::Geneva::Individuals::GFunctionIndividual> f_ptr) {
    return operator<<(s, *f_ptr);
}

/******************************************************************************/

} /* namespace Gem::Geneva::Individuals */
