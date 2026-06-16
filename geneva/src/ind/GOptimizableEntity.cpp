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

#include "geneva/ind/GOptimizableEntity.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <limits>
#include <memory>
#include <random>
#include <sstream>
#include <tuple>
#include <vector>

#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "common/GParserBuilder.hpp"
#include "courtier/GProcessingContainerT.hpp"
#include "geneva/GMultiConstraintT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include "geneva/Interface/GMutableI.hpp"
#include "geneva/Interface/GRateableI.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Genome::individual_processing_result) // NOLINT

namespace Gem::Geneva::Genome {
/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This constructor initializes the `individual_processing_result` object with
 * a raw fitness value. The transformed fitness is set to the same value as the raw fitness,
 * and the flag indicating that the transformed fitness has been set is set to false.
 *
 * @param raw_fitness The raw fitness value.
 */
individual_processing_result::individual_processing_result(const double raw_fitness)
  : raw_fitness_(raw_fitness)
  , transformed_fitness_(raw_fitness_)
  , transformed_fitness_set_(false) {
    /* nothing */
}

/******************************************************************************/
/**
 * This constructor initializes the `individual_processing_result` object with
 * both raw and transformed fitness values. It also sets the flag indicating that
 * the transformed fitness has been set.
 *
 * @param raw_fitness The raw fitness value.
 * @param transformed_fitness The transformed fitness value.
 */
individual_processing_result::individual_processing_result(
    const double raw_fitness,
    const double transformed_fitness
)
  : raw_fitness_(raw_fitness)
  , transformed_fitness_(transformed_fitness)
  , transformed_fitness_set_(true) {
    /* nothing */
}
/******************************************************************************/
/**
 * This constructor initializes the `individual_processing_result` object with
 * a raw fitness value and a function to transform the fitness. The transformed fitness
 * is calculated using the provided function. If the function is empty, an error is logged.
 *
 * @param raw_fitness The raw fitness value.
 * @param f A function to transform the raw fitness value.
 */
individual_processing_result::individual_processing_result(
    const double raw_fitness,
    std::function<double(double)> f
)
  : raw_fitness_(raw_fitness) {
    if(f) {
        transformed_fitness_ = f(raw_fitness_);
        transformed_fitness_set_ = true;
    }
    else {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In individual_processing_result(double, std::function<double(double)>): Error!" << '\n'
            << "Received an empty transformation function." << '\n'
        );
    }
}

/******************************************************************************/
/**
 * Access to the raw fitness
 */
double individual_processing_result::rawFitness() const {
    return raw_fitness_;
}

/******************************************************************************/
/**
 * Access to the transformed fitness
 */
double individual_processing_result::transformedFitness() const {
    return transformed_fitness_;
}

/******************************************************************************/
/**
     * Updates the transformed fitness using an external function
     */
void individual_processing_result::setTransformedFitnessWith(std::function<double(double)> f) {
    if(f) {
        transformed_fitness_ = f(raw_fitness_);
        transformed_fitness_set_ = true;
    }
    else {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In individual_processing_result::setTransformedFitnessWith():" << '\n'
            << "Function object f is empty." << '\n'
        );
    }
}

/******************************************************************************/
/**
     * Sets the transformed fitness to a user-defined value
     */
void individual_processing_result::setTransformedFitnessTo(const double transformed_fitness) {
    transformed_fitness_ = transformed_fitness;
    transformed_fitness_set_ = true;
}

/******************************************************************************/
/**
     * Sets the transformed fitness to the same value as the raw fitness
     */
void individual_processing_result::setTransformedFitnessToRaw() {
    transformed_fitness_ = raw_fitness_;
    transformed_fitness_set_ = true;
}

/******************************************************************************/
/**
     * Checks whether the transformed fitness was set
     */
bool individual_processing_result::transformedFitnessSet() const {
    return transformed_fitness_set_;
}

/******************************************************************************/
/**
     * Resets the object and stores a new raw value in the class
     */
void individual_processing_result::reset(const double raw_fitness) {
    raw_fitness_ = raw_fitness;
    transformed_fitness_ = raw_fitness_;
    transformed_fitness_set_ = false;
}

/******************************************************************************/
/**
 * Resets the object and stores a new raw and transformed value in the class
 */
void individual_processing_result::reset(
    const double raw_fitness,
    const double transformed_fitness
) {
    raw_fitness_ = raw_fitness;
    transformed_fitness_ = transformed_fitness;
    transformed_fitness_set_ = true;
}

/******************************************************************************/
/**
 * Resets the object and stores a new raw value in the class and triggers recalculation of the transformed value
 */
void individual_processing_result::reset(
    const double raw_fitness,
    std::function<double(double)> f
) {
    if(f) {
        raw_fitness_ = raw_fitness;
        transformed_fitness_ = f(raw_fitness_);
        transformed_fitness_set_ = true;
    }
    else {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In individual_processing_result::reset():" << '\n'
            << "Function object f is empty." << '\n'
        );
    }
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
     * The default constructor. Using this constructor will result in a single
     * fitness criterion.
     */
