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

#include "geneva/oa/GParChild.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "common/GParserBuilder.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/oa/GBase.hpp"
#include "geneva/oa/GBaseParChildPersonalityTraits.hpp"
#include "geneva/par/GParameterSet.hpp"
#include <algorithm>
#include <cstddef>
#include <memory>
#include <random>
#include <tuple>
#include <vector>

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor, As we do not have any individuals yet, we set the population
 * size, and number of parents to 0. It is the philosophy of this class not
 * to provide constructors for each and every use case. Instead, you should set
 * vital parameters, such as the population size or the parent individuals by hand
 * or do so through the configuration file.
 */
GParChild::GParChild() {
    // Make sure we start with a valid population size if the user does not supply these values
    this->setPopulationSizes(
        DEFPARCHILDPOPSIZE // overall population size
        ,
        DEFPARCHILDNPARENTS // number of parents
    );
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GParChild object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GParChild::compare_(
    const GBase &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GParChild  reference independent of this object and convert the pointer
    const GParChild *p_load =
        Gem::Common::g_convert_and_compare<GBase, GParChild>(cp, this);

    GToken token("GParChild", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GBase>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    g_compare_members(localMembers(), p_load->localMembers(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Resets the settings of this population to what was configured when
 * the optimize()-call was issued
 */
void GParChild::resetToOptimizationStart_() {
    // There is nothing to reset here, so we simply call the
    // function of the parent class
    GBase::resetToOptimizationStart_();
}

/******************************************************************************/
/**
 * Specifies the default size of the population plus the number of parents.
 * The population will be filled with additional individuals later, as required --
 * see GParChildT::adjustPopulation() . Also, all error checking is done in
 * that function.
 *
 * @param pop_size The desired size of the population
 * @param n_parents The desired number of parents
 */
void GParChild::setPopulationSizes(
    std::size_t pop_size,
    std::size_t n_parents
) {
    GBase::setDefaultPopulationSize(pop_size);
    n_parents_ = n_parents;
}

/******************************************************************************/
/**
 * Retrieve the number of parents as set by the user. This is a fixed parameter and
 * should not be changed after it has first been set. Note that, if the size of the
 * population is smaller than the alleged number of parents, the function will return
 * the size of the population instead, thus interpreting its individuals as parents.
 *
 * @return The number of parents in the population
 */
std::size_t GParChild::getNParents() const {
    return (std::min)(this->size(), n_parents_);
}

/******************************************************************************/
/**
 * Calculates the current number of children from the number of parents and the
 * size of the vector.
 *
 * @return The number of children in the population
 */
std::size_t GParChild::getNChildren() const {
    if(this->size() <= n_parents_) {
        // This will happen, when only the default population size has been set,
        // but no individuals have been added yet
        return 0;
    }
            return this->size() - n_parents_;
   
}

/******************************************************************************/
/**
 * Retrieves the defaultNChildren_ parameter. E.g. in GTransferPopulation::adaptChildren() ,
 * this factor controls when a population is considered to be complete. The corresponding
 * loop which waits for new arrivals will then be stopped, which in turn allows
 * a new generation to start.
 *
 * @return The defaultNChildren_ parameter
 */
std::size_t GParChild::getDefaultNChildren() const {
    return default_n_children_;
}

/**************************************************************************/
/**
 * Retrieve the number of processible items in the current iteration.
 *
 * @return The number of processible items in the current iteration
 */
std::size_t GParChild::getNProcessableItems_() const {
    std::tuple<std::size_t, std::size_t> range = this->getEvaluationRange_();

#ifdef DEBUG
    if(std::get<1>(range) <= std::get<0>(range)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GParChild<>::getNProcessableItems(): Error!" << '\n'
            << "Upper boundary of range <= lower boundary: " << std::get<1>(range) << "/"
            << std::get<0>(range) << '\n'
        );
    }
#endif /* DEBUG */

    return std::get<1>(range) - std::get<0>(range);
}

/******************************************************************************/
/**
 * Lets the user set the desired recombination method. No sanity checks for the
 * values are necessary, as we use an enum.
 *
 * @param recombination_method The desired recombination method
 */
void GParChild::setRecombinationMethod(
    duplicationScheme recombination_method
) {
    recombination_method_ = recombination_method;
}

/******************************************************************************/
/**
 * Retrieves the value of the recombinationMethod_ variable
 *
 * @return The value of the recombinationMethod_ variable
 */
duplicationScheme GParChild::getRecombinationMethod() const {
    return recombination_method_;
}

/******************************************************************************/
/**
 * Adds the option to increase the population by a given amount per iteration
 *
 * @param growth_rate The amount of individuals to be added in each iteration
 * @param max_population_size The maximum allowed size of the population
 */
void GParChild::setPopulationGrowth(
    std::size_t growth_rate,
    std::size_t max_population_size
) {
    growth_rate_ = growth_rate;
    max_population_size_ = max_population_size;
}

/******************************************************************************/
/**
 * Allows to retrieve the growth rate of the population
 *
 * @return The growth rate of the population per iteration
 */
std::size_t GParChild::getGrowthRate() const {
    return growth_rate_;
}

/******************************************************************************/
/**
 * Allows to retrieve the maximum population size when growth is enabled
 *
 * @return The maximum population size allowed, when growth is enabled
 */
std::size_t GParChild::getMaxPopulationSize() const {
    return max_population_size_;
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void GParChild::addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) {
    // Call our parent class'es function
    GBase::addConfigurationOptions_(gpb);

    // Add local data

    gpb.registerFileParameter<double>(
        "amalgamation_likelihood" // The name of the variable
        ,
        DEFAULTAMALGAMATIONLIKELIHOOD // The default value
        ,
        [this](double al) { this->setAmalgamationLikelihood(al); }
    ) << "The likelihood for parent individuals to be \"fused\" together"
      << '\n'
      << "rather than \"just\" being created through duplication schemes";

    gpb.registerFileParameter<std::size_t, std::size_t>(
        "size" // The name of the first variable
        ,
        "n_parents" // The name of the second variable
        ,
        DEFAULTEAPOPULATIONSIZE,
        DEFAULTEANPARENTS,
        [this](std::size_t ps, std::size_t np) { this->setPopulationSizes(ps, np); },
        "population"
    ) << "The total size of the population "
      << Gem::Common::nextComment() << "The number of parents in the population";

    gpb.registerFileParameter<duplicationScheme>(
        "recombination_method" // The name of the variable
        ,
        duplicationScheme::DEFAULTDUPLICATIONSCHEME // The default value
        ,
        [this](duplicationScheme d) { this->setRecombinationMethod(d); }
    ) << "The recombination method. Options"
      << '\n'
      << "0: default" << '\n'
      << "1: random selection from available parents" << '\n'
      << "2: selection according to the parent's value";

    gpb.registerFileParameter<std::size_t, std::size_t>(
        "growth_rate" // The name of the variable
        ,
        "max_population_size" // The name of the variable
        ,
        0 // The default value of the first variable
        ,
        0 // The default value of the second variable
        ,
        [this](std::size_t gr, std::size_t ms) { this->setPopulationGrowth(gr, ms); },
        "population_growth"
    ) << "Specifies the number of individuals added per iteration"
      << Gem::Common::nextComment()
      << "Specifies the maximum amount of individuals in the population" << '\n'
      << "if growth is enabled";
}

/******************************************************************************/
/**
 * Allows to set the likelihood for amalgamation of two units to be
 * performed instead of "just" duplication.
 */
void GParChild::setAmalgamationLikelihood(double amalgamation_likelihood) {
    if(amalgamation_likelihood < 0. || amalgamation_likelihood > 1.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In setCrossOverLikelihood(" << amalgamation_likelihood << "): Error!" << '\n'
            << "Received invalid likelihood for amalgamation. Must be in the range [0:1]." << '\n'
        );
    }

    amalgamation_likelihood_ = amalgamation_likelihood;
}

/******************************************************************************/
/**
 * Allows to retrieve the likelihood for amalgamation of two units to be
 * performed instead of "just" duplication.
 */
double GParChild::getAmalgamationLikelihood() const {
    return amalgamation_likelihood_;
}

/******************************************************************************/
/**
 * This function assigns a new value to each child individual according to the chosen
 * recombination scheme.
 */
void GParChild::doRecombine() {
    std::size_t i = 0;
    std::vector<double> threshold(n_parents_);
    double threshold_sum = 0.;
    // Calculate a weight vector
    // TODO: Check whether it is sufficient to do this only once
    if(duplicationScheme::VALUEDUPLICATIONSCHEME == recombination_method_ && n_parents_ > 1) {
        for(i = 0; i < n_parents_; i++) {
            threshold_sum += 1. / (static_cast<double>(i) + 2.);
        }
        for(i = 0; i < n_parents_ - 1; i++) {
            // Normalizing the sum to 1
            threshold[i] = (1. / (static_cast<double>(i) + 2.)) / threshold_sum;

            // Make sure the subsequent range is in the right position
            if(i > 0) {
                threshold[i] += threshold[i - 1];
            }
        }
        threshold[n_parents_ - 1] = 1.; // Necessary due to rounding errors
    }

    std::vector<std::shared_ptr<gpar::GParameterSet>>::iterator it;
    std::bernoulli_distribution amalgamation_wanted(
        amalgamation_likelihood_
    ); // true with a likelihood of amalgamation_likelihood_
    for(it = GBase::data_cnt_.begin() + n_parents_;
        it != GBase::data_cnt_.end();
        ++it) {
        // Retrieve a random number so we can decide whether to perform cross-over or duplication
        // If we do perform cross-over, we always cross the best individual with another random parent
        if(n_parents_ > 1 &&
           amalgamation_wanted(this->gr_)) { // Create individuals using a cross-over scheme
            std::shared_ptr<gpar::GParameterSet> best_parent = this->front();
            std::shared_ptr<gpar::GParameterSet> combiner =
                (n_parents_ > 2)
                    ? (*(this->begin() + this->uniform_int_distribution_(
                                             this->gr_,
                                             std::uniform_int_distribution<std::size_t>::param_type(
                                                 1,
                                                 n_parents_ - 1
                                             )
                                         )))
                    : (*(this->begin() + 1));

            (*it)->load(best_parent->crossOverWith(combiner));
        }
        else { // Just perform duplication
            switch(recombination_method_) {
            case duplicationScheme::
                DEFAULTDUPLICATIONSCHEME: // we want the RANDOMDUPLICATIONSCHEME behavior
            case duplicationScheme::RANDOMDUPLICATIONSCHEME: {
                randomRecombine(*it);
            } break;

            case duplicationScheme::VALUEDUPLICATIONSCHEME: {
                if(n_parents_ == 1) {
                    (*it)->load(*(GBase::data_cnt_.begin()));
                    (*it)
                        ->GParameterSet::getPersonalityTraits<GBaseParChildPersonalityTraits>()
                        ->setParentId(0);
                }
                else {
                    // A recombination taking into account the value does not make
                    // sense in the first iteration, as parents might not have a suitable
                    // value. Instead, this function might accidentaly trigger value
                    // calculation. Hence we fall back to random recombination in iteration 0.
                    // No value calculation takes place there.
                    if(GBase::inFirstIteration()) {
                        randomRecombine(*it);
                    }
                    else {
                        valueRecombine(*it, threshold);
                    }
                }
            } break;
            }
        }
    }
}

/******************************************************************************/
/**
 * Gives individuals an opportunity to update their internal structures. Here
 * we just trigger an update of the adaptors. We only do so for parents, as
 * they will be replicated in the next iteration. We leave the best parent
 * untouched, so that otherwise successful adaptor settings may survive.
 */
void GParChild::actOnStalls_() {
    if(this->getNParents() > 1) {
        // Update parent individuals. We leave the best parent untouched
        for(auto it = this->begin() + 1; it != this->begin() + this->getNParents(); ++it) {
            (*it)->updateAdaptorsOnStall(this->getStallCounter());
        }
    }
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GParChild::name_() const {
    return std::string("GParChild");
}

/******************************************************************************/
/**
 * Loads the data of another GParChildT object.
 *
 * @param cp A pointer to another GParChildT object
 */
void GParChild::load_(const GBase *cp) {
    // Check that we are dealing with a GParChild  reference independent of this object and convert the pointer
    const GParChild *p_load =
        Gem::Common::g_convert_and_compare<GBase, GParChild>(cp, this);

    // First load the parent class'es data ...
    GBase::load_(cp);

    // ... and then our own data, derived from the single localMembers() declaration
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/******************************************************************************/
/**
 * This function is called from GBase::optimize() and performs the
 * actual recombination, based on the recombination schemes defined by the user.
 *
 * Note that, in DEBUG mode, this implementation will enforce a minimum number of children,
 * as implied by the initial sizes of the population and the number of parents
 * present. If individuals can get lost in your setting, you must add mechanisms
 * to "repair" the population.
 */
void GParChild::recombine() {
#ifdef DEBUG
    // We require at this stage that at least the default number of
    // children is present. If individuals can get lost in your setting,
    // you must add mechanisms to "repair" the population.
    if((this->size() - n_parents_) < default_n_children_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GParChild::recombine():" << '\n'
            << "Too few children. Got " << this->size() - n_parents_ << "," << '\n'
            << "but was expecting at least " << default_n_children_ << '\n'
        );
    }
#endif

    // Do the actual recombination
    doRecombine();

    // Let children know they are children
    markChildren();

    // Tell individuals about their ids
    markIndividualPositions();
}

/******************************************************************************/
/**
 * Retrieves the adaption range in a given iteration and sorting scheme.
 *
 * @return The range inside which adaption should take place
 */
std::tuple<std::size_t, std::size_t> GParChild::getAdaptionRange() const {
    return std::tuple<std::size_t, std::size_t>{n_parents_, this->size()};
}

/******************************************************************************/
/**
 * This helper function marks parents as parents and children as children.
 */
void GParChild::markParents() {
    typename std::vector<std::shared_ptr<gpar::GParameterSet>>::iterator it;
    for(it = GBase::data_cnt_.begin();
        it != GBase::data_cnt_.begin() + n_parents_;
        ++it) {
        (*it)
            ->GParameterSet::template getPersonalityTraits<GBaseParChildPersonalityTraits>()
            ->setIsParent();
    }
}

/******************************************************************************/
/**
 * This helper function marks children as children
 */
void GParChild::markChildren() {
    typename std::vector<std::shared_ptr<gpar::GParameterSet>>::iterator it;
    for(it = GBase::data_cnt_.begin() + n_parents_;
        it != GBase::data_cnt_.end();
        ++it) {
        (*it)
            ->GParameterSet::template getPersonalityTraits<GBaseParChildPersonalityTraits>()
            ->setIsChild();
    }
}

/******************************************************************************/
/**
 * This helper function lets all individuals know about their position in the
 * population.
 */
void GParChild::markIndividualPositions() {
    std::size_t pos = 0;
    typename std::vector<std::shared_ptr<gpar::GParameterSet>>::iterator it;
    for(it = GBase::data_cnt_.begin();
        it != GBase::data_cnt_.end();
        ++it) {
        (*it)
            ->GParameterSet::template getPersonalityTraits<GBaseParChildPersonalityTraits>()
            ->setPopulationPosition(pos++);
    }
}

/******************************************************************************/
/**
 * This function implements the logic that constitutes evolutionary algorithms. The
 * function is called by GBase for each cycle of the optimization,
 *
 * @return The value of the best individual found
 */
std::tuple<double, double> GParChild::cycleLogic_() {
    // If this is not the first iteration, check whether we need to increase the population
    if(GBase::afterFirstIteration()) {
        performScheduledPopulationGrowth();
    }

    // create new children from parents
    recombine();

    // adapt children
    adaptChildren_();

    // calculate the children's (and possibly their parents' values)
    runFitnessCalculation_();

    // find out the best individuals of the population
    selectBest_();

#ifdef DEBUG
    // The dirty flag of this individual shouldn't be set
    if(not this->at(0)->is_processed()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GParChild::cycleLogic(): Error!" << '\n'
            << "Expected clean individual in best position" << '\n'
        );
    }

#endif /* DEBUG */

    // Return the primary fitness of the best individual in the collection
    return this->at(0)->getFitnessTuple();
}

/******************************************************************************/
/**
 * The function checks that the population size meets the requirements and does some
 * tagging. It is called from within GBase::optimize(), before the
 * actual optimization cycle starts.
 */
void GParChild::init() {
    // To be performed before any other action
    GBase::init();

    // Perform some checks regarding population sizes
    populationSanityChecks_();

    // Let parents know they are parents
    markParents();
    // Let children know they are children

    // Make sure derived classes (such as GTransferPopulation) have a way of finding out
    // what the desired number of children is. This is particularly important, if, in a
    // network environment, some individuals might not return and some individuals return
    // late. The factual size of the population then changes and we need to take action.
    default_n_children_ = GBase::getDefaultPopulationSize() - n_parents_;
}

/******************************************************************************/
/**
 * Does any necessary finalization work
 */
void GParChild::finalize() {
    // Last action
    GBase::finalize();
}

/******************************************************************************/
/**
 * The function checks that the population size meets the requirements and resizes the
 * population to the appropriate size, if required. An obvious precondition is that at
 * least one individual has been added to the population. Individuals that have already
 * been added will not be replaced. This function is called once before the optimization
 * cycle from within GBase::optimize()
 */
void GParChild::adjustPopulation_() {
    // Has the population size been set at all ?
    if(GBase::getDefaultPopulationSize() == 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GParChild::adjustPopulation() :" << '\n'
            << "The population size is 0." << '\n'
            << "Did you call GBase::setParentsAndPopulationSize() ?"
            << '\n'
        );
    }

    // Check how many individuals have been added already. At least one is required.
    std::size_t this_sz = this->size();
    if(this_sz == 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GParChild::adjustPopulation() :" << '\n'
            << "size of population is 0. Did you add any individuals?" << '\n'
            << "We need at least one local individual" << '\n'
        );
    }

    // Do the smart pointers actually point to any objects ?
    typename std::vector<std::shared_ptr<gpar::GParameterSet>>::iterator it;
    for(it = GBase::data_cnt_.begin();
        it != GBase::data_cnt_.end();
        ++it) {
        if(not(*it)) { // shared_ptr can be implicitly converted to bool
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParChild::adjustPopulation() :" << '\n'
                << "Found empty smart pointer." << '\n'
            );
        }
    }

    // Fill up as required. We are now sure we have a suitable number of individuals to do so
    if(this_sz < GBase::getDefaultPopulationSize()) {
        this->resize_clone(
            GBase::getDefaultPopulationSize(),
            GBase::data_cnt_[0]
        );

        // Randomly initialize new items
        for(it = GBase::data_cnt_.begin() + this_sz;
            it != GBase::data_cnt_.end();
            ++it) {
            (*it)->randomInit(activityMode::ACTIVEONLY);
        }
    }
}

