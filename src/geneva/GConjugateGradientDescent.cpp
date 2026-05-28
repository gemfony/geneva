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

#include "geneva/oa/GConjugateGradientDescent.hpp"

#include <limits>

#include "common/GLogger.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::OptimizationAlgorithms::GConjugateGradientDescent) // NOLINT

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/**
 * The default constructor
 */
GConjugateGradientDescent::GConjugateGradientDescent()
  : GConjugateGradientDescent(
        DEFAULTCGDSTARTINGPOINTS,
        DEFAULTCGDFINITESTEP,
        DEFAULTCGDSTEPSIZE
    ) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with the number of starting points and step parameters
 *
 * @param n_starting_points The number of simultaneous starting points
 * @param finite_step The size of the difference-quotient step
 * @param step_size The multiplicative factor for the step along the search direction
 */
GConjugateGradientDescent::GConjugateGradientDescent(
    const std::size_t &n_starting_points,
    const double &finite_step,
    const double &step_size
)
  : n_starting_points_(n_starting_points)
  , finite_step_(finite_step)
  , step_size_(step_size) { /* nothing */
}

/******************************************************************************/
/**
 * Retrieves the number of starting points of the algorithm
 */
std::size_t GConjugateGradientDescent::getNStartingPoints() const {
    return n_starting_points_;
}

/******************************************************************************/
/**
 * Allows to set the number of starting points for the conjugate gradient descent
 */
void GConjugateGradientDescent::setNStartingPoints(std::size_t n_starting_points) {
    if(n_starting_points == 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GConjugateGradientDescent::setNStartingPoints(const std::size_t&):" << '\n'
            << "Got invalid number of starting points." << '\n'
        );
    }

    n_starting_points_ = n_starting_points;
}

/******************************************************************************/
/**
 * Set the size of the finite step of the difference quotient
 */
void GConjugateGradientDescent::setFiniteStep(double finite_step) {
    if(finite_step <= 0. ||
       finite_step > 1000.) { // Specified in per mill of the allowed or preferred value range
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GConjugateGradientDescent::setFiniteStep(double): Error!" << '\n'
            << "Invalid value of finite_step: " << finite_step << '\n'
            << "Must be in the range ]0.:1000.]" << '\n'
        );
    }

    finite_step_ = finite_step;

    // Keep adjusted_finite_step_ consistent if called after init()
    updateDerivedQuantities();
}

/******************************************************************************/
/**
 * Retrieve the size of the finite step of the difference quotient
 */
double GConjugateGradientDescent::getFiniteStep() const {
    return finite_step_;
}

/******************************************************************************/
/**
 * Sets a multiplier for the step along the search direction
 */
void GConjugateGradientDescent::setStepSize(double step_size) {
    if(step_size <= 0. ||
       step_size > 1000.) { // Specified in per mill of the allowed or preferred value range
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GConjugateGradientDescent::setStepSize(double): Error!" << '\n'
            << "Invalid value of step_size: " << step_size << '\n'
            << "Must be in the range ]0.:1000.]" << '\n'
        );
    }

    step_size_ = step_size;
}

/******************************************************************************/
/**
 * Retrieves the current step size
 */
double GConjugateGradientDescent::getStepSize() const {
    return step_size_;
}

/******************************************************************************/
/**
 * Retrieve the number of processable items in the current iteration.
 */
std::size_t GConjugateGradientDescent::getNProcessableItems_() const {
    return this->size(); // The entire population is (re-)evaluated every iteration
}

/******************************************************************************/
/**
 * Returns information about the type of optimization algorithm
 */
std::string GConjugateGradientDescent::getAlgorithmPersonalityType_() const {
    return "PERSONALITY_CGD";
}

/******************************************************************************/
/**
 * Returns the name of this optimization algorithm
 */
