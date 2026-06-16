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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <tuple>
#include <vector>

// Boost headers go here

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GPlotDesigner.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/oa/GHesseError.hpp"
#include "geneva/oa/GLineSearch.hpp"
#include "geneva/oa/GOptimizationAlgorithmBase.hpp"
#include "geneva/oa/GConjugateGradientDescent_PersonalityTraits.hpp"

#ifdef GEM_TESTING

#ifdef GEM_TESTING
#include "geneva/individuals/GTestIndividual1.hpp"
#endif /* GEM_TESTING */

#endif /* GEM_TESTING */

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/**
 * Selects how the search direction is built from the (finite-difference) gradient. Steepest descent is
 * the former, separate "gd" algorithm folded in as the beta == 0 special case.
 */
enum class gradientMethod : std::uint8_t {
    CONJUGATE_PR_PLUS = 0, ///< Polak-Ribiere+ nonlinear conjugate gradient with restarts (the default)
    STEEPEST_DESCENT = 1,  ///< Plain steepest descent (beta == 0)
    CONJUGATE_FR = 2,      ///< Fletcher-Reeves (beta = g.g / g_prev.g_prev)
    CONJUGATE_HS = 3,      ///< Hestenes-Stiefel+ (beta = g.(g-g_prev) / d_prev.(g-g_prev))
    CONJUGATE_DY = 4       ///< Dai-Yuan (beta = g.g / d_prev.(g-g_prev))
};

/** @brief Streams a gradientMethod (as its underlying integer); required by the comparison framework. */
std::ostream &operator<<(std::ostream &, gradientMethod);
/** @brief Reads a gradientMethod from a stream. */
std::istream &operator>>(std::istream &, gradientMethod &);

/**
 * Selects whether (and how thoroughly) a MINUIT-style parameter-error estimate is computed at the
 * minimum once the descent has converged. Opt-in (off by default).
 */
enum class errorEstimationMode : std::uint8_t {
    NONE = 0,     ///< No error estimate (the default)
    DIAGONAL = 1, ///< Cheap, always-affordable parameter-fixed (parabolic) errors from the Hessian diagonal
    FULL = 2,     ///< Full Hessian -> covariance -> correlation-aware errors (small dimension only)
    MINOS = 3     ///< MINOS asymmetric errors: profiled (re-minimised) UP-contour bounds, low dimension only
};

/** @brief Streams an errorEstimationMode (as its underlying integer); required by the comparison framework. */
std::ostream &operator<<(std::ostream &, errorEstimationMode);
/** @brief Reads an errorEstimationMode from a stream. */
std::istream &operator>>(std::istream &, errorEstimationMode &);

/**
 * Default values for the conjugate gradient descent. They mirror the plain
 * gradient descent so a user can swap "gd" for "cgd" without re-tuning.
 */
constexpr std::size_t DEFAULTCGDSTARTINGPOINTS = 1;
constexpr double DEFAULTCGDFINITESTEP = 0.001;
constexpr double DEFAULTCGDSTEPSIZE = 0.1;

/******************************************************************************/
/**
 * GConjugateGradientDescent implements an approximate non-linear conjugate
 * gradient method (Polak-Ribière+ with automatic restart). It is a drop-in
 * sibling of GGradientDescent and reuses the very same population layout
 * (one "parent" per starting point followed by nFPParms difference-quotient
 * "children"). The only conceptual difference is that, instead of stepping
 * straight along the negative gradient, the step is taken along a search
 * direction that is conjugate to the previous ones:
 *
 *   g_k        = forward-difference gradient at the current point
 *   beta_k     = max(0, g_k . (g_k - g_{k-1}) / (g_{k-1} . g_{k-1}))   (PR+)
 *   d_k        = -g_k + beta_k * d_{k-1}                               (d_0 = -g_0)
 *   x_{k+1}    = x_k + lambda * d_k
 *
 * The line-search is intentionally approximated by a fixed, range-scaled step
 * (identical scaling to GGradientDescent) rather than a Wolfe line search:
 * this keeps the algorithm fully compatible with Geneva's batch/broker
 * evaluation model (one submission of the whole population per iteration).
 * Only difference quotients of the evaluation function are used; no analytic
 * gradient and no error/Hessian information is computed.
 */
