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

#include "geneva/G_OptimizationAlgorithm_GradientDescent.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GGradientDescent) // NOLINT

namespace Gem::Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GGradientDescent::GGradientDescent()
  : GGradientDescent(DEFAULTGDSTARTINGPOINTS, DEFAULTFINITESTEP, DEFAULTSTEPSIZE) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with the number of starting points and other parameters
 *
 * @param n_starting_points The number of simultaneous starting points for the gradient descent
 * @param finite_step The desired size of the incremental adaption process
 * @param step_size The size of the multiplicative factor of the adaption process
 */
GGradientDescent::GGradientDescent(
    const std::size_t &n_starting_points,
    const double &finite_step,
    const double &step_size
)
  : G_OptimizationAlgorithm_Base()
  , nStartingPoints_(n_starting_points)
  , finiteStep_(finite_step)
  , stepSize_(step_size) { /* nothing */
}

/******************************************************************************/
/**
 * Retrieves the number of starting points of the algorithm
 *
 * @return The number of simultaneous starting points of the gradient descent
 */
std::size_t GGradientDescent::getNStartingPoints() const {
    return nStartingPoints_;
}

/******************************************************************************/
/**
 * Allows to set the number of starting points for the gradient descent
 *
 * @param n_starting_points The desired number of starting points for the gradient descent
 */
void GGradientDescent::setNStartingPoints(std::size_t n_starting_points) {
    // Do some error checking
    if(n_starting_points == 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GGradientDescent::setNStartingPoints(const std::size_t&):" << '\n'
            << "Got invalid number of starting points." << '\n'
        );
    }

    nStartingPoints_ = n_starting_points;
}

/******************************************************************************/
/**
 * Set the size of the finite step of the adaption process
 *
 * @param finite_step The desired size of the adaption
 */
void GGradientDescent::setFiniteStep(double finite_step) {
    // Check that the new finite_step has an appropriate value
    if(finite_step <= 0. ||
       finite_step > 1000.) { // Specified in per mill of the allowed or preferred value range
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GGradientDescent::setFiniteStep(double): Error!" << '\n'
            << "Invalid value of finite_step: " << finite_step << '\n'
            << "Must be in the range ]0.:1000.]" << '\n'
        );
    }

    finiteStep_ = finite_step;

    // Keep stepRatio_/adjustedFiniteStep_ consistent if called after init()
    updateDerivedQuantities();
}

/******************************************************************************/
/**
 * Retrieve the size of the finite step of the adaption process
 *
 * @return The current finite step size
 */
double GGradientDescent::getFiniteStep() const {
    return finiteStep_;
}

/******************************************************************************/
/**
 * Sets a multiplier for the adaption process
 *
 * @param step_size A multiplicative factor for the adaption process
 */
void GGradientDescent::setStepSize(double step_size) {
    // Check that the new step_size has an appropriate value
    if(step_size <= 0. ||
       step_size > 1000.) { // Specified in per mill of the allowed or preferred value range
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GGradientDescent::setStepSize(double): Error!" << '\n'
            << "Invalid value of step_size: " << step_size << '\n'
            << "Must be in the range ]0.:1000.]" << '\n'
        );
    }

    stepSize_ = step_size;

    // Keep stepRatio_/adjustedFiniteStep_ consistent if called after init()
    updateDerivedQuantities();
}

/******************************************************************************/
/**
 * Retrieves the current step size
 *
 * @return The current value of the step size
 */
double GGradientDescent::getStepSize() const {
    return stepSize_;
}

/******************************************************************************/
/**
 * Retrieve the number of processable items in the current iteration.
 *
 * @return The number of processable items in the current iteration
 */
std::size_t GGradientDescent::getNProcessableItems_() const {
    return this->size(); // Evaluation always needs to be done for the entire population
}

/******************************************************************************/
/**
 * Returns information about the type of optimization algorithm. This function needs
 * to be overloaded by the actual algorithms to return the correct type.
 *
 * @return The type of optimization algorithm
 */
