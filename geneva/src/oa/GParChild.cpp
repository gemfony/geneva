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
#include "common/GThreadPool.hpp"
#include "common/GParserBuilder.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/oa/GOptimizationAlgorithmBase.hpp"
#include "geneva/oa/GBaseParChildPersonalityTraits.hpp"
#include "geneva/oa/GAdaption.hpp"
#include "geneva/oa/GAdaptionConfig.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <future>
#include <memory>
#include <random>
#include <tuple>
#include <vector>

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief The default constructor. As we do not have any individuals yet, we set the population
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
 * @brief Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GOptimizationAlgorithmBase, expected to be a GParChild
 * @param e The expected outcome of the comparison (e.g. equality or inequality)
 * @param limit The maximum deviation for floating point values (unused here; important for similarity checks)
 */
void GParChild::compare_(
    const GOptimizationAlgorithmBase &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GParChild  reference independent of this object and convert the pointer
    const GParChild *p_load =
        Gem::Common::g_convert_and_compare<GOptimizationAlgorithmBase, GParChild>(cp, this);

    GToken token("GParChild", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GOptimizationAlgorithmBase>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    g_compare_members(localMembers(), p_load->localMembers(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * @brief Resets the settings of this population to what was configured when
 * the optimize()-call was issued
 */
void GParChild::resetToOptimizationStart_() {
    // There is nothing to reset here, so we simply call the
    // function of the parent class
    GOptimizationAlgorithmBase::resetToOptimizationStart_();
}

/******************************************************************************/
/**
 * @brief Specifies the default size of the population plus the number of parents.
 * The population will be filled with additional individuals later, as required --
 * see GParChild::adjustPopulation_() . Also, all error checking is done in
 * that function.
 *
 * @param pop_size The desired total size of the population
 * @param n_parents The desired number of parents
 */
void GParChild::setPopulationSizes(
    std::size_t pop_size,
    std::size_t n_parents
) {
    GOptimizationAlgorithmBase::setDefaultPopulationSize(pop_size);
    n_parents_ = n_parents;
    // Keep the expected number of children consistent with the (possibly newly set) sizes. This is
    // definitionally pop_size - n_parents; failing to update it here is why scheduled population
    // growth was a no-op -- performScheduledPopulationGrowth() raises pop_size/n_parents via this
    // function, but selectBest_() resizes to getNParents() + getDefaultNChildren(), which used a
    // stale default_n_children_ (set once in init()) and shrank the population straight back.
    default_n_children_ = pop_size - n_parents;
}

/******************************************************************************/
/**
 * @brief Retrieve the number of parents as set by the user. This is a fixed parameter and
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
 * @brief Calculates the current number of children from the number of parents and the
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
 * @brief Retrieves the defaultNChildren_ parameter. E.g. in GTransferPopulation::adaptChildren() ,
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
 * @brief Retrieve the number of processible items in the current iteration.
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
 * @brief Lets the user set the desired recombination method. No sanity checks for the
 * values are necessary, as we use an enum.
 *
 * @param recombination_method The desired recombination (duplication) scheme
 */
void GParChild::setRecombinationMethod(
    duplicationScheme recombination_method
) {
    recombination_method_ = recombination_method;
}

/******************************************************************************/
/**
 * @brief Retrieves the value of the recombination_method_ variable
 *
 * @return The currently configured recombination (duplication) scheme
 */
duplicationScheme GParChild::getRecombinationMethod() const {
    return recombination_method_;
}

/******************************************************************************/
/**
 * @brief Adds the option to increase the population by a given amount per iteration
 *
 * @param growth_rate The number of individuals to be added in each iteration
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
 * @brief Allows to retrieve the growth rate of the population
 *
 * @return The growth rate of the population per iteration
 */
std::size_t GParChild::getGrowthRate() const {
    return growth_rate_;
}

/******************************************************************************/
/**
 * @brief Allows to retrieve the maximum population size when growth is enabled
 *
 * @return The maximum population size allowed, when growth is enabled
 */
std::size_t GParChild::getMaxPopulationSize() const {
    return max_population_size_;
}

/******************************************************************************/
/**
 * @brief Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void GParChild::addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) {
    // Call our parent class'es function
    GOptimizationAlgorithmBase::addConfigurationOptions_(gpb);

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
 * @brief Allows to set the likelihood for amalgamation of two units to be
 * performed instead of "just" duplication.
 *
 * @param amalgamation_likelihood The likelihood for amalgamation (cross-over), must be in the range [0,1]
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
 * @brief Allows to retrieve the likelihood for amalgamation of two units to be
 * performed instead of "just" duplication.
 *
 * @return The currently configured amalgamation (cross-over) likelihood, in the range [0,1]
 */
double GParChild::getAmalgamationLikelihood() const {
    return amalgamation_likelihood_;
}

/******************************************************************************/
/**
 * @brief This function assigns a new value to each child individual according to the chosen
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

    std::bernoulli_distribution amalgamation_wanted(
        amalgamation_likelihood_
    ); // true with a likelihood of amalgamation_likelihood_

    // ------------------------------------------------------------------------
    // Parallel fast path: when no cross-over can occur (amalgamation disabled)
    // and a derived algorithm supplies a thread pool, select the parent for every
    // child sequentially first -- this reproduces both the random-number sequence
    // and the chosen recombination scheme exactly -- and then run only the heavy
    // load() deep-copies in parallel. Parents are read only and each child slot is
    // written by exactly one task, so there are no data races. load() does not copy
    // the per-individual RNG (gr_ is deliberately absent from localMembers()), so
    // children keep their own generators.
    Gem::Common::GThreadPool *tp = this->tp_ptr_.get();
    const std::size_t n_children = GOptimizationAlgorithmBase::data_cnt_.size() - n_parents_;
    if(tp != nullptr && amalgamation_likelihood_ <= 0. && n_children > 1) {
        const bool value_scheme =
            (duplicationScheme::VALUEDUPLICATIONSCHEME == recombination_method_)
            && not GOptimizationAlgorithmBase::inFirstIteration();

        // (1) Sequential parent selection -- mirrors the serial path's draws exactly.
        std::vector<std::size_t> parent_pos(n_children);
        for(std::size_t c = 0; c < n_children; ++c) {
            std::size_t pp = 0;
            if(n_parents_ > 1) {
                // The serial path flips an (always-false) cross-over coin here; flip it
                // too so the random-number stream stays identical.
                (void) amalgamation_wanted(this->gr_);
                if(value_scheme) {
                    const double rand_test = GOptimizationAlgorithmBase::uniform_real_distribution_(this->gr_);
                    pp = n_parents_ - 1; // threshold[n_parents_-1] == 1, so a match is guaranteed
                    for(std::size_t par = 0; par < n_parents_; ++par) {
                        if(rand_test < threshold[par]) {
                            pp = par;
                            break;
                        }
                    }
                }
                else {
                    pp = this->uniform_int_distribution_(
                        this->gr_,
                        std::uniform_int_distribution<std::size_t>::param_type(0, n_parents_ - 1)
                    );
                }
            }
            parent_pos[c] = pp;
        }

        // (2) Parallel deep-copy of the selected parent into each child.
        std::vector<std::future<void>> futures_cnt;
        futures_cnt.reserve(n_children);
        for(std::size_t c = 0; c < n_children; ++c) {
            const std::size_t child_idx = n_parents_ + c;
            const std::size_t pp = parent_pos[c];
            futures_cnt.push_back(tp->async_schedule([this, child_idx, pp]() {
                std::unique_ptr<gen::GIndividualSlot> &child = GOptimizationAlgorithmBase::data_cnt_[child_idx];
                child->load(GOptimizationAlgorithmBase::data_cnt_[pp]);
                child->template getPersonalityTraits<GBaseParChildPersonalityTraits>()
                    ->setParentId(pp);
            }));
        }
        tp->wait();

        // Consume futures so worker-thread exceptions are surfaced rather than dropped.
        for(auto &f : futures_cnt) {
            try {
                f.get();
            }
            catch(std::exception &e) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GParChild::doRecombine() (parallel) :" << '\n'
                    << "Got error during thread execution with message:" << '\n'
                    << e.what() << '\n'
                );
            }
            catch(...) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GParChild::doRecombine() (parallel) :" << '\n'
                    << "Got unknown exception during thread execution" << '\n'
                );
            }
        }
        return;
    }

    // ------------------------------------------------------------------------
    // Serial path (original behaviour; also covers the cross-over / amalgamation case).
    std::vector<std::unique_ptr<gen::GIndividualSlot>>::iterator it;
    for(it = GOptimizationAlgorithmBase::data_cnt_.begin() + n_parents_;
        it != GOptimizationAlgorithmBase::data_cnt_.end();
        ++it) {
        // Retrieve a random number so we can decide whether to perform cross-over or duplication
        // If we do perform cross-over, we always cross the best individual with another random parent
        if(n_parents_ > 1 &&
           amalgamation_wanted(this->gr_)) { // Create individuals using a cross-over scheme
            const gen::GOptimizableEntity &best_parent = this->front()->individual();
            const gen::GOptimizableEntity &combiner =
                (n_parents_ > 2)
                    ? (*(this->begin() + this->uniform_int_distribution_(
                                             this->gr_,
                                             std::uniform_int_distribution<std::size_t>::param_type(
                                                 1,
                                                 n_parents_ - 1
                                             )
                                         )))->individual()
                    : (*(this->begin() + 1))->individual();

            (*it)->individual().load(best_parent.crossOverWith(combiner));
        }
        else { // Just perform duplication
            switch(recombination_method_) {
            case duplicationScheme::
                DEFAULTDUPLICATIONSCHEME: // we want the RANDOMDUPLICATIONSCHEME behavior
            case duplicationScheme::RANDOMDUPLICATIONSCHEME: {
                // The recombine helpers copy the chosen parent's individual into the child slot's
                // individual and record the parent id on the child slot's personality.
                randomRecombine(*it);
            } break;

            case duplicationScheme::VALUEDUPLICATIONSCHEME: {
                if(n_parents_ == 1) {
                    // Whole slot (individual + OA adaption scratch) -- see randomRecombine().
                    (*it)->load(*(GOptimizationAlgorithmBase::data_cnt_.begin()));
                    (*it)
                        ->template getPersonalityTraits<GBaseParChildPersonalityTraits>()
                        ->setParentId(0);
                }
                else {
                    // A recombination taking into account the value does not make
                    // sense in the first iteration, as parents might not have a suitable
                    // value. Instead, this function might accidentaly trigger value
                    // calculation. Hence we fall back to random recombination in iteration 0.
                    // No value calculation takes place there.
                    if(GOptimizationAlgorithmBase::inFirstIteration()) {
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
 * @brief Gives individuals an opportunity to update their internal structures. Here
 * we just trigger an update of the adaptors. We only do so for parents, as
 * they will be replicated in the next iteration. We leave the best parent
 * untouched, so that otherwise successful adaptor settings may survive.
 */
void GParChild::actOnStalls_() {
    if(adaption_config_ && this->getNParents() > 1) {
        // Update parent individuals. We leave the best parent untouched. Phase 8: reset the per-group
        // adaption state to its seeds via the OA-owned config (the data-oriented twin of the individual's
        // updateAdaptorsOnStall()), so otherwise-successful adaptor settings are not carried into a stall.
        for(auto it = this->begin() + 1; it != this->begin() + this->getNParents(); ++it) {
            resetAdaptionState((*it)->scratch(), *adaption_config_);
        }
    }
}

/******************************************************************************/
/**
 * @brief Adapts all children in parallel, driven by the OA-owned adaption config (built at init()).
 * Identical for every mu/lambda algorithm, so it lives here rather than being duplicated in each
 * derived class. The config is read-only inside the parallel schedule, so the loop is lock-free.
 */
void GParChild::adaptChildren_() {
    // Retrieve the range of individuals to be adapted
    std::tuple<std::size_t, std::size_t> range = this->getAdaptionRange();

    // Will hold the future objects generated by the async_schedule call
    std::vector<std::future<std::size_t>> futures_cnt;

    // Loop over all requested individuals and perform the adaption
    for(auto it = (this->begin() + std::get<0>(range)); it != (this->begin() + std::get<1>(range));
        ++it) {
        futures_cnt.push_back(tp_ptr_->async_schedule(
            // Note: may not pass it as a reference, as it is a local variable in the loop and might
            // vanish or have been altered once the thread has started and adaption is requested.
            // Phase 8: drive the data-oriented adaption from the OA-owned config instead of the
            // individual's own adapt(). The config is read-only here, so the schedule stays lock-free.
            [it, cfg = adaption_config_.get()]() {
                auto &flat = dynamic_cast<gen::GFlatGenome &>((*it)->individual());
                return adaptIndividual(flat, (*it)->scratch(), *cfg);
            } // Returns the number of adaptions
        ));
    }

    // Wait for all threads in the pool to complete their work
    tp_ptr_->wait();

    // Consume futures in all build modes: after wait() they are immediately ready, so this is
    // non-blocking. Without consuming them, exceptions thrown by worker threads are silently
    // discarded when the futures are destroyed.
    for(auto &f : futures_cnt) {
        try {
            f.get();
        }
        catch(std::exception &e) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParChild::adaptChildren_() :" << '\n'
                << "Got error during thread execution with message:" << '\n'
                << e.what() << '\n'
            );
        }
        catch(...) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParChild::adaptChildren_() :" << '\n'
                << "Got unknown exception during thread execution" << '\n'
            );
        }
    }
}

