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
  : public GParChild {
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
    /** @brief A standard copy constructor */
    GEvolutionaryAlgorithm(const GEvolutionaryAlgorithm &) = default;
    /** @brief The standard destructor */
    ~GEvolutionaryAlgorithm() override = default;

    /** @brief Sets the sorting scheme */
    void setSortingScheme(sortingMode smode);
    /** @brief Retrieves information about the current sorting scheme */
    sortingMode getSortingScheme() const;

    /** @brief Extracts all individuals on the pareto front */
    void extractCurrentParetoIndividuals(
        std::vector<std::shared_ptr<gpar::GOptimizableEntity>> &pareto_inds
    );

protected:
    /***************************************************************************/
    // Virtual or overridden protected functions

    /** @brief Adds local configuration options to a GParserBuilder object */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) override;

    /** @brief Loads the data of another GEvolutionaryAlgorithm object */
    void load_(const GOptimizationAlgorithmBase *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GEvolutionaryAlgorithm>(
        GEvolutionaryAlgorithm const &,
        GEvolutionaryAlgorithm const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
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

    /** @brief Emits a name for this class / object */
    std::string name_() const override;
    /** @brief Creates a deep copy of this object */
    GOptimizationAlgorithmBase *clone_() const override;

    /** @brief We submit individuals to the broker connector and wait for processed items */
    void runFitnessCalculation_() override;

    /** @brief Returns information about the type of optimization algorithm. */
    std::string getAlgorithmPersonalityType_() const override;
    /** @brief Returns the name of this optimization algorithm */
    std::string getAlgorithmName_() const override;

    /** @brief Adds the individuals of this iteration to a priority queue */
    void updateGlobalBestsPQ_(gpar::GOptimizableEntityFixedSizePriorityQueue &best_individuals) override;
    /** @brief Adds the individuals of this iteration to a priority queue */
    void updateIterationBestsPQ_(gpar::GOptimizableEntityFixedSizePriorityQueue &best_individuals) override;

    /** @brief Retrieve a GPersonalityTraits object belonging to this algorithm */
    std::shared_ptr<GPersonalityTraits> getPersonalityTraits_() const override;

    /** @brief Choose new parents, based on the selection scheme set by the user */
    void selectBest_() override;

    /** @brief Some error checks related to population sizes */
    void populationSanityChecks_() const override;
    /** @brief Retrieves the evaluation range in a given iteration and sorting scheme */
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
    /** @brief Determines whether the first individual dominates the second */
    bool aDominatesB(
        const std::unique_ptr<gpar::GOptimizableEntity> &a,
        const std::unique_ptr<gpar::GOptimizableEntity> &b
    ) const;

    /** @brief Fills the collection with individuals */
    void fillWithObjects(const std::size_t &n_individuals);

    /***************************************************************************/
    // Local data

    sortingMode sorting_mode_ = DEFAULTEASORTINGMODE; ///< The chosen sorting scheme

    /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
         * Allows to output this population. The function only outputs the parent individuals' fitness.
         */
std::ostream &operator<<(std::ostream &os, const GEvolutionaryAlgorithm &pop);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
} /* namespace Gem::Geneva::OptimizationAlgorithms */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::OptimizationAlgorithms::GEvolutionaryAlgorithm) // NOLINT

