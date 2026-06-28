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

// Standard header files go here
#include <cstdint>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

// Boost header files go here
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ptree_serialization.hpp>

// Geneva headers go here
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GSerializableFunctionObjectT.hpp"
#include "common/GSerializationHelperFunctionsT.hpp" // serialization of std::chrono time_point (GProcessable timing)
#include "common/GSerializeTupleT.hpp"               // serialization of std::tuple (best_past_primary_fitness_)
#include "courtier/GProcessable.hpp" // the non-generic processing-lifecycle base
#include "geneva/GMultiConstraintT.hpp" // GPreEvaluationValidityCheckT (registered on the shared policy)
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/Interface/GRateableI.hpp"
#include "geneva/ind/GIndividualProcessingResult.hpp"
#include "geneva/ind/GProblemPolicy.hpp"
#include "hap/GRandomT.hpp"

// aliases for ease of use
namespace pt = boost::property_tree;

namespace Gem::Geneva::Genome {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief The algorithm-facing base class of all optimizable individuals: the genome-agnostic assembly the
 * optimization algorithms hold their population as.
 *
 * GOptimizableEntity combines three concerns into one object without knowing how the parameters are
 * stored:
 *   - the courtier processing LIFECYCLE (status / routing / timing), inherited from GProcessable;
 *   - IDENTITY and the clone/compare/serialize category machinery, via GCommonInterfaceT;
 *   - EVALUATABILITY, via GRateableI (the raw / transformed fitness accessors + the fitnessCalculation()
 *     hook the user implements).
 *
 * On top of those it owns the RESULT STORE (one individual_processing_result per fitness criterion), the
 * process() orchestration that drives an evaluation (timing, the configured pre-/post-processors, the
 * feasibility check and the evaluation-policy transform), and the per-individual feasibility STATE (the
 * computed validity level, the best-known fitness and stall counters). The population-uniform feasibility
 * and ranking RULES -- the optimization direction, the policy for invalid solutions, the sigmoid
 * parameters and the constraint object -- are NOT stored per individual; they live in a single shared
 * GProblemPolicy that every entity references (the 1:N policy), so feasibility (fulfillsConstraints) is a
 * concrete base operation that reads this entity's values through the policy's constraint.
 *
 * The algorithms reach the parameters THROUGH THIS BASE: the genome value API (streamline<T> /
 * assignValueVector<T> / countParameters<T> / boundaries<T> + the FP / internal views + getVarVal<T> +
 * toPropertyTree / toCSV / crossOverWith / cannibalize) is declared here, dispatching to pure-virtual
 * hooks that the value-bearing genome layer (GFlatGenome) implements. "Read my parameters as a vector" is
 * a universal optimization operation; only the storage is genome-specific. This keeps the assembly
 * representation-agnostic: a future non-flat genome would derive GOptimizableEntity directly.
 */
class GOptimizableEntity // NOLINT(cppcoreguidelines-special-member-functions)
  : public Gem::Courtier::GProcessable
  , public Gem::Common::GCommonInterfaceT<GOptimizableEntity>
  , public Interface::GRateableI {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /**
     * @brief Single declaration of this class's plain compared/loaded local data members (the
     * pre-/post-processor veto flags and the per-individual feasibility / best-known state), driving
     * compare_(), the plain-member half of serialize() and load_() from one source. The result store,
     * the cloneable pre-/post-processors and the shared policy are handled separately (the result store is
     * serialized but not compared; the policy is shared 1:N, copied by shared-pointer), so they are NOT
     * listed here.
     *
     * @tparam Self The (const or non-const) deduced type of *this
     * @param self A reference to *this whose members are tied into the tuple
     * @return A tuple of named member references driving serialize(), load_() and compare_()
     */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(
            Gem::Common::make_member("pre_processing_disabled_", self.pre_processing_disabled_),
            Gem::Common::make_member("post_processing_disabled_", self.post_processing_disabled_),
            Gem::Common::make_member("best_past_primary_fitness_", self.best_past_primary_fitness_),
            Gem::Common::make_member("n_stalls_", self.n_stalls_),
            Gem::Common::make_member("assigned_iteration_", self.assigned_iteration_),
            Gem::Common::make_member("validity_level_", self.validity_level_),
            Gem::Common::make_member("n_adaptions_", self.n_adaptions_),
            Gem::Common::make_member("max_unsuccessful_adaptions_", self.max_unsuccessful_adaptions_),
            Gem::Common::make_member("max_retries_until_valid_", self.max_retries_until_valid_)
        );
    }

    /**
     * @brief Serializes this object to/from a Boost archive.
     *
     * The non-generic lifecycle state is serialized through the GProcessable base; the CRTP category
     * root (GCommonInterfaceT) and GRateableI carry no state. The cloneable pre-/post-processors and the
     * shared policy are serialized explicitly (boost shared-pointer tracking deduplicates a policy shared
     * by many candidates within one archive), and the plain local members come from localMembers_().
     *
     * @tparam Archive The Boost.Serialization archive type
     * @param ar The archive to read from or write to
     * @param version The serialization version (unused)
     */
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GProcessable",
            boost::serialization::base_object<Gem::Courtier::GProcessable>(*this)
        );

        // The result store is serialized (it travels on the wire) but is deliberately NOT among the
        // compared members (results are not part of per-individual identity, matching the historical
        // processing container), so it is handled here rather than in localMembers_().
        ar &BOOST_SERIALIZATION_NVP(pre_processor_ptr_) &
            BOOST_SERIALIZATION_NVP(post_processor_ptr_) &
            BOOST_SERIALIZATION_NVP(policy_) &
            BOOST_SERIALIZATION_NVP(stored_results_cnt_);

        Gem::Common::serialize_members(ar, localMembers_(*this));
    }
    ///////////////////////////////////////////////////////////////////////

