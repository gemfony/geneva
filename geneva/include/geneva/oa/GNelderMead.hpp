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
#include <memory>
#include <tuple>
#include <vector>

// Boost headers go here

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GPlotDesigner.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/oa/GOptimizationAlgorithmBase.hpp"
#include "geneva/oa/GNelderMead_PersonalityTraits.hpp"

#ifdef GEM_TESTING

#ifdef GEM_TESTING
#include "geneva/individuals/GTestIndividual1.hpp"
#endif /* GEM_TESTING */

#endif /* GEM_TESTING */

namespace Gem::Geneva::OptimizationAlgorithms {

/**
 * Default values for the Nelder-Mead downhill simplex.
 */
constexpr std::size_t DEFAULTNMSIMPLICES = 1;
constexpr double DEFAULTNMALPHA = 1.0;   ///< Reflection coefficient
constexpr double DEFAULTNMGAMMA = 2.0;   ///< Expansion coefficient
constexpr double DEFAULTNMRHO = 0.5;     ///< Contraction coefficient
constexpr double DEFAULTNMSIGMA = 0.5;   ///< Shrink coefficient
constexpr double DEFAULTNMINITIALEDGE = 0.1; ///< Initial simplex edge (fraction of range)
constexpr std::uint32_t DEFAULTNMRESTARTTHRESHOLD = 0; ///< Stall count triggering an oriented restart (0 = disabled)

/** @brief Number of speculative trial points evaluated per simplex and iteration
 *  (reflection, expansion, inside contraction, outside contraction). */
constexpr std::size_t NM_NTRIALS = 4;
/** @brief Trial slot offsets within a simplex block */
constexpr std::size_t NM_REFLECT = 0;
constexpr std::size_t NM_EXPAND = 1;
constexpr std::size_t NM_CONTRACT = 2;  ///< inside contraction
constexpr std::size_t NM_OCONTRACT = 3; ///< outside contraction

/******************************************************************************/
/**
 * GNelderMead implements the n-dimensional downhill simplex method of Nelder
 * and Mead. It is a derivative-free local optimizer and is therefore suitable
 * for non-differentiable (and moderately noisy) objective functions, for which
 * a (conjugate) gradient descent is inappropriate.
 *
 * A simplex in an n-dimensional parameter space has n+1 vertices. The
 * population layout per simplex is
 *
 *   [ v_0 ... v_n | reflect | expand | inside-contract | outside-contract ]
 *
 * i.e. n+1 vertices plus 4 speculative trial slots, giving a block size of
 * nFPParms + 5. n_simplices_ such blocks are run simultaneously (analogous to
 * the multiple starting points of the gradient descents).
 *
 * Because Geneva evaluates a fixed population per iteration through the
 * broker, the classical *sequential* simplex moves are mapped onto a batch
 * scheme: in every iteration the reflection, expansion and both (inside and
 * outside) contraction candidates are proposed and submitted together with the
 * vertices, and the standard Nelder-Mead acceptance rules are applied in the
 * next iteration once their fitnesses are known. This introduces a
 * one-iteration evaluation lag (the same approximation style used by
 * GGradientDescent's difference quotients) and converges to a local optimum
 * without any gradient information.
 *
 * An optional oriented restart (setRestartThreshold(), config "restart_threshold")
 * rebuilds every simplex around its best vertex, oriented down the local descent
 * direction, after the run has stalled for that many iterations. It escapes a
 * degenerate simplex collapse; at a genuine optimum it simply re-converges.
 */
class GNelderMead // NOLINT(cppcoreguidelines-special-member-functions)
  : public GOptimizationAlgorithmBase {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /** @brief Single declaration of this class'es local data members */
    auto localMembers() {
        return std::make_tuple(
            Gem::Common::make_member("n_simplices_", n_simplices_),
            Gem::Common::make_member("n_fp_parms_first_", n_fp_parms_first_),
            Gem::Common::make_member("alpha_", alpha_),
            Gem::Common::make_member("gamma_", gamma_),
            Gem::Common::make_member("rho_", rho_),
            Gem::Common::make_member("sigma_", sigma_),
            Gem::Common::make_member("initial_edge_", initial_edge_),
            Gem::Common::make_member("restart_threshold_", restart_threshold_)
        );
    }
    auto localMembers() const {
        return std::make_tuple(
            Gem::Common::make_member("n_simplices_", n_simplices_),
            Gem::Common::make_member("n_fp_parms_first_", n_fp_parms_first_),
            Gem::Common::make_member("alpha_", alpha_),
            Gem::Common::make_member("gamma_", gamma_),
            Gem::Common::make_member("rho_", rho_),
            Gem::Common::make_member("sigma_", sigma_),
            Gem::Common::make_member("initial_edge_", initial_edge_),
            Gem::Common::make_member("restart_threshold_", restart_threshold_)
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
    GNelderMead();
    /** @brief Initialization with the number of simplices */
    explicit GNelderMead(const std::size_t &);
    /** @brief A standard copy constructor */
    GNelderMead(const GNelderMead &) = default;
    /** @brief The destructor */
    ~GNelderMead() override = default;

    /** @brief Retrieves the number of simultaneous simplices */
    std::size_t getNSimplices() const;
    /** @brief Allows to set the number of simultaneous simplices */
    void setNSimplices(std::size_t);

    /** @brief Sets the reflection coefficient */
    void setAlpha(double);
    /** @brief Retrieves the reflection coefficient */
    double getAlpha() const;
    /** @brief Sets the expansion coefficient */
    void setGamma(double);
    /** @brief Retrieves the expansion coefficient */
    double getGamma() const;
    /** @brief Sets the contraction coefficient */
    void setRho(double);
    /** @brief Retrieves the contraction coefficient */
    double getRho() const;
    /** @brief Sets the shrink coefficient */
    void setSigma(double);
    /** @brief Retrieves the shrink coefficient */
    double getSigma() const;
    /** @brief Sets the relative size of the initial simplex */
    void setInitialEdge(double);
    /** @brief Retrieves the relative size of the initial simplex */
    double getInitialEdge() const;
    /** @brief Sets the stall count after which an oriented restart is performed (0 = disabled) */
    void setRestartThreshold(std::uint32_t);
    /** @brief Retrieves the oriented-restart stall threshold */
    std::uint32_t getRestartThreshold() const;

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
    friend void Gem::Common::compare_base_t<GNelderMead>(
        GNelderMead const &,
        GNelderMead const &,
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

    /** @brief Applies the Nelder-Mead acceptance rules using the trials of the previous iteration */
    void applyNelderMeadDecision();
    /** @brief Proposes the reflection / expansion / contraction trial points */
    void proposeTrials();

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
    /** @brief Builds the initial (non-degenerate) simplices around the seed vertices */
    void buildInitialSimplices();
    /** @brief Rebuilds every simplex around its best vertex, oriented down the local descent
     *  direction. Used to escape a degenerate collapse once the run has stalled. Returns true
     *  if at least one simplex was restarted (its vertices then need re-evaluation). */
    bool restartSimplices();
    /** @brief Shrinks simplex s by moving every non-best vertex towards the best vertex b */
    void shrinkTowardsBest(std::size_t s, std::size_t b);

    /** @brief Convenience: population index of vertex v in simplex s */
    std::size_t vertexPos(std::size_t s, std::size_t v) const;
    /** @brief Convenience: population index of trial slot t in simplex s */
    std::size_t trialPos(std::size_t s, std::size_t t) const;
    /** @brief Number of population slots used per simplex */
    std::size_t simplexBlockSize() const;

    /***************************************************************************/
    // Data

    std::size_t n_simplices_ =
        DEFAULTNMSIMPLICES;         ///< The number of simultaneous simplices
    std::size_t n_fp_parms_first_ = 0; ///< The amount of active floating point values per individual

    double alpha_ = DEFAULTNMALPHA;             ///< Reflection coefficient
    double gamma_ = DEFAULTNMGAMMA;             ///< Expansion coefficient
    double rho_ = DEFAULTNMRHO;                 ///< Contraction coefficient
    double sigma_ = DEFAULTNMSIGMA;             ///< Shrink coefficient
    double initial_edge_ = DEFAULTNMINITIALEDGE; ///< Initial simplex edge (fraction of range)
    std::uint32_t restart_threshold_ =
        DEFAULTNMRESTARTTHRESHOLD; ///< Stall count triggering an oriented restart (0 = disabled)

    std::vector<double>
        dbl_lower_parameter_boundaries_; ///< Lower boundaries of double parameters; extracted in init() (transient)
    std::vector<double>
        dbl_upper_parameter_boundaries_; ///< Upper boundaries of double parameters; extracted in init() (transient)

    bool trials_pending_ =
        false; ///< True once real trial points have been proposed (transient; gates the first decision)
};

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::OptimizationAlgorithms::GNelderMead) // NOLINT