/******************************************************************************/
/**
 * @brief Reconciles the population after a job submission. Identical across the mu/lambda algorithms (it was
 * duplicated verbatim in EA and SA), so it lives here. The per-algorithm parent/child personality type
 * is obtained through makePersonalityTraits() (the virtual factory), and the parent/child flags are
 * read/written through the GBaseParChildPersonalityTraits base interface -- both are uniform, so no
 * concrete-trait knowledge is needed.
 */
void GParChild::fixAfterJobSubmission() {
    const std::size_t np = this->getNParents();
    const std::uint32_t iteration = this->getIteration();

    // Retrieve any LATE returns the consumer buffered -- bare individuals that came back after their
    // batch had already been reconciled (only networked consumers produce these; local consumers return
    // an empty list). They no longer carry the OA personality (that lives on the population slot), so we
    // reconcile purely by assigned iteration: each is appended below as a fresh child candidate and the
    // subsequent selection keeps it only if it is competitive -- which makes this MO-safe without any
    // bespoke "fitness >" comparison.
    auto old_work_items = this->getOldWorkItems();

    // Admit late returns from the current OR the immediately-preceding iteration: a child evaluated in
    // iteration N typically returns during N+1, so a strict "== current iteration" test would discard
    // exactly the late returns we want to reap. Items staler than one generation are dropped. iteration
    // >= getAssignedIteration() always (no items from the future), so the subtraction cannot underflow.
    std::erase_if(old_work_items, [iteration](const auto &x) -> bool {
        return (iteration - x->getAssignedIteration()) > 1;
    });

    // Make it known to remaining old individuals that they are now part of a new iteration
    std::for_each(
        old_work_items.begin(),
        old_work_items.end(),
        [iteration](const auto &p) { p->setAssignedIteration(iteration); }
    );

    // Make sure that parents are at the beginning of the array.
    std::sort(
        this->begin(),
        this->end(),
        [](const auto &x, const auto &y) -> bool {
            return (
                x->template getPersonalityTraits<GBaseParChildPersonalityTraits>()->isParent() >
                y->template getPersonalityTraits<GBaseParChildPersonalityTraits>()->isParent()
            );
        }
    );

    // Attach all old work items to the end of the current population and clear the array of old items.
    // A late return is a BARE individual -- the personality object lives on the population slot, not on
    // the individual, so the freshly-wrapped slot starts with an empty personality. Install the correct
    // concrete one (the marking loop below, and selection, dereference it). It is tagged as a child by
    // that marking loop.
    for(auto &item_ptr : old_work_items) {
        auto slot = std::make_unique<gen::GIndividualSlot>(std::move(item_ptr));
        slot->setPersonality(this->makePersonalityTraits());
        this->push_back(std::move(slot));
    }
    old_work_items.clear();

    // Check that individuals do exist in the population. We cannot continue, if this is not the case
    if(this->empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GParChild::fixAfterJobSubmission(): Error!" << '\n'
            << "Population holds no data" << '\n'
        );
    }

    // Emit a warning if no children have returned
    if(this->size() <= np) {
        glogger << "In GParChild::fixAfterJobSubmission(): Warning!" << '\n'
                << "No child individuals have returned" << '\n'
                << "We have a size of " << this->size() << " with " << np << " parents" << '\n'
                << "We need to fill up the population with clones from parent individuals" << '\n'
                << GWARNING;
    }

    // Check that the last individual is not unprocessed. This is a severe error.
    if(this->back()->individual().is_due_for_processing()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GParChild::fixAfterJobSubmission(): Error!" << '\n'
            << "The last individual in the population is unprocessed" << '\n'
            << "so we cannot use it for cloning" << '\n'
        );
    }

    // Add missing individuals, as clones of the last item
    if(this->size() < this->getDefaultPopulationSize()) {
        const std::size_t fix_size = this->getDefaultPopulationSize() - this->size();
        for(std::size_t i = 0; i < fix_size; i++) {
            // This function will create a clone of its argument
            this->push_back_clone(this->back());
        }
    }

    // Mark the first np individuals as parents and the rest as children, so we have a sane population.
    for(auto it = this->begin(); it != this->begin() + np; ++it) {
        (*it)->template getPersonalityTraits<GBaseParChildPersonalityTraits>()->setIsParent();
    }
    for(auto it = this->begin() + np; it != this->end(); ++it) {
        (*it)->template getPersonalityTraits<GBaseParChildPersonalityTraits>()->setIsChild();
    }

    // We care for too many returned individuals in selectBest(). Older individuals might nevertheless
    // have a better quality. We do not want to lose them.
}

