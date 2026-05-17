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

/******************************************************************************/

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::OptimizationAlgorithms::GSimulatedAnnealing) // NOLINT

/******************************************************************************/

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

/**
 * The default constructor. All initialization work of member variable
 * is done in the class body.
 */
GSimulatedAnnealing::GSimulatedAnnealing() {
    // Make sure we start with a valid population size if the user does not supply these values
    this->setPopulationSizes(100, 1);
}

/******************************************************************************/
/**
  * Searches for compliance with expectations with respect to another object
  * of the same type
  *
  * @param cp A constant reference to another GObject object
  * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GSimulatedAnnealing::compare_(
    const GObject &cp // the other object
    ,
    const Gem::Common::expectation &e // the expectation for this object, e.g. equality
    ,
    const double & /*limit*/ // the limit for allowed deviations of floating point types
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GSimulatedAnnealing reference independent of this object and convert the pointer
    const GSimulatedAnnealing *p_load =
        Gem::Common::g_convert_and_compare<GObject, GSimulatedAnnealing>(cp, this);

    GToken token("GSimulatedAnnealing", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GParChild>(*this, *p_load, token);

    // ... and then the local data
    compare_t(IDENTITY(t0_, p_load->t0_), token);
    compare_t(IDENTITY(t_, p_load->t_), token);
    compare_t(IDENTITY(alpha_, p_load->alpha_), token);
    compare_t(IDENTITY(n_threads_, p_load->n_threads_), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Resets the settings of this population to what was configured when
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
 * Returns information about the type of optimization algorithm. This function needs
 * to be overloaded by the actual algorithms to return the correct type.
 *
 * @return The type of optimization algorithm
 */
std::string GSimulatedAnnealing::getAlgorithmPersonalityType_() const {
    return std::string("PERSONALITY_SA");
}

/******************************************************************************/
/**
  * Returns the name of this optimization algorithm
  *
  * @return The name assigned to this optimization algorithm
  */
std::string GSimulatedAnnealing::getAlgorithmName_() const {
    return std::string("Simulated Annealing");
}

/******************************************************************************/
/**
  * Adds local configuration options to a GParserBuilder object
  *
  * @param gpb The GParserBuilder object to which configuration options should be added
  */
void GSimulatedAnnealing::addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) {
    // Call our parent class'es function
    GParChild::addConfigurationOptions_(gpb);

    // Add local data
    gpb.registerFileParameter<std::uint16_t>(
        "nAdaptionThreads" // The name of the variable
        ,
        DEFAULTNSTDTHREADS // The default value
        ,
        [this](std::uint16_t nt) { this->setNThreads(nt); }
    ) << "The number of threads used to simultaneously adapt individuals"
      << '\n'
      << "0 means \"automatic\"";

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
 * Sets the number of threads this population uses for adaption. If n_threads is set
 * to 0, an attempt will be made to set the number of threads to the number of hardware
 * threading units (e.g. number of cores or hyperthreading units).
  *
  * @param n_threads The number of threads this class uses
  */
void GSimulatedAnnealing::setNThreads(std::uint16_t n_threads) {
    if(n_threads == 0) {
        glogger << "In GSimulatedAnnealing::setNThreads(n_threads):" << '\n'
                << "n_threads == 0 was requested. n_threads_ was reset to the default "
                << DEFAULTNSTDTHREADS << '\n'
                << GWARNING;

        n_threads_ = DEFAULTNSTDTHREADS;
    }
    else {
        n_threads_ = n_threads;
    }
}

/******************************************************************************/
/**
  * Retrieves the number of threads this population uses for adaption
  *
  * @return The maximum number of allowed threads
  */
std::uint16_t GSimulatedAnnealing::getNThreads() const {
    return n_threads_;
}

/******************************************************************************/
/**
  * Determines the strength of the temperature degradation. This function is used for simulated annealing.
  *
  * @param alpha The degradation speed of the temperature
  */
void GSimulatedAnnealing::setTDegradationStrength(double alpha) {
    if(alpha <= 0. || alpha >= 1.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GSimulatedAnnealing::setTDegradationStrength(const double&):" << '\n'
            << "alpha must be in (0,1) for SA cooling; got: " << alpha << '\n'
        );
    }

    alpha_ = alpha;
}

/******************************************************************************/
/**
  * Retrieves the temperature degradation strength. This function is used for simulated annealing.
  *
  * @return The temperature degradation strength
  */
double GSimulatedAnnealing::getTDegradationStrength() const {
    return alpha_;
}

/******************************************************************************/
/**
  * Sets the start temperature. This function is used for simulated annealing.
  *
  * @param t0 The start temperature
  */
void GSimulatedAnnealing::setT0(double t0) {
    if(t0 <= 0.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GSimulatedAnnealing::setT0(const double&):" << '\n'
            << "Got negative start temperature: " << t0 << '\n'
        );
    }

    t0_ = t0;
}