std::string GConjugateGradientDescent::getAlgorithmName_() const {
    return std::string("Conjugate Gradient Descent");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 */
void GConjugateGradientDescent::compare_(
    const GBase &cp,
    const Gem::Common::expectation &e,
    const double & /*limit*/
) const {
    using namespace Gem::Common;

    const GConjugateGradientDescent *p_load =
        Gem::Common::g_convert_and_compare<GBase, GConjugateGradientDescent>(cp, this);

    GToken token("GConjugateGradientDescent", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GBase>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration.
    // dbl_lower_parameter_boundaries_, dbl_upper_parameter_boundaries_, adjusted_finite_step_,
    // prev_gradient_, prev_direction_ and cg_history_valid_ are transient: recomputed in
    // init() from the serialized fields above and not restored in load_(). Comparing
    // them would cause round-trip equality tests to fail spuriously.
    g_compare_members(localMembers(), p_load->localMembers(), token);

    token.evaluate();
}

/******************************************************************************/
/**
 * Resets the settings of this population to what was configured when
 * the optimize()-call was issued
 */
void GConjugateGradientDescent::resetToOptimizationStart_() {
    dbl_lower_parameter_boundaries_.clear();
    dbl_upper_parameter_boundaries_.clear();
    adjusted_finite_step_.clear();
    prev_gradient_.clear();
    prev_direction_.clear();
    cg_history_valid_.clear();

    GBase::resetToOptimizationStart_();
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GConjugateGradientDescent::name_() const {
    return std::string("GConjugateGradientDescent");
}

/******************************************************************************/
/**
 * Loads the data of another population
 */
void GConjugateGradientDescent::load_(const GBase *cp) {
    const GConjugateGradientDescent *p_load =
        Gem::Common::g_convert_and_compare<GBase, GConjugateGradientDescent>(cp, this);

    // First load the parent class'es data (this also copies all individuals).
    GBase::load_(cp);

    // ... and then our own (serialized) data, derived from the single localMembers() declaration.
    // adjusted_finite_step_, dbl*ParameterBoundaries_, prev_gradient_, prev_direction_,
    // cg_history_valid_ are transient and recomputed in init().
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GBase *GConjugateGradientDescent::clone_() const {
    return new GConjugateGradientDescent(*this);
}

/******************************************************************************/
/**
 * The actual business logic to be performed during each iteration.
 *
 * The structure mirrors GGradientDescent: the conjugate-gradient step uses
 * the difference quotients evaluated in the *previous* iteration (they are
 * still stored on the individuals), then the children are rebuilt around the
 * stepped parents and the whole population is re-evaluated.
 *
 * @return The value of the best individual found in this iteration
 */
std::tuple<double, double> GConjugateGradientDescent::cycleLogic_() {
    if(afterFirstIteration()) {
        // Perform a conjugate-gradient step for every starting point. This
        // only makes sense once the difference quotients have been evaluated.
        this->updateParentIndividuals();
    }

    // Rebuild the difference-quotient children around the (stepped) parents
    this->updateChildParameters();

    // Trigger value calculation for all individuals (parents + children)
    runFitnessCalculation_();

    std::tuple<double, double> best_fitness =
        std::make_tuple(this->at(0)->getWorstCase(), this->at(0)->getWorstCase());
    std::tuple<double, double> fitness_candidate =
        std::make_tuple(this->at(0)->getWorstCase(), this->at(0)->getWorstCase());

    GConjugateGradientDescent::iterator it;
    auto m = this->at(0)->getMaxMode(); // All individuals share the same max mode
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
 * Rebuilds the difference-quotient children of every starting point. Identical
 * in spirit to GGradientDescent::updateChildParameters(): for starting point i
 * and direction j the child at position
 *
 *   n_starting_points_ + i * n_fp_parms_first_ + j
 *
 * is a copy of parent i with its j-th active parameter incremented by the
 * (range-scaled) finite step. This produces a forward difference quotient.
 */
void GConjugateGradientDescent::updateChildParameters() {
    for(std::size_t i = 0; i < n_starting_points_; i++) {
        std::vector<double> parm_vec;
        this->at(i)->streamline<double>(parm_vec, activityMode::ACTIVEONLY);

        for(std::size_t j = 0; j < n_fp_parms_first_; j++) {
            std::size_t child_pos = n_starting_points_ + i * n_fp_parms_first_ + j;

            // Load the current "parent" into the "child"
            this->at(child_pos)->load(this->at(i));

            // Update the child's position in the population
            this->at(child_pos)
                ->getPersonalityTraits<GConjugateGradientDescent_PersonalityTraits>()
                ->setPopulationPosition(child_pos);

            double orig_parm_val = parm_vec[j];

            // Add the finite step to the feature vector's current parameter
            parm_vec[j] += adjusted_finite_step_[j];
            this->at(child_pos)->assignValueVector<double>(parm_vec, activityMode::ACTIVEONLY);

            // Restore the original value for the next direction
            parm_vec[j] = orig_parm_val;
        }
    }
}

/******************************************************************************/
/**
 * Performs a non-linear conjugate-gradient step for every starting point.
 *
 * For each starting point the (proxy) gradient component in direction j is the
 * forward difference
 *
 *   g_j = f(x + h_j e_j) - f(x)
 *
 * (the same proxy used by GGradientDescent, i.e. not divided by h_j, so the
 * effective scaling matches the plain gradient descent). The Polak-Ribière+
 * coefficient and the new conjugate search direction are
 *
 *   beta = max(0, g . (g - g_prev) / (g_prev . g_prev))
 *   d    = -g + beta * d_prev          (with d = -g on the first step or on a restart)
 *
 * and the parameter vector is moved by step_ratio * d, with
 * step_ratio = step_size_ / finite_step_ (identical to GGradientDescent). A
 * non-positive / numerically unstable denominator triggers an automatic
 * restart (beta = 0), which keeps the method globally convergent.
 */
void GConjugateGradientDescent::updateParentIndividuals() {
    const long double step_ratio = (static_cast<long double>(step_size_)) / (static_cast<long double>(finite_step_));

    for(std::size_t i = 0; i < n_starting_points_; i++) {
        std::vector<double> parm_vec;
        this->at(i)->streamline<double>(parm_vec, activityMode::ACTIVEONLY);

#ifdef DEBUG
        if(this->at(i)->is_due_for_processing() || (this->at(i)->has_errors())) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GConjugateGradientDescent::updateParentIndividuals():" << '\n'
                << "Found individual in position " << i
                << " which is unprocessed or has errors" << '\n'
            );
        }
#endif /* DEBUG */

        const double parent_fitness = minOnly_transformed_fitness(this->at(i));

        // 1) Assemble the forward-difference gradient proxy g_j
        std::vector<double> gradient(n_fp_parms_first_, 0.);
        for(std::size_t j = 0; j < n_fp_parms_first_; j++) {
            std::size_t child_pos = n_starting_points_ + i * n_fp_parms_first_ + j;
            gradient[j] = minOnly_transformed_fitness(this->at(child_pos)) - parent_fitness;
        }

        // 2) Compute the Polak-Ribière+ beta and the conjugate direction
        std::vector<double> direction(n_fp_parms_first_, 0.);
        double beta = 0.;
        if(cg_history_valid_[i]) {
            long double numerator = 0.L;   // g . (g - g_prev)
            long double denominator = 0.L; // g_prev . g_prev
            for(std::size_t j = 0; j < n_fp_parms_first_; j++) {
                numerator += static_cast<long double>(gradient[j]) *
                             (static_cast<long double>(gradient[j]) - static_cast<long double>(prev_gradient_[i][j]));
                denominator +=
                    static_cast<long double>(prev_gradient_[i][j]) * static_cast<long double>(prev_gradient_[i][j]);
            }
            // Numerical-stability guard for the division. denominator is
            // g_{k-1}.g_{k-1}, which becomes vanishingly small near
            // convergence. Dividing by a tiny (but strictly positive)
            // denominator would blow beta up and destabilise the search
            // direction. We therefore restart (beta = 0, i.e. a steepest-
            // descent step) unless the denominator is (a) above the absolute
            // representable floor and (b) large enough relative to the
            // numerator that the quotient stays below a finite conjugate-weight
            // cap. Condition (b), denominator * beta_max > |numerator|,
            // guarantees |beta_pr| < beta_max by construction.
            constexpr long double beta_max = 1.0e4L;
            const long double abs_num = (numerator >= 0.L) ? numerator : -numerator;
            if(denominator > std::numeric_limits<long double>::min() &&
               denominator * beta_max > abs_num) {
                long double beta_pr = numerator / denominator;
                beta = (beta_pr > 0.L) ? Gem::Common::narrow<double>(beta_pr)
                                       : 0.; // PR+ clamp == automatic restart
            }
            else {
                // Degenerate / near-zero previous gradient, or a denominator
                // too small relative to the numerator -> restart.
                beta = 0.;
            }
        }

        for(std::size_t j = 0; j < n_fp_parms_first_; j++) {
            direction[j] = -gradient[j] +
                           (cg_history_valid_[i] ? beta * prev_direction_[i][j] : 0.);
        }

        // 3) Take the step x <- x + step_ratio * d
        try {
            for(std::size_t j = 0; j < n_fp_parms_first_; j++) {
                parm_vec[j] +=
                    Gem::Common::narrow<double>(step_ratio * static_cast<long double>(direction[j]));
            }
        }
        catch(std::overflow_error &e) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GConjugateGradientDescent::updateParentIndividuals(): Error!" << '\n'
                << "Bad conversion with message " << e.what() << '\n'
            );
        }

        // 4) Remember gradient/direction for the next conjugate step
        prev_gradient_[i] = gradient;
        prev_direction_[i] = direction;
        cg_history_valid_[i] = true;

        // Write the stepped parameter vector back into the parent
        this->at(i)->assignValueVector<double>(parm_vec, activityMode::ACTIVEONLY);
    }
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 */
void GConjugateGradientDescent::addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) {
    // Call our parent class'es function
    GBase::addConfigurationOptions_(gpb);

    gpb.registerFileParameter<std::size_t>(
        "n_starting_points",
        DEFAULTCGDSTARTINGPOINTS,
        [this](std::size_t nsp) { this->setNStartingPoints(nsp); }
    ) << "The number of simultaneous conjugate gradient descents";

    gpb.registerFileParameter<double>(
        "finite_step",
        DEFAULTCGDFINITESTEP,
        [this](double fs) { this->setFiniteStep(fs); }
    ) << "The size of the adjustment in the difference quotient,"
      << '\n'
      << "specified in per mill of the allowed or expected value" << '\n'
      << "range of a parameter";

    gpb.registerFileParameter<double>(
        "step_size",
        DEFAULTCGDSTEPSIZE,
        [this](double ss) { this->setStepSize(ss); }
    ) << "The size of each step along the conjugate search"
      << '\n'
      << "direction, specified in per mill of the allowed or" << '\n'
      << "expected value range of a parameter";
}

/******************************************************************************/
/**
 * Triggers fitness calculation of all individuals via the broker.
 */
void GConjugateGradientDescent::runFitnessCalculation_() {
    using namespace Gem::Courtier;

#ifdef DEBUG
    std::size_t pos = 0;
    for(const auto &item_ptr : *this) {
        if(this->afterFirstIteration() && !item_ptr->is_due_for_processing()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GConjugateGradientDescent::runFitnessCalculation():" << '\n'
                << "Found individual on position " << pos
                << " which is not due for processing" << '\n'
            );
        }
        pos++;
    }
#endif /* DEBUG */

    setProcessingFlag(this->data_cnt_, std::make_tuple(static_cast<std::size_t>(0), this->data_cnt_.size()));
    auto status = this->workOn(
        this->data_cnt_,
        true // resubmit unprocessed items
        ,
        "GConjugateGradientDescent::runFitnessCalculation()"
    );

    // A conjugate-gradient method needs a complete set of evaluated solutions.
    if(not status.is_complete || status.has_errors) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GConjugateGradientDescent::runFitnessCalculation(): Error!" << '\n'
            << "No complete set of items received or errors found in some individuals"
            << '\n'
        );
    }
}

