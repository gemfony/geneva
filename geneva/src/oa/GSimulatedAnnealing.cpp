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

#include "geneva/oa/GSimulatedAnnealing.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "common/GParserBuilder.hpp"
#include "common/GThreadPool.hpp"
#include "courtier/GProcessingContainerT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include "geneva/GenevaHelperFunctions.hpp"
#include "geneva/oa/GOptimizationAlgorithmBase.hpp"
#include "geneva/oa/GParChild.hpp"
#include "geneva/oa/GSimulatedAnnealing_PersonalityTraits.hpp"
#include "geneva/oa/GAdaption.hpp"
#include "geneva/oa/GAdaptionConfig.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <future>
#include <limits>
#include <memory>
#include <random>
#include <tuple>
#include <vector>

/******************************************************************************/

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::OptimizationAlgorithms::GSimulatedAnnealing) // NOLINT

/******************************************************************************/

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

/**
 * @brief The default constructor. All initialization work of member variables
 * is done in the class body.
 */
GSimulatedAnnealing::GSimulatedAnnealing() {
    // Make sure we start with a valid population size if the user does not supply these values
    this->setPopulationSizes(100, 1);
}

/******************************************************************************/
/**
  * @brief Searches for compliance with expectations with respect to another object
  * of the same type
  *
  * @param cp A constant reference to another GOptimizationAlgorithmBase, expected to be a GSimulatedAnnealing
  * @param e The expected outcome of the comparison (e.g. equality or inequality)
 * @param limit The maximum deviation for floating point values (unused here; important for similarity checks)
 */
void GSimulatedAnnealing::compare_(
    const GOptimizationAlgorithmBase &cp // the other object
    ,
    const Gem::Common::expectation &e // the expectation for this object, e.g. equality
    ,
    const double & /*limit*/ // the limit for allowed deviations of floating point types
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GSimulatedAnnealing reference independent of this object and convert the pointer
    const GSimulatedAnnealing *p_load =
        Gem::Common::g_convert_and_compare<GOptimizationAlgorithmBase, GSimulatedAnnealing>(cp, this);

    GToken token("GSimulatedAnnealing", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GParChild>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    g_compare_members(localMembers_(*this), localMembers_(*p_load), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * @brief Resets the settings of this population to what was configured when
 * the optimize()-call was issued
 */
void GSimulatedAnnealing::resetToOptimizationStart_() {
    // Reset the temperature
    t_ = t0_;

    // There is no more work to be done here, so we simply call the
    // function of the parent class
    GParChild::resetToOptimizationStart_();
}

/******************************************************************************/
/**
  * @brief Adds local configuration options to a GParserBuilder object
  *
  * @param gpb The GParserBuilder object to which configuration options should be added
  */
void GSimulatedAnnealing::addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) {
    // Call our parent class'es function (registers the shared n_adaption_threads option)
    GParChild::addConfigurationOptions_(gpb);

    // Add local data
    gpb.registerFileParameter<double>(
        "t0" // The name of the variable
        ,
        SA_T0 // The default value
        ,
        [this](double sat0) { this->setT0(sat0); }
    ) << "The start temperature used in simulated annealing";

    gpb.registerFileParameter<double>(
        "alpha" // The name of the variable
        ,
        SA_ALPHA // The default value
        ,
        [this](double ds) { this->setTDegradationStrength(ds); }
    ) << "The degradation strength used in the cooling"
      << '\n'
      << "schedule in simulated annealing;";
}

/******************************************************************************/
/**
  * @brief Determines the strength of the temperature degradation. This function is used for simulated annealing.
  *
  * @param alpha The temperature degradation strength; must be in the open interval (0,1)
  */
void GSimulatedAnnealing::setTDegradationStrength(double alpha) {
    if(alpha <= 0. || alpha >= 1.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GSimulatedAnnealing::setTDegradationStrength(const double&):" << '\n'
            << "alpha must be in (0,1) for SA cooling; got: " << alpha << '\n'
        );
    }

    alpha_ = alpha;
}

/******************************************************************************/
/**
  * @brief Retrieves the temperature degradation strength. This function is used for simulated annealing.
  *
  * @return The temperature degradation strength
  */
double GSimulatedAnnealing::getTDegradationStrength() const {
    return alpha_;
}

/******************************************************************************/
/**
  * @brief Sets the start temperature. This function is used for simulated annealing.
  *
  * @param t0 The start temperature; must be strictly positive
  */
void GSimulatedAnnealing::setT0(double t0) {
    if(t0 <= 0.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GSimulatedAnnealing::setT0(const double&):" << '\n'
            << "Got negative start temperature: " << t0 << '\n'
        );
    }

    t0_ = t0;
}

/******************************************************************************/
/**
  * @brief Retrieves the start temperature. This function is used for simulated annealing.
  *
  * @return The start temperature
  */
double GSimulatedAnnealing::getT0() const {
    return t0_;
}

/******************************************************************************/
/**
  * @brief Retrieves the current temperature. This function is used for simulated annealing.
  *
  * @return The current temperature
  */