GOptimizableEntity::GOptimizableEntity()
  : Gem::Courtier::GProcessingContainerT<GOptimizableEntity, individual_processing_result>(1) {
    /* nothing */
}

/******************************************************************************/
/**
     * Initialization with the number of fitness criteria
     */
GOptimizableEntity::GOptimizableEntity(const std::size_t n_fitness_criteria)
  : Gem::Courtier::GProcessingContainerT<GOptimizableEntity, individual_processing_result>(
        n_fitness_criteria
    ) {
    /* nothing */
}

/******************************************************************************/
/**
     * The copy constructor.
     *
     * @param cp A copy of another GOptimizableEntity object
     */
GOptimizableEntity::GOptimizableEntity(GOptimizableEntity const &cp)
  : Gem::Common::GCommonInterfaceT<GOptimizableEntity>(cp)
  , Interface::GMutableI(cp)
  , Interface::GRateableI(cp)
  , Gem::Courtier::GProcessingContainerT<GOptimizableEntity, individual_processing_result>(cp)
  , best_past_primary_fitness_(cp.best_past_primary_fitness_)
  , n_stalls_(cp.n_stalls_)
  , maxmode_(cp.maxmode_)
  , assigned_iteration_(cp.assigned_iteration_)
  , validity_level_(cp.validity_level_)
  , eval_policy_(cp.eval_policy_)
  , sigmoid_steepness_(cp.sigmoid_steepness_)
  , sigmoid_extremes_(cp.sigmoid_extremes_)
  , max_unsuccessful_adaptions_(cp.max_unsuccessful_adaptions_)
  , max_retries_until_valid_(cp.max_retries_until_valid_)
  , n_adaptions_(cp.n_adaptions_) {
    // Make sure any constraints are copied over
    Gem::Common::copyCloneableSmartPointer(
        cp.individual_constraint_ptr_,
        individual_constraint_ptr_
    );
}

/******************************************************************************/
/**
     * Searches for compliance with expectations with respect to another object
     * of the same type
     *
     * @param cp A constant reference to another GOptimizableEntity object
     * @param e The expected outcome of the comparison
     */