public:
    using payload_type = GOptimizableEntity;
    using result_type = individual_processing_result;

    /** @brief The default constructor (a single fitness criterion, a fresh private policy) */
    GOptimizableEntity();
    /**
     * @brief Initialization with the number of fitness criteria.
     * @param n_fitness_criteria The number of fitness criteria this candidate evaluates to
     */
    explicit GOptimizableEntity(std::size_t n_fitness_criteria);
    /**
     * @brief The copy constructor. The shared policy is copied by shared pointer (1:N), the
     * pre-/post-processors are deep-cloned.
     * @param cp The other candidate whose data is copied
     */
    GOptimizableEntity(GOptimizableEntity const &cp);
    /** @brief The destructor */
    ~GOptimizableEntity() override = default;

    /***************************************************************************/
    // Processing: the result store + the evaluation orchestration.

    /**
     * @brief Performs the evaluation of this candidate: drives the optional pre-processor, the feasibility
     * check and fitness calculation (or adopts pre-computed raw results from res_vec), the evaluation-policy
     * transform and the optional post-processor, measuring the time of each step. On an exception the
     * fitnesses are set to the worst case and a processing exception is rethrown. Only an item with the
     * DO_PROCESS status is accepted.
     * @param res_vec Optional pre-computed raw results (e.g. from a remote/GPU evaluator); if empty,
     *        fitnessCalculation() is invoked. Its size must match the criteria count.
     * @return The first stored result after processing
     */
    individual_processing_result process(
        const std::vector<individual_processing_result> &res_vec =
            std::vector<individual_processing_result>()
    );

    /**
     * @brief Sets the vector of stored results to a given collection and marks the candidate PROCESSED.
     * @param result_cnt The new result vector (size must match the configured number of stored results)
     * @return The first stored result after the assignment
     */
    individual_processing_result
    markAsProcessedWith(std::vector<individual_processing_result> const &result_cnt);

    /**
     * @brief Read-only retrieval of a stored result. Throws if the PROCESSED flag is not set.
     * @param id The position of the stored result to return
     * @return The stored result at position id
     */
    individual_processing_result getStoredResult(std::size_t id = 0) const;

    /** @brief @return The number of result slots held by this candidate */
    std::size_t getNStoredResults() const { return stored_results_cnt_.size(); }

    /***************************************************************************/
    // Pre-/post-processing (used by nested / post-optimizing algorithms).

    /** @brief @return true if pre-processing is currently allowed (not vetoed) */
    bool mayBePreProcessed() const noexcept { return not pre_processing_disabled_; }
    /** @brief Allow or veto pre-processing. @param veto true to disable, false to allow */
    void vetoPreProcessing(bool veto) noexcept { pre_processing_disabled_ = veto; }
    /** @brief Registers a pre-processor (ignored if empty). @param pre_processor_ptr The processor */
    void registerPreProcessor(
        std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GOptimizableEntity>> pre_processor_ptr
    ) {
        if(pre_processor_ptr) {
            pre_processor_ptr_ = pre_processor_ptr;
        }
    }

    /** @brief @return true if post-processing is currently allowed (not vetoed) */
    bool mayBePostProcessed() const { return not post_processing_disabled_; }
    /** @brief Allow or veto post-processing. @param veto true to disable, false to allow */
    void vetoPostProcessing(bool veto) { post_processing_disabled_ = veto; }
    /** @brief Registers a post-processor (ignored if empty). @param post_processor_ptr The processor */
    void registerPostProcessor(
        std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GOptimizableEntity>> post_processor_ptr
    ) {
        if(post_processor_ptr) {
            post_processor_ptr_ = post_processor_ptr;
        }
    }
    /** @brief @return The registered post-processor, or an empty pointer if none is registered */
    std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GOptimizableEntity>>
    postProcessor() const {
        return post_processor_ptr_;
    }
    /** @brief Removes any registered post-processor */
    void clearPostProcessor() { post_processor_ptr_.reset(); }

    /***************************************************************************/
    // Results-only return hooks (the genome overrides these).

    /** @brief @return true if this (deserialized) item arrived without its input data (results-only return) */
    bool inputDataOmitted() const { return this->inputDataOmitted_(); }
    /** @brief Grafts the input data of @p original onto this results-only item.
     *  @param original The originally-submitted item supplying the input data */
    void graftInputDataFrom(const GOptimizableEntity &original) {
        this->graftInputDataFrom_(original);
    }

    /** @brief Loads otherwise-constant data into this (freshly de-serialized) item from a template held at
     *  a remote site, so that data need not travel with every work item (a networked-client convenience;
     *  the default is a no-op, a derived type overrides the hook below if it carries such data).
     *  @param cd_ptr A template item whose constant data is loaded into this one */
    void loadConstantData(std::shared_ptr<GOptimizableEntity> cd_ptr) {
        this->loadConstantData_(cd_ptr);
    }

    /***************************************************************************/
    // Fitness accessors / multi-criterion support.

    /**
     * @brief Registers a (raw) result value of the fitness calculation at a given criterion position.
     * @param id The position of the fitness criterion (must be < the criteria count)
     * @param value The raw fitness value to register
     */
    void setResult(std::size_t id, double value);
    /** @brief @return true if more than one fitness criterion is present */
    bool hasMultipleFitnessCriteria() const { return this->getNStoredResults() > 1; }
    /**
     * @brief Retrieve the (raw, transformed) fitness tuple at a given evaluation position.
     * @param id The evaluation position (fitness criterion index); defaults to 0
     * @return A (raw, transformed) fitness tuple at the requested position
     */
    std::tuple<double, double> getFitnessTuple(std::uint32_t id = 0) const;
    /**
     * @brief Checks whether this candidate is at least as good as a set of raw boundaries.
     * @param boundaries One boundary value per fitness criterion
     * @return true if every raw fitness is at least as good as its boundary, false otherwise
     */
    bool isGoodEnough(std::vector<double> const &boundaries);

    /**
     * @brief Sets the fitness from a vector of externally-computed raw values (e.g. the GPU consumer),
     * applying the feasibility check and evaluation-policy transform, then marking the candidate PROCESSED.
     * @param f_cnt A vector of raw fitness values (size must match the criteria count)
     */
    void setFitness_(std::vector<double> const &f_cnt);

    /***************************************************************************/
    // Policy-derived accessors (forwarded to the shared GProblemPolicy).

    /** @brief Installs the shared problem policy (the 1:N feasibility/ranking rules).
     *  @param policy The shared policy to reference (must not be empty) */
    void setPolicy(std::shared_ptr<GProblemPolicy> policy);
    /** @brief @return The shared problem policy referenced by this candidate */
    std::shared_ptr<GProblemPolicy> getPolicy() const { return policy_; }

    /** @brief Sets the optimization direction on the shared policy. @param mode MAXIMIZE or MINIMIZE */
    void setMaxMode(maxMode const &mode) { policy_->setMaxMode(mode); }
    /** @brief @return The optimization direction from the shared policy */
    maxMode getMaxMode() const { return policy_->getMaxMode(); }
    /** @brief @return The worst-case evaluation value for the current direction */
    virtual double getWorstCase() const { return policy_->getWorstCase(); }
    /** @brief @return The best-case evaluation value for the current direction */
    virtual double getBestCase() const { return policy_->getBestCase(); }

    /** @brief Sets the policy for invalid solutions. @param eval_policy The evaluation policy */
    void setEvaluationPolicy(evaluationPolicy eval_policy) { policy_->setEvaluationPolicy(eval_policy); }
    /** @brief @return The evaluation policy for invalid solutions */
    evaluationPolicy getEvaluationPolicy() const { return policy_->getEvaluationPolicy(); }

    /** @brief @return The sigmoid steepness from the shared policy */
    double getSteepness() const { return policy_->getSteepness(); }
    /** @brief Sets the sigmoid steepness on the shared policy. @param steepness The steepness (> 0) */
    void setSteepness(double steepness) { policy_->setSteepness(steepness); }
    /** @brief @return The sigmoid barrier from the shared policy */
    double getBarrier() const { return policy_->getBarrier(); }
    /** @brief Sets the sigmoid barrier on the shared policy. @param barrier The barrier (> 0) */
    void setBarrier(double barrier) { policy_->setBarrier(barrier); }

    /** @brief @return The computed validity level of this candidate (<= 1 means feasible) */
    double getValidityLevel() const { return validity_level_; }
    /** @brief @return true if this candidate fulfils its constraints (validity level <= 1) */
    bool constraintsFulfilled() const { return validity_level_ <= 1.; }
    /** @brief @return true if this candidate is a valid solution (meant for processed candidates) */
    bool isValid() const;
    /** @brief @return true if this candidate is an invalid solution */
    bool isInValid() const { return not this->isValid(); }

    /***************************************************************************/
    // Best-known fitness / stall / iteration bookkeeping (per individual; moved to the OA in a later step).

    /** @brief Sets the globally best known primary fitness. @param bnf The (raw, transformed) tuple */
    void setBestKnownPrimaryFitness(std::tuple<double, double> const &bnf) {
        best_past_primary_fitness_ = bnf;
    }
    /** @brief @return The globally best known primary fitness, as a (raw, transformed) tuple */
    std::tuple<double, double> getBestKnownPrimaryFitness() const { return best_past_primary_fitness_; }

    /** @brief Sets the parent algorithm's iteration. @param parent_alg_iteration The iteration */
    void setAssignedIteration(std::uint32_t const &parent_alg_iteration) {
        assigned_iteration_ = parent_alg_iteration;
    }
    /** @brief @return The parent algorithm's current iteration */
    std::uint32_t getAssignedIteration() const { return assigned_iteration_; }

    /** @brief Sets the number of stalled optimization cycles. @param n_stalls The stall count */
    void setNStalls(std::uint32_t const &n_stalls) { n_stalls_ = n_stalls; }
    /** @brief @return The number of stalled optimization cycles */
    std::uint32_t getNStalls() const { return n_stalls_; }

    /***************************************************************************/
    // Adaption-control knobs (per individual; the OA-owned adaption free functions read them).

    /** @brief Sets the maximum number of consecutive unsuccessful adaptions (0 disables the check).
     *  @param max_unsuccessful_adaptions The maximum number of consecutive unsuccessful adaptions */
    void setMaxUnsuccessfulAdaptions(std::size_t max_unsuccessful_adaptions) {
        max_unsuccessful_adaptions_ = max_unsuccessful_adaptions;
    }
    /** @brief @return The maximum number of consecutive unsuccessful adaptions */
    std::size_t getMaxUnsuccessfulAdaptions() const { return max_unsuccessful_adaptions_; }

    /** @brief Sets the maximum number of adaption retries until a valid solution is found (0 disables).
     *  @param max_retries_until_valid The maximum number of retries */
    void setMaxRetriesUntilValid(std::size_t max_retries_until_valid) {
        max_retries_until_valid_ = max_retries_until_valid;
    }
    /** @brief @return The maximum number of adaption retries until a valid solution is found */
    std::size_t getMaxRetriesUntilValid() const { return max_retries_until_valid_; }

    /** @brief @return The number of adaptions performed during the last adaption */
    std::size_t getNAdaptions() const { return n_adaptions_; }
    /** @brief Records the number of adaptions performed. @param n The number of adaptions */
    void setNAdaptions(std::size_t n) { n_adaptions_ = n; }

    /**
     * @brief Public, non-folding access to this candidate's per-individual RNG stream (the OA-owned
     * adaption free functions draw from it; each candidate owns its own stream, so parallel adaption is
     * lock-free).
     * @return A reference to this candidate's per-individual random engine
     */
    Gem::Hap::GRandomBase &getRandomEngine() { return gr_; }

    /**
     * @brief Public constraint check used by the OA-owned adaption retry loop. Feasibility is a concrete
     * base operation: it reads this entity's parameter values (streamlineFP, below) through the shared
     * policy's constraint. Returns true if the entity satisfies its constraints and writes the validity
     * level to the out-parameter.
     * @param validity_level Out-parameter receiving the computed validity level
     * @return true if the candidate satisfies its constraints, false otherwise
     */
    bool fulfillsConstraints(double &validity_level) const {
        return policy_->fulfillsConstraints(*this, validity_level);
    }

    /**
     * @brief Registers a constraint with this entity's shared problem policy (the 1:N feasibility rule).
     * Forwarded to GProblemPolicy::registerConstraint, which clones the constraint.
     * @param c_ptr The validity-check constraint to register (must not be empty)
     */
    void registerConstraint(std::shared_ptr<GPreEvaluationValidityCheckT<GOptimizableEntity>> c_ptr) {
        policy_->registerConstraint(c_ptr);
    }

    /***************************************************************************/
    // Genome value channels (genome-agnostic). The public per-type templates are the ergonomic surface the
    // optimization algorithms use; each dispatches to a non-template virtual that the flat genome
    // (GFlatGenome) implements. The algorithms therefore read and write parameter values without knowing
    // the storage layout -- no downcast. "Read my parameters as a vector" is a universal optimization
    // operation; only the storage is genome-specific, so the base declares it and the flat impl overrides.

    /**
     * @brief Streamlines all parameters of type par_type into a vector (cleared first).
     * @tparam par_type The parameter value type (double, float, std::int32_t or bool)
     * @param par_vec The vector the parameter values are written into (cleared first)
     * @param am The activity mode controlling which parameters are included
     */
    template <typename par_type>
    void streamline(
        std::vector<par_type> &par_vec,
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) const {
        this->streamline_(par_vec, am);
    }

    /**
     * @brief Assigns values from a vector to the parameters of type par_type (marks the item for
     * reprocessing).
     * @tparam par_type The parameter value type (double, float, std::int32_t or bool)
     * @param par_vec The vector of values to scatter onto the matching parameters
     * @param am The activity mode controlling which parameters are written
     */
    template <typename par_type>
    void assignValueVector(
        std::vector<par_type> const &par_vec,
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) {
#ifdef DEBUG
        if(countParameters<par_type>() != par_vec.size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GOptimizableEntity::assignValueVector():" << '\n'
                << "Sizes don't match: " << countParameters<par_type>() << " / " << par_vec.size()
                << '\n'
            );
        }
