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

#include <istream>
#include <limits>
#include <ostream>

#include "common/GLogger.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GParserBuilder.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include "geneva/GenevaHelperFunctions.hpp"
#include "geneva/oa/GOptimizationAlgorithmBase.hpp"
#include "geneva/oa/GConjugateGradientDescent_PersonalityTraits.hpp"
#include "geneva/par/GParameterSet.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::OptimizationAlgorithms::GConjugateGradientDescent) // NOLINT

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/**
 * Streams a gradientMethod as its underlying integer (cast to int so it is written as a number, not a
 * character). Needed by the comparison / expectation framework and by configuration serialization.
 */
std::ostream &operator<<(std::ostream &o, gradientMethod gm) {
    o << static_cast<int>(gm);
    return o;
}

/******************************************************************************/
/**
 * Reads a gradientMethod from a stream.
 */
std::istream &operator>>(std::istream &i, gradientMethod &gm) {
    int tmp = 0;
    i >> tmp;
    gm = static_cast<gradientMethod>(tmp);
    return i;
}

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
 * Selects the search-direction rule. STEEPEST_DESCENT (beta == 0) reproduces the former, separate
 * gradient-descent algorithm; CONJUGATE_PR_PLUS (the default) is the Polak-Ribiere+ nonlinear CG.
 */
void GConjugateGradientDescent::setGradientMethod(gradientMethod gm) {
    gradient_method_ = gm;
}

/******************************************************************************/
/**
 * Retrieves the search-direction rule currently in use.
 */