void GOptimizableEntity::compare_(
    GOptimizableEntity const &cp,
    Gem::Common::expectation const &e,
    [[maybe_unused]] double const & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GOptimizableEntity reference independent of this object and convert the pointer
    const GOptimizableEntity *p_load =
        Gem::Common::g_convert_and_compare<GOptimizableEntity, GOptimizableEntity>(cp, this);

    GToken token("GOptimizableEntity", e);

    // Compare our CRTP base data (the category root has no GObject parent) ...
    Gem::Common::compare_base_t<Gem::Common::GCommonInterfaceT<GOptimizableEntity>>(*this, *p_load, token);

    // ... and all the local data (plain + cloneable pointers), derived from the
    // single localMembers() declaration.
    Gem::Common::g_compare_members(localMembers(), p_load->localMembers(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
     * Allows to randomly initialize parameter members. This function is responsible
     * for setting the dirty flag, so overloaded randomInit_ functions do not need
     * to take care of this. Note though that overloads of randomInit_() need to take
     * care to indicate whether modifications were made.
     *
     * @return A boolean indicating whether modifications where made
     */
bool GOptimizableEntity::randomInit(activityMode const &am) {
    bool modifications_made = this->randomInit_(am);

    if(modifications_made) {
        this->mark_as_due_for_processing();
    }

    return modifications_made;
}

/******************************************************************************/
/**
     * Allows to specify whether we want to work in maximization (maxMode::MAXIMIZE) or minimization
     * (maxMode::MINIMIZE) mode (the default). The idea is that GOptimizableEntity, depending on the
     * maxMode, changes its evaluation in such a way that the optimization algorithm always sees a
     * minimization problem.
     *
     * @param mode An enum class which indicates whether we want to work in maximization or minimization mode
     */
void GOptimizableEntity::setMaxMode(maxMode const &mode) {
    maxmode_ = mode;
}

/******************************************************************************/
/**
     * Checks whether this object is better than a given set of evaluations. This
     * function compares "real" boundaries with evaluations, hence we use "raw"
     * measurements here instead of transformed measurements.
     */
bool GOptimizableEntity::isGoodEnough(std::vector<double> const &boundaries) {
#ifdef DEBUG
    // Does the number of fitness criteria match the number of boundaries ?
    if(boundaries.size() != this->getNStoredResults()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GOptimizableEntity::isGoodEnough(): Error!" << '\n'
            << "Number of boundaries does not match number of fitness criteria" << '\n'
        );
    }

    // Has the individual been processed
    if(not this->is_processed()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GOptimizableEntity::isGoodEnough(): Error!" << '\n'
            << "Trying to compare fitness values although the individual isn't processed"
            << '\n'
        );
    }
#endif /* DEBUG */

    // Check the fitness values. If we find at least one
    // which is worse than the one supplied by the boundaries
    // vector, then this individual fails the test
    if(maxMode::MAXIMIZE == this->getMaxMode()) {
        // Maximization
        for(std::size_t i = 0; i < boundaries.size(); i++) {
            if(this->raw_fitness(i) < boundaries.at(i)) {
                return false;
            }
        }
    }
    else {
        // maxMode::MINIMIZE
        for(std::size_t i = 0; i < boundaries.size(); i++) {
            if(this->raw_fitness(i) > boundaries.at(i)) {
                return false;
            }
        }
    }

    // All fitness values are better than those supplied by boundaries
    return true;
}

/******************************************************************************/
/**
     * Retrieves the stored raw fitness with a given id
     */
double GOptimizableEntity::raw_fitness_(const std::size_t id) const {
    return this->getStoredResult(id).rawFitness();
}

/******************************************************************************/
/**
     * Retrieves the stored transformed fitness with a given id
     */
double GOptimizableEntity::transformed_fitness_(const std::size_t id) const {
    return this->getStoredResult(id).transformedFitness();
}

/******************************************************************************/
/**
     * Returns all raw fitness results in a std::vector
     */
std::vector<double> GOptimizableEntity::raw_fitness_vec_() const {
    std::size_t n_fitness_criteria = this->getNStoredResults();
    std::vector<double> result_vec;

    for(std::size_t i = 0; i < n_fitness_criteria; i++) {
        result_vec.push_back(this->raw_fitness(i));
    }

    return result_vec;
}

/******************************************************************************/
/**
     * Returns all transformed fitness results in a std::vector
     */
std::vector<double> GOptimizableEntity::transformed_fitness_vec_() const {
    std::size_t n_fitness_criteria = this->getNStoredResults();
    std::vector<double> result_vec;

    for(std::size_t i = 0; i < n_fitness_criteria; i++) {
        result_vec.push_back(this->transformed_fitness(i));
    }

    return result_vec;
}

/******************************************************************************/
/**
     * Register another result value of the fitness calculation. Multiple fitness
     * criteria are used in multi-criterion optimization. fitnessCalculation() returns
     * the main fitness value, but may also add further, secondary results. Note that,
     * whether these are actually used, depends on the optimization algorithm being
     * used. Transformation for the second fitness value will be done in the process_()
     * function. You may store the primary fitness value with this function as well.
     * As the primary (raw) value is however also returned by fitnessCalculation() and
     * integrated into the list of results, this is redundant.
     *
     * @param id The position of the fitness criterion (must be >= 0 !)
     * @param value The fitness value to be registered
     */
void GOptimizableEntity::setResult(const std::size_t id, const double value) {
#ifdef DEBUG
    if(id >= this->getNStoredResults()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GOptimizableEntity::setResult(...): Error!" << '\n'
            << "Invalid position in vector: " << id << " (expected min 0 and max "
            << this->getNStoredResults() - 1 << ")" << '\n'
        );
    }
#endif /* DEBUG */

    this->modifyStoredResult(id).reset(value);
}

/******************************************************************************/
/**
     * Determines whether more than one fitness criterion is present for this individual
     *
     * @return A boolean indicating whether more than one target function is present
     */
bool GOptimizableEntity::hasMultipleFitnessCriteria() const {
    return this->getNStoredResults() > 1;
}

/******************************************************************************/
/**
     * Retrieve the fitness tuple at a given evaluation position.
     */
std::tuple<double, double> GOptimizableEntity::getFitnessTuple(const std::uint32_t id) const {
    return std::make_tuple<double, double>(this->raw_fitness(id), this->transformed_fitness(id));
}

/******************************************************************************/
/**
     * Allows to retrieve the maxmode_ parameter
     *
     * @return The current value of the maxmode_ parameter
     */
maxMode GOptimizableEntity::getMaxMode() const {
    return maxmode_;
}

/***************************************************************************/
/**
     * Helper function that emits the worst case value depending on whether maximization
     * or minimization is performed.
     *
     * @return The worst case value, depending on maximization or minimization
     */
double GOptimizableEntity::getWorstCase() const {
    return (
        (maxMode::MAXIMIZE == this->getMaxMode()) ? std::numeric_limits<double>::lowest()
                                                  : std::numeric_limits<double>::max()
    );
}

/******************************************************************************/
/**
     * Retrieves the best possible evaluation result, depending on whether we are in
     * maximization or minimization mode
     */
double GOptimizableEntity::getBestCase() const {
    return (
        (maxMode::MAXIMIZE == this->getMaxMode()) ? std::numeric_limits<double>::max()
                                                  : std::numeric_limits<double>::lowest()
    );
}