class GConjugateGradientDescent // NOLINT(cppcoreguidelines-special-member-functions)
  : public GOptimizationAlgorithmBase {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /** @brief Single declaration of this class'es local data members */
    auto localMembers() {
        return std::make_tuple(
            Gem::Common::make_member("n_starting_points_", n_starting_points_),
            Gem::Common::make_member("n_fp_parms_first_", n_fp_parms_first_),
            Gem::Common::make_member("finite_step_", finite_step_),
            Gem::Common::make_member("step_size_", step_size_),
            Gem::Common::make_member("gradient_method_", gradient_method_),
            Gem::Common::make_member("error_estimation_", error_estimation_),
            Gem::Common::make_member("error_up_", error_up_)
        );
    }
    auto localMembers() const {
        return std::make_tuple(
            Gem::Common::make_member("n_starting_points_", n_starting_points_),
            Gem::Common::make_member("n_fp_parms_first_", n_fp_parms_first_),
            Gem::Common::make_member("finite_step_", finite_step_),
            Gem::Common::make_member("step_size_", step_size_),
            Gem::Common::make_member("gradient_method_", gradient_method_),
            Gem::Common::make_member("error_estimation_", error_estimation_),
            Gem::Common::make_member("error_up_", error_up_)
        );
    }

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp("GOptimizationAlgorithmBase", boost::serialization::base_object<GOptimizationAlgorithmBase>(*this));
        // Member list derived from the single localMembers() declaration (same NVP
        // names/order as the previous explicit list).
        Gem::Common::serialize_members(ar, this->localMembers());
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GConjugateGradientDescent();
    /** @brief Initialization with the number of starting points and step parameters */
    GConjugateGradientDescent(const std::size_t &, const double &, const double &);
    /** @brief A standard copy constructor */
    GConjugateGradientDescent(const GConjugateGradientDescent &) = default;
    /** @brief The destructor */
    ~GConjugateGradientDescent() override = default;

    /** @brief Retrieves the number of starting points of the algorithm */
    std::size_t getNStartingPoints() const;
    /** @brief Allows to set the number of starting points for the conjugate gradient descent */
    void setNStartingPoints(std::size_t);

    /** @brief Set the size of the finite step of the difference quotient */
    void setFiniteStep(double);
    /** @brief Retrieve the size of the finite step of the difference quotient */
    double getFiniteStep() const;

    /** @brief Sets a multiplier for the initial trial step along the search direction */
    void setStepSize(double);
    /** @brief Retrieves the current step size */
    double getStepSize() const;

    /** @brief Selects the search-direction rule (conjugate PR+ or plain steepest descent) */
    void setGradientMethod(gradientMethod);
    /** @brief Retrieves the search-direction rule currently in use */
    gradientMethod getGradientMethod() const;

    /** @brief Selects whether/how a MINUIT-style parameter-error estimate is computed at convergence */
    void setErrorEstimation(errorEstimationMode);
    /** @brief Retrieves the error-estimation mode currently in use */
    errorEstimationMode getErrorEstimation() const;
    /** @brief Sets the error definition UP (1 for chi^2-like, 0.5 for -logL); see MINUIT */
    void setErrorDefinition(double);
    /** @brief Retrieves the error definition UP */
    double getErrorDefinition() const;
    /** @brief Retrieves the most recent convergence error estimate (valid only if one was requested) */
    GHesseErrorResult getLastErrorEstimate() const;

protected:
    /***************************************************************************/
    // Virtual or overridden protected functions

    /** @brief Need-all algorithm: a missing or failed evaluation cannot be tolerated, so it submits
     *  through courtier under full-success-or-fatal (matches the legacy throw-on-error). */
    Gem::Courtier::GSubmissionPolicy getSubmissionPolicy_() const override {
        return Gem::Courtier::GSubmissionPolicy::full_success_or_fatal();
    }

    /** @brief Adds local configuration options to a GParserBuilder object */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) override;

    /** @brief Loads the data of another population */
    void load_(const GOptimizationAlgorithmBase *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GConjugateGradientDescent>(
        GConjugateGradientDescent const &,
        GConjugateGradientDescent const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GOptimizationAlgorithmBase & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const override;

    /** @brief Resets the settings of this population to what was configured when the optimize()-call was issued */
    void resetToOptimizationStart_() override;

    /** @brief Does some preparatory work before the optimization starts */
    void init() override;
    /** @brief Does any necessary finalization work */
    void finalize() override;

    /** @brief Updates the difference-quotient children of every starting point */
    void updateChildParameters();

    /** @brief Performs a conjugate-gradient step for every starting point */
    void updateParentIndividuals();

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

    /***************************************************************************/

private:
    /***************************************************************************/
    // Virtual or overridden private functions

    /** @brief Emits a name for this class / object */
    std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    GOptimizationAlgorithmBase *clone_() const override;

    /** @brief The actual business logic to be performed during each iteration. Returns the best achieved fitness */
    std::tuple<double, double> cycleLogic_() override;
    /** @brief Triggers fitness calculation of a number of individuals */
    void runFitnessCalculation_() override;

    /** @brief Returns information about the type of optimization algorithm */
    std::string getAlgorithmPersonalityType_() const override;
    /** @brief Returns the name of this optimization algorithm */
    std::string getAlgorithmName_() const override;

    /** @brief Retrieves the number of processable items for the current iteration */
    std::size_t getNProcessableItems_() const override;

    /** @brief Retrieve a GPersonalityTraits object belonging to this algorithm */
    std::shared_ptr<GPersonalityTraits> getPersonalityTraits_() const override;
    /** @brief Resizes the population to the desired level and does some error checks */
    void adjustPopulation_() override;

    /** @brief Gives individuals an opportunity to update their internal structures */
    void actOnStalls_() override;

    /** @brief Lets individuals know about their position in the population */
    void markIndividualPositions();
    /** @brief Recomputes adjusted_finite_step_ from finite_step_ and the parameter ranges */
    void updateDerivedQuantities();
    /** @brief (Re-)initialises the per-starting-point conjugate-gradient state */
    void resetCGState();
    /** @brief Evaluates a batch of trial parameter vectors (line-search probes) as clones of the
     *  given starting point, submitted through the same consumer the algorithm uses, and returns one
     *  min-only fitness per probe. */
    std::vector<double> evaluateProbes(
        std::size_t starting_point,
        std::vector<std::vector<double>> const &points
    );

    /***************************************************************************/
    // Data

    std::size_t n_starting_points_ =
        DEFAULTCGDSTARTINGPOINTS;   ///< The number of starting positions in the parameter space
    std::size_t n_fp_parms_first_ = 0; ///< The amount of active floating point values per individual

    double finite_step_ =
        DEFAULTCGDFINITESTEP; ///< The size of the difference-quotient step (per mill of the range)
    double step_size_ =
        DEFAULTCGDSTEPSIZE; ///< Multiplicative factor for the initial trial step (the line search refines it)

    gradientMethod gradient_method_ =
        gradientMethod::CONJUGATE_PR_PLUS; ///< Conjugate (PR+) by default; STEEPEST_DESCENT == the former GD

    errorEstimationMode error_estimation_ =
        errorEstimationMode::NONE; ///< Whether to estimate parameter errors at convergence (opt-in)
    double error_up_ = 1.;         ///< The MINUIT error definition UP (1 = chi^2, 0.5 = -logL)

    GHesseErrorResult last_error_estimate_; ///< The most recent error estimate (transient; not serialized)

    std::vector<double>
        dbl_lower_parameter_boundaries_; ///< Lower boundaries of double parameters; extracted in init() (transient)
    std::vector<double>
        dbl_upper_parameter_boundaries_; ///< Upper boundaries of double parameters; extracted in init() (transient)
    std::vector<double>
        adjusted_finite_step_; ///< Per-parameter difference-quotient step; recomputed in init() (transient)

    // The per-starting-point conjugate-gradient memory (g_{k-1} / d_{k-1} / history-valid flag) now
    // lives on the OA scratch of each starting point's central individual slot, as POD blocks keyed
    // AUXKEY_CGD_PREV_GRADIENT / _PREV_DIRECTION / _HISTORY_VALID (see GConjugateGradientDescent.cpp).
    // It remains transient (recomputed during optimization, neither serialized nor restored).
};

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::OptimizationAlgorithms::GConjugateGradientDescent) // NOLINT