std::string GGradientDescent::getAlgorithmPersonalityType_() const {
    return "PERSONALITY_GD";
}

/******************************************************************************/
/**
 * Returns the name of this optimization algorithm
 *
 * @return The name assigned to this optimization algorithm
 */
std::string GGradientDescent::getAlgorithmName_() const {
    return std::string("Gradient Descent");
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
void GGradientDescent::compare_(
    const GObject &cp,
    const Gem::Common::expectation &e,
    const double & /*limit*/
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GGradientDescent reference independent of this object and convert the pointer
    const GGradientDescent *p_load =
        Gem::Common::g_convert_and_compare<GObject, GGradientDescent>(cp, this);

    GToken token("GGradientDescent", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<G_OptimizationAlgorithm_Base>(*this, *p_load, token);

    // ... and then the local data
    compare_t(IDENTITY(nStartingPoints_, p_load->nStartingPoints_), token);
    compare_t(IDENTITY(nFPParmsFirst_, p_load->nFPParmsFirst_), token);
    compare_t(IDENTITY(finiteStep_, p_load->finiteStep_), token);
    compare_t(IDENTITY(stepSize_, p_load->stepSize_), token);
    // stepRatio_, dblLowerParameterBoundaries_, dblUpperParameterBoundaries_, adjustedFiniteStep_
    // are transient: recomputed in init() from the serialized fields above and not restored in
    // load_(). Comparing them would cause round-trip equality tests to fail spuriously.

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Resets the settings of this population to what was configured when
 * the optimize()-call was issued
 */
void GGradientDescent::resetToOptimizationStart_() {
    dblLowerParameterBoundaries_
        .clear(); // Holds lower boundaries of double parameters; Will be extracted in init()
    dblUpperParameterBoundaries_
        .clear(); // Holds upper boundaries of double parameters; Will be extracted in init()
    adjustedFiniteStep_
        .clear(); // A step-size normalized to each parameter range; Will be recalculated in init()

    // There is no more work to be done here, so we simply call the
    // function of the parent class
    G_OptimizationAlgorithm_Base::resetToOptimizationStart_();
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GGradientDescent::name_() const {
    return std::string("GGradientDescent");
}

/******************************************************************************/
/**
 * Loads the data of another population
 *
 * @param cp A pointer to another GGradientDescent object, camouflaged as a GObject
 */
void GGradientDescent::load_(const GObject *cp) {
    // Check that we are dealing with a GGradientDescent reference independent of this object and convert the pointer
    const GGradientDescent *p_load =
        Gem::Common::g_convert_and_compare<GObject, GGradientDescent>(cp, this);

    // First load the parent class'es data.
    // This will also take care of copying all individuals.
    G_OptimizationAlgorithm_Base::load_(cp);

    // ... and then our own data
    nStartingPoints_ = p_load->nStartingPoints_;
    nFPParmsFirst_ = p_load->nFPParmsFirst_;
    finiteStep_ = p_load->finiteStep_;
    stepSize_ = p_load->stepSize_;
    // stepRatio_ = p_load->stepRatio_; // temporary parameter
    // dbl_lower_parameter_boundaries_cnt_ = p_load->dbl_lower_parameter_boundaries_cnt_; // temporary parameter
    // dbl_upper_parameter_boundaries_cnt_ = p_load->dbl_upper_parameter_boundaries_cnt_; // temporary parameter
    // adjustedFiniteStep_ = p_load->adjustedFiniteStep_; // temporary parameter
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GObject *GGradientDescent::clone_() const {
    return new GGradientDescent(*this);
}

/******************************************************************************/
/**
 * The actual business logic to be performed during each iteration.
 *
 * @return The value of the best individual found in this iteration
 */
std::tuple<double, double> GGradientDescent::cycleLogic_() {
    if(afterFirstIteration()) {
        // Update the parameters of the parent individuals. This
        // only makes sense once the individuals have been evaluated
        this->updateParentIndividuals();
    }

    // Update the individual parameters in each dimension of the "children"
    this->updateChildParameters();

    // Trigger value calculation for all individuals (including parents)
    runFitnessCalculation_();

    std::tuple<double, double> best_fitness =
        std::make_tuple(this->at(0)->getWorstCase(), this->at(0)->getWorstCase());
    std::tuple<double, double> fitness_candidate =
        std::make_tuple(this->at(0)->getWorstCase(), this->at(0)->getWorstCase());

    // Retrieve information about the best fitness found and disallow re-evaluation
    GGradientDescent::iterator it;
    auto m = this->at(0)->getMaxMode(); // We assume that all individuals have the same max mode
    for(it = this->begin(); it != this->begin() + this->getNStartingPoints(); ++it) {
        std::get<G_RAW_FITNESS>(fitness_candidate) = (*it)->raw_fitness(0);
        std::get<G_TRANSFORMED_FITNESS>(fitness_candidate) = (*it)->transformed_fitness(0);

        if(isBetter(
               std::get<G_TRANSFORMED_FITNESS>(fitness_candidate),
               std::get<G_TRANSFORMED_FITNESS>(best_fitness),
               m
           )) {
            best_fitness = fitness_candidate;
        }
    }

    return best_fitness;
}

/******************************************************************************/
/**
 * Updates the individual parameters of children
 */
void GGradientDescent::updateChildParameters() {
    // Loop over all starting points
    for(std::size_t i = 0; i < nStartingPoints_; i++) {
        // Extract the fp vector
        std::vector<double> parm_vec;
        this->at(i)->streamline<double>(
            parm_vec,
            activityMode::ACTIVEONLY
        ); // Only extract active parameters

        // Loop over all directions
        for(std::size_t j = 0; j < nFPParmsFirst_; j++) {
            // Calculate the position of the child
            std::size_t child_pos = nStartingPoints_ + i * nFPParmsFirst_ + j;

            // Load the current "parent" into the "child"
            this->at(child_pos)->GObject::load(this->at(i));

            // Update the child's position in the population
            this->at(child_pos)
                ->getPersonalityTraits<GGradientDescent_PersonalityTraits>()
                ->setPopulationPosition(child_pos);

            // Make a note of the current parameter's value
            double orig_parm_val = parm_vec[j];

            // Add the finite step to the feature vector's current parameter
            parm_vec[j] += adjustedFiniteStep_[j];

            // Attach the feature vector to the child individual
            this->at(child_pos)->assignValueVector<double>(parm_vec, activityMode::ACTIVEONLY);

            // Restore the original value in the feature vector
            parm_vec[j] = orig_parm_val;
        }
    }
}

/**********************************************************************************************************/
/**
 * Performs a step of the parent individuals.
 * TODO: keep going in the same direction as long as there is an improvement
 */
void GGradientDescent::updateParentIndividuals() {
    for(std::size_t i = 0; i < nStartingPoints_; i++) {
        // Extract the fp vector
        std::vector<double> parm_vec;
        this->at(i)->streamline<double>(parm_vec, activityMode::ACTIVEONLY);

#ifdef DEBUG
        // Make sure the parents are clean
        if(this->at(i)->is_due_for_processing() || (this->at(i)->has_errors())) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GGradientDescent::updateParentIndividuals():" << '\n'
                << "Found individual in position " << i << " which is unprocessed or has errors"
                << '\n'
            );
        }
#endif /* DEBUG */

        // Retrieve the fitness of the individual again
        double parent_fitness = minOnly_transformed_fitness(this->at(i));

        // Calculate the adaption of each parameter
        // double gradient = 0.;
        for(std::size_t j = 0; j < nFPParmsFirst_; j++) {
            // Calculate the position of the child
            std::size_t child_pos = nStartingPoints_ + i * nFPParmsFirst_ + j;

            // Calculate the step to be performed in a given direction and
            // adjust the parameter vector of each parent
            try {
                parm_vec[j] -= Gem::Common::narrow_cast<double>(
                    stepRatio_ * (Gem::Common::narrow_cast<long double>(
                                     minOnly_transformed_fitness(this->at(child_pos)) -
                                     Gem::Common::narrow_cast<long double>(parent_fitness)
                                 ))
                );
            }
            catch(std::overflow_error &e) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, time_and_place)
                    << "In GGradientDescent::updateParentIndividuals(): Error!" << '\n'
                    << "Bad conversion with message " << e.what() << '\n'
                );
            }
        }

        // Load the parameter vector back into the parent
        this->at(i)->assignValueVector<double>(parm_vec, activityMode::ACTIVEONLY);
    }
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void GGradientDescent::addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) {
    // Call our parent class'es function
    G_OptimizationAlgorithm_Base::addConfigurationOptions_(gpb);

    // Add local data
    gpb.registerFileParameter<std::size_t>(
        "n_starting_points" // The name of the variable
        ,
        DEFAULTGDSTARTINGPOINTS // The default value
        ,
        [this](std::size_t nsp) { this->setNStartingPoints(nsp); }
    ) << "The number of simultaneous gradient descents";

    gpb.registerFileParameter<double>(
        "finite_step" // The name of the variable
        ,
        DEFAULTFINITESTEP // The default value
        ,
        [this](double fs) { this->setFiniteStep(fs); }
    ) << "The size of the adjustment in the difference quotient,"
      << '\n'
      << "specified in per mill of the allowed or expected value" << '\n'
      << "range of a parameter";

    gpb.registerFileParameter<double>(
        "step_size" // The name of the variable
        ,
        DEFAULTSTEPSIZE // The default value
        ,
        [this](double ss) { this->setStepSize(ss); }
    ) << "The size of each step into the"
      << '\n'
      << "direction of steepest descent," << '\n'
      << "specified in per mill of the allowed or expected value" << '\n'
      << "range of a parameter";
}