/******************************************************************************/
/**
 * Does some preparatory work before the optimization starts
 */
void GConjugateGradientDescent::init() {
    // To be performed before any other action
    GBase::init();

    // Extract the boundaries of all active parameters
    this->at(0)->boundaries(
        dbl_lower_parameter_boundaries_,
        dbl_upper_parameter_boundaries_,
        activityMode::ACTIVEONLY
    );

#ifdef DEBUG
    if(dbl_lower_parameter_boundaries_.size() != dbl_upper_parameter_boundaries_.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GConjugateGradientDescent::init(): Error!" << '\n'
            << "Found invalid sizes: " << dbl_lower_parameter_boundaries_.size() << " / "
            << dbl_upper_parameter_boundaries_.size() << '\n'
        );
    }

    if(step_size_ <= 0. || step_size_ > 1000.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GConjugateGradientDescent::init(): Error!" << '\n'
            << "Invalid value of step_size_: " << step_size_ << '\n'
            << "Must be in the range ]0.:1000.]" << '\n'
        );
    }

    if(finite_step_ <= 0. || finite_step_ > 1000.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GConjugateGradientDescent::init(): Error!" << '\n'
            << "Invalid value of finite_step_: " << finite_step_ << '\n'
            << "Must be in the range ]0.:1000.]" << '\n'
        );
    }
