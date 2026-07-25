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
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

// Boost header files go here
#include <boost/json.hpp>

// Geneva headers go here
#include "common/GReflectiveInterfaceT.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GSerializableFunctionObjectT.hpp"
#include "common/GSerializationHelperFunctionsT.hpp" // serialization of std::chrono time_point (GProcessable timing)
#include "courtier/GProcessable.hpp" // the non-generic processing-lifecycle base
#include "courtier/GWireSerializationContext.hpp" // the wire scope: scratch is skipped on transport
#include "geneva/genome/GMultiConstraintT.hpp" // GPreEvaluationValidityCheckT (registered on the shared policy)
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/Interface/GRateableI.hpp"
#include "geneva/genome/GAuxiliaryStore.hpp" // the OA-owned scratch (personality + per-group adaption PODs)
#include "geneva/genome/GIndividualProcessingResult.hpp"
#include "geneva/genome/GProblemPolicy.hpp"
#include "hap/GRandomT.hpp"

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
 *   - EVALUATABILITY, via GRateableI (the raw / transformed fitness accessors) plus the pure-virtual
 *     evaluate() hook the concrete individual implements to compute its raw result vector.
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
 * toJSON / toCSV / crossOverWith / cannibalize) is declared here, dispatching to pure-virtual
 * hooks that the value-bearing genome layer (GGenome) implements. "Read my parameters as a vector" is
 * a universal optimization operation; only the storage is genome-specific. This keeps the assembly
 * representation-agnostic: a future non-flat genome would derive GOptimizableEntity directly.
 */