/******************************************************************************/
/**
 * Triggers fitness calculation of a number of individuals. This function performs the same task as done
 * in GBaseGD, albeit by delegating work to the broker. Items are evaluated up to the maximum position
 * in the vector. Note that we always start the evaluation with the first item in the vector.
 */
void GGradientDescent::runFitnessCalculation_() {
    using namespace Gem::Courtier;

#ifdef DEBUG
    std::size_t pos = 0;
    for(const auto &item_ptr : *this) {
        // Make sure the evaluated individuals are marked to be processed
        if(this->afterFirstIteration() && !item_ptr->is_due_for_processing()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GGradientDescent::runFitnessCalculation():" << '\n'
                << "Found individual om position " << pos << " which is not due for processing"
                << '\n'
            );
        }

        pos++;
    }
#endif /* DEBUG */

    //--------------------------------------------------------------------------------
    // Submit all work items and wait for their return

    setProcessingFlag(this->data_cnt_, std::make_tuple(std::size_t(0), this->data_cnt_.size()));
    auto status = this->workOn(
        this->data_cnt_,
        true // resubmit unprocessed items
        ,
        "GGradientDescent::runFitnessCalculation()"
    );

    //--------------------------------------------------------------------------------
    // Some error checks

    // Check if all work items have returned or whether there were errors. Both
    // cannot be tolerated, as a gradient method needs a complete set of evaluated solutions.
    if(not status.is_complete || status.has_errors) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GGradientDescent::runFitnessCalculation(): Error!" << '\n'
            << "No complete set of items received or errors found in some individuals" << '\n'
        );
    }

    //--------------------------------------------------------------------------------
}