/******************************************************************************/
/**
 * @brief Emits a name for this class / object
 *
 * @return The string "GParChild"
 */
std::string GParChild::name_() const {
    return std::string("GParChild");
}

/******************************************************************************/
/**
 * @brief Loads the data of another GParChild object.
 *
 * @param cp A pointer to another GOptimizationAlgorithmBase, expected to be a GParChild
 */
void GParChild::load_(const GOptimizationAlgorithmBase *cp) {
    // Check that we are dealing with a GParChild  reference independent of this object and convert the pointer
    const GParChild *p_load =
        Gem::Common::g_convert_and_compare<GOptimizationAlgorithmBase, GParChild>(cp, this);

    // First load the parent class'es data ...
    GOptimizationAlgorithmBase::load_(cp);

    // ... and then our own data, derived from the single localMembers() declaration
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/******************************************************************************/
/**
 * @brief This function is called from GOptimizationAlgorithmBase::optimize() and performs the
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
 * @brief Retrieves the adaption range in a given iteration and sorting scheme.
 *
 * @return A tuple holding the half-open [start, end) range of population positions to be adapted (children only)
 */
std::tuple<std::size_t, std::size_t> GParChild::getAdaptionRange() const {
    return std::tuple<std::size_t, std::size_t>{n_parents_, this->size()};
}

/******************************************************************************/
/**
 * @brief This helper function marks the first n_parents_ individuals in the population as parents.
 */
void GParChild::markParents() {
    typename std::vector<std::unique_ptr<gen::GIndividualSlot>>::iterator it;
    for(it = GOptimizationAlgorithmBase::data_cnt_.begin();
        it != GOptimizationAlgorithmBase::data_cnt_.begin() + n_parents_;
        ++it) {
        (*it)
            ->template getPersonalityTraits<GBaseParChildPersonalityTraits>()
            ->setIsParent();
    }
}

/******************************************************************************/
/**
 * @brief This helper function marks the individuals behind the parents as children
 */
void GParChild::markChildren() {
    typename std::vector<std::unique_ptr<gen::GIndividualSlot>>::iterator it;
    for(it = GOptimizationAlgorithmBase::data_cnt_.begin() + n_parents_;
        it != GOptimizationAlgorithmBase::data_cnt_.end();
        ++it) {
        (*it)
            ->template getPersonalityTraits<GBaseParChildPersonalityTraits>()
            ->setIsChild();
    }
}

/******************************************************************************/
/**
 * @brief This helper function lets all individuals know about their position in the
 * population.
 */
void GParChild::markIndividualPositions() {
    std::size_t pos = 0;
    for(const auto &individual : GOptimizationAlgorithmBase::data_cnt_) {
        individual
            ->template getPersonalityTraits<GBaseParChildPersonalityTraits>()
            ->setPopulationPosition(pos++);
    }
}

/******************************************************************************/
/**
 * @brief This function implements the logic that constitutes evolutionary algorithms. The
 * function is called by GOptimizationAlgorithmBase for each cycle of the optimization,
 *
 * @return A tuple holding the raw and transformed primary fitness of the best individual found
 */
std::tuple<double, double> GParChild::cycleLogic_() {
    // If this is not the first iteration, check whether we need to increase the population
    if(GOptimizationAlgorithmBase::afterFirstIteration()) {
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
    if(not this->at(0)->individual().is_processed()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GParChild::cycleLogic(): Error!" << '\n'
            << "Expected clean individual in best position" << '\n'
        );
    }

#endif /* DEBUG */

    // Return the primary fitness of the best individual in the collection
    return this->at(0)->individual().getFitnessTuple();
}

/******************************************************************************/
/**
 * @brief The function checks that the population size meets the requirements and does some
 * tagging. It is called from within GOptimizationAlgorithmBase::optimize(), before the
 * actual optimization cycle starts.
 */
void GParChild::init() {
    // To be performed before any other action
    GOptimizationAlgorithmBase::init();

    // Perform some checks regarding population sizes
    populationSanityChecks_();

    // Let parents know they are parents
    markParents();
    // Let children know they are children

    // Make sure derived classes (such as GTransferPopulation) have a way of finding out
    // what the desired number of children is. This is particularly important, if, in a
    // network environment, some individuals might not return and some individuals return
    // late. The factual size of the population then changes and we need to take action.
    default_n_children_ = GOptimizationAlgorithmBase::getDefaultPopulationSize() - n_parents_;

    // Build the OA-owned adaption configuration from a representative genome (all individuals share the
    // same genome layout). It drives the data-oriented adaption free functions (Phase 8), replacing the
    // individual's own adapt(). It is transient run scratch, rebuilt on every optimize().
    adaption_config_.reset();
    if(not this->empty()) {
        if(const auto *flat = dynamic_cast<const gen::GFlatGenome *>(&this->at(0)->individual())) {
            // The genome carries only structure -- the adaptors live on an OA-owned GAdaptionConfig that
            // MUST be provided explicitly (via setAdaptionConfig(), e.g. Go2::registerAdaptionConfig() for
            // this algorithm's personality type). Adaption intent is never inferred from the genome, so an
            // adapting algorithm with no config is a hard error. The provided config is validated against
            // the population's genome, rejecting a config authored for a different genome early.
            if(not provided_adaption_config_) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GParChild::init(): Error!" << '\n'
                    << "This adapting algorithm has no adaption config. The genome carries only structure;"
                    << '\n'
                    << "author an OA-owned GAdaptionConfig and provide it via setAdaptionConfig() (or, with"
                    << '\n'
                    << "Go2, register it via go.registerAdaptionConfig(\"" << getAlgorithmPersonalityType()
                    << "\", cfg))." << '\n'
                );
            }
            provided_adaption_config_->checkConsistency(*flat);
            adaption_config_ = provided_adaption_config_;

            // Seed each slot's OA-owned scratch with the per-group adaption state from the shared
            // config. The state (sigma / ad_prob / counter, …) formerly lived on the individual's
            // auxiliary store; it now lives on the GIndividualSlot, OA-owned. Children created by
            // recombination copy their chosen parent's whole slot (scratch included), so the evolved
            // state propagates exactly as it did when it rode on the individual.
            // On a checkpoint resume the slots already carry their restored, evolved adaption state --
            // preserve it (skip the re-seed) so a resumed run keeps its sigma rather than restarting.
            if(not this->resumedFromCheckpoint()) {
                for(auto const &slot : *this) {
                    adaption_config_->installInto(slot->scratch());
                }
            }
        }
    }
}