/******************************************************************************/
/**
  * Retrieves the start temperature. This function is used for simulated annealing.
  *
  * @return The start temperature
  */
double GSimulatedAnnealing::getT0() const {
    return t0_;
}

/******************************************************************************/
/**
  * Retrieves the current temperature. This function is used for simulated annealing.
  *
  * @return The current temperature
  */
double GSimulatedAnnealing::getT() const {
    return t_;
}

/******************************************************************************/
/**
  * Emits a name for this class / object
  */
std::string GSimulatedAnnealing::name_() const {
    return std::string("GSimulatedAnnealing");
}

/******************************************************************************/
/**
  * Loads the data of another GSimulatedAnnealingT object, camouflaged as a GObject.
  *
  * @param cp A pointer to another GSimulatedAnnealingT object, camouflaged as a GObject
  */
void GSimulatedAnnealing::load_(const GObject *cp) {
    // Check that we are dealing with a GSimulatedAnnealing reference independent
    // of this object and convert the pointer
    const GSimulatedAnnealing *p_load =
        Gem::Common::g_convert_and_compare<GObject, GSimulatedAnnealing>(cp, this);

    // First load the parent class'es data ...
    GParChild::load_(cp);

    // ... and then our own data
    t0_ = p_load->t0_;
    t_ = p_load->t_;
    alpha_ = p_load->alpha_;
    n_threads_ = p_load->n_threads_;
}

/******************************************************************************/
/**
  * Creates a deep copy of this object
  *
  * @return A deep copy of this object
  */
GObject *GSimulatedAnnealing::clone_() const {
    return new GSimulatedAnnealing(*this);
}

/******************************************************************************/
/**
  * Some error checks related to population sizes
  */
void GSimulatedAnnealing::populationSanityChecks_() const {
    // First check that we have been given a suitable value for the number of parents.
    // Note that a number of checks (e.g. population size != 0) has already been done
    // in the parent class.
    if(this->n_parents_ == 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GSimulatedAnnealing::populationSanityChecks(): Error!" << '\n'
            << "Number of parents is set to 0"
        );
    }

    // We need at least as many children as parents
    std::size_t pop_size = this->getPopulationSize();
    if(pop_size <= this->n_parents_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GSimulatedAnnealing::populationSanityChecks() :" << '\n'
            << "Requested size of population is too small :" << pop_size << " " << this->n_parents_
            << '\n'
        );
    }
}

/******************************************************************************/
/**
  * Adapt all children in parallel. Evaluation is done in a separate function (runFitnessCalculation).
  */
void GSimulatedAnnealing::adaptChildren_() {
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
            [it]() { return (*it)->adapt(); } // Returns the number of adaptions
        ));
    }

    // Wait for all threads in the pool to complete their work
    tp_ptr_->wait();

    // Consume futures in all build modes: after wait() they are immediately ready,
    // so this is non-blocking. Without consuming them, exceptions thrown by worker
    // threads are silently discarded when the futures are destroyed.
    for(auto &f : futures_cnt) {
        try {
            f.get();
        }
        catch(std::exception &e) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GSimulatedAnnealing::adaptChildren_() :" << '\n'
                << "Got error during thread execution with message:" << '\n'
                << e.what() << '\n'
            );
        }
        catch(...) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GSimulatedAnnealing::adaptChildren_() :" << '\n'
                << "Got unknown exception during thread execution" << '\n'
            );
        }
    }
}

/******************************************************************************/
/**
  * We submit individuals to the broker connector and wait for processed items.
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
        if(not this->at(i)->is_due_for_processing()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GSimulatedAnnealing::runFitnessCalculation(): Error!" << '\n'
                << "Tried to evaluate children in range " << std::get<0>(range) << " - "
                << std::get<1>(range) << '\n'
                << "but found \"clean\" individual in position " << i << '\n'
            );
        }
    }
#endif

    //--------------------------------------------------------------------------------
    // Set the "DO_PROCESS" flag in all required work items, the "DO_IGNORE" flag in all others.

    setProcessingFlag(this->data_cnt_, range);

    //--------------------------------------------------------------------------------
    // Now submit work items and wait for results.
    auto status = this->workOn(
        this->data_cnt_,
        false // do not resubmit unprocessed items
        ,
        "GSimulatedAnnealing::runFitnessCalculation()"
    );

    //--------------------------------------------------------------------------------
    // Take care of unprocessed items, if these exist. We simply remove them and continue.
    if(not status.is_complete) {
        std::size_t n_erased =
            std::erase_if(this->data_cnt_, [this](std::shared_ptr<gpar::GParameterSet> p) -> bool {
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
            std::erase_if(this->data_cnt_, [this](std::shared_ptr<gpar::GParameterSet> p) -> bool {
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
 * Fixes the population after a job submission
 */
