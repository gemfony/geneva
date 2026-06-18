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
#include <tuple>
#include <vector>

// Boost headers go here

// Geneva headers go here
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/oa/GAdaptionConfig.hpp"
#include "geneva/oa/GAdaptiveEvolutionaryAlgorithm_PersonalityTraits.hpp"
#include "geneva/oa/GParChild.hpp"
#include "geneva/oa/GTunableManifest.hpp"
#include "geneva/oa/GOptimizationAlgorithmT.hpp"

#ifdef GEM_TESTING
#include "geneva/individuals/GTestIndividual1.hpp"
#endif /* GEM_TESTING */

namespace Gem::Geneva::OptimizationAlgorithms {
/**
 * The default sorting mode (matches the classic EA)
 */
constexpr auto DEFAULTEAASORTINGMODE = sortingMode::MUCOMMANU_SINGLEEVAL;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief A dimension-aware adaptive evolutionary algorithm ("eaa").
 *
 * This is structurally a copy of GEvolutionaryAlgorithm (the same (mu,lambda) / (mu+lambda) machinery,
 * the same five sorting modes incl. the two Pareto modes), with one addition: a `stepControl` strategy
 * read from the OA-owned adaption config that fixes the classic algorithm's poor FINE convergence at
 * high dimension. The stock EA's mutative self-adaptive isotropic Gaussian uses a FIXED sigma self-
 * adaption rate (default 0.8) that is never scaled by the parameter count n -- ~100x too hot at n=10000,
 * so far from the optimum coarse convergence works but near the optimum the over-hot log-normal sigma
 * update random-walks sigma instead of letting it settle and fine convergence stalls. The modes:
 *
 * - SELF_ADAPT        : classic behaviour (identical to GEvolutionaryAlgorithm).
 * - SELF_ADAPT_SCALED : sigma_sigma rescaled to the textbook tau = c/sqrt(2n) (shared sigma) or
 *                       c/sqrt(2*sqrt(n)) (per-coordinate). THE DEFAULT for this class -- its reason to
 *                       exist.
 * - ONE_FIFTH         : a single global sigma driven by the Rechenberg 1/5 success rule.
 * - CSA               : a single global sigma driven by cumulative step-size adaptation (evolution path).
 *
 * In addition, the algorithm intermediate-recombines the per-individual sigma (averages the surviving
 * parents' sigma into every child after recombination) for the per-individual-sigma modes; this is the
 * sigma-recombination the classic GParChild::doRecombine lacks (it recombines VALUES only).
 *
 * The choice of strategy lives on the GAdaptionConfig (an OA-owned object), so the same structure-only
 * genome can be driven by either the classic EA or this one without change.
 */
class GAdaptiveEvolutionaryAlgorithm // NOLINT(cppcoreguidelines-special-member-functions)
  : public GOptimizationAlgorithmT<GAdaptiveEvolutionaryAlgorithm, GParChild> {
public:
    // Identifiers consumed by the GOptimizationAlgorithmT scaffold.
    static constexpr std::string_view oa_class_name = "GAdaptiveEvolutionaryAlgorithm";
    static constexpr std::string_view oa_algorithm_name = "Adaptive Evolutionary Algorithm";
    static constexpr std::string_view oa_personality_type = "PERSONALITY_EAA";

private:
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /** @brief Single declaration of this class'es local data members */
    auto localMembers() {
        return std::make_tuple(
            Gem::Common::make_member("sorting_mode_", sorting_mode_),
            Gem::Common::make_member("step_control_", step_control_),
            Gem::Common::make_member("learning_rate_c_", learning_rate_c_),
            Gem::Common::make_member("recombine_sigma_", recombine_sigma_)
        );
    }
    auto localMembers() const {
        return std::make_tuple(
            Gem::Common::make_member("sorting_mode_", sorting_mode_),
            Gem::Common::make_member("step_control_", step_control_),
            Gem::Common::make_member("learning_rate_c_", learning_rate_c_),
            Gem::Common::make_member("recombine_sigma_", recombine_sigma_)
        );
    }

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp("GParChild", boost::serialization::base_object<GParChild>(*this));
        Gem::Common::serialize_members(ar, this->localMembers());
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /** @brief The default constructor */
    GAdaptiveEvolutionaryAlgorithm();
    /** @brief A standard copy constructor. @param The object to be copied */
    GAdaptiveEvolutionaryAlgorithm(const GAdaptiveEvolutionaryAlgorithm &) = default;
    /** @brief The standard destructor */
    ~GAdaptiveEvolutionaryAlgorithm() override = default;

    /** @brief Sets the sorting scheme. @param smode The selection/sorting scheme to use */
    void setSortingScheme(sortingMode smode);
    /** @brief Retrieves the current sorting scheme. @return The currently configured sorting scheme */
    sortingMode getSortingScheme() const;

    /** @brief Sets the step-size-control strategy. The default is SELF_ADAPT_SCALED.
     *  @param sc The strategy (SELF_ADAPT, SELF_ADAPT_SCALED, ONE_FIFTH or CSA) */
    void setStepControl(stepControl sc);
    /** @brief Retrieves the step-size-control strategy. @return The configured strategy. */
    stepControl getStepControl() const;

    /** @brief Sets the learning-rate constant c for SELF_ADAPT_SCALED (tau = c/sqrt(2n)).
     *  @param c The constant (≈1 by convention). */
    void setLearningRateConstant(double c);
    /** @brief Retrieves the learning-rate constant c. @return The configured constant. */
    double getLearningRateConstant() const;

    /** @brief Enables/disables intermediate recombination of the per-individual sigma.
     *  @param r true to average parents' sigma into children after recombination. */
    void setSigmaRecombination(bool r);
    /** @brief Whether intermediate sigma-recombination is enabled. @return true if enabled. */
    bool getSigmaRecombination() const;

    /** @brief Extracts all individuals on the pareto front.
     *  @param pareto_inds Output vector filled with the individuals currently tagged as on the front */
    void extractCurrentParetoIndividuals(
        std::vector<std::shared_ptr<gen::GOptimizableEntity>> &pareto_inds
    );

    /** @brief The knobs a meta-optimizer may tune. @return The ordered list of tunable parameters. */
    static std::vector<TunableParam> tunableManifest();

    /** @brief Requests INLINE evaluation (see GEvolutionaryAlgorithm). @param inln true to evaluate inline */
    void setInlineEvaluation(bool inln) { inline_evaluation_ = inln; }
    /** @brief Whether inline (in-thread) evaluation is enabled. @return true if inline */
    [[nodiscard]] bool getInlineEvaluation() const { return inline_evaluation_; }

protected:
    /***************************************************************************/
    // Virtual or overridden protected functions

    /** @brief Adds local configuration options. @param gpb The parser-builder */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) override;

    /** @brief Loads the data of another GAdaptiveEvolutionaryAlgorithm. @param cp The other object */
    void load_(const GOptimizationAlgorithmBase *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GAdaptiveEvolutionaryAlgorithm>(
        GAdaptiveEvolutionaryAlgorithm const &,
        GAdaptiveEvolutionaryAlgorithm const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations. @param cp other @param e expectation @param limit fp limit */
    void compare_(
        const GOptimizationAlgorithmBase &cp,
        const Gem::Common::expectation &e,
        const double &limit
    ) const override;

    /** @brief Resets the settings to what was configured at the optimize()-call */
    void resetToOptimizationStart_() override;

    /** @brief Performs initialization work before the optimization loop starts (installs the step controller). */
    void init() override;

    /** @brief Recombination + (optionally) intermediate sigma-recombination of children. */
    void recombine() override;

    /** @brief Applies modifications to this object */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail */
    void specificTestsFailuresExpected_GUnitTests_() override;

    /***************************************************************************/

private:
    /***************************************************************************/
    // Overloaded or virtual base functions (mirror GEvolutionaryAlgorithm).

    /** @brief We submit individuals to the broker connector and wait for processed items */
    void runFitnessCalculation_() override;

    /** @brief Evaluates the population's [start, end) range and returns the executor status. */
    virtual Gem::Courtier::executor_status_t
    evaluatePopulationRange_(std::size_t start, std::size_t end);

    /** @brief Adds the iteration's individuals to the global-best priority queue. */
    void updateGlobalBestsPQ_(gen::GOptimizableEntityFixedSizePriorityQueue &best_individuals) override;
    /** @brief Adds the iteration's individuals to this iteration's best priority queue. */
    void updateIterationBestsPQ_(gen::GOptimizableEntityFixedSizePriorityQueue &best_individuals) override;

    /** @brief Retrieve a personality-traits object belonging to this algorithm. */
    std::shared_ptr<GPersonalityTraits> getPersonalityTraits_() const override;

    /** @brief Choose new parents, based on the selection scheme set by the user (then drive the controller). */
    void selectBest_() override;

    /** @brief Some error checks related to population sizes */
    void populationSanityChecks_() const override;
    /** @brief Retrieves the evaluation range in a given iteration and sorting scheme. */
    std::tuple<std::size_t, std::size_t> getEvaluationRange_() const override;

    /***************************************************************************/
    // Step-control machinery (the addition over GEvolutionaryAlgorithm).

    /** @brief Installs the step-size controller onto the OA-owned adaption config at init() (rescales /
     *  suppresses self-adaption per the chosen mode) and seeds the controller's global-sigma state. */
    void installStepController();
    /** @brief Drives the global-sigma controller (ONE_FIFTH / CSA) once per generation after selection,
     *  then pushes the updated global sigma into every population slot's scratch. A no-op for the
     *  self-adaptive modes. */
    void driveGlobalSigmaController();

    /***************************************************************************/
    // Selection helpers (verbatim from GEvolutionaryAlgorithm so all five sorting modes work, Pareto incl.).

    void sortMuPlusNuMode();
    void sortMuCommaNuMode();
    void sortMunu1pretainMode();
    void sortMuPlusNuParetoMode();
    void sortMuCommaNuParetoMode();
    bool aDominatesB(
        const std::unique_ptr<gen::GOptimizableEntity> &a,
        const std::unique_ptr<gen::GOptimizableEntity> &b
    ) const;

    void fillWithObjects(const std::size_t &n_individuals);

    /***************************************************************************/
    // Local data

    sortingMode sorting_mode_ = DEFAULTEAASORTINGMODE; ///< The chosen sorting scheme

    /** @brief The step-size-control strategy applied to the adaption config (default SELF_ADAPT_SCALED). */
    stepControl step_control_ = stepControl::SELF_ADAPT_SCALED;
    /** @brief The learning-rate constant c for SELF_ADAPT_SCALED (tau = c/sqrt(2n)). */
    double learning_rate_c_ = 1.;
    /** @brief Whether to intermediate-recombine the per-individual sigma after recombination. */
    bool recombine_sigma_ = true;

    /** @brief Evaluate the population inline (transient; not serialized/compared). */
    bool inline_evaluation_ = false;

    /***************************************************************************/
    // Transient run scratch for the global-sigma controllers (ONE_FIFTH / CSA). NOT serialized / compared
    // / part of localMembers(): rebuilt at init() from the seed sigma in the adaption config.
    double global_sigma_ = 1.;       ///< the single global step size (ONE_FIFTH / CSA)
    double p_sigma_ = 0.;            ///< the CSA evolution-path accumulator (scalar proxy)
    double prev_best_fitness_ = 0.;  ///< the previous generation's best transformed (min-only) fitness
    bool   have_prev_best_ = false;  ///< whether prev_best_fitness_ is meaningful yet
    std::size_t controller_dim_ = 0; ///< the adapted dimension n the controller reasons about

    /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/** @brief Streams a summary of the parent individuals' fitness values. */
std::ostream &operator<<(std::ostream &os, const GAdaptiveEvolutionaryAlgorithm &pop);

/******************************************************************************/
} /* namespace Gem::Geneva::OptimizationAlgorithms */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::OptimizationAlgorithms::GAdaptiveEvolutionaryAlgorithm) // NOLINT