/******************************************************************************/
/**
 * Does some preparatory work before the optimization starts
 */
void GGradientDescent::init() {
    // To be performed before any other action
    G_OptimizationAlgorithm_Base::init();

    // Extract the boundaries of all parameters
    this->at(0)->boundaries(
        dblLowerParameterBoundaries_,
        dblUpperParameterBoundaries_,
        activityMode::ACTIVEONLY
    );

#ifdef DEBUG
    // Size matters!
    if(dblLowerParameterBoundaries_.size() != dblUpperParameterBoundaries_.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GGradientDescent::init(): Error!" << '\n'
            << "Found invalid sizes: " << dblLowerParameterBoundaries_.size() << " / "
            << dblUpperParameterBoundaries_.size() << '\n'
        );
    }

    // Check that stepSize_ has an appropriate value
    if(stepSize_ <= 0. ||
       stepSize_ > 1000.) { // Specified in per mill of the allowed or preferred value range
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GGradientDescent::init(): Error!" << '\n'
            << "Invalid values of stepSize_: " << stepSize_ << '\n'
            << "Must be in the range ]0.:1000.]" << '\n'
        );
    }

    // Check that finiteStep_ has an appropriate value
    if(finiteStep_ <= 0. ||
       finiteStep_ > 1000.) { // Specified in per mill of the allowed or preferred value range
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GGradientDescent::init(): Error!" << '\n'
            << "Invalid values of finiteStep_: " << finiteStep_ << '\n'
            << "Must be in the range ]0.:1000.]" << '\n'
        );
    }