/******************************************************************************/
/**
     * Retrieves the steepness_ variable (used for the sigmoid transformation)
     */
double GOptimizableEntity::getSteepness() const {
    return sigmoid_steepness_;
}

/******************************************************************************/
/**
     * Sets the steepness variable (used for the sigmoid transformation)
     */
void GOptimizableEntity::setSteepness(const double steepness) {
    if(steepness <= 0.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GOptimizableEntity::setSteepness(double steepness): Error!" << '\n'
            << "Invalid value of steepness parameter: " << steepness << '\n'
        );
    }

    sigmoid_steepness_ = steepness;
}

/******************************************************************************/
/**
     * Retrieves the barrier_ variable (used for the sigmoid transformation)
     */
double GOptimizableEntity::getBarrier() const {
    return sigmoid_extremes_;
}

/******************************************************************************/
/**
     * Sets the barrier variable (used for the sigmoid transformation)
     */
void GOptimizableEntity::setBarrier(const double barrier) {
    if(barrier <= 0.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GOptimizableEntity::setBarrier(double barrier): Error!" << '\n'
            << "Invalid value of barrier parameter: " << barrier << '\n'
        );
    }

    sigmoid_extremes_ = barrier;
}

/******************************************************************************/
/**
     * Sets the maximum number of adaption attempts that may pass without
     * actual modifications. Setting this to 0 disables this check. You should only
     * do this if you are sure that an adaption will eventually happen. Otherwise
     * you would get an endless loop.
     */
void GOptimizableEntity::setMaxUnsuccessfulAdaptions(const std::size_t max_unsuccessful_adaptions) {
    max_unsuccessful_adaptions_ = max_unsuccessful_adaptions;
}

/******************************************************************************/
/**
     * Retrieves the maximum number of adaption attempts that may pass without
     * actual modifications
     */
std::size_t GOptimizableEntity::getMaxUnsuccessfulAdaptions() const {
    return max_unsuccessful_adaptions_;
}

/******************************************************************************/
/**
     * Allows to set the maximum number of retries during the adaption of individuals
     * until a valid individual was found. Setting this value to 0 will disable retries.
     */
void GOptimizableEntity::setMaxRetriesUntilValid(const std::size_t max_retries_until_valid) {
    max_retries_until_valid_ = max_retries_until_valid;
}

/******************************************************************************/
/**
     * Allows to retrieve the current maximum number of retries during the adaption of
     * individuals until a valid individual was found.
     */
std::size_t GOptimizableEntity::getMaxRetriesUntilValid() const {
    return max_retries_until_valid_;
}

/******************************************************************************/
/**
     * Retrieves the number of adaptions performed during the last call to adapt()
     * (or 0, if no adaptions were performed so far).
     */
std::size_t GOptimizableEntity::getNAdaptions() const {
    return n_adaptions_;
}

/******************************************************************************/
/**
     * Allows to set the current iteration of the parent optimization algorithm.
     *
     * @param parent_alg_iteration The current iteration of the optimization algorithm
     */
void GOptimizableEntity::setAssignedIteration(std::uint32_t const &parent_alg_iteration) {
    assigned_iteration_ = parent_alg_iteration;
}

/******************************************************************************/
/**
     * Gives access to the parent optimization algorithm's iteration
     *
     * @return The parent optimization algorithm's current iteration
     */
std::uint32_t GOptimizableEntity::getAssignedIteration() const {
    return assigned_iteration_;
}

/******************************************************************************/
/**
     * Allows to specify the number of optimization cycles without improvement of the primary fitness criterion
     *
     * @param n_stalls The number of optimization cycles without improvement in the parent algorithm
     */
void GOptimizableEntity::setNStalls(std::uint32_t const &n_stalls) {
    n_stalls_ = n_stalls;
}

/******************************************************************************/
/**
     * Allows to retrieve the number of optimization cycles without improvement of the primary fitness criterion
     *
     * @return The number of optimization cycles without improvement in the parent algorithm
     */
std::uint32_t GOptimizableEntity::getNStalls() const {
    return n_stalls_;
}

/******************************************************************************/
/**
     * Allows to check whether random crashs of individuals are enabled
     */
std::tuple<bool, double> GOptimizableEntity::getRandomCrash() const {
    return std::tuple<bool, double>{use_random_crash_, random_crash_prob_};
};

/******************************************************************************/
/**
     * Allows to enable random crashs of individuals for testing purposes
     */
void GOptimizableEntity::setRandomCrash(const bool use_random_crash, const double crash_prob) {
    // Check that the crash probability is in the allowed value range
    Gem::Common::checkRangeCompliance(crash_prob, 0., 1., "GOptimizableEntity::setRandomCrash()");

    // Set the value as demanded
    use_random_crash_ = use_random_crash;
    random_crash_prob_ = crash_prob;
}