// This class folds onto the GReflectiveInterfaceBaseT mixin (it stays the abstract category root, so clone_()
// remains pure). Its second stateful base Gem::Courtier::GProcessable is NOT a container -- it has no
// single data member to tie in -- so it is carried through make_base_object_member<GProcessable>, which
// serialises it as base_object, copy-assigns its slice on load, and excludes it from comparison. The
// serialized-but-not-comparable members (result store, shared policy, pre-/post-processors) use the
// cmp_skip factories, and the OA scratch (whose wire form is irreducibly custom) uses
// make_owner_serialized_ptr_member so this class's own serialize() below can emit it wire-conditionally.
// load_(), compare_() and name_() are thus generated from the single localMembers_() declaration; only
// serialize() stays hand-written, for the scratch wire protocol.
class GOptimizableEntity // NOLINT(cppcoreguidelines-special-member-functions)
  : public Gem::Courtier::GProcessable
  , public Gem::Common::GReflectiveInterfaceBaseT<GOptimizableEntity, Gem::Common::GCommonInterfaceT<GOptimizableEntity>>
  , public Interface::GRateableI {
    ///////////////////////////////////////////////////////////////////////
    friend struct Gem::Weft::access;
    friend struct Gem::Common::GReflectiveInterfaceAccess;

    /**
     * @brief Single declaration of ALL this class's data, each entry carrying the participation policy it
     * needs, so the GReflectiveInterfaceBaseT-generated load_(), compare_() and name_() (and the plain half of the
     * hand-written serialize() below) all derive from this one source.
     *
     *  - the GProcessable lifecycle base (a stateful non-container base) rides make_base_object_member<>;
     *  - the four plain veto/feasibility members are ordinary make_member (serialized + loaded + compared);
     *  - the cloneable pre-/post-processors, the shared 1:N policy and the result store are state but not
     *    per-individual identity, so they use the cmp_skip factories (serialized + loaded, not compared);
     *  - the OA scratch is copy-loaded and not compared, and its wire form (serialized on a checkpoint,
     *    omitted on the wire) rides make_wire_omitted_ptr_member, so it too is fully single-sourced.
     *
     * @tparam Self The (const or non-const) deduced type of *this
     * @param self A reference to *this whose members are tied into the tuple
     * @return A tuple of named member references driving serialize(), load_() and compare_()
     */
    template <typename Self>
    auto localMembers_(this Self &self) {
        return std::make_tuple(
            Gem::Common::make_base_object_member<Gem::Courtier::GProcessable>("GProcessable", self),
            Gem::Common::make_member("pre_processing_disabled_", self.pre_processing_disabled_),
            Gem::Common::make_member("post_processing_disabled_", self.post_processing_disabled_),
            Gem::Common::make_member("assigned_iteration_", self.assigned_iteration_),
            Gem::Common::make_member("validity_level_", self.validity_level_),
            Gem::Common::make_uncompared_cloneable_member("pre_processor_ptr_", self.pre_processor_ptr_),
            Gem::Common::make_uncompared_cloneable_member("post_processor_ptr_", self.post_processor_ptr_),
            Gem::Common::make_uncompared_member("policy_", self.policy_),
            Gem::Common::make_uncompared_member("stored_results_cnt_", self.stored_results_cnt_),
            Gem::Courtier::make_wire_omitted_ptr_member("scratch_", self.scratch_)
        );
    }

    /**
     * @brief Disambiguating serialize(): two stateful bases in the inheritance set (the GReflectiveInterfaceBaseT
     * mixin and GProcessable) declare a serialize(), so this one-liner resolves the ambiguity and emits the
     * single member list. Every member -- the GProcessable base slice (base_object), the plain members, the
     * serialized-but-uncompared processors / policy / result store, and the OA scratch (serialized on a
     * checkpoint, omitted on the wire, via make_wire_omitted_ptr_member) -- comes from localMembers_(). The
     * stateless GCommonInterfaceT root and GRateableI interface contribute nothing.
     *
     * @tparam Archive The GArchive codec type
     * @param ar The archive to read from or write to
     * @param version The serialization version (unused)
     */
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        Gem::Common::serialize_members(ar, this->localMembers_());
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The class name, consumed by the GReflectiveInterfaceBaseT-generated name_() / compare token. */
    static constexpr std::string_view class_name = "GOptimizableEntity";

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
     *        evaluate() is invoked. Its size must match the criteria count.
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
    [[nodiscard]] individual_processing_result getStoredResult(std::size_t id = 0) const;

    /** @brief @return The number of result slots held by this candidate */
    [[nodiscard]] std::size_t getNStoredResults() const { return stored_results_cnt_.size(); }

    /** @brief Read-only retrieval of the whole result store (no PROCESSED-flag precondition, unlike
     *  getStoredResult()), used to carry results across a return reconciliation. @return The result store */
    [[nodiscard]] const std::vector<individual_processing_result> &getStoredResults() const {
        return stored_results_cnt_;
    }

    /***************************************************************************/
    // Pre-/post-processing (used by nested / post-optimizing algorithms).

    /** @brief @return true if pre-processing is currently allowed (not vetoed) */
    [[nodiscard]] bool mayBePreProcessed() const noexcept { return not pre_processing_disabled_; }
    /** @brief Allow or veto pre-processing. @param veto true to disable, false to allow */
    void vetoPreProcessing(bool veto) noexcept { pre_processing_disabled_ = veto; }
    /** @brief Registers a pre-processor (ignored if empty). @param pre_processor_ptr The processor */
    void registerPreProcessor(
        const std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GOptimizableEntity>>& pre_processor_ptr
    ) {
        if(pre_processor_ptr) {
            pre_processor_ptr_ = pre_processor_ptr;
        }
    }

    /** @brief @return true if post-processing is currently allowed (not vetoed) */
    [[nodiscard]] bool mayBePostProcessed() const { return not post_processing_disabled_; }
    /** @brief Allow or veto post-processing. @param veto true to disable, false to allow */
    void vetoPostProcessing(bool veto) { post_processing_disabled_ = veto; }
    /** @brief Registers a post-processor (ignored if empty). @param post_processor_ptr The processor */
    void registerPostProcessor(
        const std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GOptimizableEntity>>& post_processor_ptr
    ) {
        if(post_processor_ptr) {
            post_processor_ptr_ = post_processor_ptr;
        }
    }
    /** @brief @return The registered post-processor, or an empty pointer if none is registered */
    [[nodiscard]] std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GOptimizableEntity>>
    postProcessor() const {
        return post_processor_ptr_;
    }
    /** @brief Removes any registered post-processor */
    void clearPostProcessor() { post_processor_ptr_.reset(); }

    /** @brief Loads otherwise-constant data into this (freshly de-serialized) item from a template held at
     *  a remote site, so that data need not travel with every work item (a networked-client convenience;
     *  the default is a no-op, a derived type overrides the hook below if it carries such data).
     *  @param cd_ptr A template item whose constant data is loaded into this one */
    void loadConstantData(std::shared_ptr<GOptimizableEntity> cd_ptr) {
        this->loadConstantData_(std::move(cd_ptr));
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
    [[nodiscard]] bool hasMultipleFitnessCriteria() const { return this->getNStoredResults() > 1; }
    /**
     * @brief Retrieve the (raw, transformed) fitness tuple at a given evaluation position.
     * @param id The evaluation position (fitness criterion index); defaults to 0
     * @return A (raw, transformed) fitness tuple at the requested position
     */
    [[nodiscard]] std::tuple<double, double> getFitnessTuple(std::uint32_t id = 0) const;
    /**
     * @brief Checks whether this candidate is at least as good as a set of raw boundaries.
     * @param boundaries One boundary value per fitness criterion
     * @return true if every raw fitness is at least as good as its boundary, false otherwise
     */
    bool isGoodEnough(std::vector<double> const &boundaries);

    /***************************************************************************/
    // Policy-derived accessors (forwarded to the shared GProblemPolicy).

    /** @brief Installs the shared problem policy (the 1:N feasibility/ranking rules).
     *  @param policy The shared policy to reference (must not be empty) */
    void setPolicy(std::shared_ptr<GProblemPolicy> policy);
    /** @brief @return The shared problem policy referenced by this candidate */
    [[nodiscard]] std::shared_ptr<GProblemPolicy> getPolicy() const { return policy_; }

    /** @brief Sets the optimization direction on the shared policy. @param mode MAXIMIZE or MINIMIZE */
    void setMaxMode(maxMode const &mode) { policy_->setMaxMode(mode); }
    /** @brief @return The optimization direction from the shared policy */
    [[nodiscard]] maxMode getMaxMode() const { return policy_->getMaxMode(); }
    /** @brief @return The worst-case evaluation value for the current direction */
    [[nodiscard]] virtual double getWorstCase() const { return policy_->getWorstCase(); }
    /** @brief @return The best-case evaluation value for the current direction */
    [[nodiscard]] virtual double getBestCase() const { return policy_->getBestCase(); }

    /** @brief Sets the policy for invalid solutions. @param eval_policy The evaluation policy */
    void setEvaluationPolicy(evaluationPolicy eval_policy) { policy_->setEvaluationPolicy(eval_policy); }
    /** @brief @return The evaluation policy for invalid solutions */
    [[nodiscard]] evaluationPolicy getEvaluationPolicy() const { return policy_->getEvaluationPolicy(); }

    /** @brief @return The sigmoid steepness from the shared policy */
    [[nodiscard]] double getSteepness() const { return policy_->getSteepness(); }
    /** @brief Sets the sigmoid steepness on the shared policy. @param steepness The steepness (> 0) */
    void setSteepness(double steepness) { policy_->setSteepness(steepness); }
    /** @brief @return The sigmoid barrier from the shared policy */
    [[nodiscard]] double getBarrier() const { return policy_->getBarrier(); }
    /** @brief Sets the sigmoid barrier on the shared policy. @param barrier The barrier (> 0) */
    void setBarrier(double barrier) { policy_->setBarrier(barrier); }

    /** @brief @return The computed validity level of this candidate (<= 1 means feasible) */
    [[nodiscard]] double getValidityLevel() const { return validity_level_; }
    /** @brief @return true if this candidate fulfils its constraints (validity level <= 1) */
    [[nodiscard]] bool constraintsFulfilled() const { return validity_level_ <= 1.; }
    /** @brief @return true if this candidate is a valid solution (meant for processed candidates) */
    [[nodiscard]] bool isValid() const;
    /** @brief @return true if this candidate is an invalid solution */
    [[nodiscard]] bool isInValid() const { return not this->isValid(); }

    /***************************************************************************/
    // Iteration bookkeeping (per individual). The stall count and best-known fitness are OA state and
    // live on GOptimizationAlgorithmBase; the adaption-retry limits are OA adaption policy and live on
    // the OA-owned GAdaptionConfig -- neither is carried on the individual any more.

    /** @brief Sets the parent algorithm's iteration. @param parent_alg_iteration The iteration */
    void setAssignedIteration(std::uint32_t const &parent_alg_iteration) {
        assigned_iteration_ = parent_alg_iteration;
    }
    /** @brief @return The parent algorithm's current iteration */
    [[nodiscard]] std::uint32_t getAssignedIteration() const { return assigned_iteration_; }

    /** @brief @return The number of adaptions performed during the last adaption (read from the OA
     *  scratch, where the adaption machinery records it; 0 if no scratch is attached) */
    [[nodiscard]] std::size_t getNAdaptions() const { return scratch_ ? scratch_->getNAdaptions() : 0; }

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
        policy_->registerConstraint(std::move(c_ptr));
    }

    /***************************************************************************/
    // OA-owned scratch (the personality OBJECT + per-group adaption POD blocks). Held ON the work item so
    // it permutes coherently through every sort / select / swap; it is server-side state, dropped at the
    // algorithm boundary (resetPersonality / clearScratch), and never travels on the wire (see serialize()).

    /** @brief The optimization-algorithm-owned scratch (personality + POD adaption state).
     *  @return A reference to the scratch store */
    GAuxiliaryStore &scratch() noexcept { return *scratch_; }
    /** @brief The OA-owned scratch (const). @return A const reference to the scratch store */
    [[nodiscard]] const GAuxiliaryStore &scratch() const noexcept { return *scratch_; }

    /**
     * @brief Converts the personality base pointer to the desired type. Only accessible when
     * personality_type derives from GPersonalityTraits.
     * @tparam personality_type The concrete personality-traits type (must derive from GPersonalityTraits)
     * @return A shared pointer to the personality traits cast to personality_type
     */
    template <typename personality_type>
        requires std::derived_from<personality_type, GPersonalityTraits>
    std::shared_ptr<personality_type> getPersonalityTraits() {
#ifdef DEBUG
        if(not scratch_->personalityRef()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GOptimizableEntity::getPersonalityTraits<personality_type>() : Empty personality "
                   "pointer found"
                << '\n'
            );
        }
#endif /* DEBUG */
        return Gem::Common::convertSmartPointer<GPersonalityTraits, personality_type>(
            scratch_->personalityRef()
        );
    }

    /** @brief The personality-traits base pointer.
     *  @return A shared pointer to this entity's GPersonalityTraits
     *  @throw geneva_exception (DEBUG builds) if the personality pointer is empty */
    std::shared_ptr<GPersonalityTraits> getPersonalityTraits() {
#ifdef DEBUG
        if(not scratch_->personalityRef()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GOptimizableEntity::getPersonalityTraits() : Empty personality pointer found" << '\n'
            );
        }