gradientMethod GConjugateGradientDescent::getGradientMethod() const {
    return gradient_method_;
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
    const GOptimizationAlgorithmBase &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    const GConjugateGradientDescent *p_load =
        Gem::Common::g_convert_and_compare<GOptimizationAlgorithmBase, GConjugateGradientDescent>(cp, this);

    GToken token("GConjugateGradientDescent", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GOptimizationAlgorithmBase>(*this, *p_load, token);

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

    GOptimizationAlgorithmBase::resetToOptimizationStart_();
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
void GConjugateGradientDescent::load_(const GOptimizationAlgorithmBase *cp) {
    const GConjugateGradientDescent *p_load =
        Gem::Common::g_convert_and_compare<GOptimizationAlgorithmBase, GConjugateGradientDescent>(cp, this);

    // First load the parent class'es data (this also copies all individuals).
    GOptimizationAlgorithmBase::load_(cp);

    // ... and then our own (serialized) data, derived from the single localMembers() declaration.
    // adjusted_finite_step_, dbl*ParameterBoundaries_, prev_gradient_, prev_direction_,
    // cg_history_valid_ are transient and recomputed in init().
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GOptimizationAlgorithmBase *GConjugateGradientDescent::clone_() const {
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
        this->at(i)->streamlineFP(parm_vec, activityMode::ACTIVEONLY);

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
            this->at(child_pos)->assignFPValueVector(parm_vec, activityMode::ACTIVEONLY);

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
    // The line search's first trial step reproduces the former fixed step (step_size_/finite_step_) and
    // backtracks from there, so the method is never worse than the old fixed step and -- by the Armijo
    // sufficient-decrease test -- never moves a starting point uphill.
    const double step_ratio = step_size_ / finite_step_;

    // A representative parameter-space scale for the initial trial step. The gradient below is
    // normalised (units of 1/parameter), so a bare step_ratio would be mis-scaled; multiplying by the
    // mean difference-quotient step restores the old fixed step's magnitude as the first trial.
    double mean_h = 0.;
    if(not adjusted_finite_step_.empty()) {
        for(double h : adjusted_finite_step_) {
            mean_h += h;
        }
        mean_h /= static_cast<double>(adjusted_finite_step_.size());
    }

    // A conjugate direction loses accuracy after about n steps, so restart to steepest descent every
    // n_fp_parms iterations (the classical periodic restart) to keep the nonlinear CG globally
    // convergent.
    const bool periodic_restart =
        (n_fp_parms_first_ > 0) && (this->getIteration() % n_fp_parms_first_ == 0);

    GLineSearch line_search;

    for(std::size_t i = 0; i < n_starting_points_; i++) {
        std::vector<double> parm_vec;
        this->at(i)->streamlineFP(parm_vec, activityMode::ACTIVEONLY);

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

        const double parent_fitness = minOnly_transformed_fitness(*this->at(i));

        // 1) Normalised forward-difference gradient g_j = (f(x + h_j e_j) - f(x)) / h_j. Normalising by
        //    h_j (instead of folding 1/h into the step as the old fixed-step proxy did) makes g a proper
        //    gradient, so the line search's Armijo test and the conjugate-gradient beta are correctly
        //    scaled.
        std::vector<double> gradient(n_fp_parms_first_, 0.);
        for(std::size_t j = 0; j < n_fp_parms_first_; j++) {
            const std::size_t child_pos = n_starting_points_ + i * n_fp_parms_first_ + j;
            const double h = adjusted_finite_step_[j];
            if(h > 0.) {
                gradient[j] =
                    (minOnly_transformed_fitness(*this->at(child_pos)) - parent_fitness) / h;
            }
        }

        // 2) Build the search direction: plain steepest descent, or the Polak-Ribiere+ conjugate
        //    direction with Powell and periodic restarts.
        std::vector<double> direction(n_fp_parms_first_, 0.);
        double beta = 0.;
        const bool want_conjugate = (gradient_method_ == gradientMethod::CONJUGATE_PR_PLUS) &&
                                    cg_history_valid_[i] && not periodic_restart;
        if(want_conjugate) {
            long double g_dot_g = 0.L;           // g_k . g_k
            long double g_dot_gprev = 0.L;       // g_k . g_{k-1}
            long double gprev_dot_gprev = 0.L;   // g_{k-1} . g_{k-1}
            long double numerator = 0.L;         // g_k . (g_k - g_{k-1})  (Polak-Ribiere)
            for(std::size_t j = 0; j < n_fp_parms_first_; j++) {
                const long double g = gradient[j];
                const long double gp = prev_gradient_[i][j];
                g_dot_g += g * g;
                g_dot_gprev += g * gp;
                gprev_dot_gprev += gp * gp;
                numerator += g * (g - gp);
            }

            // Powell restart (Powell, "Restart procedures for the conjugate gradient method", Math.
            // Prog. 12, 1977): restart to steepest descent when successive gradients are insufficiently
            // orthogonal, i.e. |g_k . g_{k-1}| / ||g_k||^2 >= 0.1.
            const long double abs_overlap = (g_dot_gprev >= 0.L) ? g_dot_gprev : -g_dot_gprev;
            const bool powell_restart = (g_dot_g > 0.L) && (abs_overlap / g_dot_g >= 0.1L);

            // Numerical-stability guard: g_{k-1}.g_{k-1} vanishes near convergence, so divide only when
            // the denominator is above the representable floor and large enough relative to the
            // numerator that the quotient stays below a finite cap (which guarantees |beta_pr| < cap).
            constexpr long double beta_max = 1.0e4L;
            const long double abs_num = (numerator >= 0.L) ? numerator : -numerator;
            if(not powell_restart &&
               gprev_dot_gprev > std::numeric_limits<long double>::min() &&
               gprev_dot_gprev * beta_max > abs_num) {
                const long double beta_pr = numerator / gprev_dot_gprev;
                beta = (beta_pr > 0.L) ? Gem::Common::narrow<double>(beta_pr) : 0.; // PR+ clamp
            }
        }

        for(std::size_t j = 0; j < n_fp_parms_first_; j++) {
            direction[j] =
                -gradient[j] + (cg_history_valid_[i] ? beta * prev_direction_[i][j] : 0.);
        }

        // 3) The directional derivative grad f . d must be negative for a descent direction. A stale
        //    conjugate direction occasionally fails this; fall back to steepest descent so the line
        //    search has an acceptable direction.
        double g_dot_d = 0.;
        for(std::size_t j = 0; j < n_fp_parms_first_; j++) {
            g_dot_d += gradient[j] * direction[j];
        }
        if(not(g_dot_d < 0.)) {
            g_dot_d = 0.;
            for(std::size_t j = 0; j < n_fp_parms_first_; j++) {
                direction[j] = -gradient[j];
                g_dot_d += gradient[j] * direction[j]; // == -||g||^2 <= 0
            }
        }

        // 4) Line search along the direction. The probes are evaluated through the same consumer the
        //    algorithm uses (so they run on the GPU when the GPU consumer is active).
        GLineSearchOptions opts;
        opts.alpha_init = (mean_h > 0.) ? step_ratio * mean_h : step_ratio;
        const std::size_t starting_point = i;
        const GLineSearchResult lr = line_search.search(
            [this, starting_point](std::vector<std::vector<double>> const &points) {
                return this->evaluateProbes(starting_point, points);
            },
            parm_vec,
            direction,
            parent_fitness,
            g_dot_d,
            opts
        );

        // 5) Apply the accepted step. A starting point for which no step satisfied Armijo is left in
        //    place: it has effectively converged (zero gradient) or sits where the current direction
        //    cannot improve it.
        if(lr.success) {
            this->at(i)->assignFPValueVector(lr.x_new, activityMode::ACTIVEONLY);
        }

        // 6) Remember gradient/direction for the next conjugate step.
        prev_gradient_[i] = gradient;
        prev_direction_[i] = direction;
        cg_history_valid_[i] = true;
    }
}

/******************************************************************************/
/**
 * Evaluates a batch of trial parameter vectors (the line-search probes) by cloning the given starting
 * point, assigning each probe's floating point values, and submitting the lot through the same
 * span+policy consumer path the main algorithm uses (this->workOn). Returns one min-only fitness per
 * probe, in input order. Because submission goes through the broker/executor, the probes are evaluated
 * on whatever consumer is active -- serial, multi-threaded, GPU or networked -- so the line search is
 * fully decoupled from where evaluation happens.
 */
std::vector<double> GConjugateGradientDescent::evaluateProbes(
    std::size_t starting_point,
    std::vector<std::vector<double>> const &points
) {
    std::vector<std::unique_ptr<gpar::GParameterSet>> probes;
    probes.reserve(points.size());
    for(auto const &pt : points) {
        auto probe = this->at(starting_point)->clone_unique();
        probe->assignFPValueVector(pt, activityMode::ACTIVEONLY);
        probes.push_back(std::move(probe));
    }

    this->workOn(probes, 0, probes.size());

    std::vector<double> values;
    values.reserve(probes.size());
    for(auto const &probe : probes) {
        values.push_back(minOnly_transformed_fitness(*probe));
    }
    return values;
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 */
void GConjugateGradientDescent::addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) {
    // Call our parent class'es function
    GOptimizationAlgorithmBase::addConfigurationOptions_(gpb);

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
    ) << "The size of the INITIAL trial step along the search direction"
      << '\n'
      << "(in per mill of the value range); the line search then refines it";

    gpb.registerFileParameter<int>(
        "gradient_method",
        static_cast<int>(gradientMethod::CONJUGATE_PR_PLUS),
        [this](int gm) { this->setGradientMethod(static_cast<gradientMethod>(gm)); }
    ) << "The search-direction rule: 0 = Polak-Ribiere+ conjugate gradient" << '\n'
      << "(the default), 1 = plain steepest descent (the former \"gd\")";
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

    auto status = this->workOn(this->data_cnt_, 0, this->data_cnt_.size());

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
    GOptimizationAlgorithmBase::init();

    // Extract the boundaries of all active parameters
    this->at(0)->boundariesFP(
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
    GOptimizationAlgorithmBase::finalize();
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

    n_fp_parms_first_ = this->at(0)->countFPParameters(activityMode::ACTIVEONLY);

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
        if(this->at(i)->countFPParameters(activityMode::ACTIVEONLY) != n_fp_parms_first_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GConjugateGradientDescent::adjustPopulation():" << '\n'
                << "Found individual in position " << i << " with different" << '\n'
                << "number of floating point parameters than the first one: "
                << this->at(i)->countFPParameters(activityMode::ACTIVEONLY) << "/"
                << n_fp_parms_first_ << '\n'
            );
        }
    }
#endif

    GOptimizationAlgorithmBase::setDefaultPopulationSize(
        n_starting_points_ * (n_fp_parms_first_ + 1)
    );

    // Create the requested number of (randomized) starting points
    if(n_start < n_starting_points_) {
        for(std::size_t i = 0; i < (n_starting_points_ - n_start); i++) {
            this->push_back(this->at(0)->clone_unique());
            this->back()->randomInit(activityMode::ACTIVEONLY);
        }
    }
    else {
        this->resize(n_starting_points_);
    }

    // Add the difference-quotient children for every starting point
    for(std::size_t i = 0; i < n_starting_points_; i++) {
        for(std::size_t j = 0; j < n_fp_parms_first_; j++) {
            this->push_back(this->at(i)->clone_unique());
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

    if(GOptimizationAlgorithmBase::modify_GUnitTests_()) {
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
    GOptimizationAlgorithmBase::specificTestsNoFailureExpected_GUnitTests_();
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
    GOptimizationAlgorithmBase::specificTestsFailuresExpected_GUnitTests_();
#else  /* GEM_TESTING */
    Gem::Common::condnotset(
        "GConjugateGradientDescent::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
