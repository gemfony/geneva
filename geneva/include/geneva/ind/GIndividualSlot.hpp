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
#include <memory>
#include <string>
#include <tuple>

// Boost header files go here
#include <boost/serialization/export.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/unique_ptr.hpp>

// Geneva headers go here
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "common/GMemberReflectionT.hpp"
#include "courtier/GProcessingContainerT.hpp" // GProcessableT -- the lean courtier work-item base
#include "courtier/GWireSerializationContext.hpp" // the wire-vs-checkpoint (de)serialization scope
#include "geneva/ind/GAuxiliaryStore.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"

namespace Gem::Geneva::Genome {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A population element: the pairing of an individual ("the part that travels") with the
 * optimization-algorithm-owned scratch it accumulates while a given algorithm holds it.
 *
 * Rationale (struct-based population): the per-individual scratch an optimization algorithm
 * accumulates (the personality traits + the per-group adaption POD state, plus swarm velocity / pbest,
 * gradient, ...) is kept OUT of the individual so the individual stays pure data and needs no
 * serialization-purpose split (transport vs. checkpoint). GIndividualSlot holds that scratch on the
 * population element:
 *  - individual_ — the genome + parameter bounds (shared GGenomeLayout) + fitness + multi-constraint +
 *    the courtier processing container (correlation id / status). This is exactly the object that is
 *    shipped to a remote worker and back; it is, by itself, pure data.
 *  - scratch_    — the GAuxiliaryStore (personality object + opaque per-group POD blocks), owned by
 *    whichever algorithm currently holds the slot, dropped at the algorithm boundary.
 *
 * Bundling scratch WITH the individual in one struct keeps the two coherent through every EA
 * sort / select / swap; a parallel vector<Scratch> would have to be permuted in lockstep.
 *
 * Courtier stays individual-based: the optimization algorithm swaps individual_ out into a submission
 * span for workOn() and back afterwards (see individualPtr() / releaseIndividual() / resetIndividual()),
 * so the broker still deals in individuals — no courtier change.
 *
 * Serialization vs. comparison:
 *  - compare_() compares the wrapped individual ONLY. The scratch is OA-installed and is deliberately
 *    kept out of the compared identity, so two slots holding equal individuals but touched by different
 *    algorithms compare equal (mirrors how the personality was already excluded from the individual's
 *    compared identity).
 *  - serialize() is FULL: the individual plus the scratch personality. A checkpoint / general
 *    serialization needs the personality back in place (e.g. a resumed swarm's personal-best lives in
 *    it). The transient POD adaption blocks are re-seeded from the genome on load and are not
 *    serialized. Because scratch now lives on the slot rather than the individual, the individual's own
 *    serialize() can become unconditionally pure (the transport/checkpoint flag is retired in 10.1).
 *
 * Access is via the explicit .individual() accessor (there is intentionally NO operator-> forwarding to
 * the individual): it maximises compiler-guided migration — an individual-method call left on a slot is
 * a hard compile error until rebound — and avoids the silent-meaning trap that clone_unique()/clone()/
 * load() (present on BOTH the slot and the individual via the common interface) would otherwise create.
 */
class GIndividualSlot // NOLINT(cppcoreguidelines-special-member-functions)
  : public Gem::Common::GCommonInterfaceT<GIndividualSlot>
  , public Gem::Courtier::GProcessableT<GIndividualSlot> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /***************************************************************************/
    /**
     * The single declaration of this class'es compared/serialized members. Only the wrapped individual
     * is listed: it is the slot's compared identity. The scratch personality is serialized separately
     * (see serialize()) so it rides along for a checkpoint without being part of the comparison.
     */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(
            Gem::Common::make_cloneable_member("individual_", self.individual_)
        );
    }

    /**
     * @brief Boost.Serialization hook: serializes the wrapped individual plus the OA-owned scratch.
     * @tparam Archive The Boost.Serialization archive type.
     * @param ar The archive to read from / write to.
     * @param version The serialization format version (ignored).
     */
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        // The Gem::Common::GCommonInterfaceT<GIndividualSlot> CRTP base carries no state. The courtier
        // work-item base (GProcessableT) carries the transport coordination (status / correlation id /
        // counters) and travels both directions, so it is serialized as a base_object.
        ar &make_nvp(
            "GProcessableT",
            boost::serialization::base_object<Gem::Courtier::GProcessableT<GIndividualSlot>>(*this)
        );

        // The wrapped individual -- the part that travels -- derived from the single localMembers()
        // declaration so serialize()/load_()/compare_() stay in lock-step.
        Gem::Common::serialize_members(ar, localMembers_(*this));

        // The pre-/post-processor objects (e.g. an EA post-optimizer) are work-item processing config:
        // they must reach the remote worker to run there, so they travel in BOTH modes (NOT scratch).
        ar &make_nvp("pre_processor_ptr_", pre_processor_ptr_) &
            make_nvp("post_processor_ptr_", post_processor_ptr_) &
            make_nvp("pre_processing_disabled_", pre_processing_disabled_) &
            make_nvp("post_processing_disabled_", post_processing_disabled_);

        // The OA-owned scratch (the personality OBJECT *and* the per-group POD blocks -- adaption sigma,
        // swarm velocity, conjugate-gradient memory) is serialized in exactly TWO modes, driven by the
        // thread-local wire (de)serialization scope (the same switch the genome's layout send-once uses):
        //  - CHECKPOINT (no active wire scope): the scratch rides along, so a resumed algorithm keeps its
        //    evolved state in place -- self-contained archive. On resume the optimization algorithm
        //    preserves this restored scratch instead of re-seeding it (see
        //    GOptimizationAlgorithmBase::resumed_from_checkpoint_).
        //  - WIRE (a scope is active and enabled): the scratch is OMITTED. A remote worker needs no
        //    OA scratch to evaluate, and on the return the server keeps the LIVE slot's scratch (which
        //    never travelled) -- only the genome + coordination move back in (see adoptIndividualFrom()).
        // The decision is symmetric on both wire ends (the scope is engaged on serialize AND deserialize),
        // so the archive layouts always match. scratch_ is OUT of localMembers() so it is serialized but
        // NOT part of the compared identity.
        const auto *wire_ctx = Gem::Courtier::GWireSerializationScope::current();
        const bool over_the_wire = (wire_ctx != nullptr) && wire_ctx->enabled;
        if(not over_the_wire) {
            ar &make_nvp("scratch_", scratch_);
        }
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The courtier work-item result type, mirrored from the wrapped genome so the slot satisfies
     *  the duck-typed work-item contract (Gem::Courtier::ProcessableWorkItem) the client / command
     *  container check -- without deriving from GProcessingContainerT (the genome still owns it). */
    using result_type = GOptimizableEntity::result_type;

    /** @brief The default constructor creates an empty slot (no individual) */
    GIndividualSlot() = default;

    /**
     * @brief Wraps an existing individual into a fresh slot (the scratch starts empty).
     * @param ind The individual to take ownership of (moved into the slot).
     */
    explicit GIndividualSlot(std::unique_ptr<GOptimizableEntity> ind)
      : individual_(std::move(ind)) {
    }

    /**
     * @brief The copy constructor deep-clones the individual and deep-copies the scratch.
     * @param cp The slot to copy from.
     */
    GIndividualSlot(const GIndividualSlot &cp)
      : Gem::Common::GCommonInterfaceT<GIndividualSlot>(cp)
      , Gem::Courtier::GProcessableT<GIndividualSlot>(cp)
      , scratch_(cp.scratch_)
      , pre_processing_disabled_(cp.pre_processing_disabled_)
      , post_processing_disabled_(cp.post_processing_disabled_) {
        Gem::Common::copyCloneableSmartPointer(cp.individual_, individual_);
        Gem::Common::copyCloneableSmartPointer(cp.pre_processor_ptr_, pre_processor_ptr_);
        Gem::Common::copyCloneableSmartPointer(cp.post_processor_ptr_, post_processor_ptr_);
    }

    /** @brief The move constructor */
    GIndividualSlot(GIndividualSlot &&) noexcept = default;

    /** @brief The destructor */
    ~GIndividualSlot() override = default;

    /**
     * @brief Copy assignment via the load_() deep-copy protocol.
     * @param cp The slot to copy from.
     * @return *this.
     */
    GIndividualSlot &operator=(const GIndividualSlot &cp) {
        if(this != &cp) {
            this->load_(&cp);
        }
        return *this;
    }

    /** @brief Move assignment. @return *this. */
    GIndividualSlot &operator=(GIndividualSlot &&) noexcept = default;

    /***************************************************************************/
    // Access. Explicit -- no operator-> forwarding to the individual (see the class note).

    /** @brief Whether this slot currently holds an individual. @return true if an individual is held. */
    bool hasIndividual() const noexcept {
        return static_cast<bool>(individual_);
    }

    /** @brief The wrapped individual. @return A reference to the held individual. */
    GOptimizableEntity &individual() noexcept {
        return *individual_;
    }
    /** @brief The wrapped individual (const). @return A const reference to the held individual. */
    const GOptimizableEntity &individual() const noexcept {
        return *individual_;
    }

    /**
     * @brief The wrapped individual, statically cast to the user's concrete problem type. Convenience for
     * the user-facing OUTPUT boundary (e.g. inspecting a best slot as the problem subclass it really is).
     * @tparam derived_type The concrete GFlatIndividualT subclass the genome was built as.
     * @return A reference to the wrapped individual as derived_type.
     */
    template <typename derived_type>
    derived_type &genomeAs() noexcept {
        return static_cast<derived_type &>(*individual_);
    }
    /** @brief The wrapped individual as the user's concrete problem type (const). @tparam derived_type The concrete subclass. @return A const reference as derived_type. */
    template <typename derived_type>
    const derived_type &genomeAs() const noexcept {
        return static_cast<const derived_type &>(*individual_);
    }

    /**
     * @brief The owning pointer to the wrapped individual. Used by the optimization algorithm to swap
     * the individual out into a courtier submission span for workOn() and back afterwards, so the broker
     * keeps dealing in individuals.
     * @return A reference to the owning unique_ptr holding the individual.
     */
    std::unique_ptr<GOptimizableEntity> &individualPtr() noexcept {
        return individual_;
    }
    /**
     * @brief The owning pointer to the wrapped individual (const).
     * @return A const reference to the owning unique_ptr holding the individual.
     */
    const std::unique_ptr<GOptimizableEntity> &individualPtr() const noexcept {
        return individual_;
    }

    /***************************************************************************/
    // Processing-coordination surface. The slot IS the courtier work item now: the transport status /
    // correlation id / dispatch state / counters and the process() lifecycle are INHERITED from
    // GProcessableT (no longer forwarded to the genome). process() drives the genome's evaluate() via
    // runProcessing_() below. What remains here forwards GENOME data the algorithms read off the slot:
    // the fitness criterion count / stored result, and the assigned-iteration tag (a genome member).
    // NOTE: the fitness accessors (raw_fitness / transformed_fitness / getFitnessTuple) are NOT here --
    // fitness is solution data on the genome (accessed via individual()).

    /** @brief A stored fitness result of the wrapped genome. @param id The result index. @return The stored result. */
    individual_processing_result getStoredResult(std::size_t id = 0) const {
        return individual_->getStoredResult(id);
    }
    /** @brief The number of stored fitness criteria on the wrapped genome. @return The criterion count. */
    std::size_t getNStoredResults() const { return individual_->getNStoredResults(); }

    /** @brief Sets the iteration in which the wrapped genome was submitted. @param iter The assigned iteration. */
    void setAssignedIteration(std::uint32_t const &iter) { individual_->setAssignedIteration(iter); }
    /** @brief The iteration in which the wrapped genome was submitted. @return The assigned iteration. */
    std::uint32_t getAssignedIteration() const { return individual_->getAssignedIteration(); }

    /** @brief Marks this work item PROCESSED directly (for an external evaluator that computed the
     *  genome's fitness out of band, e.g. the GPU consumer's scatter()). */
    void markProcessed() { this->mark_as_processed_(); }

    /***************************************************************************/
    // Pre-/post-processing (work-item operations run on the wrapped genome around evaluation). The
    // processor objects are owned by the SLOT and stamped onto it at population creation (by Go2 / the
    // optimization algorithm, from the content factory). They travel to a remote worker so the
    // processing runs there. See GProcessableT's before/afterProcessing_ seams, overridden below.

    /** @brief Whether pre-processing may run. @return true unless vetoed. */
    bool mayBePreProcessed() const noexcept { return not pre_processing_disabled_; }
    /** @brief Vetoes / allows pre-processing. @param veto true to disable. */
    void vetoPreProcessing(bool veto) noexcept { pre_processing_disabled_ = veto; }
    /** @brief Registers a pre-processor (ignored if empty). @param p The pre-processor. */
    void registerPreProcessor(
        std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GOptimizableEntity>> p
    ) {
        if(p) { pre_processor_ptr_ = std::move(p); }
    }
    /** @brief Whether post-processing may run. @return true unless vetoed. */
    bool mayBePostProcessed() const noexcept { return not post_processing_disabled_; }
    /** @brief Vetoes / allows post-processing. @param veto true to disable. */
    void vetoPostProcessing(bool veto) noexcept { post_processing_disabled_ = veto; }
    /** @brief Registers a post-processor (ignored if empty). @param p The post-processor. */
    void registerPostProcessor(
        std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GOptimizableEntity>> p
    ) {
        if(p) { post_processor_ptr_ = std::move(p); }
    }
    /** @brief The registered post-processor (or empty). @return The post-processor. */
    std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GOptimizableEntity>> postProcessor() const {
        return post_processor_ptr_;
    }
    /** @brief Removes any registered post-processor. */
    void clearPostProcessor() { post_processor_ptr_.reset(); }
    /***************************************************************************/

    /**
     * @brief Moves the individual out of the slot, leaving it empty.
     * @return The owning pointer to the individual (the slot is empty afterwards).
     */
    std::unique_ptr<GOptimizableEntity> releaseIndividual() {
        return std::move(individual_);
    }

    /**
     * @brief Moves an individual into the slot, replacing any previous one.
     * @param ind The individual to take ownership of (moved into the slot).
     */
    void resetIndividual(std::unique_ptr<GOptimizableEntity> ind) {
        individual_ = std::move(ind);
    }

    /**
     * @brief Reconciles a returned (remotely evaluated) slot INTO this live slot, preserving this slot's
     * scratch. The evaluated genome (which carries its own fitness) moves in, AND the returned slot's
     * transport coordination -- crucially the processing STATUS (PROCESSED / error) -- is carried onto
     * this live slot; this slot's OA scratch is left untouched because it never travelled over the wire
     * (see serialize()). This is the server-side return path: the live slot stays in the population and is
     * updated in place, so its scratch is never lost.
     * @param returned The deserialized result slot whose genome + coordination are adopted (it is emptied).
     */
    void adoptIndividualFrom(GIndividualSlot &returned) {
        // The returned genome arrived over the wire with its population-invariant shared structure (the
        // flat genome's layout) OMITTED -- re-attach it from THIS live slot's genome, which still holds it,
        // before the move replaces our genome. A no-op for genomes that carry no detachable structure.
        if(individual_ && returned.individual_) {
            returned.individual_->adoptOmittedStructureFrom(*individual_);
        }
        individual_ = std::move(returned.individual_);
        // The transport coordination (processing status, counters, timing) now lives on the SLOT, not the
        // genome -- so it must be carried over explicitly: without this the live slot would stay
        // not-PROCESSED and the algorithm would treat the item as unfinished (resubmit / FATAL).
        this->load_processable_(&returned);
    }

    /** @brief The optimization-algorithm-owned scratch (personality + POD adaption state). @return A reference to the scratch store. */
    GAuxiliaryStore &scratch() noexcept {
        return scratch_;
    }
    /** @brief The OA-owned scratch (const). @return A const reference to the scratch store. */
    const GAuxiliaryStore &scratch() const noexcept {
        return scratch_;
    }

    /***************************************************************************/
    // Personality (the per-individual OA object). In the struct-based population it belongs to the slot
    // (OA-owned scratch). These accessors give the optimization algorithms a uniform personality surface.

    /**
     * @brief Converts the personality base pointer to the desired type. Only accessible when
     * personality_type derives from GPersonalityTraits.
     * @tparam personality_type The concrete personality-traits type to convert to (must derive from GPersonalityTraits).
     * @return A shared pointer to the personality traits cast to personality_type.
     * @throw geneva_exception (DEBUG builds) if the personality pointer is empty.
     */
    template <typename personality_type>
        requires std::derived_from<personality_type, GPersonalityTraits>
    std::shared_ptr<personality_type> getPersonalityTraits() {
#ifdef DEBUG
        if(not scratch_.personalityRef()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GIndividualSlot::getPersonalityTraits<personality_type>() : Empty personality "
                   "pointer found"
                << '\n'
            );
        }