#endif /* DEBUG */

    updateDerivedQuantities();
    resetCGState();
    markIndividualPositions();
}

/******************************************************************************/
/**
 * Recomputes the per-parameter difference-quotient step from finite_step_ and
 * the extracted parameter ranges. Before init() the boundary vectors are
 * empty, so adjusted_finite_step_ is simply cleared and init() fills it once the
 * boundaries are known.
 */
void GConjugateGradientDescent::updateDerivedQuantities() {
    try {
        adjusted_finite_step_.clear();
        long double finite_step_ratio = (static_cast<long double>(finite_step_)) / (static_cast<long double>(1000.));
        for(std::size_t pos = 0; pos < dbl_lower_parameter_boundaries_.size(); pos++) {
            long double parameter_range = static_cast<long double>(dbl_upper_parameter_boundaries_[pos]) -
                                          static_cast<long double>(dbl_lower_parameter_boundaries_[pos]);
            adjusted_finite_step_.push_back(
                Gem::Common::narrow<double>(finite_step_ratio * parameter_range)
            );
        }
    }
    catch(std::overflow_error &e) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GConjugateGradientDescent::updateDerivedQuantities(): Error!" << '\n'
            << "Bad conversion with message " << e.what() << '\n'
        );
    }
}

/******************************************************************************/
/**
 * (Re-)initialises the per-starting-point conjugate-gradient memory. Called
 * from init() once n_fp_parms_first_ is known (it is set in adjustPopulation_,
 * which runs before init()).
 */