/******************************************************************************/
/**
 * @brief Stores the externally-supplied OA-owned adaption configuration. init() adopts it -- after a
 * checkConsistency() against the population's genome -- as the algorithm's run config; an adapting
 * algorithm with no config provided is a hard error (the genome carries no adaption intent). Passing a
 * null pointer clears it.
 *
 * @param config The OA-owned adaption configuration to store (shared ownership; a null pointer clears it)
 */
void GParChild::setAdaptionConfig(std::shared_ptr<GAdaptionConfigBase> config) {
    provided_adaption_config_ = std::move(config);
}

/******************************************************************************/
/**
 * @brief Does any necessary finalization work
 */
void GParChild::finalize() {
    // Last action
    GOptimizationAlgorithmBase::finalize();
}

/******************************************************************************/
/**
 * @brief The function checks that the population size meets the requirements and resizes the
 * population to the appropriate size, if required. An obvious precondition is that at
 * least one individual has been added to the population. Individuals that have already
 * been added will not be replaced. This function is called once before the optimization
 * cycle from within GOptimizationAlgorithmBase::optimize()
 */
void GParChild::adjustPopulation_() {
    // Has the population size been set at all ?
    if(GOptimizationAlgorithmBase::getDefaultPopulationSize() == 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GParChild::adjustPopulation() :" << '\n'
            << "The population size is 0." << '\n'
            << "Did you call GOptimizationAlgorithmBase::setParentsAndPopulationSize() ?"
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
    typename std::vector<std::unique_ptr<gen::GIndividualSlot>>::iterator it;
    for(const auto &individual : GOptimizationAlgorithmBase::data_cnt_) {
        if(not individual) { // shared_ptr can be implicitly converted to bool
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParChild::adjustPopulation() :" << '\n'
                << "Found empty smart pointer." << '\n'
            );
        }
    }

    // Fill up as required. We are now sure we have a suitable number of individuals to do so
    if(this_sz < GOptimizationAlgorithmBase::getDefaultPopulationSize()) {
        this->resize_clone(
            GOptimizationAlgorithmBase::getDefaultPopulationSize(),
            GOptimizationAlgorithmBase::data_cnt_[0]
        );

        // Randomly initialize new items
        for(it = GOptimizationAlgorithmBase::data_cnt_.begin() + this_sz;
            it != GOptimizationAlgorithmBase::data_cnt_.end();
            ++it) {
            (*it)->individual().randomInit(activityMode::ACTIVEONLY);
        }
    }
}

