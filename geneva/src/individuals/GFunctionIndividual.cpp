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
    const GPreEvaluationValidityCheckT<gpar::GOptimizableEntity> &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GDoubleSumConstraint reference independent of this object and convert the pointer
    const GDoubleSumConstraint *p_load =
        Gem::Common::g_convert_and_compare<
            GPreEvaluationValidityCheckT<gpar::GOptimizableEntity>,
            GDoubleSumConstraint>(cp, this);

    Gem::Common::GToken token("GDoubleSumConstraint", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<gpar::GOptimizableEntityConstraint>(*this, *p_load, token);

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
    gpar::GOptimizableEntityConstraint::addConfigurationOptions_(gpb);
}

/******************************************************************************/
/**
 * Checks whether a given individual is valid
 */
double GDoubleSumConstraint::check_(const gpar::GOptimizableEntity *p) const {
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
void GDoubleSumConstraint::load_(const GPreEvaluationValidityCheckT<gpar::GOptimizableEntity> *cp) {
    // Check that we are dealing with a GDoubleSumConstraint reference independent of this object and convert the pointer
    const GDoubleSumConstraint *p_load =
        Gem::Common::g_convert_and_compare<
            GPreEvaluationValidityCheckT<gpar::GOptimizableEntity>,
            GDoubleSumConstraint>(cp, this);

    // Load our parent class'es data ...
    gpar::GOptimizableEntityConstraint::load_(cp);

    // ... and then our local data, derived from the single localMembers() declaration
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GPreEvaluationValidityCheckT<gpar::GOptimizableEntity> *GDoubleSumConstraint::clone_() const {
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
    const GPreEvaluationValidityCheckT<gpar::GOptimizableEntity> &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GDoubleSumGapConstraint reference independent of this object and convert the pointer
    const GDoubleSumGapConstraint *p_load =
        Gem::Common::g_convert_and_compare<
            GPreEvaluationValidityCheckT<gpar::GOptimizableEntity>,
            GDoubleSumGapConstraint>(cp, this);

    Gem::Common::GToken token("GDoubleSumGapConstraint", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<gpar::GOptimizableEntityConstraint>(*this, *p_load, token);

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
    gpar::GOptimizableEntityConstraint::addConfigurationOptions_(gpb);
}

/******************************************************************************/
/**
 * Checks whether a given individual is valid
 */
double GDoubleSumGapConstraint::check_(const gpar::GOptimizableEntity *p) const {
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
void GDoubleSumGapConstraint::load_(const GPreEvaluationValidityCheckT<gpar::GOptimizableEntity> *cp) {
    // Check that we are dealing with a GDoubleSumGapConstraint reference independent of this object and convert the pointer
    const GDoubleSumGapConstraint *p_load =
        Gem::Common::g_convert_and_compare<
            GPreEvaluationValidityCheckT<gpar::GOptimizableEntity>,
            GDoubleSumGapConstraint>(cp, this);

    // Load our parent class'es data ...
    gpar::GOptimizableEntityConstraint::load_(cp);

    // ... and then our local data, derived from the single localMembers() declaration
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GPreEvaluationValidityCheckT<gpar::GOptimizableEntity> *GDoubleSumGapConstraint::clone_() const {
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
    const GPreEvaluationValidityCheckT<gpar::GOptimizableEntity> &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    // Check that we are dealing with a GSphereConstraint reference independent of this object and convert the pointer
    const GSphereConstraint *p_load =
        Gem::Common::g_convert_and_compare<
            GPreEvaluationValidityCheckT<gpar::GOptimizableEntity>,
            GSphereConstraint>(cp, this);

    Gem::Common::GToken token("GSphereConstraint", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<gpar::GOptimizableEntityConstraint>(*this, *p_load, token);

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
    gpar::GOptimizableEntityConstraint::addConfigurationOptions_(gpb);
}

/******************************************************************************/
/**
 * Checks whether a given individual is valid
 */
double GSphereConstraint::check_(const gpar::GOptimizableEntity *p) const {
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
void GSphereConstraint::load_(const GPreEvaluationValidityCheckT<gpar::GOptimizableEntity> *cp) {
    // Check that we are dealing with a GSphereConstraint reference independent of this object and convert the pointer
    const GSphereConstraint *p_load =
        Gem::Common::g_convert_and_compare<
            GPreEvaluationValidityCheckT<gpar::GOptimizableEntity>,
            GSphereConstraint>(cp, this);

    // Load our parent class'es data ...
    gpar::GOptimizableEntityConstraint::load_(cp);

    // ... and then our local data, derived from the single localMembers() declaration
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GPreEvaluationValidityCheckT<gpar::GOptimizableEntity> *GSphereConstraint::clone_() const {
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
    const gpar::GOptimizableEntity &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    // Check that we are dealing with a GFunctionIndividual reference independent of this object and convert the pointer
    const GFunctionIndividual *p_load =
        Gem::Common::g_convert_and_compare<gpar::GOptimizableEntity, GFunctionIndividual>(cp, this);

    Gem::Common::GToken token("GFunctionIndividual", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<gpar::GFlatGenome>(*this, *p_load, token);

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
    // Call our parent class'es function
    gpar::GFlatGenome::addConfigurationOptions_(gpb);

    // Local data
    gpb.registerFileParameter<solverFunction>(
        "demo_function" // The name of the variable
        ,
        GO_DEF_EVALFUNCTION // The default value
        ,
        [this](solverFunction sf) { this->setDemoFunction(sf); }
    ) << "Specifies which benchmark function to minimise (maximise for NEGPARABOLA)."
      << '\n'
      << "Select by integer ID:" << '\n'
      << " 0: Parabola            -- unimodal, separable baseline; global min f=0 at origin"
      << '\n'
      << " 1: Berlich noisy para  -- radial cosine overlay; global min f=0 at origin" << '\n'
      << " 2: Rosenbrock          -- narrow banana valley, n>=2; global min f=0 at (1,...,1)"
      << '\n'
      << " 3: Ackley (variant)    -- Geneva pairwise variant, n>=2; NOT the canonical form"
      << '\n'
      << " 4: Rastrigin           -- highly multimodal, separable; global min f=0 at origin"
      << '\n'
      << " 5: Schwefel            -- deceptive, global opt near boundary; domain [-500,500]"
      << '\n'
      << " 6: Salomon             -- concentric-ring landscape; global min f=0 at origin"
      << '\n'
      << " 7: Negative Parabola   -- maximisation test; global max f=0 at origin" << '\n'
      << " 8: Ackley (canonical)  -- CEC/BBOB standard; plateau + deep basin; domain "
         "[-32.768,32.768]"
      << '\n'
      << " 9: Griewank            -- multimodal with quadratic envelope; global min f=0 at origin"
      << '\n'
      << "10: Levy                -- narrow-basin multimodal; global min f=0 at (1,...,1)"
      << '\n'
      << "11: Styblinski-Tang     -- asymmetric wells; global min ~-39.166*n at (~-2.903,...)"
      << '\n'
      << "12: Ellipsoid           -- ill-conditioned (1e6), unimodal; global min f=0 at origin"
      << '\n'
      << "13: Michalewicz (m=10)  -- steep ridges; domain [0,pi]; set min_var=0 max_var=3.14159!"
      << '\n'
      << "14: Zakharov            -- unimodal, non-separable coupling; global min f=0 at origin"
      << '\n';
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
void GFunctionIndividual::load_(const gpar::GOptimizableEntity *cp) {
    // Check that we are dealing with a GFunctionIndividual reference independent of this object and convert the pointer
    const GFunctionIndividual *p_load =
        Gem::Common::g_convert_and_compare<gpar::GOptimizableEntity, GFunctionIndividual>(cp, this);

    // Load our parent class'es data ...
    gpar::GFlatGenome::load_(cp);

    // ... and then our local data, derived from the single localMembers() declaration
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GFlatGenome
 */
gpar::GFlatGenome *GFunctionIndividual::clone_() const {
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
    if(gpar::GFlatGenome::modify_GUnitTests_()) {
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
    gpar::GFlatGenome::specificTestsNoFailureExpected_GUnitTests_();

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
    gpar::GFlatGenome::specificTestsFailuresExpected_GUnitTests_();

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
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode. It initializes a
 * target item as needed.
 *
 * @param config_file The name of the configuration file
 */
GFunctionIndividualFactory::GFunctionIndividualFactory(std::filesystem::path const &config_file)
  : gpar::GOptimizableEntityFactory(config_file) { /* nothing */
}

/******************************************************************************/
/**
 * The default constructor. Only needed for (de-)serialization purposes, hence empty.
 */
GFunctionIndividualFactory::GFunctionIndividualFactory()
  : gpar::GOptimizableEntityFactory("empty") { /* nothing */
}

/******************************************************************************/
/**
 * Loads the data of another GFunctionIndividualFactory object
 */
void GFunctionIndividualFactory::load(
    std::shared_ptr<Gem::Common::GFactoryT<gpar::GOptimizableEntity>> cp_raw_ptr
) {
    // Load our parent class'es data
    gpar::GOptimizableEntityFactory::load(cp_raw_ptr);

    // Convert the base pointer
    std::shared_ptr<GFunctionIndividualFactory> cp_ptr = Gem::Common::convertSmartPointer<
        Gem::Common::GFactoryT<gpar::GOptimizableEntity>,
        GFunctionIndividualFactory>(cp_raw_ptr);

    // And then our own
    ad_prob_ = cp_ptr->ad_prob_;
    adapt_ad_prob_ = cp_ptr->adapt_ad_prob_;
    min_ad_prob_ = cp_ptr->min_ad_prob_;
    max_ad_prob_ = cp_ptr->max_ad_prob_;
    adaption_threshold_ = cp_ptr->adaption_threshold_;
    use_bi_gaussian_ = cp_ptr->use_bi_gaussian_;
    sigma1_ = cp_ptr->sigma1_;
    sigma_sigma1_ = cp_ptr->sigma_sigma1_;
    min_sigma1_ = cp_ptr->min_sigma1_;
    max_sigma1_ = cp_ptr->max_sigma1_;
    sigma2_ = cp_ptr->sigma2_;
    sigma_sigma2_ = cp_ptr->sigma_sigma2_;
    min_sigma2_ = cp_ptr->min_sigma2_;
    max_sigma2_ = cp_ptr->max_sigma2_;
    delta_ = cp_ptr->delta_;
    sigma_delta_ = cp_ptr->sigma_delta_;
    min_delta_ = cp_ptr->min_delta_;
    max_delta_ = cp_ptr->max_delta_;
    par_dim_ = cp_ptr->par_dim_;
    min_var_ = cp_ptr->min_var_;
    max_var_ = cp_ptr->max_var_;
    p_t_ = cp_ptr->p_t_;
    i_m_ = cp_ptr->i_m_;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
std::shared_ptr<Gem::Common::GFactoryT<gpar::GOptimizableEntity>> GFunctionIndividualFactory::clone() const {
    return std::make_shared<GFunctionIndividualFactory>(*this);
}

/******************************************************************************/
/**
 * (Re-)Set the dimension of the function
 */
void GFunctionIndividualFactory::setParDim(std::size_t par_dim) {
    if(par_dim == 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFunctionIndividualFactory::setParDim(): Error!" << '\n'
            << "Dimension of the function is set to 0" << '\n'
        );
    }

    par_dim_ = par_dim;
}

/******************************************************************************/
/**
 * Extract the minimum and maximum boundaries of the variables
 */
std::tuple<double, double> GFunctionIndividualFactory::getVarBoundaries() const {
    return std::tuple<double, double>{min_var_, max_var_};
}

/******************************************************************************/
/**
 * Set the minimum and maximum boundaries of the variables
 */
void GFunctionIndividualFactory::setVarBoundaries(std::tuple<double, double> boundaries) {
    double min = std::get<0>(boundaries);
    double max = std::get<1>(boundaries);

    if(min >= max) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFunctionIndividualFactory::setVarBoundaries(): Error!" << '\n'
            << "Received invalid boundaries " << min << " / " << max << '\n'
        );
    }

    setMinVar(min);
    setMaxVar(max);
}

/******************************************************************************/
/**
 * Get the value of the adaption_threshold_ variable
 */
std::uint32_t GFunctionIndividualFactory::getAdaptionThreshold() const {
    return adaption_threshold_;
}

/******************************************************************************/
/**
 * Set the value of the adaption_threshold_ variable
 */
void GFunctionIndividualFactory::setAdaptionThreshold(std::uint32_t adaption_threshold) {
    adaption_threshold_ = adaption_threshold;
}

/******************************************************************************/
/**
 * Allows to retrieve the adProb_ variable
 */
double GFunctionIndividualFactory::getAdProb() const {
    return ad_prob_;
}

/******************************************************************************/
/**
 * Set the value of the adProb_ variable
 */
void GFunctionIndividualFactory::setAdProb(double ad_prob) {
    ad_prob_ = ad_prob;
}

/******************************************************************************/
/**
 * Allows to retrieve the delta_ variable
 */
double GFunctionIndividualFactory::getDelta() const {
    return delta_;
}

/******************************************************************************/
/**
 * Set the value of the delta_ variable
 */
void GFunctionIndividualFactory::setDelta(double delta) {
    delta_ = delta;
}

/******************************************************************************/
/**
 * Allows to retrieve the iM_ variable
 */
initMode GFunctionIndividualFactory::getIM() const {
    return i_m_;
}

/******************************************************************************/
/**
 * Set the value of the iM_ variable
 */
void GFunctionIndividualFactory::setIM(initMode m) {
    i_m_ = m;
}

/******************************************************************************/
/**
 * Allows to retrieve the max_delta_ variable
 */
double GFunctionIndividualFactory::getMaxDelta() const {
    return max_delta_;
}

/******************************************************************************/
/**
 * Set the value of the max_delta_ variable
 */
void GFunctionIndividualFactory::setMaxDelta(double max_delta) {
    max_delta_ = max_delta;
}

/******************************************************************************/
/**
 * Allows to retrieve the max_sigma1_ variable
 */
double GFunctionIndividualFactory::getMaxSigma1() const {
    return max_sigma1_;
}

/******************************************************************************/
/**
 * Set the value of the max_sigma1_ variable
 */
void GFunctionIndividualFactory::setMaxSigma1(double max_sigma1) {
    max_sigma1_ = max_sigma1;
}

/******************************************************************************/
/**
 * Allows to retrieve the max_sigma2_ variable
 */
double GFunctionIndividualFactory::getMaxSigma2() const {
    return max_sigma2_;
}

/******************************************************************************/
/**
 * Set the value of the max_sigma2_ variable
 */
void GFunctionIndividualFactory::setMaxSigma2(double max_sigma2) {
    max_sigma2_ = max_sigma2;
}

/******************************************************************************/
/**
 * Allows to retrieve the maxVar_ variable
 */
double GFunctionIndividualFactory::getMaxVar() const {
    return max_var_;
}

/******************************************************************************/
/**
 * Set the value of the maxVar_ variable
 */
void GFunctionIndividualFactory::setMaxVar(double max_var) {
    max_var_ = max_var;
}

/******************************************************************************/
/**
 * Allows to retrieve the min_delta_ variable
 */
double GFunctionIndividualFactory::getMinDelta() const {
    return min_delta_;
}

/******************************************************************************/
/**
 * Set the value of the min_delta_ variable
 */
void GFunctionIndividualFactory::setMinDelta(double min_delta) {
    min_delta_ = min_delta;
}

/******************************************************************************/
/**
 * Allows to retrieve the allowed value range of delta
 */
std::tuple<double, double> GFunctionIndividualFactory::getDeltaRange() const {
    return std::tuple<double, double>{min_delta_, max_delta_};
}

/******************************************************************************/
/**
 * Allows to set the allowed value range of delta
 */
void GFunctionIndividualFactory::setDeltaRange(std::tuple<double, double> range) {
    double min = std::get<0>(range);
    double max = std::get<1>(range);

    if(min < 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFunctionIndividualFactory::setDeltaRange(): Error" << '\n'
            << "min must be >= 0. Got : " << max << '\n'
        );
    }

    if(min >= max) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFunctionIndividualFactory::setDeltaRange(): Error" << '\n'
            << "Invalid range specified: " << min << " / " << max << '\n'
        );
    }

    min_delta_ = min;
    max_delta_ = max;
}

/******************************************************************************/
/**
 * Allows to retrieve the min_sigma1_ variable
 */
double GFunctionIndividualFactory::getMinSigma1() const {
    return min_sigma1_;
}

/******************************************************************************/
/**
 * Set the value of the min_sigma1_ variable
 */
void GFunctionIndividualFactory::setMinSigma1(double min_sigma1) {
    min_sigma1_ = min_sigma1;
}

/******************************************************************************/
/**
 * Allows to retrieve the allowed value range of sigma1_
 */
std::tuple<double, double> GFunctionIndividualFactory::getSigma1Range() const {
    return std::tuple<double, double>{min_sigma1_, max_sigma1_};
}

/******************************************************************************/
/**
 * Allows to set the allowed value range of sigma1_
 */
void GFunctionIndividualFactory::setSigma1Range(std::tuple<double, double> range) {
    double min = std::get<0>(range);
    double max = std::get<1>(range);

    if(min < 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFunctionIndividualFactory::setSigma1Range(): Error" << '\n'
            << "min must be >= 0. Got : " << max << '\n'
        );
    }

    if(min >= max) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFunctionIndividualFactory::setSigma1Range(): Error" << '\n'
            << "Invalid range specified: " << min << " / " << max << '\n'
        );
    }

    min_sigma1_ = min;
    max_sigma1_ = max;
}

/******************************************************************************/
/**
 * Allows to retrieve the min_sigma2_ variable
 */
double GFunctionIndividualFactory::getMinSigma2() const {
    return min_sigma2_;
}

/******************************************************************************/
/**
 * Set the value of the min_sigma2_ variable
 */
void GFunctionIndividualFactory::setMinSigma2(double min_sigma2) {
    min_sigma2_ = min_sigma2;
}

/******************************************************************************/
/**
 * Allows to retrieve the allowed value range of sigma2_
 */
std::tuple<double, double> GFunctionIndividualFactory::getSigma2Range() const {
    return std::tuple<double, double>{min_sigma2_, max_sigma2_};
}

/******************************************************************************/
/**
 * Allows to set the allowed value range of sigma2_
 */
void GFunctionIndividualFactory::setSigma2Range(std::tuple<double, double> range) {
    double min = std::get<0>(range);
    double max = std::get<1>(range);

    if(min < 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFunctionIndividualFactory::setSigma2Range(): Error" << '\n'
            << "min must be >= 0. Got : " << max << '\n'
        );
    }

    if(min >= max) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFunctionIndividualFactory::setSigma2Range(): Error" << '\n'
            << "Invalid range specified: " << min << " / " << max << '\n'
        );
    }

    min_sigma2_ = min;
    max_sigma2_ = max;
}

/******************************************************************************/
/**
 * Allows to retrieve the minVar_ variable
 */
double GFunctionIndividualFactory::getMinVar() const {
    return min_var_;
}

/******************************************************************************/
/**
 * Set the value of the minVar_ variable
 */
void GFunctionIndividualFactory::setMinVar(double min_var) {
    min_var_ = min_var;
}

/******************************************************************************/
/**
 * Allows to retrieve the parDim_ variable
 */
std::size_t GFunctionIndividualFactory::getParDim() const {
    return par_dim_;
}

/******************************************************************************/
/**
 * Allows to retrieve the pT_ variable
 */
parameterType GFunctionIndividualFactory::getPT() const {
    return p_t_;
}

/******************************************************************************/
/**
 * Set the value of the pT_ variable
 */
void GFunctionIndividualFactory::setPT(parameterType pt) {
    p_t_ = pt;
}

/******************************************************************************/
/**
 * Allows to retrieve the sigma1_ variable
 */
double GFunctionIndividualFactory::getSigma1() const {
    return sigma1_;
}

/******************************************************************************/
/**
 * Set the value of the sigma1_ variable
 */
void GFunctionIndividualFactory::setSigma1(double sigma1) {
    sigma1_ = sigma1;
}

/******************************************************************************/
/**
 * Allows to retrieve the sigma2_ variable
 */
double GFunctionIndividualFactory::getSigma2() const {
    return sigma2_;
}

/******************************************************************************/
/**
 * Set the value of the sigma2_ variable
 */
void GFunctionIndividualFactory::setSigma2(double sigma2) {
    sigma2_ = sigma2;
}

/******************************************************************************/
/**
 * Allows to retrieve the sigma_delta_ variable
 */
double GFunctionIndividualFactory::getSigmaDelta() const {
    return sigma_delta_;
}

/******************************************************************************/
/**
 * Set the value of the sigma_delta_ variable
 */
void GFunctionIndividualFactory::setSigmaDelta(double sigma_delta) {
    sigma_delta_ = sigma_delta;
}

/******************************************************************************/
/**
 * Allows to retrieve the sigma_sigma1_ variable
 */
double GFunctionIndividualFactory::getSigmaSigma1() const {
    return sigma_sigma1_;
}

/******************************************************************************/
/**
 * Set the value of the sigma_sigma1_ variable
 */
void GFunctionIndividualFactory::setSigmaSigma1(double sigma_sigma1) {
    sigma_sigma1_ = sigma_sigma1;
}

/******************************************************************************/
/**
 * Allows to retrieve the sigma_sigma2_ variable
 */
double GFunctionIndividualFactory::getSigmaSigma2() const {
    return sigma_sigma2_;
}

/******************************************************************************/
/**
 * Set the value of the sigma_sigma2_ variable
 */
void GFunctionIndividualFactory::setSigmaSigma2(double sigma_sigma2) {
    sigma_sigma2_ = sigma_sigma2;
}

/******************************************************************************/
/**
 * Allows to retrieve the use_bi_gaussian_ variable
 */
bool GFunctionIndividualFactory::getUseBiGaussian() const {
    return use_bi_gaussian_;
}

/******************************************************************************/
/**
 * Set the value of the use_bi_gaussian_ variable
 */
void GFunctionIndividualFactory::setUseBiGaussian(bool use_bi_gaussian) {
    use_bi_gaussian_ = use_bi_gaussian;
}

/******************************************************************************/
/**
 * Allows to retrieve the rate of evolutionary adaption of adProb_
 */
double GFunctionIndividualFactory::getAdaptAdProb() const {
    return adapt_ad_prob_;
}

/******************************************************************************/
/**
 * Allows to specify an adaption factor for adProb_ (or 0, if you do not want this feature)
 */
void GFunctionIndividualFactory::setAdaptAdProb(double adapt_ad_prob) {
#ifdef DEBUG
    if(adapt_ad_prob < 0.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFunctionIndividualFactory::setAdaptAdProb(): Error!" << '\n'
            << "Invalid value for adapt_ad_prob given: " << adapt_ad_prob << '\n'
        );
    }
#endif /* DEBUG */

    adapt_ad_prob_ = adapt_ad_prob;
}

/******************************************************************************/
/**
 * Allows to retrieve the allowed range for adProb_ variation
 */
std::tuple<double, double> GFunctionIndividualFactory::getAdProbRange() const {
    return std::tuple<double, double>{min_ad_prob_.value(), max_ad_prob_.value()};
}

/******************************************************************************/
/**
 * Allows to set the allowed range for adaption probability variation
 */
void GFunctionIndividualFactory::setAdProbRange(double min_ad_prob, double max_ad_prob) {
#ifdef DEBUG
    if(min_ad_prob < 0.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFunctionIndividualFactory::setAdProbRange(): Error!" << '\n'
            << "min_ad_prob < 0: " << min_ad_prob << '\n'
        );
    }

    if(min_ad_prob > max_ad_prob) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFunctionIndividualFactory::setAdProbRange(): Error!" << '\n'
            << "Invalid min_ad_prob and/or max_ad_prob: " << min_ad_prob << " / " << max_ad_prob << '\n'
        );
    }

    if(max_ad_prob > 1.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFunctionIndividualFactory::setAdProbRange(): Error!" << '\n'
            << "max_ad_prob > 1: " << max_ad_prob << '\n'
        );
    }