double GSimulatedAnnealing::getT() const {
    return t_;
}

/******************************************************************************/
/**
  * @brief Loads the data of another GSimulatedAnnealing object.
  *
  * @param cp A pointer to another GOptimizationAlgorithmBase, expected to be a GSimulatedAnnealing
  */
void GSimulatedAnnealing::load_(const GOptimizationAlgorithmBase *cp) {
    // Check that we are dealing with a GSimulatedAnnealing reference independent
    // of this object and convert the pointer
    const GSimulatedAnnealing *p_load =
        Gem::Common::g_convert_and_compare<GOptimizationAlgorithmBase, GSimulatedAnnealing>(cp, this);

    // First load the parent class'es data ...
    GParChild::load_(cp);

    // ... and then our own data, derived from the single localMembers() declaration
    Gem::Common::g_load_members(localMembers_(*this), localMembers_(*p_load));
}

/******************************************************************************/
/**
  * @brief Some error checks related to population sizes
  */
void GSimulatedAnnealing::populationSanityChecks_() const {
    // First check that we have been given a suitable value for the number of parents.
    // Note that a number of checks (e.g. population size != 0) has already been done
    // in the parent class.
    if(this->n_parents_ == 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GSimulatedAnnealing::populationSanityChecks(): Error!" << '\n'
            << "Number of parents is set to 0"
        );
    }

    // We need at least as many children as parents
    std::size_t pop_size = this->getPopulationSize();
    if(pop_size <= this->n_parents_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GSimulatedAnnealing::populationSanityChecks() :" << '\n'
            << "Requested size of population is too small :" << pop_size << " " << this->n_parents_
            << '\n'
        );
    }
}


/******************************************************************************/
/**
  * @brief We submit individuals to the broker connector and wait for processed items.
 */
void GSimulatedAnnealing::runFitnessCalculation_() {
    //--------------------------------------------------------------------------------
    // Start by marking the work to be done in the individuals.
    // "range" will hold the start- and end-points of the range
    // to be worked on
    std::tuple<std::size_t, std::size_t> range = getEvaluationRange_();

#ifdef DEBUG
    // There should be no situation in which a "clean" child is submitted
    // through this function. There MAY be situations, where in the first iteration
    // parents are clean, e.g. when they were extracted from another optimization.
    for(std::size_t i = this->getNParents(); i < this->size(); i++) {
        // "Dirty" (needs evaluation) is the GENOME's fitness-validity state (set by adaption); the slot's
        // transport DO_PROCESS is only set later by the consumer on the submitted span.
        if(not this->at(i)->individual().fitnessIsStale()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GSimulatedAnnealing::runFitnessCalculation(): Error!" << '\n'
                << "Tried to evaluate children in range " << std::get<0>(range) << " - "
                << std::get<1>(range) << '\n'
                << "but found \"clean\" individual in position " << i << '\n'
            );
        }
    }
#endif

    //--------------------------------------------------------------------------------
    // Submit the [start, end) evaluation range and wait for results. courtier marks the span
    // DO_PROCESS and reconciles it in place -- no per-item flagging needed.
    auto status = this->workOnPopulation(std::get<0>(range), std::get<1>(range));

    //--------------------------------------------------------------------------------
    // Take care of unprocessed items, if these exist. We simply remove them and continue.
    if(not status.is_complete) {
        std::size_t n_erased =
            std::erase_if(this->data_cnt_, [this](const std::unique_ptr<gen::GIndividualSlot> &p) -> bool {
                return (p->getProcessingStatus() == Gem::Courtier::processingStatus::DO_PROCESS);
            });

#ifdef DEBUG
        glogger << "In GSimulatedAnnealing::runFitnessCalculation(): " << '\n'
                << "Removed " << n_erased << " unprocessed work items in iteration "
                << this->getIteration() << '\n'
                << GLOGGING;
#endif
    }

    // Remove items for which an error has occurred during processing
    // We simply remove them and continue.
    if(status.has_errors) {
        std::size_t n_erased =
            std::erase_if(this->data_cnt_, [this](const std::unique_ptr<gen::GIndividualSlot> &p) -> bool {
                return p->has_errors();
            });

#ifdef DEBUG
        glogger << "In GSimulatedAnnealing::runFitnessCalculation(): " << '\n'
                << "Removed " << n_erased << " erroneous work items in iteration "
                << this->getIteration() << '\n'
                << GLOGGING;
#endif
    }

    //--------------------------------------------------------------------------------
    // Now fix the population -- it may be smaller than its nominal size
    fixAfterJobSubmission();
}


/******************************************************************************/
/**
  * @brief Choose new parents, based on the SA selection scheme.
  */
void GSimulatedAnnealing::selectBest_() {
    // Sort according to the "Simulated Annealing" scheme
    sortSAMode();

    // Let parents know they are parents
    this->markParents();

#ifdef DEBUG
    // Make sure our population is not smaller than its nominal size -- this
    // should have been taken care of in fixAfterJobSubmission() .
    if(this->size() < this->getDefaultPopulationSize()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GSimulatedAnnealing::selectBest(): Error!" << '\n'
            << "Size of population is smaller than expected: " << this->size() << " / "
            << this->getDefaultPopulationSize() << '\n'
        );
    }
