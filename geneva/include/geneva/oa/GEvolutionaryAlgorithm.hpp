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
#include "geneva/oa/GEvolutionaryAlgorithm_PersonalityTraits.hpp"
#include "geneva/oa/GParChild.hpp"
#include "geneva/oa/GTunableManifest.hpp"
#include "geneva/oa/GOptimizationAlgorithmT.hpp"

#ifdef GEM_TESTING

#ifdef GEM_TESTING
#include "geneva/individuals/GTestIndividual1.hpp"
#endif /* GEM_TESTING */

#endif /* GEM_TESTING */

namespace Gem::Geneva::OptimizationAlgorithms {
/**
     * The default sorting mode
     */
constexpr auto DEFAULTEASORTINGMODE = sortingMode::MUCOMMANU_SINGLEEVAL;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
     * This is a specialization of the GParChildT<executor_type> class. The class adds
     * an infrastructure for evolutionary algorithms.
     */
class GEvolutionaryAlgorithm // NOLINT(cppcoreguidelines-special-member-functions)
  : public GOptimizationAlgorithmT<GEvolutionaryAlgorithm, GParChild> {
public:
    // Identifiers consumed by the GOptimizationAlgorithmT scaffold.
    static constexpr std::string_view oa_class_name = "GEvolutionaryAlgorithm";
    static constexpr std::string_view oa_algorithm_name = "Evolutionary Algorithm";
    static constexpr std::string_view oa_personality_type = "PERSONALITY_EA";

private:
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /** @brief Single declaration of this class'es local data members */
    auto localMembers() {
        return std::make_tuple(
            Gem::Common::make_member("sorting_mode_", sorting_mode_)
        );
    }
    auto localMembers() const {
        return std::make_tuple(
            Gem::Common::make_member("sorting_mode_", sorting_mode_)
        );
    }

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp("GParChild", boost::serialization::base_object<GParChild>(*this));
        // Member list derived from the single localMembers() declaration (same NVP
        // names/order as the previous explicit list).
        Gem::Common::serialize_members(ar, this->localMembers());
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /** @brief The default constructor */
    GEvolutionaryAlgorithm();
    /**
     * @brief A standard copy constructor.
     * @param The object to be copied
     */
    GEvolutionaryAlgorithm(const GEvolutionaryAlgorithm &) = default;
    /** @brief The standard destructor */
    ~GEvolutionaryAlgorithm() override = default;

    /**
     * @brief Sets the sorting scheme.
     * @param smode The selection/sorting scheme to use (e.g. mu+nu, mu,nu, munu1pretain)
     */
    void setSortingScheme(sortingMode smode);
    /**
     * @brief Retrieves information about the current sorting scheme.
     * @return The currently configured sorting/selection scheme
     */
    sortingMode getSortingScheme() const;

    /**
     * @brief Extracts all individuals on the pareto front.
     * @param pareto_inds Output vector that, on return, is filled with the individuals currently tagged as lying on the pareto front
     */
    void extractCurrentParetoIndividuals(
        std::vector<std::shared_ptr<gen::GOptimizableEntity>> &pareto_inds
    );

    /**
     * @brief The knobs a meta-optimizer may tune on an evolutionary algorithm, with default search
     * ranges. This is the single source of truth for which parameters exist, which value channel carries
     * each, and in what order -- a meta-optimizer builds its search genome from it (one labelled group
     * per descriptor) and reads values back by name, so no hand-maintained index can drift.
     * @return The ordered list of tunable parameters (population, cross-over, sigma / ad_prob knobs)
     */
    static std::vector<TunableParam> tunableManifest();

    /**
     * @brief Requests INLINE evaluation: the population is evaluated in the calling thread (each item's
     * process() run directly), bypassing the process-wide work consumer. Needed for a nested refinement
     * EA (e.g. the post-optimizer) that itself runs inside an individual's process() -- on a consumer
     * worker or a remote client -- where submitting to that same consumer would re-enter it or find none.
     * Transient execution choice (not serialized).
     * @param inln true to evaluate inline in the calling thread; false (default) to use the work consumer
     */
    void setInlineEvaluation(bool inln) { inline_evaluation_ = inln; }
    /** @brief Whether inline (in-thread) evaluation is enabled.
     *  @return true if the population is evaluated inline rather than through the work consumer */
    [[nodiscard]] bool getInlineEvaluation() const { return inline_evaluation_; }

protected:
    /***************************************************************************/
    // Virtual or overridden protected functions

    /**
     * @brief Adds local configuration options to a GParserBuilder object.
     * @param gpb The parser-builder to which this algorithm's configuration options are added
     */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) override;