void GConjugateGradientDescent::resetCGState() {
    prev_gradient_.assign(n_starting_points_, std::vector<double>(n_fp_parms_first_, 0.));
    prev_direction_.assign(n_starting_points_, std::vector<double>(n_fp_parms_first_, 0.));
    cg_history_valid_.assign(n_starting_points_, false);
}

/******************************************************************************/
/**
 * Does any necessary finalization work
 */
void GConjugateGradientDescent::finalize() {
    GBase::finalize();
}

/******************************************************************************/
/**
 * Retrieve a GPersonalityTraits object belonging to this algorithm
 */
std::shared_ptr<GPersonalityTraits> GConjugateGradientDescent::getPersonalityTraits_() const {
    return std::make_shared<GConjugateGradientDescent_PersonalityTraits>();
}

/******************************************************************************/
/**
 * Gives individuals an opportunity to update their internal structures. A
 * conjugate gradient descent is largely deterministic; nothing to do here.
 */
void GConjugateGradientDescent::actOnStalls_() {
    /* nothing */
}

/******************************************************************************/
/**
 * Resizes the population to the desired level and does some error checks.
 * The layout is identical to GGradientDescent:
 * n_starting_points_ * (n_fp_parms_first_ + 1) individuals.
 */
void GConjugateGradientDescent::adjustPopulation_() {
    std::size_t n_start = this->size();

    if(n_start == 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GConjugateGradientDescent::adjustPopulation():" << '\n'
            << "You didn't add any individuals to the collection. We need at least one."
            << '\n'
        );
    }

    n_fp_parms_first_ = this->at(0)->countParameters<double>(activityMode::ACTIVEONLY);

    if(n_fp_parms_first_ == 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GConjugateGradientDescent::adjustPopulation():" << '\n'
            << "No floating point parameters in individual." << '\n'
        );
    }

    // Conjugate gradient descent operates on the floating point parameters
    // only. Any integer / boolean parameters are left unchanged -- this is
    // normal, user-expected behaviour, so it is merely logged (not warned about).
    {
        const std::size_t n_int_parms =
            this->at(0)->countParameters<std::int32_t>(activityMode::ACTIVEONLY);
        const std::size_t n_bool_parms =
            this->at(0)->countParameters<bool>(activityMode::ACTIVEONLY);
        if(n_int_parms + n_bool_parms > 0) {
            glogger
                << "In GConjugateGradientDescent::adjustPopulation_(): Note:" << '\n'
                << "The individual carries " << n_int_parms << " integer and " << n_bool_parms
                << " boolean parameter(s) alongside " << n_fp_parms_first_
                << " floating point parameter(s)." << '\n'
                << "Conjugate gradient descent only operates on the floating point parameters;"
                << '\n'
                << "the non-differentiable parameters are left unchanged." << '\n'
                << GLOGGING;
        }
    }