#endif /* DEBUG */

    ////////////////////////////////////////////////////////////
    // At this point we have a sorted list of individuals and can take care of
    // too many members, so the next iteration finds a "standard" population. This
    // function will remove the last items.
    this->resize(this->getNParents() + this->getDefaultNChildren());

    // Let children know they are children
    this->markChildren();

    // Everything should be back to normal ...
}

/******************************************************************************/
/**
  * @brief Retrieves the evaluation range in a given iteration and sorting scheme. Depending on the
  * iteration and sorting scheme, the start point will be different. The end-point is not meant
  * to be inclusive.
  *
  * @return A tuple holding the half-open [start, end) range of population positions to be evaluated
  */
std::tuple<std::size_t, std::size_t> GSimulatedAnnealing::getEvaluationRange_() const {
    // We evaluate all individuals in the first iteration This happens so pluggable
    // optimization monitors do not need to distinguish between algorithms
    return std::tuple<std::size_t, std::size_t>{
        this->inFirstIteration() ? 0 : this->getNParents(),
        this->size()
    };
}

/******************************************************************************/
/**
  * @brief Retrieve a GPersonalityTraits object belonging to this algorithm
  *
  * @return A shared pointer to a freshly created GSimulatedAnnealing_PersonalityTraits object
  */
std::shared_ptr<GPersonalityTraits> GSimulatedAnnealing::getPersonalityTraits_() const {
    return std::make_shared<GSimulatedAnnealing_PersonalityTraits>();
}

/******************************************************************************/
/**
 * @brief Performs a simulated annealing style sorting and selection
 */
void GSimulatedAnnealing::sortSAMode() {
    // Position the n_parents best children of the population right behind the parents
    std::partial_sort(
        this->begin() + this->n_parents_,
        this->begin() + 2 * this->n_parents_,
        this->end(),
        [](const auto &x_ptr, const auto &y_ptr) -> bool {
            return minOnly_transformed_fitness(x_ptr->individual()) <
                   minOnly_transformed_fitness(y_ptr->individual());
        }
    );

    // Check for each parent whether it should be replaced by the corresponding child
    for(std::size_t np = 0; np < this->n_parents_; np++) {
        double p_pass = saProb(
            minOnly_transformed_fitness(this->at(np)->individual()),
            minOnly_transformed_fitness(this->at(this->n_parents_ + np)->individual())
        );
        if(p_pass >= 1.) {
            this->at(np)->load(this->at(this->n_parents_ + np));
        }
        else {
            double challenge = this->uniform_real_distribution_(
                this->gr_,
                std::uniform_real_distribution<double>::param_type(0., 1.)
            );
            if(challenge < p_pass) {
                this->at(np)->load(this->at(this->n_parents_ + np));
            }
        }
    }

    // Sort the new parents -- it is possible that a child with a worse fitness has replaced a parent
    std::sort(
        this->begin(),
        this->begin() + this->n_parents_,
        [](const auto &x_ptr, const auto &y_ptr) -> bool {
            return minOnly_transformed_fitness(x_ptr->individual()) <
                   minOnly_transformed_fitness(y_ptr->individual());
        }
    );

    // Make sure the temperature gets updated
    updateTemperature();
}

/******************************************************************************/
/**
  * @brief Calculates the simulated annealing probability for a child to replace a parent.
  * Note that this function only sees minimization problems, as maximization problems
  * are transformed to minimization problems inside of GOptimizableEntity.
  *
  * @param f_min_only_parent The "min only" (minimization-transformed) fitness of the parent
  * @param f_min_only_child The "min only" (minimization-transformed) fitness of the child
  * @return A double value representing the Boltzmann likelihood for the child to replace the parent
  */
double
GSimulatedAnnealing::saProb(const double &f_min_only_parent, const double &f_min_only_child) const {
    // Minimisation: a worse child has f_child > f_parent, so the exponent is negative and the result
    // lies in (0, 1); a child that is at least as good gives a non-negative exponent, hence a result
    // >= 1, which the caller treats as "always accept". t_ is floored > 0 by updateTemperature(), so
    // there is no division by zero.
    return exp(-(f_min_only_child - f_min_only_parent) / t_);
}

/******************************************************************************/
/**
  * @brief Updates the temperature. This function is used for simulated annealing.
  */
void GSimulatedAnnealing::updateTemperature() {
    // Clamp to the smallest normalised double so t_ never enters the subnormal range or reaches 0.
    // With t_ == 0 and a fitness gap of 0, saProb() would compute 0/0 = NaN; the floor prevents that
    // without changing late-phase behaviour: exp(-(f_child - f_parent) / min()) is approximately 0 for
    // any positive gap, so worse candidates are effectively never accepted once the floor is reached.
    // (std::max applies the floor AFTER the multiply, so even if t_ * alpha_ underflows to 0 the result
    // is min(), never 0.)
    t_ = std::max(t_ * alpha_, std::numeric_limits<double>::min());
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