void GSimulatedAnnealing::fixAfterJobSubmission() {
    std::size_t np = this->getNParents();
    std::uint32_t iteration = this->getIteration();

    // Retrieve a vector of old work items
    auto old_work_items = this->getOldWorkItems();

    // Remove parents from older iterations from old work items -- we do not want them.
    std::erase_if(old_work_items, [iteration](std::shared_ptr<gpar::GParameterSet> x) -> bool {
        return x->getPersonalityTraits<GSimulatedAnnealing_PersonalityTraits>()->isParent() &&
               x->getAssignedIteration() != iteration;
    });

    // Make it known to remaining old individuals that they are now part of a new iteration
    std::for_each(
        old_work_items.begin(),
        old_work_items.end(),
        [iteration](std::shared_ptr<gpar::GParameterSet> p) { p->setAssignedIteration(iteration); }
    );

    // Make sure that parents are at the beginning of the array.
    sort(
        this->begin(),
        this->end(),
        [](std::shared_ptr<gpar::GParameterSet> x, std::shared_ptr<gpar::GParameterSet> y) -> bool {
            return (
                x->getPersonalityTraits<GSimulatedAnnealing_PersonalityTraits>()->isParent() >
                y->getPersonalityTraits<GSimulatedAnnealing_PersonalityTraits>()->isParent()
            );
        }
    );

    // Attach all old work items to the end of the current population and clear the array of old items
    for(const auto &item_ptr : old_work_items) {
        this->push_back(item_ptr);
    }
    old_work_items.clear();

    // Check that individuals do exist in the population. We cannot continue, if this is not the case
    if(this->empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GSimulatedAnnealing::fixAfterJobSubmission(): Error!" << '\n'
            << "Population holds no data" << '\n'
        );
    }
            // Emit a warning if no children have returned
        if(this->size() <= this->getNParents()) {
            glogger << "In GSimulatedAnnealing::fixAfterJobSubmission(): Warning!" << '\n'
                    << "No child individuals have returned" << '\n'
                    << "We need to fill up the population with clones from parent individuals"
                    << '\n'
                    << GWARNING;
        }
   

    // Check that the last individual is not unprocessed. This is a severe error.
    if(this->back()->is_due_for_processing()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GSimulatedAnnealing::fixAfterJobSubmission():" << '\n'
            << "The last individual in the population is is unprocessed" << '\n'
            << "so we cannot use it for cloning" << '\n'
        );
    }

    // Add missing individuals, as clones of the last item
    if(this->size() < this->getDefaultPopulationSize()) {
        std::size_t fix_size = this->getDefaultPopulationSize() - this->size();
        for(std::size_t i = 0; i < fix_size; i++) {
            // This function will create a clone of its argument
            this->push_back_clone(this->back());
        }
    }

    // Mark the first this->n_parents_ individuals as parents and the rest of the individuals as children.
    // We want to have a sane population.
    typename GBase::iterator it;
    for(it = this->begin(); it != this->begin() + np; ++it) {
        (*it)
            ->GParameterSet::template getPersonalityTraits<GSimulatedAnnealing_PersonalityTraits>()
            ->setIsParent();
    }
    for(it = this->begin() + np; it != this->end(); ++it) {
        (*it)
            ->GParameterSet::template getPersonalityTraits<GSimulatedAnnealing_PersonalityTraits>()
            ->setIsChild();
    }

    // We care for too many returned individuals in the selectBest() function. Older
    // individuals might nevertheless have a better quality. We do not want to loose them.
}

/******************************************************************************/
/**
  * Choose new parents, based on the SA selection scheme.
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
            g_error_streamer(DO_LOG, time_and_place)
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
  * Retrieves the evaluation range in a given iteration and sorting scheme. Depending on the
  * iteration and sorting scheme, the start point will be different. The end-point is not meant
  * to be inclusive.
  *
  * @return The range inside which evaluation should take place
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
 * Does any necessary initialization work
  */
void GSimulatedAnnealing::init() {
    // To be performed before any other action. Place any further work after this call.
    GParChild::init();

    // Initialize our thread pool
    tp_ptr_ = std::make_shared<Gem::Common::GThreadPool>(n_threads_);
}

