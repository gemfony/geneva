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
#include <vector>

// Boost header files go here
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>

// Geneva headers go here
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GSerializableFunctionObjectT.hpp"
#include "common/GSerializationHelperFunctionsT.hpp" // serialization of std::chrono time_point (GProcessable timing)
#include "common/GSerializeTupleT.hpp"               // serialization of std::tuple (best_past_primary_fitness_)
#include "courtier/GProcessable.hpp" // the non-generic processing-lifecycle base
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/Interface/GRateableI.hpp"
#include "geneva/ind/GIndividualProcessingResult.hpp"
#include "geneva/ind/GProblemPolicy.hpp"
#include "hap/GRandomT.hpp"

namespace Gem::Geneva::Genome {

/******************************************************************************/
// An exception thrown if an exception was thrown during a candidate's processing.
class g_candidate_processing_exception : public geneva_exception {
    using geneva_exception::geneva_exception;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief The genome-agnostic candidate-solution assembly: the algorithm-facing root of the individual
 * hierarchy.
 *
 * GCandidateSolution combines three concerns into one object without knowing how the parameters are
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
 * GProblemPolicy that every candidate references (the 1:N policy).
 *
 * What this class deliberately does NOT know is the genome: there are no parameter value channels here.
 * Reading and writing parameter values, the structural layout and feasibility checking (which reads the
 * parameters) live one layer down on GFlatGenomeBase, reached through the pure-virtual hooks declared
 * here (randomInit_, checkFeasibility_) and GRateableI::fitnessCalculation(). This keeps the assembly
 * representation-agnostic: a future non-flat genome would derive GCandidateSolution directly.
 */
class GCandidateSolution // NOLINT(cppcoreguidelines-special-member-functions)
  : public Gem::Courtier::GProcessable
  , public Gem::Common::GCommonInterfaceT<GCandidateSolution>
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
    using payload_type = GCandidateSolution;
    using result_type = individual_processing_result;

    /** @brief The default constructor (a single fitness criterion, a fresh private policy) */
    GCandidateSolution();
    /**
     * @brief Initialization with the number of fitness criteria.
     * @param n_fitness_criteria The number of fitness criteria this candidate evaluates to
     */
    explicit GCandidateSolution(std::size_t n_fitness_criteria);
    /**
     * @brief The copy constructor. The shared policy is copied by shared pointer (1:N), the
     * pre-/post-processors are deep-cloned.
     * @param cp The other candidate whose data is copied
     */
    GCandidateSolution(GCandidateSolution const &cp);
    /** @brief The destructor */
    ~GCandidateSolution() override = default;

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
        std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GCandidateSolution>> pre_processor_ptr
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
        std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GCandidateSolution>> post_processor_ptr
    ) {
        if(post_processor_ptr) {
            post_processor_ptr_ = post_processor_ptr;
        }
    }
    /** @brief @return The registered post-processor, or an empty pointer if none is registered */
    std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GCandidateSolution>>
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
    void graftInputDataFrom(const GCandidateSolution &original) {
        this->graftInputDataFrom_(original);
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
     * @brief Public constraint check used by the OA-owned adaption retry loop.
     * @param validity_level Out-parameter receiving the computed validity level
     * @return true if the candidate satisfies its constraints, false otherwise
     */
    bool fulfillsConstraints(double &validity_level) const {
        return this->checkFeasibility_(validity_level);
    }

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

    /** @brief Loads the data of another GCandidateSolution. @param cp The source candidate */
    void load_(const GCandidateSolution *cp) override;

    /** @brief Allow access to this class's compare_ function */
    friend void Gem::Common::compare_base_t<GCandidateSolution>(
        GCandidateSolution const &,
        GCandidateSolution const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another candidate.
     *  @param cp The other candidate to compare against
     *  @param e The expectation (e.g. equality)
     *  @param limit The limit for allowed floating-point deviations */
    void compare_(
        GCandidateSolution const &cp,
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

    /** @brief Checks whether this candidate fulfils its constraint (genome-specific: the constraint reads
     *  parameter values). Writes the validity level to the out-parameter.
     *  @param validity_level Out-parameter receiving the computed validity level
     *  @return true if the candidate is feasible (or no constraint is registered) */
    virtual bool checkFeasibility_(double &validity_level) const = 0;

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
    // Results-only return hooks (default: full item; the genome overrides).

    /** @brief @return false in the base (a full item); genome overrides for results-only returns */
    virtual bool inputDataOmitted_() const { return false; }
    /** @brief Default no-op graft; the genome overrides. @param original The originally-submitted item */
    virtual void graftInputDataFrom_([[maybe_unused]] const GCandidateSolution &original) { /* nothing */ }

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
    std::string name_() const override { return std::string("GCandidateSolution"); }
    /** @brief Creates a deep clone of this object (supplied by the concrete leaf). @return A heap copy */
    GCandidateSolution *clone_() const override = 0;

    /***************************************************************************/
    // Data.

    bool pre_processing_disabled_ = false;  ///< Whether pre-processing was disabled entirely
    bool post_processing_disabled_ = false; ///< Whether post-processing was disabled entirely

    std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GCandidateSolution>>
        pre_processor_ptr_; ///< Actions to be performed before processing
    std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GCandidateSolution>>
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
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::Genome::GCandidateSolution) // NOLINT
/******************************************************************************/