/******************************************************************************/
/**
     * Adds local configuration options to a GParserBuilder object
     *
     * @param gpb The GParserBuilder object to which configuration options should be added
     */
void GOptimizableEntity::addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) {
    // Call our CRTP base class'es function (the category root has no GObject parent)
    Gem::Common::GCommonInterfaceT<GOptimizableEntity>::addConfigurationOptions_(gpb);

    // Add local data
    gpb.registerFileParameter<evaluationPolicy>(
        "eval_policy" // The name of the variable
        ,
        Gem::Geneva::evaluationPolicy::USESIMPLEEVALUATION
        // The default value
        ,
        [this](const evaluationPolicy ep) { this->setEvaluationPolicy(ep); }
    ) << "Specifies which strategy should be used to calculate the evaluation:"
      << '\n'
      << "0 (a.k.a. USESIMPLEEVALUATION): Always call the evaluation function, even for invalid "
         "solutions"
      << '\n'
      << "1 (a.k.a. USEWORSTCASEFORINVALID) : Assign the worst possible value to our fitness and "
         "evaluate only valid solutions"
      << '\n'
      << "2 (a.k.a. USESIGMOID): Assign a multiple of validity_level_ and sigmoid barrier to "
         "invalid solutions, apply a sigmoid function to valid evaluations"
      << '\n';

    gpb.registerFileParameter<double>(
        "steepness" // The name of the variable
        ,
        Gem::Geneva::FITNESSSIGMOIDSTEEPNESS // The default value
        ,
        [this](const double ss) { this->setSteepness(ss); }
    ) << "When using a sigmoid function to transform the individual's fitness,"
      << '\n'
      << "this parameter influences the steepness of the function at the center of the sigmoid."
      << '\n'
      << "The parameter must have a value > 0.";

    gpb.registerFileParameter<double>(
        "barrier" // The name of the variable
        ,
        Gem::Geneva::WORSTALLOWEDVALIDFITNESS // The default value
        ,
        [this](const double barrier) { this->setBarrier(barrier); }
    ) << "When using a sigmoid function to transform the individual's fitness,"
      << '\n'
      << "this parameter sets the upper/lower boundary of the sigmoid." << '\n'
      << "The parameter must have a value > 0.;";

    gpb.registerFileParameter<std::size_t>(
        "max_unsuccessful_adaptions" // The name of the variable
        ,
        DEFMAXUNSUCCESSFULADAPTIONS // The default value
        ,
        [this](const std::size_t mua) { this->setMaxUnsuccessfulAdaptions(mua); }
    ) << "The maximum number of unsuccessful adaptions in a row for one call to adapt()";

    gpb.registerFileParameter<std::size_t>(
        "max_retries_until_valid" // The name of the variable
        ,
        DEFMAXRETRIESUNTILVALID // The default value
        ,
        [this](const std::size_t mruv) { this->setMaxRetriesUntilValid(mruv); }
    ) << "The maximum allowed number of retries during the"
      << '\n'
      << "adaption of individuals until a valid solution was found" << '\n'
      << "A parameter set is considered to be \"valid\" if" << '\n'
      << "it passes all validity checks;";

    // Add local data
    gpb.registerFileParameter<maxMode>(
        "maxmode" // The name of the variable
        ,
        maxMode::MINIMIZE // The default value
        ,
        [this](const maxMode mm) { this->setMaxMode(mm); }
    ) << "Specifies whether the individual should be maximized (1) or minimized (0)"
      << '\n'
      << "Note that minimization is the by far most common option.";

    gpb.registerFileParameter<bool, double>(
        "use_random_crash" // The name of the variable
        ,
        "random_crash_prob",
        GPS_DEF_USE_RANDOMCRASH // The default value
        ,
        GPS_DEF_RANDOMCRASHPROB,
        [this](const bool use_rc, const double rc_prob) { this->setRandomCrash(use_rc, rc_prob); },
        "random_crash_parameters"
    ) << "Indicates whether random crashes should occur for debugging purposes"
      << '\n'
      << Gem::Common::nextComment() << "The probability of a random crash to occur";
}

/******************************************************************************/
/**
     * Emits a name for this class / object
     *
     * @return The name of this class / object
     */
std::string GOptimizableEntity::name_() const {
    return std::string("GOptimizableEntity");
}

/******************************************************************************/
/**
     * Check how valid a given solution is
     *
     * @return The validity level of this solution
     */
double GOptimizableEntity::getValidityLevel() const {
    return validity_level_;
}

/******************************************************************************/
/**
     * @return A boolean indicating, whether all constraints were fulfilled
     */
bool GOptimizableEntity::constraintsFulfilled() const {
    if(validity_level_ <= 1.) {
        return true;
    }
            return false;

}

/******************************************************************************/
/**
     * Allows to register a constraint with this individual. Note that the constraint
     * object will be cloned.
     */