#endif /* DEBUG */
        this->assignValueVector_(par_vec, am);
        // As we have modified our internal data sets, make sure the item is reprocessed
        this->mark_as_due_for_processing();
    }

    /**
     * @brief The number of parameters of type par_type.
     * @tparam par_type The parameter value type (double, float, std::int32_t or bool)
     * @param am The activity mode controlling which parameters are counted
     * @return The number of parameters of the requested type
     */
    template <typename par_type>
    std::size_t countParameters(activityMode const &am = activityMode::DEFAULTACTIVITYMODE) const {
        if constexpr(std::is_same_v<par_type, double>) {
            return countParametersDouble_(am);
        }
        else if constexpr(std::is_same_v<par_type, float>) {
            return countParametersFloat_(am);
        }
        else if constexpr(std::is_same_v<par_type, std::int32_t>) {
            return countParametersInt32_(am);
        }
        else if constexpr(std::is_same_v<par_type, bool>) {
            return countParametersBool_(am);
        }
        else {
            static_assert(sizeof(par_type) == 0, "countParameters: unsupported parameter type");
            return 0;
        }
    }

    /**
     * @brief Lower/upper boundaries of all parameters of type par_type (cleared first).
     * @tparam par_type The parameter value type (double, float, std::int32_t or bool)
     * @param l_bnd_vec The vector the lower boundaries are written into (cleared first)
     * @param u_bnd_vec The vector the upper boundaries are written into (cleared first)
     * @param am The activity mode controlling which parameters are included
     */
    template <typename par_type>
    void boundaries(
        std::vector<par_type> &l_bnd_vec,
        std::vector<par_type> &u_bnd_vec,
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) const {
        this->boundaries_(l_bnd_vec, u_bnd_vec, am);
    }

    /***************************************************************************/
    // The precision-agnostic floating point view (double + float widened to double). Implemented once
    // here on top of the per-type channels above, so every genome layout gets it for free.

    /** @brief @param am The activity mode. @return The combined count of double and float parameters */
    std::size_t
    countFPParameters(activityMode const &am = activityMode::DEFAULTACTIVITYMODE) const {
        return countParameters<double>(am) + countParameters<float>(am);
    }

    /** @brief Streamlines all FP parameters into a single double vector (double-typed first, then widened
     *  float-typed). @param par_vec The output vector (cleared first). @param am The activity mode */
    void streamlineFP(
        std::vector<double> &par_vec,
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) const {
        par_vec.clear();
        this->streamline<double>(par_vec, am);

        std::vector<float> float_vec;
        this->streamline<float>(float_vec, am);
        par_vec.reserve(par_vec.size() + float_vec.size());
        for(float f : float_vec) {
            par_vec.push_back(static_cast<double>(f));
        }
    }

    /** @brief Scatters a double vector produced by streamlineFP() back onto the FP parameters.
     *  @param par_vec The combined double vector. @param am The activity mode */
    void assignFPValueVector(
        std::vector<double> const &par_vec,
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) {
        const std::size_t n_double = countParameters<double>(am);
        const std::size_t n_float = countParameters<float>(am);

#ifdef DEBUG
        if(n_double + n_float != par_vec.size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GOptimizableEntity::assignFPValueVector():" << '\n'
                << "Sizes don't match: " << (n_double + n_float) << " / " << par_vec.size() << '\n'
            );
        }