/******************************************************************************/
/**
 * Does any necessary finalization work
 */
void GSimulatedAnnealing::finalize() {
    // Terminate our thread pool
    tp_ptr_.reset();

    // Last action. Place any "local" finalization action before this call.
    GParChild::finalize();
}

/******************************************************************************/
/**
  * Retrieve a GPersonalityTraits object belonging to this algorithm
  */
std::shared_ptr<GPersonalityTraits> GSimulatedAnnealing::getPersonalityTraits_() const {
    return std::make_shared<GSimulatedAnnealing_PersonalityTraits>();
}

/******************************************************************************/
/**
 * Performs a simulated annealing style sorting and selection
 */
void GSimulatedAnnealing::sortSAMode() {
    // Position the n_parents best children of the population right behind the parents
    std::partial_sort(
        this->begin() + this->n_parents_,
        this->begin() + 2 * this->n_parents_,
        this->end(),
        [](std::shared_ptr<gpar::GParameterSet> x_ptr, std::shared_ptr<gpar::GParameterSet> y_ptr) -> bool {
            return minOnly_transformed_fitness(x_ptr) < minOnly_transformed_fitness(y_ptr);
        }
    );

    // Check for each parent whether it should be replaced by the corresponding child
    for(std::size_t np = 0; np < this->n_parents_; np++) {
        double p_pass = saProb(
            minOnly_transformed_fitness(this->at(np)),
            minOnly_transformed_fitness(this->at(this->n_parents_ + np))
        );
        if(p_pass >= 1.) {
            this->at(np)->GObject::load(this->at(this->n_parents_ + np));
        }
        else {
            double challenge = this->uniform_real_distribution_(
                this->gr_,
                std::uniform_real_distribution<double>::param_type(0., 1.)
            );
            if(challenge < p_pass) {
                this->at(np)->GObject::load(this->at(this->n_parents_ + np));
            }
        }
    }

    // Sort the new parents -- it is possible that a child with a worse fitness has replaced a parent
    std::sort(
        this->begin(),
        this->begin() + this->n_parents_,
        [](std::shared_ptr<gpar::GParameterSet> x_ptr, std::shared_ptr<gpar::GParameterSet> y_ptr) -> bool {
            return minOnly_transformed_fitness(x_ptr) < minOnly_transformed_fitness(y_ptr);
        }
    );

    // Make sure the temperature gets updated
    updateTemperature();
}

/******************************************************************************/
/**
  * Calculates the simulated annealing probability for a child to replace a parent.
  * Note that this function only sees minimization problems, as maximization problems
  * are transformed to minimization problems inside of GParameterSet.
  *
  * @param f_min_only_parent The "min only" fitness of the parent
  * @param f_min_only_child The "min only" fitness of the child
  * @return A double value in the range [0,1[, representing the likelihood for the child to replace the parent
  */
double
GSimulatedAnnealing::saProb(const double &f_min_only_parent, const double &f_min_only_child) {
    return exp(-(f_min_only_child - f_min_only_parent) / t_);
}

/******************************************************************************/
/**
  * Updates the temperature. This function is used for simulated annealing.
  */
void GSimulatedAnnealing::updateTemperature() {
    // Clamp to the smallest normalised double so t_ never enters the subnormal
    // range or reaches 0.  With t_ == 0 and Δf == 0, saProb() would compute
    // 0/0 = NaN; the floor prevents that without changing late-phase behaviour
    // (exp(-Δf / min()) ≈ 0 for any positive Δf, so worse candidates are
    // never accepted once the temperature hits the floor).
    t_ = std::max(t_ * alpha_, std::numeric_limits<double>::min());
}

/******************************************************************************/
/**
  * Applies modifications to this object. This is needed for testing purposes
  *
  * @return A boolean which indicates whether modifications were made
  */
bool GSimulatedAnnealing::modify_GUnitTests_() {
#ifdef GEM_TESTING

    bool result = false;

    // Call the parent class'es function
    if(GParChild::modify_GUnitTests_()) {
        result = true;
    }

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GSimulatedAnnealing::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
  * Performs self tests that are expected to succeed. This is needed for testing purposes
  */
void GSimulatedAnnealing::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Call the parent class'es function
    GParChild::specificTestsNoFailureExpected_GUnitTests_();

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GSimulatedAnnealing::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
  * Performs self tests that are expected to fail. This is needed for testing purposes
  */
void GSimulatedAnnealing::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Call the parent class'es function
    GParChild::specificTestsFailuresExpected_GUnitTests_();

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

#else  /* GEM_TESTING */
    Gem::Common::condnotset(
        "GSimulatedAnnealing::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