void GOptimizableEntity::registerConstraint(
    std::shared_ptr<GPreEvaluationValidityCheckT<GOptimizableEntity>> c_ptr
) {
    if(not c_ptr) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GOptimizableEntity::registerConstraint(): Error!" << '\n'
            << "Tried to register empty constraint object" << '\n'
        );
    }

    // We store clones, so individual objects do not share the same object
    individual_constraint_ptr_ =
        c_ptr->clone<GPreEvaluationValidityCheckT<GOptimizableEntity>>();
}

/******************************************************************************/
/**
     * Allows to set the policy to use in case this individual represents an invalid solution
     */
void GOptimizableEntity::setEvaluationPolicy(const evaluationPolicy eval_policy) {
    eval_policy_ = eval_policy;
}

/******************************************************************************/
/**
     * Allows to retrieve the current policy in case this individual represents an invalid solution
     */
evaluationPolicy GOptimizableEntity::getEvaluationPolicy() const {
    return eval_policy_;
}

/******************************************************************************/
/**
     * Checks whether this solution is valid. This function is meant to be called
     * for "clean" individuals only and will throw when called for unprocessed or
     * erroneous individuals.
     */
bool GOptimizableEntity::isValid() const {
#ifdef DEBUG
    if(this->is_due_for_processing() || this->has_errors()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GOptimizableEntity::isValid():" << '\n'
            << "Function was called for unprocessed or erroneous individual" << '\n'
        );
    }
#endif

    if(validity_level_ <= 1.) {
        return true;
    }
            return false;

}

/******************************************************************************/
/**
     * Checks whether this solution is invalid
     */
bool GOptimizableEntity::isInValid() const {
    return not this->isValid();
}

/******************************************************************************/
/**
     * Allows to set the globally best known primary fitness so far
     *
     * @param bnf The best known primary fitness so far
     */
void GOptimizableEntity::setBestKnownPrimaryFitness(const std::tuple<double, double> &bnf) {
    best_past_primary_fitness_ = bnf;
}

/******************************************************************************/
/**
     * Retrieves the value of the globally best known primary fitness so far
     *
     * @return The best known primary fitness so far
     */
std::tuple<double, double> GOptimizableEntity::getBestKnownPrimaryFitness() const {
    return best_past_primary_fitness_;
}

/******************************************************************************/
/**
     * Performs all necessary (remote-)processing steps for this object.
     */
void GOptimizableEntity::process_(const std::vector<individual_processing_result> &res_vec) {
#ifdef DEBUG
    //---------------------------------------------
    // Crash if we have been asked to (only active in DEBUG mode)
    if(use_random_crash_) {
        std::uniform_real_distribution<double> dist01{0., 1.};
        if(dist01(this->gr_) <= random_crash_prob_) {
            glogger << "GOptimizableEntity is performing random crash for debugging purposes"
                    << '\n'
                    << '\n'
                    << GLOGGING;

            throw;
        }
    }
#endif

    // Find out, whether this is a valid solution
    if(this->individualFulfillsConstraints(validity_level_)
       // Needs to be called first, or else the validity_level_ will not be filled
       || evaluationPolicy::USESIMPLEEVALUATION == eval_policy_) {
        // Trigger actual fitness calculation using the user-supplied function. This will
        // also register any secondary "raw" fitness values used in multi-criterion optimization.
        // Transformation of values is taken care of below.
        double main_raw_result = 0.;

        try {
            if(not res_vec.empty()) {
                // Check that sizes match
                if(res_vec.size() != this->getNStoredResults()) {
                    throw geneva_exception(
                        g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                        << "In GOptimizableEntity::process_ : Error!" << '\n'
                        << "res_vec has invalid size. Got " << res_vec.size() << '\n'
                        << "Expected " << this->getNStoredResults() << '\n'
                    );
                }

                // Just assign the main *raw* result
                main_raw_result = res_vec.begin()->rawFitness();

                // Extract all additional *raw* results. Then we are on par with fitnessCalculation()
                std::size_t pos = 0;
                for(const auto &res : res_vec) {
                    if(pos == 0) {
                        continue; // Skip the main raw result
                    }

                    this->setResult(pos, res_vec.at(pos).rawFitness());

                    pos++;
                }
            }
            else {
                // If we are dealing with multiple fitness criteria,
                // then fitnessCalculation() will set additional raw values
                main_raw_result = this->fitnessCalculation();
            }
        }
        catch(...) {
            // Make sure we invalidate all fitness values, if an exception was thrown
            this->setAllFitnessTo(this->getWorstCase());

            // Rethrow the exception
            throw;
        }

        // Make sure the main result is stored
        // TODO: result setting should be done in the parent class'es process()-function, not in process_()
        this->setResult(0, main_raw_result);
        // TODO: When using multiple criteria: Are we setting the other transformed results also to raw?
        this->modifyStoredResult(0).setTransformedFitnessToRaw();

        // Take care of erroneous calculations, flagged by the user. It is assumed here that marking
        // entire solutions as invalid after the evaluation happens relatively rarely so that a flat
        // "worst" quality surface for such solutions does not hinder progress of the optimization
        // procedure too much
        if(this->error_flagged_by_user()) {
            // has the user indicated a problem without throwing an error ?
            // Fill the raw and transformed vectors with the worst case scenario.
            this->setAllFitnessTo(this->getWorstCase());
        }
        else {
            // So this is a valid solution!
            for(std::size_t i = 0; i < this->getNStoredResults(); i++) {
                if(evaluationPolicy::USESIGMOID == eval_policy_) {
                    // Update the fitness value to use sigmoidal values
                    this->modifyStoredResult(i).setTransformedFitnessWith(
                        [this](const double raw_value) {
                            return Gem::Common::grational_sigmoid(
                                raw_value,
                                this->sigmoid_extremes_,
                                this->sigmoid_steepness_
                            );
                        }
                    );
                }
                else {
                    // All other transformation policies use the same value for the transformed fitness as a (valid) raw fitness
                    this->modifyStoredResult(i).setTransformedFitnessToRaw();
                }
            }
        }
    }
    else {
        // Some constraints were violated. Act on the chosen policy
        if(evaluationPolicy::USEWORSTCASEFORINVALID == eval_policy_) {
            this->setAllFitnessTo(this->getWorstCase());
        }
        else if(evaluationPolicy::USESIGMOID == eval_policy_) {
            double uniform_fitness_value = 0.;
            if(maxMode::MAXIMIZE == this->getMaxMode()) {
                // maximize
                if(std::numeric_limits<double>::max() == validity_level_) {
                    uniform_fitness_value = this->getWorstCase();
                }
                else {
                    uniform_fitness_value = -validity_level_ * sigmoid_extremes_;
                }
            }
            else {
                // minimize
                if(std::numeric_limits<double>::max() == validity_level_) {
                    uniform_fitness_value = this->getWorstCase();
                }
                else {
                    uniform_fitness_value = validity_level_ * sigmoid_extremes_;
                }
            }

            this->setAllFitnessTo(this->getWorstCase(), uniform_fitness_value);
        }
    }
}