#endif /* DEBUG */

        if(n_double > 0) {
            std::vector<double> double_vec(
                par_vec.begin(),
                par_vec.begin() + static_cast<std::ptrdiff_t>(n_double)
            );
            this->assignValueVector<double>(double_vec, am);
        }
        if(n_float > 0) {
            std::vector<float> float_vec(n_float);
            for(std::size_t i = 0; i < n_float; ++i) {
                float_vec[i] = static_cast<float>(par_vec[n_double + i]);
            }
            this->assignValueVector<float>(float_vec, am);
        }
    }

    /** @brief Lower/upper boundaries of all FP parameters (matching streamlineFP() ordering).
     *  @param l_bnd_vec Lower bounds out (cleared). @param u_bnd_vec Upper bounds out (cleared). @param am The activity mode */
    void boundariesFP(
        std::vector<double> &l_bnd_vec,
        std::vector<double> &u_bnd_vec,
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) const {
        std::vector<double> l_double;
        std::vector<double> u_double;
        this->boundaries<double>(l_double, u_double, am);

        std::vector<float> l_float;
        std::vector<float> u_float;
        this->boundaries<float>(l_float, u_float, am);

        l_bnd_vec.clear();
        u_bnd_vec.clear();
        l_bnd_vec.reserve(l_double.size() + l_float.size());
        u_bnd_vec.reserve(u_double.size() + u_float.size());
        l_bnd_vec.insert(l_bnd_vec.end(), l_double.begin(), l_double.end());
        u_bnd_vec.insert(u_bnd_vec.end(), u_double.begin(), u_double.end());
        for(float v : l_float) {
            l_bnd_vec.push_back(static_cast<double>(v));
        }
        for(float v : u_float) {
            u_bnd_vec.push_back(static_cast<double>(v));
        }
    }

    /***************************************************************************/
    // The INTERNAL (normalized) floating-point view (normalized-genome architecture §2.3): the OAs read
    // and write the raw normalized internal coordinate (the two-reader split). Ordering matches
    // streamlineFP: double-typed first, then float-typed widened to double.

    /** @brief Streamlines all FP parameters in their raw INTERNAL representation into a double vector.
     *  @param par_vec The output vector (cleared first). @param am The activity mode */
    void streamlineFPInternal(
        std::vector<double> &par_vec,
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) const {
        par_vec.clear();
        this->streamlineInternal_(par_vec, am);

        std::vector<float> float_vec;
        this->streamlineInternal_(float_vec, am);
        par_vec.reserve(par_vec.size() + float_vec.size());
        for(float f : float_vec) {
            par_vec.push_back(static_cast<double>(f));
        }
    }

    /** @brief Scatters a vector of raw INTERNAL values back onto the FP parameters (folds bounded values,
     *  never range-validates). @param par_vec The combined internal-value vector. @param am The activity mode */
    void assignFPValueVectorInternal(
        std::vector<double> const &par_vec,
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) {
        const std::size_t n_double = countParameters<double>(am);
        const std::size_t n_float = countParameters<float>(am);

#ifdef DEBUG
        if(n_double + n_float != par_vec.size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GOptimizableEntity::assignFPValueVectorInternal():" << '\n'
                << "Sizes don't match: " << (n_double + n_float) << " / " << par_vec.size() << '\n'
            );
        }
