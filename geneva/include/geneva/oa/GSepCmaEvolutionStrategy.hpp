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
#include <cstddef>
#include <string_view>
#include <tuple>
#include <vector>

// Boost headers go here

// Geneva headers go here
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/oa/GOptimizationAlgorithmBase.hpp"
#include "geneva/oa/GOptimizationAlgorithmT.hpp"
#include "geneva/oa/GSepCmaEvolutionStrategy_PersonalityTraits.hpp"

#ifdef GEM_TESTING
#include "geneva/individuals/GTestIndividual1.hpp"
#endif /* GEM_TESTING */

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/**
 * The default offspring count (lambda) per generation. 0 means "use the textbook
 * default 4 + floor(3*ln(n))".
 */
constexpr std::size_t DEFAULTSEPCMALAMBDA = 0;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * GSepCmaEvolutionStrategy implements a derandomized evolution strategy that scales to very high
 * dimensions (10000+ parameters). It is a from-scratch CSA-ES with an optional separable (diagonal)
 * covariance matrix adaptation (sep-CMA-ES). All updates are O(n) per generation, so unlike full
 * CMA-ES (O(n^2)) it remains usable for n in the tens of thousands.
 *
 * Unlike the classic (mu,lambda) self-adaptive ES (GEvolutionaryAlgorithm), this algorithm does NOT
 * carry a per-individual step size, and it does NOT use a per-genome adaption config: it maintains a
 * single search distribution entirely inside the algorithm object:
 *
 *   - m       : the distribution mean (length n)
 *   - sigma   : a single global step size
 *   - C       : a diagonal covariance (length n; per-coordinate variance)
 *   - p_sigma : the conjugate evolution path (drives CSA step-size control)
 *   - p_c     : the anisotropic evolution path (drives the rank-1 C update)
 *
 * Each generation samples lambda offspring  x_k = m + sigma * sqrt(C) o N(0,I), evaluates them through
 * the one process consumer (transport-agnostic, mirroring the stock EA's submission path), selects the
 * mu best, weighted-recombines them (positive log weights), and updates m -> p_sigma -> sigma (CSA) ->
 * p_c -> C (rank-1 [+ optional rank-mu]).
 *
 * The constants (c_sigma, d_sigma, c_c, c_1, c_mu) all carry the standard 1/n and 1/sqrt(n) dimension
 * scalings (Hansen, "The CMA Evolution Strategy: A Tutorial", 2016; Ros & Hansen, "A Simple Modification
 * in CMA-ES Achieving Linear Time and Space Complexity", PPSN 2008). THIS dimension scaling is what the
 * stock self-adaptive EA lacks at high n.
 *
 * It also supports an NSGA-II-style multi-objective selection mode (fast non-dominated sort + crowding
 * distance) used as the recombination ranking key.
 */
class GSepCmaEvolutionStrategy // NOLINT(cppcoreguidelines-special-member-functions)
  : public GOptimizationAlgorithmT<GSepCmaEvolutionStrategy> {
public:
    // Identifiers consumed by the GOptimizationAlgorithmT scaffold.
    static constexpr std::string_view oa_class_name = "GSepCmaEvolutionStrategy";
    static constexpr std::string_view oa_algorithm_name = "Separable CMA / CSA Evolution Strategy";
    static constexpr std::string_view oa_personality_type = "PERSONALITY_SEPCMA";

private:
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /** @brief Single declaration of this class'es local data members (drives serialize/load/compare). */
    auto localMembers() {
        return std::make_tuple(
            Gem::Common::make_member("lambda_", lambda_),
            Gem::Common::make_member("mu_", mu_),
            Gem::Common::make_member("use_diagonal_cma_", use_diagonal_cma_),
            Gem::Common::make_member("pareto_mode_", pareto_mode_),
            Gem::Common::make_member("initial_sigma_", initial_sigma_),
            Gem::Common::make_member("n_", n_),
            Gem::Common::make_member("sigma_", sigma_),
            Gem::Common::make_member("m_", m_),
            Gem::Common::make_member("C_", C_),
            Gem::Common::make_member("p_sigma_", p_sigma_),
            Gem::Common::make_member("p_c_", p_c_),
            Gem::Common::make_member("lower_", lower_),
            Gem::Common::make_member("upper_", upper_),
            Gem::Common::make_member("state_initialized_", state_initialized_),
            Gem::Common::make_member("mu_eff_", mu_eff_),
            Gem::Common::make_member("c_sigma_", c_sigma_),
            Gem::Common::make_member("d_sigma_", d_sigma_),
            Gem::Common::make_member("c_c_", c_c_),
            Gem::Common::make_member("c_1_", c_1_),
            Gem::Common::make_member("c_mu_", c_mu_),
            Gem::Common::make_member("chi_n_", chi_n_),
            Gem::Common::make_member("weights_", weights_)
        );
    }
    auto localMembers() const {
        return std::make_tuple(
            Gem::Common::make_member("lambda_", lambda_),
            Gem::Common::make_member("mu_", mu_),
            Gem::Common::make_member("use_diagonal_cma_", use_diagonal_cma_),
            Gem::Common::make_member("pareto_mode_", pareto_mode_),
            Gem::Common::make_member("initial_sigma_", initial_sigma_),
            Gem::Common::make_member("n_", n_),
            Gem::Common::make_member("sigma_", sigma_),
            Gem::Common::make_member("m_", m_),
            Gem::Common::make_member("C_", C_),
            Gem::Common::make_member("p_sigma_", p_sigma_),
            Gem::Common::make_member("p_c_", p_c_),
            Gem::Common::make_member("lower_", lower_),
            Gem::Common::make_member("upper_", upper_),
            Gem::Common::make_member("state_initialized_", state_initialized_),
            Gem::Common::make_member("mu_eff_", mu_eff_),
            Gem::Common::make_member("c_sigma_", c_sigma_),
            Gem::Common::make_member("d_sigma_", d_sigma_),
            Gem::Common::make_member("c_c_", c_c_),
            Gem::Common::make_member("c_1_", c_1_),
            Gem::Common::make_member("c_mu_", c_mu_),
            Gem::Common::make_member("chi_n_", chi_n_),
            Gem::Common::make_member("weights_", weights_)
        );
    }

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GOptimizationAlgorithmBase",
            boost::serialization::base_object<GOptimizationAlgorithmBase>(*this)
        );
        Gem::Common::serialize_members(ar, this->localMembers());
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GSepCmaEvolutionStrategy();
    /** @brief A standard copy constructor */
    GSepCmaEvolutionStrategy(const GSepCmaEvolutionStrategy &) = default;
    /** @brief The standard destructor */
    ~GSepCmaEvolutionStrategy() override = default;

    /**
     * @brief Sets the number of offspring sampled per generation (lambda); 0 selects the textbook default.
     * @param lambda The number of offspring per generation (0 == 4 + floor(3*ln(n)))
     */
    void setLambda(std::size_t lambda);
    /** @brief Retrieves the number of offspring sampled per generation (lambda). @return lambda */
    std::size_t getLambda() const;

    /**
     * @brief Sets the number of selected parents (mu); 0 means lambda/2.
     * @param mu The number of recombination parents (0 == lambda/2)
     */
    void setMu(std::size_t mu);
    /** @brief Retrieves the number of selected parents (mu). @return mu */
    std::size_t getMu() const;

    /**
     * @brief Enables / disables the diagonal (sep-CMA) covariance update.
     * @param use_diagonal_cma true to run the diagonal covariance adaptation; false for pure CSA-ES
     */
    void setUseDiagonalCMA(bool use_diagonal_cma);
    /** @brief Checks whether the diagonal (sep-CMA) covariance update is active. @return the flag */
    bool getUseDiagonalCMA() const;

    /**
     * @brief Enables / disables NSGA-II-style multi-objective selection.
     * @param pareto_mode true to rank offspring by non-dominated sort + crowding distance
     */
    void setParetoMode(bool pareto_mode);
    /** @brief Checks whether NSGA-II-style multi-objective selection is active. @return the flag */
    bool getParetoMode() const;

    /**
     * @brief Sets the initial global step size (sigma) as a fraction of the parameter range.
     * @param initial_sigma The initial sigma fraction (must be > 0)
     */
    void setInitialSigma(double initial_sigma);
    /** @brief Retrieves the initial global step size fraction. @return initial_sigma */
    double getInitialSigma() const;

    /** @brief Retrieves the current global step size (sigma). @return the current sigma */
    double getSigma() const;

protected:
    /***************************************************************************/
    // Virtual or overridden protected functions

    /**
     * @brief Adds local configuration options to a GParserBuilder object.
     * @param gpb The parser-builder to which this algorithm's configuration options are added
     */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) override;
    /**
     * @brief Loads the data of another GSepCmaEvolutionStrategy object.
     * @param cp A pointer to the other object (downcast from GOptimizationAlgorithmBase)
     */
    void load_(const GOptimizationAlgorithmBase *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GSepCmaEvolutionStrategy>(
        GSepCmaEvolutionStrategy const &,
        GSepCmaEvolutionStrategy const &,
        Gem::Common::GToken &
    );

    /**
     * @brief Searches for compliance with expectations with respect to another object of the same type.
     * @param cp The other object to compare against (downcast from GOptimizationAlgorithmBase)
     * @param e The expectation for this object, e.g. equality
     * @param limit The limit for allowed deviations of floating point types
     */
    void compare_(
        const GOptimizationAlgorithmBase &cp,
        const Gem::Common::expectation &e,
        const double &limit
    ) const override;

    /** @brief Resets the settings of this population to what was configured when optimize() was issued */
    void resetToOptimizationStart_() override;

    /** @brief Does any necessary initialization work before the optimization cycle starts */
    void init() override;
    /** @brief Does any necessary finalization work */
    void finalize() override;

    /** @brief Applies modifications to this object */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

    /***************************************************************************/

private:
    /***************************************************************************/
    // Overloaded or virtual base functions. name_(), clone_(), getAlgorithmName_(),
    // getAlgorithmPersonalityType_() are generated by the GOptimizationAlgorithmT scaffold.

    /** @brief The actual business logic performed during each iteration */
    std::tuple<double, double> cycleLogic_() override;
    /** @brief Submits offspring to the one process consumer and waits for processed items */
    void runFitnessCalculation_() override;

    /** @brief Retrieves the number of processable items for the current iteration */
    std::size_t getNProcessableItems_() const override;

    /** @brief Retrieve a GPersonalityTraits object belonging to this algorithm */
    std::shared_ptr<GPersonalityTraits> getPersonalityTraits_() const override;

    /** @brief Gives derived classes an opportunity to update their internal structures */
    void actOnStalls_() override;

    /** @brief Sizes the population to lambda offspring (and derives n / constants) and does error checks */
    void adjustPopulation_() override;

    /***************************************************************************/
    // Algorithm-internal helpers

    /** @brief Determines the dimension n and the parameter bounds from the first individual */
    void determineDimensionAndBounds();
    /** @brief Derives lambda/mu/weights/constants once the dimension n is known */
    void setUpStrategyParameters();
    /** @brief Samples lambda offspring from the current distribution into the population */
    void sampleOffspring();
    /** @brief Returns the indices of the population sorted best-first for selection */
    std::vector<std::size_t> rankPopulation() const;
    /** @brief Computes the NSGA-II crowding/non-dominated ranking order (best-first) */
    std::vector<std::size_t> rankPopulationPareto() const;
    /** @brief Determines whether individual a dominates individual b (Pareto sense) */
    bool aDominatesB(
        const gen::GOptimizableEntity &a,
        const gen::GOptimizableEntity &b
    ) const;
    /** @brief Performs the mean / sigma / C / path updates from the ranked offspring */
    void updateDistribution(const std::vector<std::size_t> &ranked);
    /** @brief Clamps a flat parameter vector to the [lower,upper) box */
    void clampToBox(std::vector<double> &x) const;

    /***************************************************************************/
    // User-facing strategy parameters (configured by the user / factory)

    std::size_t lambda_ = DEFAULTSEPCMALAMBDA; ///< Number of offspring per generation (0 = auto)
    std::size_t mu_ = 0;                        ///< Number of selected parents (0 = lambda/2)
    bool use_diagonal_cma_ = true;             ///< Whether to run the diagonal covariance update
    bool pareto_mode_ = false;                 ///< Whether to use NSGA-II-style MO selection
    double initial_sigma_ =
        0.3; ///< Initial global step size as a fraction of the parameter range

    /***************************************************************************/
    // Internal distribution state (initialized at setup, evolves each cycle)

    std::size_t n_ = 0;                ///< The number of (double) parameters
    double sigma_ = 0.;               ///< The current global step size
    std::vector<double> m_;          ///< The distribution mean
    std::vector<double> C_;          ///< The diagonal covariance (per-coordinate variance)
    std::vector<double> p_sigma_;    ///< The conjugate evolution path (CSA)
    std::vector<double> p_c_;        ///< The anisotropic evolution path (rank-1)
    std::vector<double> lower_;      ///< Lower parameter boundaries
    std::vector<double> upper_;      ///< Upper parameter boundaries
    bool state_initialized_ = false; ///< Whether the distribution state has been set up

    /***************************************************************************/
    // Derived strategy constants (computed once in setUpStrategyParameters())

    double mu_eff_ = 0.;             ///< Variance-effective selection mass
    double c_sigma_ = 0.;           ///< CSA step-size cumulation rate
    double d_sigma_ = 0.;           ///< CSA step-size damping
    double c_c_ = 0.;               ///< Cumulation rate for the rank-1 path
    double c_1_ = 0.;               ///< Rank-1 covariance learning rate
    double c_mu_ = 0.;              ///< Rank-mu covariance learning rate
    double chi_n_ = 0.;            ///< Expectation of ||N(0,I)||
    std::vector<double> weights_; ///< Positive recombination weights (length mu)
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::OptimizationAlgorithms::GSepCmaEvolutionStrategy) // NOLINT
