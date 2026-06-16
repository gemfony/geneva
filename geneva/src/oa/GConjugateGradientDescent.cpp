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

#include <algorithm>
#include <istream>
#include <limits>
#include <ostream>
#include <sstream>

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
#include "geneva/ind/GOptimizableEntity.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <span>
#include <vector>

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::OptimizationAlgorithms::GConjugateGradientDescent) // NOLINT

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/**
 * The auxiliary-store keys under which each starting point keeps its conjugate-gradient memory on the
 * OA scratch of its CENTRAL individual's GIndividualSlot (the slot at population position == starting
 * point). Phase 10.3 optional tail: this per-starting-point scratch (previously parallel vectors on the
 * algorithm) now rides on the slot as plain POD blocks. The two double blocks hold g_{k-1} / d_{k-1}
 * (n_fp_parms entries each); the one-byte block flags whether a previous gradient/direction exists. The
 * values are distinct from the adaption AuxKeys 1-7 and the swarm velocity key 8.
 */
constexpr Gem::Geneva::Parameters::AuxKey AUXKEY_CGD_PREV_GRADIENT = 9;
constexpr Gem::Geneva::Parameters::AuxKey AUXKEY_CGD_PREV_DIRECTION = 10;
constexpr Gem::Geneva::Parameters::AuxKey AUXKEY_CGD_HISTORY_VALID = 11;

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
/** @brief Streams an errorEstimationMode as its underlying integer. */
std::ostream &operator<<(std::ostream &o, errorEstimationMode em) {
    o << static_cast<int>(em);
    return o;
}

/******************************************************************************/
/** @brief Reads an errorEstimationMode from a stream. */
std::istream &operator>>(std::istream &i, errorEstimationMode &em) {
    int tmp = 0;
    i >> tmp;
    em = static_cast<errorEstimationMode>(tmp);
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
/** @brief Selects whether/how a MINUIT-style parameter-error estimate is computed at convergence. */
void GConjugateGradientDescent::setErrorEstimation(errorEstimationMode em) {
    error_estimation_ = em;
}

/******************************************************************************/
/** @brief Retrieves the error-estimation mode currently in use. */
errorEstimationMode GConjugateGradientDescent::getErrorEstimation() const {
    return error_estimation_;
}

/******************************************************************************/
/** @brief Sets the error definition UP (the objective increase defining one standard deviation). */
void GConjugateGradientDescent::setErrorDefinition(double up) {
    if(up <= 0.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GConjugateGradientDescent::setErrorDefinition(double): Error!" << '\n'
            << "UP must be positive, got " << up << '\n'
        );
    }
    error_up_ = up;
}

/******************************************************************************/
/** @brief Retrieves the error definition UP. */
double GConjugateGradientDescent::getErrorDefinition() const {
    return error_up_;
}

/******************************************************************************/
/** @brief Retrieves the most recent convergence error estimate. */
GHesseErrorResult GConjugateGradientDescent::getLastErrorEstimate() const {
    return last_error_estimate_;
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
    // dbl_lower_parameter_boundaries_, dbl_upper_parameter_boundaries_ and adjusted_finite_step_ are
    // transient: recomputed in init() from the serialized fields above and not restored in load_().
    // Comparing them would cause round-trip equality tests to fail spuriously. (The conjugate-gradient
    // memory likewise transient now lives on the central slots' OA scratch, outside this object.)
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
    // The per-starting-point conjugate-gradient memory now lives on the central slots' OA scratch and is
    // dropped at the optimization-algorithm boundary (resetIndividualPersonalities -> clearScratch).

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
    // adjusted_finite_step_ and dbl*ParameterBoundaries_ are transient and recomputed in init(); the
    // conjugate-gradient memory is transient too and lives on the central slots' OA scratch.
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
        std::make_tuple(this->at(0)->individual().getWorstCase(), this->at(0)->individual().getWorstCase());
    std::tuple<double, double> fitness_candidate =
        std::make_tuple(this->at(0)->individual().getWorstCase(), this->at(0)->individual().getWorstCase());

    GConjugateGradientDescent::iterator it;
    auto m = this->at(0)->individual().getMaxMode(); // All individuals share the same max mode
    for(it = this->begin(); it != this->begin() + this->getNStartingPoints(); ++it) {
        std::get<G_RAW_FITNESS>(fitness_candidate) = (*it)->individual().raw_fitness(0);
        std::get<G_TRANSFORMED_FITNESS>(fitness_candidate) = (*it)->individual().transformed_fitness(0);

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
        this->at(i)->individual().streamlineFP(parm_vec, activityMode::ACTIVEONLY);

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
            this->at(child_pos)->individual().assignFPValueVector(parm_vec, activityMode::ACTIVEONLY);

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
        this->at(i)->individual().streamlineFP(parm_vec, activityMode::ACTIVEONLY);

#ifdef DEBUG
        if(this->at(i)->individual().is_due_for_processing() || (this->at(i)->individual().has_errors())) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GConjugateGradientDescent::updateParentIndividuals():" << '\n'
                << "Found individual in position " << i
                << " which is unprocessed or has errors" << '\n'
            );
        }
