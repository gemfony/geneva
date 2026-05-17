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
#include <memory>
#include <vector>

// Boost headers go here

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GPlotDesigner.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/par/GParameterSet.hpp"
#include "geneva/oa/GBase.hpp"
#include "geneva/oa/GConjugateGradientDescent_PersonalityTraits.hpp"

#ifdef GEM_TESTING

#include "geneva/GTestIndividual1.hpp"

#endif /* GEM_TESTING */

namespace Gem::Geneva::OptimizationAlgorithms {

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
  : public GBase {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GBase",
            boost::serialization::base_object<GBase>(*this)
        ) & BOOST_SERIALIZATION_NVP(n_starting_points_) &
            BOOST_SERIALIZATION_NVP(n_fp_parms_first_) & BOOST_SERIALIZATION_NVP(finite_step_) &
            BOOST_SERIALIZATION_NVP(step_size_);
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

    /** @brief Sets a multiplier for the step along the search direction */
    void setStepSize(double);
    /** @brief Retrieves the current step size */
    double getStepSize() const;

protected:
    /***************************************************************************/
    // Virtual or overridden protected functions

    /** @brief Adds local configuration options to a GParserBuilder object */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) override;

    /** @brief Loads the data of another population */
    void load_(const GObject *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GConjugateGradientDescent>(
        GConjugateGradientDescent const &,
        GConjugateGradientDescent const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GObject & // the other object
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
    virtual void updateChildParameters();

    /** @brief Performs a conjugate-gradient step for every starting point */
    virtual void updateParentIndividuals();

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
    GObject *clone_() const override;

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

    /***************************************************************************/
    // Data

    std::size_t n_starting_points_ =
        DEFAULTCGDSTARTINGPOINTS;   ///< The number of starting positions in the parameter space
    std::size_t n_fp_parms_first_ = 0; ///< The amount of active floating point values per individual

    double finite_step_ =
        DEFAULTCGDFINITESTEP; ///< The size of the difference-quotient step (per mill of the range)
    double step_size_ =
        DEFAULTCGDSTEPSIZE; ///< Multiplicative factor for the step along the search direction

    std::vector<double>
        dbl_lower_parameter_boundaries_; ///< Lower boundaries of double parameters; extracted in init() (transient)
    std::vector<double>
        dbl_upper_parameter_boundaries_; ///< Upper boundaries of double parameters; extracted in init() (transient)
    std::vector<double>
        adjusted_finite_step_; ///< Per-parameter difference-quotient step; recomputed in init() (transient)

    // Per-starting-point conjugate-gradient memory. All transient: recomputed
    // during optimization and therefore neither serialized nor restored in
    // load_() (mirrors the treatment of adjusted_finite_step_ in GGradientDescent).
    std::vector<std::vector<double>> prev_gradient_;  ///< g_{k-1} for every starting point
    std::vector<std::vector<double>> prev_direction_; ///< d_{k-1} for every starting point
    std::vector<bool>
        cg_history_valid_; ///< Whether a previous gradient/direction exists per starting point
};

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::OptimizationAlgorithms::GConjugateGradientDescent) // NOLINT