/******************************************************************************/
/**
     * Loads the data of another GOptimizableEntity object.
     *
     * @param cp A copy of another GOptimizableEntity object
     */
void GOptimizableEntity::load_(const GOptimizableEntity *cp) {
    // Check that we are dealing with a GOptimizableEntity reference independent of this object and convert the pointer
    const GOptimizableEntity *p_load =
        Gem::Common::g_convert_and_compare<GOptimizableEntity, GOptimizableEntity>(cp, this);

    // This is the category root; there is no GObject parent class to load.
    // Load the stateful processing base class' data
    Gem::Courtier::GProcessingContainerT<GOptimizableEntity, individual_processing_result>::load_pc(
        p_load
    );

    // All local data, derived from the single localMembers() declaration: plain
    // members are assigned, the cloneable smart pointers are deep-cloned (the tie
    // dispatches on the member kind). The OA-owned scratch (personality + the per-group adaption POD
    // state) is not held here — it lives on the GIndividualSlot and is copied by GIndividualSlot::load_.
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/******************************************************************************/
/**
     * Sets the fitness to a given set of values and clears the dirty flag. This is meant
     * to be used by external methods of performing the actual evaluation, such as the
     * OpenCL-Consumer. The fitness vector is interpreted as raw fitness values, and
     * transformed fitness values are calculated as needed.
     *
     * @param f_cnt A vector of raw fitness values
     */
void GOptimizableEntity::setFitness_(std::vector<double> const &f_cnt) {
#ifdef DEBUG
    if(f_cnt.size() != this->getNStoredResults()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GOptimizableEntity::setFitness_(...): Error!" << '\n'
            << "Invalid size of fitness vector: " << '\n'
            << f_cnt.size() << ", expected: " << this->getNStoredResults() << '\n'
        );
    }
#endif /* DEBUG */

    // Find out, whether this is a valid solution
    if(this->individualFulfillsConstraints(validity_level_)
       // Needs to be called first, or else the validity_level_ will not be filled
       || evaluationPolicy::USESIMPLEEVALUATION == eval_policy_) {
        // Create a vector of individual_processing_result objects
        std::vector<individual_processing_result> processing_results(
            f_cnt.size(),
            individual_processing_result()
        );

        // Take care of the transformed fitness
        std::size_t pos = 0;
        for(auto &p : processing_results) {
            // Set the raw fitness
            p.reset(f_cnt.at(pos));

            if(evaluationPolicy::USESIGMOID == eval_policy_) {
                // Update the fitness value to use sigmoidal values
                p.setTransformedFitnessWith([this](const double raw_value) {
                    return Gem::Common::grational_sigmoid(
                        raw_value,
                        this->sigmoid_extremes_,
                        this->sigmoid_steepness_
                    );
                });
            }
            else {
                // All other transformation policies use the same value for the transformed fitness as a (valid) raw fitness
                p.setTransformedFitnessToRaw();
            }

            pos++;
        }

        // Transfer the data into the individual
        this->markAsProcessedWith(processing_results);
    }
    else {
        // Some constraints were violated. Act on the chosen policy
        if(evaluationPolicy::USEWORSTCASEFORINVALID == eval_policy_) {
            this->setAllFitnessTo(this->getWorstCase());
        }
        else if(evaluationPolicy::USESIGMOID == eval_policy_) {
            double uniform_fitness_value = 0.;
            if(maxMode::MAXIMIZE == this->getMaxMode()) {
                // maximize
                if(std::numeric_limits<double>::max() == validity_level_) {
                    uniform_fitness_value = this->getWorstCase();
                }
                else {
                    uniform_fitness_value = -validity_level_ * sigmoid_extremes_;
                }
            }
            else {
                // minimize
                if(std::numeric_limits<double>::max() == validity_level_) {
                    uniform_fitness_value = this->getWorstCase();
                }
                else {
                    uniform_fitness_value = validity_level_ * sigmoid_extremes_;
                }
            }

            this->setAllFitnessTo(this->getWorstCase(), uniform_fitness_value);
        }
    }
}