#endif /* DEBUG */

    // Compute the quantities derived from stepSize_/finiteStep_ and the
    // parameter boundaries extracted above.
    updateDerivedQuantities();

    // Tell individuals about their position in the population
    markIndividualPositions();
}

/******************************************************************************/
/**
 * Recomputes the quantities derived from finiteStep_, stepSize_ and the
 * extracted parameter boundaries. Called from init() (after the boundaries
 * have been extracted) and from setFiniteStep()/setStepSize() so that a
 * post-init change to those raw inputs does not leave the derived state
 * stale. Before init() the boundary vectors are empty, so adjustedFiniteStep_
 * is simply cleared and init() fills it once the boundaries are known.
 */
void GGradientDescent::updateDerivedQuantities() {
    // Set the step ratio. We do the calculation in long double precision to preserve accuracy
    stepRatio_ = ((long double)stepSize_) / ((long double)finiteStep_);

    // Calculate a specific finiteStep_ value for each parameter in long double precision
    try {
        adjustedFiniteStep_.clear();
        long double finite_step_ratio = ((long double)finiteStep_) / ((long double)1000.);
        for(std::size_t pos = 0; pos < dblLowerParameterBoundaries_.size(); pos++) {
            long double parameter_range = // NOLINT(cppcoreguidelines-init-variables)
                (long double)dblUpperParameterBoundaries_[pos] -
                (long double)dblLowerParameterBoundaries_[pos];
            adjustedFiniteStep_.push_back(
                Gem::Common::narrow_cast<double>(finite_step_ratio * parameter_range)
            );
        }
    }
    catch(std::overflow_error &e) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GGradientDescent::updateDerivedQuantities(): Error!" << '\n'
            << "Bad conversion with message " << e.what() << '\n'
        );
    }
}

/******************************************************************************/
/**
 * Does any necessary finalization work
 */
void GGradientDescent::finalize() {
    // Last action
    G_OptimizationAlgorithm_Base::finalize();
}

/******************************************************************************/
/**
 * Retrieve a GPersonalityTraits object belonging to this algorithm
 */
std::shared_ptr<GPersonalityTraits> GGradientDescent::getPersonalityTraits_() const {
    return std::make_shared<GGradientDescent_PersonalityTraits>();
}

/******************************************************************************/
/**
 * Gives individuals an opportunity to update their internal structures. The
 * inner workings of a gradient descent are largely deterministic. An option
 * to improve this might be to scan the immediate vicinity of the best solution(s)
 * or to run a small EA.
 */
void GGradientDescent::actOnStalls_() {
    /* nothing */
}

