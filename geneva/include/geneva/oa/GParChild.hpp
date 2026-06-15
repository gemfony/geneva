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
#include <concepts>
#include <memory>
#include <tuple>
#include <type_traits>

// Boost headers go here

// Geneva headers go here
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/oa/GOptimizationAlgorithmBase.hpp"
#include "geneva/oa/GBaseParChildPersonalityTraits.hpp"

namespace Gem::Geneva::Parameters {
class GFlatGenome;
} // namespace Gem::Geneva::Parameters

namespace Gem::Geneva::OptimizationAlgorithms {

// The OA-owned adaption configuration, built from the population's shared genome layout (Phase 8).
class GAdaptionConfigBase;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The GParChildT class adds the notion of parents and children to
 * the GOptimizationAlgorithmBase class. The evolutionary adaptation is realized
 * through the cycle of adaption, evaluation, and sorting, as defined in this
 * class.
 *
 * It forms the base class for either multi populations (i.e. evolutionary algorithms
 * that may act on other optimization algorithms (including themselves), or a hierarchy of
 * algorithms acting on parameter objects.
 *
 * Populations are collections of individuals, which themselves are objects
 * exhibiting at least the GOptimizableEntity class' API, most notably the GOptimizableEntity::fitness()
 * and GOptimizableEntity::adapt() functions.
 *
 * In order to add parents to an instance of this class use the default constructor,
 * then add at least one GOptimizableEntity-derivative to it, and call setPopulationSizes().
 * The population will then be "filled up" with missing individuals as required, before the
 * optimization starts.
 */
class GParChild // NOLINT(cppcoreguidelines-special-member-functions)
  : public GOptimizationAlgorithmBase {
    /////////////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /** @brief Single declaration of this class'es local data members */
    auto localMembers() {
        return std::make_tuple(
            Gem::Common::make_member("n_parents_", n_parents_),
            Gem::Common::make_member("recombination_method_", recombination_method_),
            Gem::Common::make_member("default_n_children_", default_n_children_),
            Gem::Common::make_member("growth_rate_", growth_rate_),
            Gem::Common::make_member("max_population_size_", max_population_size_),
            Gem::Common::make_member("amalgamation_likelihood_", amalgamation_likelihood_)
        );
    }
    auto localMembers() const {
        return std::make_tuple(
            Gem::Common::make_member("n_parents_", n_parents_),
            Gem::Common::make_member("recombination_method_", recombination_method_),
            Gem::Common::make_member("default_n_children_", default_n_children_),
            Gem::Common::make_member("growth_rate_", growth_rate_),
            Gem::Common::make_member("max_population_size_", max_population_size_),
            Gem::Common::make_member("amalgamation_likelihood_", amalgamation_likelihood_)
        );
    }

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp("GOptimizationAlgorithmBase", boost::serialization::base_object<GOptimizationAlgorithmBase>(*this));
        // The member list is derived from the single localMembers() declaration,
        // emitting the same NVP names in the same order as the previous explicit list.
        Gem::Common::serialize_members(ar, this->localMembers());
    }
    /////////////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /** @brief The default constructor */
    GParChild();
    /** @brief A standard copy constructor */
    
    GParChild(const GParChild &cp) = default;
    /** @brief The standard destructor */
    ~GParChild() override = default;

    /** @brief  Specifies the default size of the population plus the number of parents */
    void setPopulationSizes(std::size_t pop_size, std::size_t n_parents);

    /** @brief Retrieve the number of parents as set by the user */
    std::size_t getNParents() const;
    /** @brief Calculates the current number of children from the number of parents and the size of the vector. */
    std::size_t getNChildren() const;
    /** @brief Retrieves the defaultNChildren_ parameter */
    std::size_t getDefaultNChildren() const;

    /** @brief Lets the user set the desired recombination method */
    void setRecombinationMethod(duplicationScheme recombination_method);

    /** @brief Retrieves the value of the recombinationMethod_ variable */
    duplicationScheme getRecombinationMethod() const;

    /** @brief Adds the option to increase the population by a given amount per iteration */
    void setPopulationGrowth(std::size_t growth_rate, std::size_t max_population_size);
    /** @brief Allows to retrieve the growth rate of the population */
    std::size_t getGrowthRate() const;
    /** @brief Allows to retrieve the maximum population size when growth is enabled */
    std::size_t getMaxPopulationSize() const;

    /** @brief Allows to set the likelihood for amalgamation of two units to be performed instead of "just" duplication. */
    void setAmalgamationLikelihood(double amalgamation_likelihood);
    /** @brief Allows to retrieve the likelihood for amalgamation of two units to be performed instead of "just" duplication. */
    double getAmalgamationLikelihood() const;

    /***************************************************************************/
    /**
     * Retrieves a specific parent individual and casts it to the desired type. Note that this
     * function will only be accessible to the compiler if individual_type is a derivative of GOptimizableEntity,
     * thanks to the magic of the std::enable_if and type_traits.
     *
     * @param parent_id The id of the parent that should be returned
     * @return A converted shared_ptr to the parent
     */
    template <typename parent_type>
        requires std::derived_from<parent_type, gpar::GOptimizableEntity>
    std::shared_ptr<parent_type> getParentIndividual(std::size_t parent_id) {
#ifdef DEBUG
        // Check that the parent id is in a valid range
        if(parent_id >= this->getNParents()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParChild::getParentIndividual<>() : Error" << '\n'
                << "Requested parent id which does not exist: " << parent_id << " / "
                << this->getNParents() << '\n'
            );

            // Make the compiler happy
            return std::shared_ptr<parent_type>();
        }
#endif /* DEBUG */

        // Does error checks on the conversion internally
        return Gem::Common::convertSmartPointer<gpar::GOptimizableEntity, parent_type>(
            *(this->begin() + parent_id)
        );
    }