/******************************************************************************/
/**
 * @brief Increases the population size if requested by the user. This will happen until the population size exceeds
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
            GOptimizationAlgorithmBase::getDefaultPopulationSize(),
            GOptimizationAlgorithmBase::data_cnt_[0]
        );
    }
}

/******************************************************************************/
/**
 * @brief This function implements the RANDOMDUPLICATIONSCHEME scheme: a parent is chosen at random and
 * its whole slot (individual plus OA-owned adaption scratch) is copied into the given child slot.
 *
 * @param child The child slot into which the randomly chosen parent's slot is loaded
 */
void GParChild::randomRecombine(const std::unique_ptr<gen::GIndividualSlot> &child) {
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

    // Load the chosen parent's WHOLE slot into the child slot -- the individual plus the OA-owned
    // scratch (the per-group adaption state, e.g. the evolved sigma). The scratch must ride along so a
    // child inherits its parent's adapted state, exactly as it did when that state lived on the
    // individual's auxiliary store; markChildren() (called after recombine) resets the child flag, and
    // setParentId records the chosen parent below. This mirrors the parallel path in doRecombine().
    child->load(*(GOptimizationAlgorithmBase::data_cnt_.begin() + parent_pos));
    child->template getPersonalityTraits<GBaseParChildPersonalityTraits>()->setParentId(parent_pos);
}