#ifdef DEBUG
    for(std::size_t i = 1; i < this->size(); i++) {
        if(this->at(i)->countParameters<double>(activityMode::ACTIVEONLY) != n_fp_parms_first_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GConjugateGradientDescent::adjustPopulation():" << '\n'
                << "Found individual in position " << i << " with different" << '\n'
                << "number of floating point parameters than the first one: "
                << this->at(i)->countParameters<double>(activityMode::ACTIVEONLY) << "/"
                << n_fp_parms_first_ << '\n'
            );
        }
    }
#endif

    GBase::setDefaultPopulationSize(
        n_starting_points_ * (n_fp_parms_first_ + 1)
    );

    // Create the requested number of (randomized) starting points
    if(n_start < n_starting_points_) {
        for(std::size_t i = 0; i < (n_starting_points_ - n_start); i++) {
            this->push_back(this->at(0)->clone<gpar::GParameterSet>());
            this->back()->randomInit(activityMode::ACTIVEONLY);
        }
    }
    else {
        this->resize(n_starting_points_);
    }

    // Add the difference-quotient children for every starting point
    for(std::size_t i = 0; i < n_starting_points_; i++) {
        for(std::size_t j = 0; j < n_fp_parms_first_; j++) {
            this->push_back(this->at(i)->clone<gpar::GParameterSet>());
        }
    }

#ifdef DEBUG
    if(this->size() != n_starting_points_ * (n_fp_parms_first_ + 1)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GConjugateGradientDescent::adjustPopulation():" << '\n'
            << "Population size is " << this->size() << '\n'
            << "but expected " << n_starting_points_ * (n_fp_parms_first_ + 1) << '\n'
        );
    }
#endif /* DEBUG */
}

/******************************************************************************/
/**
 * Lets all individuals know about their position in the population.
 */
void GConjugateGradientDescent::markIndividualPositions() {
    for(std::size_t pos = 0; pos < this->size(); pos++) {
        this->at(pos)
            ->getPersonalityTraits<GConjugateGradientDescent_PersonalityTraits>()
            ->setPopulationPosition(pos);
    }
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 */
bool GConjugateGradientDescent::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    if(GBase::modify_GUnitTests_()) {
        result = true;
    }

    return result;
#else  /* GEM_TESTING */
    Gem::Common::condnotset("GConjugateGradientDescent::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GConjugateGradientDescent::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    GBase::specificTestsNoFailureExpected_GUnitTests_();
#else  /* GEM_TESTING */
    Gem::Common::condnotset(
        "GConjugateGradientDescent::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GConjugateGradientDescent::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    GBase::specificTestsFailuresExpected_GUnitTests_();
#else  /* GEM_TESTING */
    Gem::Common::condnotset(
        "GConjugateGradientDescent::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