    /**
     * @brief Loads the data of another GEvolutionaryAlgorithm object.
     * @param cp A pointer to the other object whose data is loaded into this one (downcast from GOptimizationAlgorithmBase)
     */
    void load_(const GOptimizationAlgorithmBase *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GEvolutionaryAlgorithm>(
        GEvolutionaryAlgorithm const &,
        GEvolutionaryAlgorithm const &,
        Gem::Common::GToken &
    );

    /**
     * @brief Searches for compliance with expectations with respect to another object of the same type.
     * @param cp The other object to compare against (downcast from GOptimizationAlgorithmBase)
     * @param e The expectation for this object, e.g. equality
     * @param limit The limit for allowed deviations of floating point types
     */
    void compare_(
        const GOptimizationAlgorithmBase &cp // the other object
        ,
        const Gem::Common::expectation &e // the expectation for this object, e.g. equality
        ,
        const double &limit // the limit for allowed deviations of floating point types
    ) const override;

    /** @brief Resets the settings of this population to what was configured when the optimize()-call was issued */
    void resetToOptimizationStart_() override;

    /** @brief Applies modifications to this object */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

    /***************************************************************************/

private:
    /***************************************************************************/
    // Overloaded or virtual base functions

    // name_(), clone_(), getAlgorithmName_() and getAlgorithmPersonalityType_() are generated by the
    // GOptimizationAlgorithmT scaffold from the oa_* identifiers above. (The GUnitTests methods are
    // overridden below, since EA has real algorithm-specific tests.)

    /** @brief We submit individuals to the broker connector and wait for processed items */
    void runFitnessCalculation_() override;

    /**
     * @brief Evaluates the population's [start, end) range and returns the executor status. The default
     * routes through the process's work consumer (workOnPopulation); a subclass may override this to
     * evaluate elsewhere (e.g. GMetaEvolutionaryAlgorithm runs its umbrella-individuals on its own
     * orchestration thread pool, leaving the work consumer to the sub-optimizations they spawn).
     * @param start First population index to evaluate (inclusive)
     * @param end One past the last population index to evaluate
     * @return The executor status (completeness + error flags) for the evaluated range
     */
    virtual Gem::Courtier::executor_status_t
    evaluatePopulationRange_(std::size_t start, std::size_t end);

    /**
     * @brief Adds the individuals of this iteration to a priority queue holding the global bests.
     * @param best_individuals The fixed-size priority queue of global best individuals to update
     */
    void updateGlobalBestsPQ_(gen::GOptimizableEntityFixedSizePriorityQueue &best_individuals) override;
    /**
     * @brief Adds the individuals of this iteration to a priority queue holding this iteration's bests.
     * @param best_individuals The fixed-size priority queue of this iteration's best individuals to update
     */
    void updateIterationBestsPQ_(gen::GOptimizableEntityFixedSizePriorityQueue &best_individuals) override;

    /**
     * @brief Retrieve a GPersonalityTraits object belonging to this algorithm.
     * @return A shared pointer to a freshly created personality-traits object for this algorithm
     */
    std::shared_ptr<GPersonalityTraits> getPersonalityTraits_() const override;

    /** @brief Choose new parents, based on the selection scheme set by the user */
    void selectBest_() override;

    /** @brief Some error checks related to population sizes */
    void populationSanityChecks_() const override;
    /**
     * @brief Retrieves the evaluation range in a given iteration and sorting scheme.
     * @return A tuple holding the [start, end) index range of individuals that need to be (re-)evaluated
     */
    std::tuple<std::size_t, std::size_t> getEvaluationRange_() const override;

    /***************************************************************************/

    /** @brief Selection, MUPLUSNU_SINGLEEVAL style */
    void sortMuPlusNuMode();
    /** @brief Selection, MUCOMMANU_SINGLEEVAL style */
    void sortMuCommaNuMode();
    /** @brief Selection, MUNU1PRETAIN_SINGLEEVAL style */
    void sortMunu1pretainMode();

    /** @brief Selection according to the pareto tag, also taking into account the parents of a population (i.e. in MUPLUSNU mode). */
    void sortMuPlusNuParetoMode();
    /** @brief Selection according to the pareto tag, not taking into account the parents of a population (i.e. in MUCOMMANU mode). */
    void sortMuCommaNuParetoMode();
    /**
     * @brief Determines whether the first individual dominates the second.
     * @param a The first individual (the potential dominator)
     * @param b The second individual (the potentially dominated one)
     * @return true if individual a dominates individual b, false otherwise
     */
    bool aDominatesB(
        const std::unique_ptr<gen::GOptimizableEntity> &a,
        const std::unique_ptr<gen::GOptimizableEntity> &b
    ) const;

    /**
     * @brief Fills the collection with individuals.
     * @param n_individuals The number of individuals to add to the collection
     */
    void fillWithObjects(const std::size_t &n_individuals);

    /***************************************************************************/
    // Local data

    sortingMode sorting_mode_ = DEFAULTEASORTINGMODE; ///< The chosen sorting scheme

    /** @brief Evaluate the population inline (in-thread) instead of via the work consumer. Transient
     *  execution choice for nested refiners (see setInlineEvaluation()); not serialized/compared. */
    bool inline_evaluation_ = false;

    /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief Allows to output this population. The function only outputs the parent individuals' fitness.
 * @param os The output stream to write to
 * @param pop The evolutionary-algorithm population to stream
 * @return A reference to the output stream, for chaining
 */
std::ostream &operator<<(std::ostream &os, const GEvolutionaryAlgorithm &pop);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
} /* namespace Gem::Geneva::OptimizationAlgorithms */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::OptimizationAlgorithms::GEvolutionaryAlgorithm) // NOLINT