#endif /* DEBUG */

        const double parent_fitness = minOnly_transformed_fitness(this->at(i)->individual());

        // The conjugate-gradient memory for this starting point lives on its central individual's slot
        // scratch (position i). Robustness: a central slot spliced in after a lost return has no CG
        // block -- install it (history invalid) so it restarts cleanly with steepest descent.
        auto &cg_scratch = this->at(i)->scratch();
        if(not cg_scratch.hasAux(AUXKEY_CGD_HISTORY_VALID)) {
            cg_scratch.installAuxBlock<double>(AUXKEY_CGD_PREV_GRADIENT, n_fp_parms_first_, gpar::AuxScope::PerIndividual);
            cg_scratch.installAuxBlock<double>(AUXKEY_CGD_PREV_DIRECTION, n_fp_parms_first_, gpar::AuxScope::PerIndividual);
            cg_scratch.installAuxBlock<std::uint8_t>(AUXKEY_CGD_HISTORY_VALID, 1, gpar::AuxScope::PerIndividual);
        }
        std::span<double> prev_gradient = cg_scratch.metaRecords<double>(AUXKEY_CGD_PREV_GRADIENT);
        std::span<double> prev_direction = cg_scratch.metaRecords<double>(AUXKEY_CGD_PREV_DIRECTION);
        std::uint8_t &cg_valid = cg_scratch.metaScalar<std::uint8_t>(AUXKEY_CGD_HISTORY_VALID);

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
                    (minOnly_transformed_fitness(this->at(child_pos)->individual()) - parent_fitness) / h;
            }
        }

        // 2) Build the search direction: plain steepest descent, or the Polak-Ribiere+ conjugate
        //    direction with Powell and periodic restarts.
        std::vector<double> direction(n_fp_parms_first_, 0.);
        double beta = 0.;
        const bool want_conjugate = (gradient_method_ != gradientMethod::STEEPEST_DESCENT) &&
                                    cg_valid && not periodic_restart;
        if(want_conjugate) {
            long double g_dot_g = 0.L;         // g_k . g_k
            long double g_dot_gprev = 0.L;     // g_k . g_{k-1}
            long double gprev_dot_gprev = 0.L; // g_{k-1} . g_{k-1}
            long double g_dot_y = 0.L;         // g_k . (g_k - g_{k-1})   (Polak-Ribiere / Hestenes-Stiefel num)
            long double d_dot_y = 0.L;         // d_{k-1} . (g_k - g_{k-1}) (Hestenes-Stiefel / Dai-Yuan den)
            for(std::size_t j = 0; j < n_fp_parms_first_; j++) {
                const long double g = gradient[j];
                const long double gp = prev_gradient[j];
                const long double y = g - gp;
                g_dot_g += g * g;
                g_dot_gprev += g * gp;
                gprev_dot_gprev += gp * gp;
                g_dot_y += g * y;
                d_dot_y += static_cast<long double>(prev_direction[j]) * y;
            }

            // Powell restart (Powell, "Restart procedures for the conjugate gradient method", Math.
            // Prog. 12, 1977): restart to steepest descent when successive gradients are insufficiently
            // orthogonal, i.e. |g_k . g_{k-1}| / ||g_k||^2 >= 0.1.
            const long double abs_overlap = (g_dot_gprev >= 0.L) ? g_dot_gprev : -g_dot_gprev;
            const bool powell_restart = (g_dot_g > 0.L) && (abs_overlap / g_dot_g >= 0.1L);

            // Per-formula numerator / denominator (the conjugate-gradient family differs only here):
            //   PR+ : g.(g-g_prev) / g_prev.g_prev    FR : g.g / g_prev.g_prev
            //   HS+ : g.(g-g_prev) / d_prev.(g-g_prev) DY : g.g / d_prev.(g-g_prev)
            long double num = 0.L;
            long double den = 0.L;
            switch(gradient_method_) {
            case gradientMethod::CONJUGATE_FR:
                num = g_dot_g;
                den = gprev_dot_gprev;
                break;
            case gradientMethod::CONJUGATE_HS:
                num = g_dot_y;
                den = d_dot_y;
                break;
            case gradientMethod::CONJUGATE_DY:
                num = g_dot_g;
                den = d_dot_y;
                break;
            case gradientMethod::CONJUGATE_PR_PLUS:
            default:
                num = g_dot_y;
                den = gprev_dot_gprev;
                break;
            }

            // Numerical-stability guard: the denominator vanishes near convergence (and d.(g-g_prev) can
            // change sign), so divide only when |den| is above the representable floor and large enough
            // relative to the numerator that |beta| stays below a finite cap. The "+" clamp (beta >= 0,
            // i.e. an automatic restart when beta would be negative) keeps every variant globally
            // convergent and matches PR+/HS+.
            constexpr long double beta_max = 1.0e4L;
            const long double abs_num = (num >= 0.L) ? num : -num;
            const long double abs_den = (den >= 0.L) ? den : -den;
            if(not powell_restart && abs_den > std::numeric_limits<long double>::min() &&
               abs_den * beta_max > abs_num) {
                const long double beta_cg = num / den;
                beta = (beta_cg > 0.L) ? Gem::Common::narrow<double>(beta_cg) : 0.;
            }
        }

        for(std::size_t j = 0; j < n_fp_parms_first_; j++) {
            direction[j] =
                -gradient[j] + (cg_valid ? beta * prev_direction[j] : 0.);
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
            this->at(i)->individual().assignFPValueVector(lr.x_new, activityMode::ACTIVEONLY);
        }

        // 6) Remember gradient/direction for the next conjugate step (on the slot's scratch).
        std::copy(gradient.begin(), gradient.end(), prev_gradient.begin());
        std::copy(direction.begin(), direction.end(), prev_direction.begin());
        cg_valid = 1;
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
    std::vector<std::unique_ptr<gpar::GOptimizableEntity>> probes;
    probes.reserve(points.size());
    for(auto const &pt : points) {
        auto probe = this->at(starting_point)->individual().clone_unique();
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
    ) << "The search-direction rule: 0 = Polak-Ribiere+ conjugate gradient (the default)," << '\n'
      << "1 = plain steepest descent (the former \"gd\"), 2 = Fletcher-Reeves," << '\n'
      << "3 = Hestenes-Stiefel+, 4 = Dai-Yuan";

    gpb.registerFileParameter<int>(
        "error_estimation",
        static_cast<int>(errorEstimationMode::NONE),
        [this](int em) { this->setErrorEstimation(static_cast<errorEstimationMode>(em)); }
    ) << "MINUIT-style parameter-error estimate at convergence:" << '\n'
      << "0 = none (default), 1 = diagonal (parabolic) errors," << '\n'
      << "2 = full Hessian -> covariance (small dimension only)," << '\n'
      << "3 = MINOS asymmetric (profiled) errors (small dimension only)";

    gpb.registerFileParameter<double>(
        "error_definition",
        1.,
        [this](double up) { this->setErrorDefinition(up); }
    ) << "The MINUIT error definition UP: the objective increase" << '\n'
      << "defining one standard deviation (1 = chi^2, 0.5 = -logL)";
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
        if(this->afterFirstIteration() && !item_ptr->individual().is_due_for_processing()) {
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

    auto status = this->workOnPopulation(0, this->data_cnt_.size());

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
    this->at(0)->individual().boundariesFP(
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
    // On a checkpoint resume the central slots already carry their restored conjugate-gradient memory --
    // preserve it (skip the zero-reset) so a resumed run continues the conjugate sequence.
    if(this->resumedFromCheckpoint()) {
        return;
    }

    // The per-starting-point conjugate-gradient memory lives on the OA scratch of each starting point's
    // central individual slot (position == starting point). Install a zeroed g_{k-1} / d_{k-1} double
    // block and a "history invalid" flag on each, so the first cgStep falls back to steepest descent.
    for(std::size_t i = 0; i < n_starting_points_ && i < this->size(); ++i) {
        auto &scratch = this->at(i)->scratch();
        scratch.installAuxBlock<double>(AUXKEY_CGD_PREV_GRADIENT, n_fp_parms_first_, gpar::AuxScope::PerIndividual);
        scratch.installAuxBlock<double>(AUXKEY_CGD_PREV_DIRECTION, n_fp_parms_first_, gpar::AuxScope::PerIndividual);
        scratch.installAuxBlock<std::uint8_t>(AUXKEY_CGD_HISTORY_VALID, 1, gpar::AuxScope::PerIndividual);
        // installAuxBlock zero-initialises, so the gradient/direction are 0 and the flag is false.
    }
}

/******************************************************************************/
/**
 * Does any necessary finalization work
 */
void GConjugateGradientDescent::finalize() {
    // Optional MINUIT-style parameter-error estimate at the converged minimum (opt-in). The curvature
    // probes are evaluated through the same consumer the algorithm uses, exactly like the line search.
    if(error_estimation_ != errorEstimationMode::NONE && n_fp_parms_first_ > 0 && not this->empty()) {
        // Pick the best starting point (the lowest min-only fitness).
        std::size_t best = 0;
        double best_fitness = minOnly_transformed_fitness(this->at(0)->individual());
        for(std::size_t i = 1; i < n_starting_points_ && i < this->size(); ++i) {
            const double f = minOnly_transformed_fitness(this->at(i)->individual());
            if(f < best_fitness) {
                best_fitness = f;
                best = i;
            }
        }

        std::vector<double> x_min;
        this->at(best)->individual().streamlineFP(x_min, activityMode::ACTIVEONLY);

        GHesseErrorOptions opts;
        opts.up = error_up_;
        // FULL and MINOS both build the covariance (MINOS seeds its profile brackets from the
        // correlation-aware sigma); MINOS additionally computes the asymmetric profiled bounds.
        opts.full_covariance = (error_estimation_ == errorEstimationMode::FULL ||
                                error_estimation_ == errorEstimationMode::MINOS);
        opts.minos = (error_estimation_ == errorEstimationMode::MINOS);

        const std::size_t best_point = best;
        GHesseError estimator;
        last_error_estimate_ = estimator.estimate(
            [this, best_point](std::vector<std::vector<double>> const &points) {
                return this->evaluateProbes(best_point, points);
            },
            x_min,
            best_fitness,
            adjusted_finite_step_,
            opts
        );

        if(last_error_estimate_.valid) {
            std::ostringstream oss;
            oss << "GConjugateGradientDescent: parameter-error estimate at the minimum (UP = "
                << error_up_ << ", "
                << (last_error_estimate_.covariance_valid ? "profiled" : "parameter-fixed") << "):\n";
            const std::size_t shown =
                std::min<std::size_t>(last_error_estimate_.parameter_errors.size(), 20u);
            for(std::size_t j = 0; j < shown; ++j) {
                oss << "  parameter[" << j << "] +/- " << last_error_estimate_.parameter_errors[j]
                    << '\n';
            }
            if(last_error_estimate_.parameter_errors.size() > shown) {
                oss << "  ... (" << (last_error_estimate_.parameter_errors.size() - shown)
                    << " more parameters)\n";
            }
            oss << "  curvature condition number = " << last_error_estimate_.condition_number
                << " (a large value flags a flat / ill-conditioned minimum)";
            glogger << oss.str() << '\n' << GLOGGING;
        }
        else {
            glogger << "GConjugateGradientDescent: no usable error estimate "
                       "(no positive curvature at the stopping point)."
                    << '\n'
                    << GLOGGING;
        }
    }

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

    n_fp_parms_first_ = this->at(0)->individual().countFPParameters(activityMode::ACTIVEONLY);

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
        // countParameters<T> is part of the genome-agnostic value-channel interface.
        auto const &ind0 = this->at(0)->individual();
        const std::size_t n_int_parms =
            ind0.countParameters<std::int32_t>(activityMode::ACTIVEONLY);
        const std::size_t n_bool_parms =
            ind0.countParameters<bool>(activityMode::ACTIVEONLY);
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
        if(this->at(i)->individual().countFPParameters(activityMode::ACTIVEONLY) != n_fp_parms_first_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GConjugateGradientDescent::adjustPopulation():" << '\n'
                << "Found individual in position " << i << " with different" << '\n'
                << "number of floating point parameters than the first one: "
                << this->at(i)->individual().countFPParameters(activityMode::ACTIVEONLY) << "/"
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
            this->back()->individual().randomInit(activityMode::ACTIVEONLY);
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