#endif /* DEBUG */
        return scratch_->personalityRef();
    }

    /** @brief Sets the personality of this entity.
     *  @param gpt The personality-traits object to install (must be non-null)
     *  @throw geneva_exception if gpt is empty */
    void setPersonality(std::shared_ptr<GPersonalityTraits> gpt) {
        if(not gpt) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GOptimizableEntity::setPersonality() : Empty personality pointer passed" << '\n'
            );
        }
        scratch_->personalityRef() = std::move(gpt);
    }

    /** @brief Resets the OA-owned scratch (personality + any POD blocks held on this entity) */
    void resetPersonality() { scratch_->clearScratch(); }

    /** @brief A string identifier for the current personality.
     *  @return The personality's name(), or "PERSONALITY_NONE" if no personality is set */
    [[nodiscard]] std::string getPersonality() const {
        if(scratch_->personalityRef()) {
            return scratch_->personalityRef()->name();
        }
        return std::string("PERSONALITY_NONE");
    }

    /***************************************************************************/
    // Genome value channels (genome-agnostic). The public per-type templates are the ergonomic surface the
    // optimization algorithms use; each dispatches to a non-template virtual that the flat genome
    // (GGenome) implements. The algorithms therefore read and write parameter values without knowing
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
    [[nodiscard]] [[nodiscard]] [[nodiscard]] std::size_t countParameters(activityMode const &am = activityMode::DEFAULTACTIVITYMODE) const {
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
    [[nodiscard]] std::size_t
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
        par_vec.append_range(float_vec); // each float implicitly widened to double
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
            std::vector<double> const double_vec(
                par_vec.begin(),
                par_vec.begin() + static_cast<std::ptrdiff_t>(n_double)
            );
            this->assignValueVector<double>(double_vec, am);
        }
        if(n_float > 0) {
            std::vector<float> float_vec;
            float_vec.append_range(par_vec | std::views::drop(n_double)); // each double implicitly narrowed to float
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
        l_bnd_vec.append_range(l_double);
        l_bnd_vec.append_range(l_float); // each float implicitly widened to double
        u_bnd_vec.append_range(u_double);
        u_bnd_vec.append_range(u_float); // each float implicitly widened to double
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
        par_vec.append_range(float_vec); // each float implicitly widened to double
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
            std::vector<double> const double_vec(
                par_vec.begin(),
                par_vec.begin() + static_cast<std::ptrdiff_t>(n_double)
            );
            this->assignValueVectorInternal_(double_vec, am);
        }
        if(n_float > 0) {
            std::vector<float> float_vec;
            float_vec.append_range(par_vec | std::views::drop(n_double)); // each double implicitly narrowed to float
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

    /** @brief Transformation of the entity's parameters into a JSON object.
     *  @return A boost::json::object holding this entity's parameters, metadata and results (the body only;
     *          callers place it under whatever key they need) */
    [[nodiscard]] virtual boost::json::object toJSON() const = 0;

    /** @brief Transformation of the entity's parameters into a list of comma-separated values.
     *  @param with_name_and_type Whether to prefix each value with its name and type
     *  @param with_commas Whether to separate the values with commas
     *  @param use_raw_fitness Whether to emit the raw rather than the transformed fitness
     *  @param show_validity Whether to append the validity flag
     *  @return The CSV representation of this entity */
    [[nodiscard]] virtual std::string toCSV(
        bool with_name_and_type = false,
        bool with_commas = true,
        bool use_raw_fitness = true,
        bool show_validity = true
    ) const = 0;

    /** @brief Perform a cross-over operation between this entity and another.
     *  @param cp The other entity to cross over with
     *  @return A shared pointer to the resulting offspring entity */
    [[nodiscard]] virtual std::shared_ptr<GOptimizableEntity> crossOverWith(GOptimizableEntity const &cp) const = 0;

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

    /**
     * @brief Sets the fitness from a vector of externally-computed raw values, applying the feasibility
     * check and evaluation-policy transform, then marking the candidate PROCESSED. An internal helper (an
     * alternative to process(res_vec) used on the genome copy/move path); not a public API -- external
     * results are injected through process(res_vec).
     * @param f_cnt A vector of raw fitness values (size must match the criteria count)
     */
    void setFitness_(std::vector<double> const &f_cnt);

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
    /** @brief Replaces the whole result store WITHOUT touching the processing status (unlike
     *  markAsProcessedWith(), which forces PROCESSED). Used to carry results across a return
     *  reconciliation, where the status is set separately from the returned lifecycle.
     *  @param results The result store to install */
    void setStoredResults(const std::vector<individual_processing_result> &results) {
        stored_results_cnt_ = results;
    }
    /** @brief Records the feasibility (validity) level -- normally filled by fulfillsConstraints() during
     *  evaluation; exposed here so a return reconciliation can carry the worker-computed value.
     *  @param validity_level The validity level (<= 1 == feasible) */
    void setValidityLevel(double validity_level) { validity_level_ = validity_level; }

    /** @brief @return A reference to the shared problem policy (for the genome's feasibility check) */
    GProblemPolicy &policy() { return *policy_; }
    /** @brief @return A const reference to the shared problem policy */
    [[nodiscard]] const GProblemPolicy &policy() const { return *policy_; }

    /***************************************************************************/
    // Secondary-result combiners (the user's evaluate() may return one of these).

    /** @brief @return The sum of all stored transformed fitness values */
    [[nodiscard]] double sumCombiner() const;
    /** @brief @return The sum of the absolute values of all stored transformed fitness values */
    [[nodiscard]] double fabsSumCombiner() const;
    /** @brief @return The square root of the sum of squares of all stored transformed fitness values */
    [[nodiscard]] double squaredSumCombiner() const;
    /** @brief @return The square root of the weighed sum of squares of all stored transformed fitness values
     *  @param weights The per-criterion weights (size must match the criteria count) */
    [[nodiscard]] double weighedSquaredSumCombiner(std::vector<double> const &weights) const;

    /***************************************************************************/
    // GCommonInterfaceT / configuration contract.

    /** @brief Adds local configuration options (eval policy, sigmoid, max mode, adaption limits).
     *  @param gpb The parser builder the configuration options are registered with */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) override;

    // load_(), compare_() and name_() are generated by the Gem::Common::GReflectiveInterfaceBaseT base from
    // class_name and the single localMembers_() declaration (which carries the GProcessable base slice, the
    // plain members and the serialized-but-uncompared processors / policy / result store / scratch).
    // clone_() stays pure here -- this is the abstract category root; each concrete candidate supplies it.

    /***************************************************************************/
    // A candidate carries NO random-number state of its own -- it is pure data. Every external
    // operation that needs randomness on it (adaption, random-init, cross-over position) leases a
    // proxy from the process-global Gem::Hap::randomLeasePool() for the duration of the operation, so
    // the number of live proxies is bounded by peak concurrency (O(worker threads)), not by the
    // population size.

    /***************************************************************************/
    // Pure-virtual hooks implemented by the genome layer.

    /** @brief Randomly initializes the parameters (genome-specific).
     *  @param am The activity mode selecting which parameters are affected
     *  @return true if at least one parameter was changed */
    virtual bool randomInit_(activityMode const &am) = 0;

    /** @brief The evaluation hook a concrete individual implements: computes and RETURNS this individual's
     *  raw result vector (size 1 for a single-criterion problem, one entry per criterion otherwise -- main
     *  at index 0). It reads the individual through @c this (genome via streamline(), any per-run context via
     *  the individual's own accessors) and RETURNS the results; the caller (@c runEvaluation_) writes them and
     *  applies feasibility + policy + PROCESSED, so the hook itself sets no fitness. The single evaluation
     *  call site dispatches here for a locally-evaluated individual. Implemented by every concrete
     *  individual (the value-bearing genome layer's problem definition).
     *  @return The raw result vector */
    virtual std::vector<double> evaluate() = 0;