/******************************************************************************/
/**
 * Increases the population size if requested by the user. This will happen until the population size exceeds
 * a predefined value, set with setPopulationGrowth() .
 */
void GParChild::performScheduledPopulationGrowth() {
    if(growth_rate_ != 0 &&
       (this->getDefaultPopulationSize() + growth_rate_ <= max_population_size_) &&
       (this->size() < max_population_size_)) {
        // Set a new default population size
        this->setPopulationSizes(
            this->getDefaultPopulationSize() + growth_rate_,
            this->getNParents()
        );

        // Add missing items as copies of the last individual in the list
        this->resize_clone(
            GBase::getDefaultPopulationSize(),
            GBase::data_cnt_[0]
        );
    }
}

/******************************************************************************/
/**
 * This function implements the RANDOMDUPLICATIONSCHEME scheme. This functions uses BOOST's
 * numeric_cast function for safe conversion between std::size_t and uint16_t.
 *
 * @param child The individual for which a new value should be chosen
 */
void GParChild::randomRecombine(std::shared_ptr<gpar::GParameterSet> &child) {
    std::size_t parent_pos = 0;

    if(n_parents_ == 1) {
        parent_pos = 0;
    }
    else {
        // Choose a parent to be used for the recombination. Note that
        // numeric_cast may throw. Exceptions need to be caught in surrounding functions.
        // try/catch blocks would add a non-negligible overhead in this function. uniform_int(max)
        // returns integer values in the range [0,max]. As we want to have values in the range
        // 0,1, ... n_parents_-1, we need to subtract one from the argument.
        parent_pos = uniform_int_distribution_(
            this->gr_,
            std::uniform_int_distribution<std::size_t>::param_type(0, n_parents_ - 1)
        );
    }

    // Load the parent data into the individual
    child->load(*(GBase::data_cnt_.begin() + parent_pos));

    // Let the individual know the id of the parent
    child->GParameterSet::template getPersonalityTraits<GBaseParChildPersonalityTraits>()
        ->setParentId(parent_pos);
}