#endif /* DEBUG */
        return Gem::Common::convertSmartPointer<GPersonalityTraits, personality_type>(scratch_.personalityRef());
    }

    /**
     * @brief The personality-traits base pointer.
     * @return A shared pointer to the slot's GPersonalityTraits.
     * @throw geneva_exception (DEBUG builds) if the personality pointer is empty.
     */
    std::shared_ptr<GPersonalityTraits> getPersonalityTraits() {
#ifdef DEBUG
        if(not scratch_.personalityRef()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GIndividualSlot::getPersonalityTraits() : Empty personality pointer found" << '\n'
            );
        }
#endif /* DEBUG */
        return scratch_.personalityRef();
    }

    /**
     * @brief Sets the personality of this slot.
     * @param gpt The personality-traits object to install (must be non-null).
     * @throw geneva_exception if gpt is empty.
     */
    void setPersonality(std::shared_ptr<GPersonalityTraits> gpt) {
        if(not gpt) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GIndividualSlot::setPersonality() : Empty personality pointer passed" << '\n'
            );
        }
        scratch_.personalityRef() = std::move(gpt);
    }

    /** @brief Resets the OA-owned scratch (personality + any POD blocks held on the slot) */
    void resetPersonality() {
        scratch_.clearScratch();
    }

    /**
     * @brief A string identifier for the current personality.
     * @return The personality's name(), or "PERSONALITY_NONE" if no personality is set.
     */
    std::string getPersonality() const {
        if(scratch_.personalityRef()) {
            return scratch_.personalityRef()->name();
        }
        return std::string("PERSONALITY_NONE");
    }