#endif /* DEBUG */

        if(n_double > 0) {
            std::vector<double> double_vec(
                par_vec.begin(),
                par_vec.begin() + static_cast<std::ptrdiff_t>(n_double)
            );
            this->assignValueVectorInternal_(double_vec, am);
        }
        if(n_float > 0) {
            std::vector<float> float_vec(n_float);
            for(std::size_t i = 0; i < n_float; ++i) {
                float_vec[i] = static_cast<float>(par_vec[n_double + i]);
            }
            this->assignValueVectorInternal_(float_vec, am);
        }
        // As with the external assign, modifying the parameters marks the item for reprocessing.
        this->mark_as_due_for_processing();
    }

    /***************************************************************************/
    /**
     * @brief Retrieves a parameter of a given type at the specified active index.
     * @tparam val_type The value type to retrieve (double, float, std::int32_t or bool)
     * @param target A (type-index, name, position) tuple; the third element is the active index
     * @return The parameter value of the requested type at the requested index
     */
    template <typename val_type>
    val_type getVarVal(std::tuple<std::size_t, std::string, std::size_t> const &target) {
        static_assert(
            std::is_same_v<val_type, double> || std::is_same_v<val_type, float> ||
                std::is_same_v<val_type, std::int32_t> || std::is_same_v<val_type, bool>,
            "GOptimizableEntity::getVarVal<>(): unsupported value type (use double, float, std::int32_t or bool)"
        );

        const std::size_t idx = std::get<2>(target);

        if constexpr (std::is_same_v<val_type, double>) {
            return this->getVarVal_d_(idx);
        } else if constexpr (std::is_same_v<val_type, float>) {
            return this->getVarVal_f_(idx);
        } else if constexpr (std::is_same_v<val_type, std::int32_t>) {
            return this->getVarVal_i_(idx);
        } else { // bool, by the static_assert above
            return this->getVarVal_b_(idx);
        }
    }

    /***************************************************************************/
    // Algorithm-facing genome operations (declared here so the algorithms reach them through the base;
    // the flat genome implements them over its value channels).

    /** @brief Transformation of the entity's parameters into a boost::property_tree object.
     *  @param ptr The property tree to populate. @param base_name The base path / key prefix */
    virtual void toPropertyTree(pt::ptree &ptr, std::string const &base_name = "parameterset") const = 0;

    /** @brief Transformation of the entity's parameters into a list of comma-separated values.
     *  @param with_name_and_type Whether to prefix each value with its name and type
     *  @param with_commas Whether to separate the values with commas
     *  @param use_raw_fitness Whether to emit the raw rather than the transformed fitness
     *  @param show_validity Whether to append the validity flag
     *  @return The CSV representation of this entity */
    virtual std::string toCSV(
        bool with_name_and_type = false,
        bool with_commas = true,
        bool use_raw_fitness = true,
        bool show_validity = true
    ) const = 0;

    /** @brief Perform a cross-over operation between this entity and another.
     *  @param cp The other entity to cross over with
     *  @return A shared pointer to the resulting offspring entity */
    virtual std::shared_ptr<GOptimizableEntity> crossOverWith(GOptimizableEntity const &cp) const = 0;

    /** @brief Retrieves parameters relevant for the evaluation from another GOptimizableEntity.
     *  @param cp The entity whose evaluation-relevant parameters are absorbed into this object */
    virtual void cannibalize(GOptimizableEntity &cp) = 0;

    /***************************************************************************/
    // Randomization.

    /**
     * @brief Randomly initializes the parameters (marking the item for reprocessing on change).
     * @param am The activity mode selecting which parameters are (re-)initialized
     * @return true if at least one parameter was changed
     */
    bool randomInit(activityMode const &am);