/******************************************************************************/
/**
     * Combines evaluation results by adding the individual results
     *
     *  @return The result of the combination
     */
double GOptimizableEntity::sumCombiner() const {
    double result = 0.;

    for(std::size_t id = 0; id < this->getNStoredResults(); id++) {
        result += this->transformed_fitness(id);
    }

    return result;
}

/******************************************************************************/
/**
     * Combines evaluation results by adding the absolute values of individual results
     *
     *  @return The result of the combination
     */
double GOptimizableEntity::fabsSumCombiner() const {
    double result = 0.;

    for(std::size_t id = 0; id < this->getNStoredResults(); id++) {
        result += std::abs(this->transformed_fitness(id));
    }

    return result;
}

/******************************************************************************/
/**
     * Combines evaluation results by calculating the square root of the squared sum.
     * It is assumed that the result of this function is returned as
     * the main result of the fitnessCalculation() function.
     *
     * @return The result of the combination
     */
double GOptimizableEntity::squaredSumCombiner() const {
    double result = 0.;

    for(std::size_t id = 0; id < this->getNStoredResults(); id++) {
        result += Gem::Common::gsquared(this->transformed_fitness(id));
    }

    return sqrt(result);
}

/******************************************************************************/
/**
     * Combines evaluation results by calculating the square root of the weighed squared sum. Note that we
     * only evaluate the secondary results here. It is assumed that the result of this function is returned as
     * the main result of the fitnessCalculation() function.
     *
     * @param weights The weights to be multiplied with the cached results
     * @return The result of the combination
     */
double GOptimizableEntity::weighedSquaredSumCombiner(std::vector<double> const &weights) const {
    if(this->getNStoredResults() != weights.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GOptimizableEntity::weighedSquaredSumCombine(): Error!" << '\n'
            << "Sizes of transformedCurrentFitnessVec_ and the weights vector don't match: "
            << this->getNStoredResults() << " / " << weights.size() << '\n'
        );
    }

    double result = 0.;
    auto cit_weights = weights.begin();

    for(std::size_t id = 0; id < this->getNStoredResults(); id++, ++cit_weights) {
        result += Gem::Common::gsquared((*cit_weights) * this->transformed_fitness(id));
    }

    return sqrt(result);
}

/******************************************************************************/
/**
     * Checks whether this solution fulfills the set of constraints. Note that this
     * function may be called prior to evaluation in order to check
     */
bool GOptimizableEntity::individualFulfillsConstraints(double &validity_level) const {
    if(individual_constraint_ptr_) {
        return individual_constraint_ptr_->isValid(this, validity_level);
    }
            // Always valid, if no constraint object has been registered
        validity_level = 0.;
        return true;


    // Make the compiler happy
    return false;
}

/***************************************************************************/
/**
     * Allows to set all fitnesses to the same value (raw and transformed values seperately)
     */
void GOptimizableEntity::setAllFitnessTo(const double raw_value, const double transformed_value) {
    for(std::size_t i = 0; i < this->getNStoredResults(); i++) {
        this->modifyStoredResult(i).reset(raw_value);
        this->modifyStoredResult(i).setTransformedFitnessTo(transformed_value);
    }
}

/***************************************************************************/
/**
     * Allows to set all fitnesses to the same value (both raw and transformed values)
     */
void GOptimizableEntity::setAllFitnessTo(const double val) {
    this->setAllFitnessTo(val, val);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
} /* namespace Gem::Geneva::Genome */