/******************************************************************************/
/**
 * This function implements the VALUEDUPLICATIONSCHEME scheme. The range [0.,1.[ is divided
 * into nParents_ sub-areas with different size (the largest for the first parent,
 * the smallest for the last). Parents are chosen for recombination according to a
 * random number evenly distributed between 0 and 1. This way parents with higher
 * fitness are more likely to be chosen for recombination.
 *
 * @param p The child individual for which a parent should be chosen
 * @param threshold A std::vector<double> holding the recombination likelihoods for each parent
 */
void GParChild::valueRecombine(
    std::shared_ptr<gpar::GParameterSet> &p,
    const std::vector<double> &threshold
) {
    bool done = false;
    double rand_test // get the test value // NOLINT(cppcoreguidelines-init-variables)
        = GBase::uniform_real_distribution_(this->gr_);

    for(std::size_t par = 0; par < n_parents_; par++) {
        if(rand_test < threshold[par]) {
            // Load the parent's data
            p->load(*(GBase::data_cnt_.begin() + par));
            // Let the individual know the parent's id
            p->GParameterSet::template getPersonalityTraits<GBaseParChildPersonalityTraits>()
                ->setParentId(par);
            done = true;

            break;
        }
    }

    if(not done) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GParChild::valueRecombine():" << '\n'
            << "Could not recombine." << '\n'
        );
    }
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GParChild::modify_GUnitTests_() {
#ifdef GEM_TESTING

    bool result = false;

    // Call the parent class'es function
    if(GBase::modify_GUnitTests_()) {
        result = true;
    }

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GParChild::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GParChild::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Call the parent class'es function
    GBase::specificTestsNoFailureExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GParChild::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GParChild::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Call the parent class'es function
    GBase::specificTestsFailuresExpected_GUnitTests_();

#else  /* GEM_TESTING */
    Gem::Common::condnotset(
        "GParChild::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