protected:
    /***************************************************************************/
    /**
     * @brief Loads the data of another GIndividualSlot into this one (deep copy).
     * @param cp The slot whose data is copied into this one.
     */
    void load_(const GIndividualSlot *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GIndividualSlot>(
        GIndividualSlot const &,
        GIndividualSlot const &,
        Gem::Common::GToken &
    );

    /**
     * @brief Searches for compliance with expectations with respect to another object of the same type.
     * @param cp The other object to compare against (the wrapped individual only; scratch excluded).
     * @param e The expectation for this object, e.g. equality.
     * @param limit The limit for allowed deviations of floating point types.
     */
    void compare_(
        GIndividualSlot const &cp // the other object
        ,
        Gem::Common::expectation const &e // the expectation for this object, e.g. equality
        ,
        [[maybe_unused]] double const &limit // the limit for allowed deviations of floating point types
    ) const override;

    /**
     * @brief Applies modifications to this object. This is needed for testing purposes.
     * @return true if at least one modification was applied, false otherwise.
     */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /***************************************************************************/
    /** @brief Emits a name for this class / object. @return The class name string. */
    std::string name_() const override;
    /** @brief Creates a deep clone of this object. @return A heap-allocated deep copy of this slot. */
    GIndividualSlot *clone_() const override;

    /***************************************************************************/
    /**
     * @brief The actual processing work (GProcessableT seam): drives the wrapped genome's own
     * evaluation, then translates a genome-side user-flagged error into a transport error so the
     * lean lifecycle envelope reports it (-> resubmission). A thrown fitnessCalculation propagates
     * to the envelope's catch (-> EXCEPTION_CAUGHT) on its own.
     */
    void runProcessing_() final {
        individual_->evaluate();
        if(individual_->evaluationFailed()) {
            this->force_set_error(individual_->evaluationErrorDescription());
        }
    }

    /** @brief Pre-processing seam: runs the registered pre-processor (if any, not vetoed) on the genome. */
    void beforeProcessing_() final {
        if(this->mayBePreProcessed() && pre_processor_ptr_) {
            (*pre_processor_ptr_)(*individual_);
        }
    }

    /** @brief Post-processing seam: runs the registered post-processor (if any, not vetoed) on the genome. */
    void afterProcessing_() final {
        if(this->mayBePostProcessed() && post_processor_ptr_) {
            (*post_processor_ptr_)(*individual_);
        }
    }

    /***************************************************************************/
    // Data

    /** @brief The wrapped individual -- the part that travels (genome + bounds + fitness + constraint) */
    std::unique_ptr<GOptimizableEntity> individual_;

    /** @brief The optimization-algorithm-owned scratch (personality object + per-group POD blocks) */
    GAuxiliaryStore scratch_;

    /** @brief Optional pre-/post-processor objects (work-item operations run on the genome around
     *  evaluation; serialized so they reach a remote worker). */
    std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GOptimizableEntity>> pre_processor_ptr_;
    std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GOptimizableEntity>> post_processor_ptr_;
    bool pre_processing_disabled_ = false; ///< Whether pre-processing has been vetoed
    bool post_processing_disabled_ = false; ///< Whether post-processing has been vetoed
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::Genome */

/******************************************************************************/
/** @brief Needed for Boost.Serialization of the slot through a (polymorphic) pointer */
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::Genome::GIndividualSlot) // NOLINT
/******************************************************************************/