#endif /* DEBUG */

    min_ad_prob_ = min_ad_prob;
    max_ad_prob_ = max_ad_prob;
}

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr<gpar::GOptimizableEntity> GFunctionIndividualFactory::getObject_(
    Gem::Common::GParserBuilder &gpb,
    [[maybe_unused]] const std::size_t & id
) {
    // Will hold the result
    std::shared_ptr<GFunctionIndividual> target(new GFunctionIndividual());

    // Make the object's local configuration options known
    target->addConfigurationOptions(gpb);

    return target;
}

/******************************************************************************/
/**
 * Allows to describe local configuration options for gradient descents
 */
void GFunctionIndividualFactory::describeLocalOptions_(Gem::Common::GParserBuilder &gpb) {
    // Describe our own options
    using namespace Gem::Courtier;

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)

    comment = "";
    comment += "The probability for random adaption of values in evolutionary algorithms;";
    gpb.registerFileParameter<double>(
        "ad_prob",
        ad_prob_.reference(),
        GFI_DEF_ADPROB,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment +=
        "Determines the rate of adaption of ad_prob. Set to 0, if you do not need this feature;";
    gpb.registerFileParameter<double>(
        "adapt_ad_prob",
        adapt_ad_prob_.reference(),
        GFI_DEF_ADAPTADPROB,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The lower allowed boundary for ad_prob-variation;";
    gpb.registerFileParameter<double>(
        "min_ad_prob",
        min_ad_prob_.reference(),
        GFI_DEF_MINADPROB,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The upper allowed boundary for ad_prob-variation;";
    gpb.registerFileParameter<double>(
        "max_ad_prob",
        max_ad_prob_.reference(),
        GFI_DEF_MAXADPROB,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The number of successful calls to an adaptor after which adaption;";
    comment += "of mutation parameters takes place (e.g sigma-variation in gauss mutation);";
    gpb.registerFileParameter<std::uint32_t>(
        "adaption_threshold",
        adaption_threshold_.reference(),
        GFI_DEF_ADAPTIONTHRESHOLD,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "Whether to use a double gaussion for the adaption of parmeters in ES;";
    gpb.registerFileParameter<bool>(
        "use_bi_gaussian",
        use_bi_gaussian_.reference(),
        GFI_DEF_USEBIGAUSSIAN,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment +=
        "The sigma for gauss-adaption in ES;(or the sigma of the left peak of a double gaussian);";
    gpb.registerFileParameter<double>(
        "sigma1",
        sigma1_.reference(),
        GFI_DEF_SIGMA1,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "Influences the self-adaption of gauss-mutation in ES;";
    gpb.registerFileParameter<double>(
        "sigma_sigma1",
        sigma_sigma1_.reference(),
        GFI_DEF_SIGMASIGMA1,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The minimum value of sigma1;";
    gpb.registerFileParameter<double>(
        "min_sigma1",
        min_sigma1_.reference(),
        GFI_DEF_MINSIGMA1,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The maximum value of sigma1;";
    gpb.registerFileParameter<double>(
        "max_sigma1",
        max_sigma1_.reference(),
        GFI_DEF_MAXSIGMA1,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The sigma of the right peak of a double gaussian (if any);";
    gpb.registerFileParameter<double>(
        "sigma2",
        sigma2_.reference(),
        GFI_DEF_SIGMA2,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "Influences the self-adaption of gauss-mutation in ES;";
    gpb.registerFileParameter<double>(
        "sigma_sigma2",
        sigma_sigma2_.reference(),
        GFI_DEF_SIGMASIGMA2,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The minimum value of sigma2;";
    gpb.registerFileParameter<double>(
        "min_sigma2",
        min_sigma2_.reference(),
        GFI_DEF_MINSIGMA2,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The maximum value of sigma2;";
    gpb.registerFileParameter<double>(
        "max_sigma2",
        max_sigma2_.reference(),
        GFI_DEF_MAXSIGMA2,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The start distance between both peaks used for bi-gaussian mutations in ES;";
    gpb.registerFileParameter<double>(
        "delta",
        delta_.reference(),
        GFI_DEF_DELTA,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The width of the gaussian used for mutations of the delta parameter;";
    gpb.registerFileParameter<double>(
        "sigma_delta",
        sigma_delta_.reference(),
        GFI_DEF_SIGMADELTA,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The minimum allowed value of delta;";
    gpb.registerFileParameter<double>(
        "min_delta",
        min_delta_.reference(),
        GFI_DEF_MINDELTA,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The maximum allowed value of delta;";
    gpb.registerFileParameter<double>(
        "max_delta",
        max_delta_.reference(),
        GFI_DEF_MAXDELTA,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The number of dimensions used for the demo function;";
    gpb.registerFileParameter<std::size_t>(
        "par_dim",
        par_dim_.reference(),
        GFI_DEF_PARDIM,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The lower boundary of the initialization range for parameters;";
    gpb.registerFileParameter<double>(
        "min_var",
        min_var_.reference(),
        GFI_DEF_MINVAR,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The upper boundary of the initialization range for parameters;";
    gpb.registerFileParameter<double>(
        "max_var",
        max_var_.reference(),
        GFI_DEF_MAXVAR,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment +=
        "Indicates what type of parameter object should be used;(0) GDoubleCollection;(1) "
        "GConstrainedDoubleCollection;(2) GDoubleObjectCollection; (3) "
        "GConstrainedDoubleObjectCollection; (4) GConstrainedDoubleObjects on the root level;";
    gpb.registerFileParameter<parameterType>(
        "parameter_type",
        p_t_.reference(),
        GFI_DEF_PARAMETERTYPE,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "Indicates how the parameters are initialized;(0) randomly;(1) with a value on the "
               "perimeter of the allowed or recommended value range";
    gpb.registerFileParameter<initMode>(
        "init_mode",
        i_m_.reference(),
        GFI_DEF_INITMODE,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    // Allow our parent class to describe its options
    gpar::GOptimizableEntityFactory::describeLocalOptions_(gpb);
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object. In practice,
 * we add the parameter objects here
 *
 * @param p A smart-pointer to be acted on during post-processing
 */
void GFunctionIndividualFactory::postProcess_(std::shared_ptr<gpar::GOptimizableEntity> &p) {
    const std::size_t n_data = par_dim_.value();
    const double min_v = min_var_.value();
    const double max_v = max_var_.value();

    // Build the flat genome's STRUCTURE only. The five legacy modes differ in constrained-vs-unbounded
    // and whether the parameters share one adaption group (a *collection*) or each carry their own (a
    // collection of *objects* / individual objects). The configured Gauss / bi-Gauss adaptor settings
    // now live on the OA-owned config (see getAdaptionConfig()), not in the genome layout. The start
    // value is the lower perimeter; the optimization algorithm random-initialises within [min, max].
    gpar::GGenomeBuilder b;
    switch(p_t_.value()) {
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
            << "In GFunctionIndividualFactory::postProcess_(): Error!"
            << "Found invalid pT_: " << p_t_ << '\n'
        );
    }
    }

    dynamic_cast<gpar::GFlatGenome &>(*p).setGenome(b.build());
}

/******************************************************************************/
/**
 * Builds the OA-owned adaption configuration for a genome produced by this factory. Every double group
 * (one shared group for the collection modes, one per parameter for the object modes) receives the
 * configured single-Gauss or bi-Gauss adaptor -- the exact settings the factory formerly baked into the
 * genome layout via applyAdaptor().
 */
std::shared_ptr<OptimizationAlgorithms::GAdaptionConfigBase>
GFunctionIndividualFactory::getAdaptionConfig(const gpar::GFlatGenome &sample) const {
    namespace oa = Gem::Geneva::OptimizationAlgorithms;
    auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(sample);
    for(std::size_t i = 0; i < cfg->doubleGroups().size(); i++) {
        if(use_bi_gaussian_.value()) {
            cfg->groupDouble(i).biGauss(
                sigma1_.value(), sigma_sigma1_.value(), min_sigma1_.value(), max_sigma1_.value(),
                sigma2_.value(), sigma_sigma2_.value(), min_sigma2_.value(), max_sigma2_.value(),
                delta_.value(), sigma_delta_.value(), min_delta_.value(), max_delta_.value(),
                ad_prob_.value(), /* use_symmetric_sigmas = */ false, adapt_ad_prob_.value(),
                adaption_threshold_.value()
            );
        }
        else {
            cfg->groupDouble(i).gauss(
                sigma1_.value(), sigma_sigma1_.value(), min_sigma1_.value(), max_sigma1_.value(),
                ad_prob_.value(), adapt_ad_prob_.value(), adaption_threshold_.value(),
                Gem::Geneva::adaptionMode::WITHPROBABILITY, min_ad_prob_.value(), max_ad_prob_.value()
            );
        }
    }
    return cfg;
}

/******************************************************************************/

} /* namespace Gem::Geneva::Individuals */