    /** @brief Hands this mu/lambda algorithm an OA-owned adaption config to use (Phase 8 step 4). When
     *  set, init() adopts it (after validating it matches the population's genome) instead of deriving a
     *  default from the genome layout. Transient run scratch -- NOT serialized. */
    void setAdaptionConfig(std::shared_ptr<GAdaptionConfigBase> config) override;

protected:
    /***************************************************************************/
    // Virtual or overridden protected functions

    /** @brief Adds local configuration options to a GParserBuilder object */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) override;

    /** @brief Loads the data of another GParChildT object. */
    void load_(const GOptimizationAlgorithmBase *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GParChild>(
        GParChild const &,
        GParChild const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GOptimizationAlgorithmBase &cp,
        const Gem::Common::expectation &e,
        const double &limit
    ) const override;

    /** @brief The function checks that the population size meets the requirements and resizes the population to the appropriate size, if required. */
    void adjustPopulation_() override;

    /** @brief Resets the settings of this population to what was configured when the optimize()-call was issued */
    void resetToOptimizationStart_() override;

    /** @brief performs initialization work before the optimization loop starts */
    void init() override;
    /** @brief Does any necessary finalization work atfer the optimization loop has ended */
    void finalize() override;

    /** @brief Applies modifications to this object */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail */
    void specificTestsFailuresExpected_GUnitTests_() override;

    /***************************************************************************/

    /** @brief This function is called from GOptimizationAlgorithmBase::optimize() and performs the actual recombination */
    virtual void recombine();

    /** @brief Retrieves the adaption range in a given iteration and sorting scheme. */
    std::tuple<std::size_t, std::size_t> getAdaptionRange() const;

    /** @brief This helper function marks parents as parents and children as children. */
    void markParents();
    /** @brief This helper function marks children as children */
    void markChildren();
    /** @brief This helper function lets all individuals know about their position in the population. */
    void markIndividualPositions();

    /** @brief Increases the population size if requested by the user */
    void performScheduledPopulationGrowth();

    /** @brief This function implements the RANDOMDUPLICATIONSCHEME scheme */
    void randomRecombine(const std::unique_ptr<gpar::GIndividualSlot> &child);
    /** @brief  This function implements the VALUEDUPLICATIONSCHEME scheme */
    void
    valueRecombine(const std::unique_ptr<gpar::GIndividualSlot> &child, const std::vector<double> &threshold);

    /***************************************************************************/

    std::size_t n_parents_ = DEFPARCHILDNPARENTS; ///< The number of parents
    duplicationScheme recombination_method_ =
        duplicationScheme::DEFAULTDUPLICATIONSCHEME;         ///< The chosen recombination method
    std::size_t default_n_children_ = DEFPARCHILDNCHILDREN; ///< Expected number of children
    std::size_t growth_rate_ = 0; ///< Specifies the amount of individuals added per iteration
    std::size_t max_population_size_ =
        0; ///< Specifies the maximum amount of individuals in the population if growth is enabled
    double amalgamation_likelihood_ =
        DEFAULTAMALGAMATIONLIKELIHOOD; ///< Likelihood for children to be created by cross-over rather than "just" duplication (note that they may nevertheless be mutated)

    /***************************************************************************/
    /**
     * @brief Builds the OA-owned adaption configuration from a representative genome (all individuals in
     * the population share the same genome layout). EA and SA override this to return their respective
     * GEAAdaptionConfig / GSAAdaptionConfig; the base produces a plain GAdaptionConfigBase. Phase 8: the
     * config drives the data-oriented adaption free functions instead of the individual's adapt().
     */
    virtual std::shared_ptr<GAdaptionConfigBase> makeAdaptionConfig_(const gpar::GFlatGenome &genome) const;

    /**
     * @brief The OA-owned adaption configuration, rebuilt at init() from the population's shared genome
     * layout. Transient run scratch: NOT serialized, NOT compared, NOT part of localMembers(); a clone
     * rebuilds it at its own init(). Read-only during the parallel adaptChildren_, so it is shared safely.
     */
    std::shared_ptr<GAdaptionConfigBase> adaption_config_;

    /**
     * @brief An optionally externally-supplied adaption configuration (Phase 8 step 4, set via
     * setAdaptionConfig() -- Go2 installs the one it holds for this algorithm's type). When present, init()
     * uses it (validating it against the population's genome) in place of the layout-derived default.
     * Transient run scratch: NOT serialized, NOT compared, NOT part of localMembers().
     */
    std::shared_ptr<GAdaptionConfigBase> provided_adaption_config_;

private:
    /***************************************************************************/
    // Virtual or overridden private functions

    /** @brief Emits a name for this class / object */
    std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    GOptimizationAlgorithmBase *clone_() const override = 0;

    /** @brief This function implements the logic that constitutes evolutionary algorithms */
    std::tuple<double, double> cycleLogic_() override;
    /** @brief Calculates the fitness of all required individuals; to be re-implemented in derived classes */
    void runFitnessCalculation_() override = 0;

    /** @brief Returns the name of this optimization algorithm */
    std::string getAlgorithmName_() const override = 0;
    /** @brief Returns information about the type of optimization algorithm */
    std::string getAlgorithmPersonalityType_() const override = 0;

    /** @brief Retrieve the number of processible items in the current iteration. */
    std::size_t getNProcessableItems_() const override;

    /** @brief Gives individuals an opportunity to update their internal structures */
    void actOnStalls_() override;

    /** @brief Adapts all children of this population */
    virtual void adaptChildren_() = 0;
    /** @brief Choose new parents, based on the selection scheme set by the user */
    virtual void selectBest_() = 0;

    /** @brief Retrieves the evaluation range in a given iteration and sorting scheme */
    virtual std::tuple<std::size_t, std::size_t>
    getEvaluationRange_() const = 0; // Depends on selection scheme
    /** @brief Some error checks related to population sizes */
    virtual void
    populationSanityChecks_() const = 0; // TODO: Take code from old init() function

    /***************************************************************************/

    /** @brief This function assigns a new value to each child individual */
    void doRecombine();

    /***************************************************************************/
    // Data

    std::uniform_int_distribution<std::size_t>
        uniform_int_distribution_; ///< Access to uniformly distributed random numbers

    /***************************************************************************/
};

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::OptimizationAlgorithms::GParChild) // NOLINT
/******************************************************************************/