private:
    /***************************************************************************/
    // GRateableI surface (read the stored results).

    /** @brief @param id The criterion index. @return The stored raw fitness for that criterion */
    [[nodiscard]] double raw_fitness_(std::size_t id) const final { return this->getStoredResult(id).rawFitness(); }
    /** @brief @param id The criterion index. @return The stored transformed fitness for that criterion */
    [[nodiscard]] double transformed_fitness_(std::size_t id) const final {
        return this->getStoredResult(id).transformedFitness();
    }
    /** @brief @return A vector of all stored raw fitness results */
    [[nodiscard]] std::vector<double> raw_fitness_vec_() const final;
    /** @brief @return A vector of all stored transformed fitness results */
    [[nodiscard]] std::vector<double> transformed_fitness_vec_() const final;

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

    [[nodiscard]] virtual std::size_t countParametersDouble_(activityMode const &) const = 0;
    [[nodiscard]] virtual std::size_t countParametersFloat_(activityMode const &) const = 0;
    [[nodiscard]] virtual std::size_t countParametersInt32_(activityMode const &) const = 0;
    [[nodiscard]] virtual std::size_t countParametersBool_(activityMode const &) const = 0;

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

    /** @brief Re-attaches the OA-owned scratch from @p original onto this individual (the wire omits the
     *  scratch, so a networked return arrives without it; the consumer grafts it back from the retained
     *  original -- see Gem::Courtier::GProcessable::graftOaScratchFrom()).
     *  @param original The originally-submitted item supplying the scratch (ignored if not a GOptimizableEntity) */
    void graftOaScratchFrom_(const Gem::Courtier::GProcessable &original) override {
        const auto *src = dynamic_cast<const GOptimizableEntity *>(&original);
        if(src != nullptr && src->scratch_) {
            // This deep-copy runs under the consumer's mutex (the original is reconciled in place). It is
            // deliberately a COPY, not a move: every call site discards the original immediately afterwards,
            // so a move WOULD be safe and O(1) -- but coupling correctness to that "source dies next"
            // invariant is a footgun a future reorder could trip silently. SIGNPOST: if profiling at high
            // client/return rates ever shows this mutex as hot, switch to an explicit consume -- take the
            // source by rvalue-ref (graftOaScratchFrom_(GProcessable&&)) so call sites must std::move it and
            // the steal is visible -- rather than turning this into a silent move. Until then, keep the copy.
            scratch_ = std::make_unique<GAuxiliaryStore>(*src->scratch_);
        }
    }

    /** @brief In-place return reconciliation (see Gem::Courtier::GProcessable::absorbResultsFrom): absorbs
     *  the returned item's computed results + evaluation-derived local state + processing lifecycle, while
     *  KEEPING this live population element's own genome value channels (a results-only return leaves them
     *  untouched; a full return grafts the genome separately, see GNetworkedConsumerT::checkin) and its
     *  OA-owned scratch. Keeping the object in place (rather than swapping in the deserialized return) is
     *  what preserves its heap address across a networked round-trip.
     *  @param src The returned, evaluated item whose results + lifecycle are absorbed */
    void absorbResultsFrom_(const Gem::Courtier::GProcessable &src) override;

    /** @brief In-place full deep copy (see Gem::Courtier::GProcessable::loadContentFrom): replaces this
     *  item's whole content (genome + results + scratch + lifecycle) with a copy of @p src without
     *  relocating the object, so a concurrent snapshot of population addresses stays valid. Used by the
     *  clone-on-partial-return refill to substitute a viable sibling into a failed slot.
     *  @param src The source item to deep-copy in place
     *  @return true (the optimization individual supports in-place substitution) */
    bool loadContentFrom_(const Gem::Courtier::GProcessable &src) override;

    /** @brief Default no-op constant-data load; a derived type overrides if it deposits constant data at a
     *  remote site. @param cd_ptr A template item whose constant data would be loaded into this one */
    virtual void loadConstantData_([[maybe_unused]] std::shared_ptr<GOptimizableEntity> cd_ptr) { /* nothing */ }

    /***************************************************************************/
    // Evaluation internals.

    /** @brief The evaluation body run inside process(): feasibility check + evaluate()/res_vec
     *  adoption + the evaluation-policy transform. @param res_vec Optional pre-computed raw results */
    void runEvaluation_(const std::vector<individual_processing_result> &res_vec);

    /** @brief Adopts the raw results into the result store -- either from a non-empty res_vec (external
     *  GPU/network path) or from a local evaluate() -- returning the main (index-0) raw result and storing
     *  the secondary criteria. Worst-cases the whole surface and rethrows on any failure. Extracted from
     *  runEvaluation_(). @param res_vec Optional pre-computed raw results @return The main raw result */
    double adoptRawResults_(const std::vector<individual_processing_result> &res_vec);

    /** @brief Applies the configured invalidity policy (worst-case, or the sigmoid barrier value)
     *  to the whole quality surface of a constraint-violating candidate. Shared by
     *  runEvaluation_() and setFitness_() (formerly two identical copies). */
    void applyInvalidityPolicy_();

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

    /** @brief Creates a deep clone of this object (supplied by the concrete leaf). @return A heap copy */
    [[nodiscard]] GOptimizableEntity *clone_() const override = 0;

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

    std::uint32_t assigned_iteration_ = 0; ///< The parent algorithm's optimization-cycle iteration
    double validity_level_ = 0.;        ///< How valid the current solution is (<= 1 == feasible)

    /** @brief The OA-owned scratch (personality object + per-group adaption POD blocks). Always allocated
     *  (so the accessors never null-check); deep-copied on clone/load; serialized only on a checkpoint
     *  (omitted on the wire, see serialize()); excluded from the compared identity (not in localMembers_). */
    std::unique_ptr<GAuxiliaryStore> scratch_ = std::make_unique<GAuxiliaryStore>();
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::Genome */

/******************************************************************************/
/**
 * @brief Needed for GArchive serialization
 */
/******************************************************************************/