/******************************************************************************/
/**
 * @brief This function implements the VALUEDUPLICATIONSCHEME scheme. The range [0.,1.[ is divided
 * into nParents_ sub-areas with different size (the largest for the first parent,
 * the smallest for the last). Parents are chosen for recombination according to a
 * random number evenly distributed between 0 and 1. This way parents with higher
 * fitness are more likely to be chosen for recombination.
 *
 * @param child The child slot into which the chosen parent's slot is loaded
 * @param threshold A std::vector<double> holding the cumulative recombination likelihoods for each parent
 */
void GParChild::valueRecombine(
    const std::unique_ptr<gen::GIndividualSlot> &child,
    const std::vector<double> &threshold
) {
    bool done = false;
    double rand_test // get the test value // NOLINT(cppcoreguidelines-init-variables)
        = GOptimizationAlgorithmBase::uniform_real_distribution_(this->gr_);

    for(std::size_t par = 0; par < n_parents_; par++) {
        if(rand_test < threshold[par]) {
            // Load the chosen parent's WHOLE slot (individual + OA-owned adaption scratch) into the
            // child slot, so the child inherits the parent's evolved adaption state; setParentId records
            // the chosen parent. See randomRecombine() for the rationale.
            child->load(*(GOptimizationAlgorithmBase::data_cnt_.begin() + par));
            child->template getPersonalityTraits<GBaseParChildPersonalityTraits>()->setParentId(par);
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
 * @brief Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GParChild::modify_GUnitTests_() {
#ifdef GEM_TESTING

    bool result = false;

    // Call the parent class'es function
    if(GOptimizationAlgorithmBase::modify_GUnitTests_()) {
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
 * @brief Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GParChild::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Call the parent class'es function
    GOptimizationAlgorithmBase::specificTestsNoFailureExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GParChild::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GParChild::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Call the parent class'es function
    GOptimizationAlgorithmBase::specificTestsFailuresExpected_GUnitTests_();

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