protected:
    /***************************************************************************/
    // Result-store mutators (for the genome / process orchestration).

    /** @brief Modifiable retrieval of a stored result. @param id The position. @return A modifiable reference */
    individual_processing_result &modifyStoredResult(std::size_t id = 0) {
        return stored_results_cnt_.at(id);
    }
    /** @brief Sets the number of stored results, copying new_val into new positions.
     *  @param n_stored_results The new number of result slots. @param new_val The fill value */
    void setNStoredResults(std::size_t n_stored_results, individual_processing_result new_val) {
        stored_results_cnt_.resize(n_stored_results, new_val);
    }
    /** @brief Sets the number of stored results, default-initializing new positions.
     *  @param n_stored_results The new number of result slots */
    void setNStoredResults(std::size_t n_stored_results) {
        stored_results_cnt_.resize(n_stored_results, individual_processing_result());
    }
    /** @brief Registers a result at a given position. @param id The position. @param r The result */
    void registerResult(std::size_t id, const individual_processing_result &r) {
        stored_results_cnt_.at(id) = r;
    }

    /** @brief @return A reference to the shared problem policy (for the genome's feasibility check) */
    GProblemPolicy &policy() { return *policy_; }
    /** @brief @return A const reference to the shared problem policy */
    const GProblemPolicy &policy() const { return *policy_; }

    /***************************************************************************/
    // Secondary-result combiners (the user's fitnessCalculation() may return one of these).

    /** @brief @return The sum of all stored transformed fitness values */
    double sumCombiner() const;
    /** @brief @return The sum of the absolute values of all stored transformed fitness values */
    double fabsSumCombiner() const;
    /** @brief @return The square root of the sum of squares of all stored transformed fitness values */
    double squaredSumCombiner() const;
    /** @brief @return The square root of the weighed sum of squares of all stored transformed fitness values
     *  @param weights The per-criterion weights (size must match the criteria count) */
    double weighedSquaredSumCombiner(std::vector<double> const &weights) const;

    /***************************************************************************/
    // GCommonInterfaceT / configuration contract.

    /** @brief Adds local configuration options (eval policy, sigmoid, max mode, adaption limits).
     *  @param gpb The parser builder the configuration options are registered with */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) override;

    /** @brief Loads the data of another GOptimizableEntity. @param cp The source candidate */
    void load_(const GOptimizableEntity *cp) override;

    /** @brief Allow access to this class's compare_ function */
    friend void Gem::Common::compare_base_t<GOptimizableEntity>(
        GOptimizableEntity const &,
        GOptimizableEntity const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another candidate.
     *  @param cp The other candidate to compare against
     *  @param e The expectation (e.g. equality)
     *  @param limit The limit for allowed floating-point deviations */
    void compare_(
        GOptimizableEntity const &cp,
        Gem::Common::expectation const &e,
        double const &limit
    ) const override;

    /***************************************************************************/
    // A per-individual random engine + a reusable integer distribution (used by adaption helpers).

    Gem::Hap::GRandom gr_; ///< Per-individual engine; follows the HAP_RANDOM_SOURCE-selected backend
    std::uniform_int_distribution<std::size_t> uniform_int_; ///< Uniformly distributed integer randoms

    /***************************************************************************/
    // Pure-virtual hooks implemented by the genome layer.

    /** @brief Randomly initializes the parameters (genome-specific).
     *  @param am The activity mode selecting which parameters are affected
     *  @return true if at least one parameter was changed */
    virtual bool randomInit_(activityMode const &am) = 0;

    /** @brief The user-implemented fitness calculation for the main quality criterion (GRateableI).
     *  @return The computed raw fitness of the main quality criterion */
    double fitnessCalculation() override = 0;

private:
    /***************************************************************************/
    // GRateableI surface (read the stored results).

    /** @brief @param id The criterion index. @return The stored raw fitness for that criterion */
    double raw_fitness_(std::size_t id) const final { return this->getStoredResult(id).rawFitness(); }
    /** @brief @param id The criterion index. @return The stored transformed fitness for that criterion */
    double transformed_fitness_(std::size_t id) const final {
        return this->getStoredResult(id).transformedFitness();
    }
    /** @brief @return A vector of all stored raw fitness results */
    std::vector<double> raw_fitness_vec_() const final;
    /** @brief @return A vector of all stored transformed fitness results */
    std::vector<double> transformed_fitness_vec_() const final;

    /***************************************************************************/
    // Per-type genome value channels -- the non-template dispatch targets of the public
    // streamline<T>/assignValueVector<T>/countParameters<T>/boundaries<T> templates (and the getVarVal<T>
    // template). Implemented by the concrete flat genome. These are the entire seam that makes value
    // access genome-agnostic; each overload takes the per-type value/boundary vector(s) and the
    // activityMode controlling which parameters participate.

    /** @brief Retrieve the value of the active parameter at the given index, per type (genome dispatch).
     *  @param idx The index of the active parameter to retrieve
     *  @return The active parameter value at @p idx (double for _d_, float for _f_, int32 for _i_, bool for _b_) */
    virtual double getVarVal_d_(std::size_t idx) = 0;
    virtual float getVarVal_f_(std::size_t idx) = 0;
    virtual std::int32_t getVarVal_i_(std::size_t idx) = 0;
    virtual bool getVarVal_b_(std::size_t idx) = 0;

    virtual void streamline_(std::vector<double> &, activityMode const &) const = 0;
    virtual void streamline_(std::vector<float> &, activityMode const &) const = 0;
    virtual void streamline_(std::vector<std::int32_t> &, activityMode const &) const = 0;
    virtual void streamline_(std::vector<bool> &, activityMode const &) const = 0;

    virtual void assignValueVector_(std::vector<double> const &, activityMode const &) = 0;
    virtual void assignValueVector_(std::vector<float> const &, activityMode const &) = 0;
    virtual void assignValueVector_(std::vector<std::int32_t> const &, activityMode const &) = 0;
    virtual void assignValueVector_(std::vector<bool> const &, activityMode const &) = 0;

    virtual std::size_t countParametersDouble_(activityMode const &) const = 0;
    virtual std::size_t countParametersFloat_(activityMode const &) const = 0;
    virtual std::size_t countParametersInt32_(activityMode const &) const = 0;
    virtual std::size_t countParametersBool_(activityMode const &) const = 0;

    virtual void boundaries_(std::vector<double> &, std::vector<double> &, activityMode const &) const = 0;
    virtual void boundaries_(std::vector<float> &, std::vector<float> &, activityMode const &) const = 0;
    virtual void boundaries_(std::vector<std::int32_t> &, std::vector<std::int32_t> &, activityMode const &) const = 0;
    virtual void boundaries_(std::vector<bool> &, std::vector<bool> &, activityMode const &) const = 0;

    // The INTERNAL (normalized) FP channels (§2.3): only double/float have an internal/external
    // distinction (int / bool are not normalized).
    virtual void streamlineInternal_(std::vector<double> &, activityMode const &) const = 0;
    virtual void streamlineInternal_(std::vector<float> &, activityMode const &) const = 0;
    virtual void assignValueVectorInternal_(std::vector<double> const &, activityMode const &) = 0;
    virtual void assignValueVectorInternal_(std::vector<float> const &, activityMode const &) = 0;

    /***************************************************************************/
    // Results-only return hooks (default: full item; the genome overrides).

    /** @brief @return false in the base (a full item); genome overrides for results-only returns */
    virtual bool inputDataOmitted_() const { return false; }
    /** @brief Default no-op graft; the genome overrides. @param original The originally-submitted item */
    virtual void graftInputDataFrom_([[maybe_unused]] const GOptimizableEntity &original) { /* nothing */ }
    /** @brief Default no-op constant-data load; a derived type overrides if it deposits constant data at a
     *  remote site. @param cd_ptr A template item whose constant data would be loaded into this one */
    virtual void loadConstantData_([[maybe_unused]] std::shared_ptr<GOptimizableEntity> cd_ptr) { /* nothing */ }

    /***************************************************************************/
    // Evaluation internals.

    /** @brief The evaluation body run inside process(): feasibility check + fitnessCalculation()/res_vec
     *  adoption + the evaluation-policy transform. @param res_vec Optional pre-computed raw results */
    void runEvaluation_(const std::vector<individual_processing_result> &res_vec);

    /** @brief Runs the registered pre-processor (if allowed) on this candidate */
    void preProcess_();
    /** @brief Runs the registered post-processor (if allowed) on this candidate */
    void postProcess_();

    /** @brief Sets every raw and transformed fitness to the same value. @param val The value */
    void setAllFitnessTo(double val) { this->setAllFitnessTo(val, val); }
    /** @brief Sets every raw fitness to raw_value and every transformed fitness to transformed_value.
     *  @param raw_value The raw value. @param transformed_value The transformed value */
    void setAllFitnessTo(double raw_value, double transformed_value);

    /** @brief Resets every stored result to a default-constructed value */
    void clear_stored_results_vec();
    /** @brief Bridges the GProcessable status machine to the result store (clears it on a status reset) */
    void clearStoredResults_() override { this->clear_stored_results_vec(); }

    /** @brief Emits a name for this class / object. @return The class / object name */
    std::string name_() const override { return std::string("GOptimizableEntity"); }
    /** @brief Creates a deep clone of this object (supplied by the concrete leaf). @return A heap copy */
    GOptimizableEntity *clone_() const override = 0;

    /***************************************************************************/
    // Data.

    bool pre_processing_disabled_ = false;  ///< Whether pre-processing was disabled entirely
    bool post_processing_disabled_ = false; ///< Whether post-processing was disabled entirely

    std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GOptimizableEntity>>
        pre_processor_ptr_; ///< Actions to be performed before processing
    std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GOptimizableEntity>>
        post_processor_ptr_; ///< Actions to be performed after processing

    std::vector<individual_processing_result> stored_results_cnt_ =
        std::vector<individual_processing_result>(1, individual_processing_result()); ///< The result store

    /** @brief The shared, problem-uniform feasibility / ranking policy (referenced 1:N) */
    std::shared_ptr<GProblemPolicy> policy_ = std::make_shared<GProblemPolicy>();

    std::tuple<double, double> best_past_primary_fitness_{std::make_tuple(0., 0.)}; ///< Globally best known primary fitness
    std::uint32_t n_stalls_ = 0;        ///< Number of stalls of the primary fitness criterion
    std::uint32_t assigned_iteration_ = 0; ///< The parent algorithm's optimization-cycle iteration
    double validity_level_ = 0.;        ///< How valid the current solution is (<= 1 == feasible)

    std::size_t n_adaptions_ = 0; ///< Number of adaptions performed during the last adaption
    std::size_t max_unsuccessful_adaptions_ =
        Gem::Geneva::DEFMAXUNSUCCESSFULADAPTIONS; ///< Max consecutive unsuccessful adaptions per adapt()
    std::size_t max_retries_until_valid_ =
        Gem::Geneva::DEFMAXRETRIESUNTILVALID; ///< Max adaption retries until a valid solution is found
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::Genome */

/******************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::Genome::GOptimizableEntity) // NOLINT
/******************************************************************************/