/******************************************************************************/
/**
 * Resizes the population to the desired level and does some error checks.
 */
void GGradientDescent::adjustPopulation_() {
    // Check how many individuals we already have
    std::size_t n_start = this->size();

    // Do some error checking ...

    // We need at least one individual
    if(n_start == 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GGradientDescent::adjustPopulation():" << '\n'
            << "You didn't add any individuals to the collection. We need at least one."
            << '\n'
        );
    }

    // Update the number of active floating point parameters in the individuals
    nFPParmsFirst_ = this->at(0)->countParameters<double>(activityMode::ACTIVEONLY);

    // Check that the first individual has floating point parameters (double for the moment)
    if(nFPParmsFirst_ == 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GGradientDescent::adjustPopulation():" << '\n'
            << "No floating point parameters in individual." << '\n'
        );
    }

    // Check that all individuals currently available have the same amount of parameters
#ifdef DEBUG
    for(std::size_t i = 1; i < this->size(); i++) {
        if(this->at(i)->countParameters<double>(activityMode::ACTIVEONLY) != nFPParmsFirst_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GGradientDescent::adjustPopulation():" << '\n'
                << "Found individual in position " << i << " with different" << '\n'
                << "number of floating point parameters than the first one: "
                << this->at(i)->countParameters<double>(activityMode::ACTIVEONLY) << "/"
                << nFPParmsFirst_ << '\n'
            );
        }
    }
#endif

    // Set the default size of the population
    G_OptimizationAlgorithm_Base::setDefaultPopulationSize(nStartingPoints_ * (nFPParmsFirst_ + 1));

    // First create a suitable number of start individuals and initialize them as required
    if(n_start < nStartingPoints_) {
        for(std::size_t i = 0; i < (nStartingPoints_ - n_start); i++) {
            // Create a copy of the first individual
            this->push_back(this->at(0)->clone<GParameterSet>());
            // Make sure our start values differ
            this->back()->randomInit(activityMode::ACTIVEONLY);
        }
    }
    else {
        // Start with a defined size. This will remove surplus items.
        this->resize(nStartingPoints_);
    }

    // Add the required number of clones for each starting point. These will be
    // used for the calculation of the difference quotient for each parameter
    for(std::size_t i = 0; i < nStartingPoints_; i++) {
        for(std::size_t j = 0; j < nFPParmsFirst_; j++) {
            this->push_back(this->at(i)->clone<GParameterSet>());
        }
    }

    // We now should have nStartingPoints_ sets of individuals,
    // each of size nFPParmsFirst_.
#ifdef DEBUG
    if(this->size() != nStartingPoints_ * (nFPParmsFirst_ + 1)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GGradientDescent::adjustPopulation():" << '\n'
            << "Population size is " << this->size() << '\n'
            << "but expected " << nStartingPoints_ * (nFPParmsFirst_ + 1) << '\n'
        );
    }
#endif /* DEBUG */
}

/******************************************************************************/
/**
 * This helper function lets all individuals know about their position in the
 * population.
 */
void GGradientDescent::markIndividualPositions() {
    for(std::size_t pos = 0; pos < this->size(); pos++) {
        this->at(pos)
            ->getPersonalityTraits<GGradientDescent_PersonalityTraits>()
            ->setPopulationPosition(pos);
    }
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 */
bool GGradientDescent::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    // Call the parent class'es function
    if(G_OptimizationAlgorithm_Base::modify_GUnitTests_())
        result = true;

    return result;
#else  /* GEM_TESTING */
    Gem::Common::condnotset("GGradientDescent::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GGradientDescent::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Call the parent class'es function
    G_OptimizationAlgorithm_Base::specificTestsNoFailureExpected_GUnitTests_();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GGradientDescent::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GGradientDescent::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Call the parent class'es function
    G_OptimizationAlgorithm_Base::specificTestsFailuresExpected_GUnitTests_();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GGradientDescent::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva */
