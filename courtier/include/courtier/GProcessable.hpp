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

#include "common/GGlobalDefines.hpp"

// Standard headers
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <random>
#include <string>
#include <tuple>

// Boost headers
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

// Geneva headers
#include "common/GCommonHelperFunctions.hpp"    // timeAndPlace
#include "common/GErrorStreamer.hpp"            // g_error_streamer, DO_LOG
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "courtier/GCourtierEnums.hpp"          // processingStatus, dispatchState, the id typedefs
#include "courtier/GCourtierHelperFunctions.hpp" // psToStr

namespace Gem::Courtier {

/******************************************************************************/
namespace detail {

/**
 * @brief Mints a process-unique 128-bit work-item lineage id: a per-process random salt combined with a
 * monotonic atomic counter. The salt makes ids minted in different runs (or after a checkpoint resume,
 * where the counter restarts at 0) non-colliding with serialized ids from an earlier run; the counter
 * makes them unique within the run. Minting is cheap (no per-call RNG) so it is affordable at every
 * work-item construction.
 * @return A fresh, process-unique 128-bit lineage id
 */
inline SUBMISSION_UUID_TYPE mint_submission_uuid() {
    static const std::uint64_t salt = [] {
        std::random_device rd;
        return (static_cast<std::uint64_t>(rd()) << 32) ^ static_cast<std::uint64_t>(rd());
    }();
    static std::atomic<std::uint64_t> counter{0};
    return SUBMISSION_UUID_TYPE{salt, counter.fetch_add(1, std::memory_order_relaxed)};
}

/**
 * @brief A self-minting lineage id whose value-semantics encode the D11 identity contract in ONE place, so
 * the enclosing GProcessable can keep its special members defaulted:
 *  - default construction / move  -> a fresh (or transferred) id, as usual;
 *  - COPY construction            -> a FRESH id (a copy of a work item is a NEW individual: offspring / refill);
 *  - COPY assignment              -> KEEPS the target's own id (load(other) must not steal the source's lineage);
 *  - Boost (de)serialization      -> the raw value is written/read, so a wire or checkpoint round-trip PRESERVES
 *                                    the id (a deserialized object is default-constructed -> fresh -> then the
 *                                    archived value overwrites it).
 * The asymmetry is intentional and is pinned by the [proc][id] unit tests.
 */
struct LineageId {
    SUBMISSION_UUID_TYPE value = mint_submission_uuid();

    LineageId() = default;
    LineageId(const LineageId & /* cp */) : value(mint_submission_uuid()) { /* a copy is a new individual */ }
    LineageId(LineageId &&) noexcept = default;
    LineageId &operator=(const LineageId & /* cp */) { return *this; /* keep our own lineage on load */ }
    LineageId &operator=(LineageId &&) noexcept = default;
    ~LineageId() = default;
};

} // namespace detail

/******************************************************************************/
// An exception to be thrown if an exception was thrown during a work item's processing. It lives on the
// non-generic lifecycle base so both the courtier transports (which catch it around a process() call) and
// any processable (GProcessingContainerT or a self-contained work item such as the geneva individual)
// throw and catch the SAME type.
class g_processing_exception : public geneva_exception {
    using geneva_exception::geneva_exception;
};

/******************************************************************************/
/**
 * @brief The non-generic processing lifecycle of a courtier work item.
 *
 * GProcessable holds everything about a work item that is INDEPENDENT of what it computes: its
 * processing status (the UNPROCESSED / DO_PROCESS / PROCESSED / EXCEPTION_CAUGHT / ERROR_FLAGGED
 * state machine), its accumulated error descriptions, the transport routing ids (the per-dispatch
 * correlation id and the stable lineage id), the transient per-batch dispatch-scheduling state, and
 * the processing timing. It carries NO result store and is NOT
 * templated on a result type, so the courtier transport and consumer machinery can reason about a work
 * item's lifecycle without knowing what it evaluates to.
 *
 * The result store, the actual process() orchestration and the (typed) pre-/post-processors live one
 * layer down, on whichever class turns a GProcessable into an actual submittable work item. There are two
 * such implementations: the generic, geneva-free GProcessingContainerT<processable_type,
 * processing_result_type> (used by the courtier-internal demo work items / tests), and geneva's
 * specialized Gem::Geneva::Genome::GOptimizableEntity (which derives this class DIRECTLY and supplies its
 * own result store + process() with optimization-specific orchestration). Either way the only coupling
 * between the status machine and the result store -- clearing stored results when the status is reset --
 * is bridged by the virtual clearStoredResults_() hook, which the result-bearing derived class overrides.
 */
class GProcessable {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /**
     * @brief Serialises the non-generic lifecycle state (status, errors, routing ids, timing).
     * The transient dispatch-scheduling state (dispatch_state_) is deliberately NOT serialised.
     *
     * @tparam Archive The Boost.Serialization archive type
     * @param ar The archive to (de)serialise the lifecycle state with
     * @param version The (unused) serialization version number
     */
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;
        // The stable lineage id travels by value (its two 64-bit halves) so a wire / checkpoint round-trip
        // PRESERVES it -- see detail::LineageId for why this preserves while clone() mints fresh.
        ar &make_nvp("submission_uuid_hi", submission_uuid_.value[0]) &
            make_nvp("submission_uuid_lo", submission_uuid_.value[1]) &
            BOOST_SERIALIZATION_NVP(correlation_id_) &
            BOOST_SERIALIZATION_NVP(pre_processing_time_) &
            BOOST_SERIALIZATION_NVP(processing_time_) &
            BOOST_SERIALIZATION_NVP(post_processing_time_) &
            BOOST_SERIALIZATION_NVP(stored_error_descriptions_) &
            BOOST_SERIALIZATION_NVP(processing_status_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    GProcessable() = default;
    GProcessable(const GProcessable &) = default;
    GProcessable(GProcessable &&) noexcept = default;
    GProcessable &operator=(const GProcessable &) = default;
    GProcessable &operator=(GProcessable &&) noexcept = default;
    virtual ~GProcessable() = default;

    /***************************************************************************/
    // Processing status

    /** @brief @return The current processing status of this work item */
    processingStatus getProcessingStatus() const noexcept { return processing_status_; }

    /** @brief @return A string representation of the current processing status (debugging) */
    std::string getProcessingStatusAsStr() const noexcept { return psToStr(processing_status_); }

    /** @brief @return true if the item carries the PROCESSED flag */
    bool is_processed() const noexcept {
        return (processingStatus::PROCESSED == this->getProcessingStatus());
    }

    /** @brief @return true if the item is currently UNPROCESSED */
    bool is_unprocessed() const noexcept {
        return (processingStatus::UNPROCESSED == this->getProcessingStatus());
    }

    /** @brief @return true if the item is due for processing (DO_PROCESS) */
    bool is_due_for_processing() const noexcept {
        return (processingStatus::DO_PROCESS == this->getProcessingStatus());
    }

    /** @brief @return true if processing produced an error (caught exception or user-flagged) */
    bool has_errors() const noexcept {
        return (processingStatus::EXCEPTION_CAUGHT == processing_status_) ||
               (processingStatus::ERROR_FLAGGED == processing_status_);
    }

    /** @brief @return true if an error was explicitly flagged by the user (ERROR_FLAGGED) */
    bool error_flagged_by_user() const noexcept {
        return (processingStatus::ERROR_FLAGGED == processing_status_);
    }

    /**
     * @brief Sets a given new processing state. Which new states are accepted depends on the current
     * state (see the body). Setting a new state of PROCESSED via this function is not allowed (it is
     * reached only by a successful process()/markAsProcessedWith()) and throws unless already set.
     * Some transitions erase existing information (past error messages and any stored results, the
     * latter via the clearStoredResults_() hook).
     *
     * @param target_ps The desired new processing status
     */
    void set_processing_status(processingStatus target_ps = processingStatus::UNPROCESSED) {
        // Do nothing if the new state is equal to the old one
        if(target_ps == processing_status_) {
            return;
        }

        // We do not accept setting a target state of PROCESSED via this function
        if(target_ps == processingStatus::PROCESSED) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GProcessable::set_processing_status():" << '\n'
                << "An attempt was made to set the processing state to PROCESSED" << '\n'
                << "which is not allowed through this function." << '\n'
            );
        }

        // We want to enforce specific targets depending on the current state
        switch(processing_status_) {
            using enum Gem::Courtier::processingStatus;
            //------------------------------------------------------------------------------------

        case UNPROCESSED:
            if(target_ps == processingStatus::DO_PROCESS) {
                processing_status_ = target_ps;
                stored_error_descriptions_.clear();
                this->clearStoredResults_();
            }
            else {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GProcessable::set_processing_status():" << '\n'
                    << "Got invalid target processing status " << psToStr(target_ps) << '\n'
                    << "Expected a new state of DO_PROCESS for the" << '\n'
                    << "current state of " << psToStr(processing_status_) << '\n'
                );
            }
            break;

            //------------------------------------------------------------------------------------

        case DO_PROCESS:
            if(target_ps == processingStatus::UNPROCESSED) {
                processing_status_ = target_ps;
                stored_error_descriptions_.clear();
                this->clearStoredResults_();
            }
            else {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GProcessable::set_processing_status():" << '\n'
                    << "Got invalid target processing status " << psToStr(target_ps) << '\n'
                    << "Expected a new state of UNPROCESSED for the" << '\n'
                    << "current state of " << psToStr(processing_status_) << '\n'
                );
            }
            break;

            //------------------------------------------------------------------------------------

        case PROCESSED:
            if(target_ps == processingStatus::UNPROCESSED ||
               target_ps == processingStatus::DO_PROCESS) {
                processing_status_ = target_ps;
                stored_error_descriptions_.clear();
                this->clearStoredResults_();
            }
            else {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GProcessable::set_processing_status():" << '\n'
                    << "Got invalid target processing status " << psToStr(target_ps) << '\n'
                    << "Expected a new state of UNPROCESSED or DO_PROCESS for the" << '\n'
                    << "current state of " << psToStr(processing_status_) << '\n'
                );
            }
            break;

            //------------------------------------------------------------------------------------

        case EXCEPTION_CAUGHT:
        case ERROR_FLAGGED:
            if(target_ps == processingStatus::UNPROCESSED ||
               target_ps == processingStatus::DO_PROCESS) {
                processing_status_ = target_ps;
                stored_error_descriptions_.clear();
                this->clearStoredResults_();
            }
            else {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GProcessable::set_processing_status():" << '\n'
                    << "Got invalid target processing status " << psToStr(target_ps) << '\n'
                    << "Expected a new state of UNPROCESSED or DO_PROCESS for the" << '\n'
                    << "current state of " << psToStr(processing_status_) << '\n'
                );
            }
            break;

            //------------------------------------------------------------------------------------
        };
    }

    /** @brief Marks this item as being due for processing (DO_PROCESS). */
    void mark_as_due_for_processing() { processing_status_ = processingStatus::DO_PROCESS; }

    /** @brief Sets the UNPROCESSED flag so the item will not be processed. */
    void mark_as_ignorable() { processing_status_ = processingStatus::UNPROCESSED; }

    /***************************************************************************/
    // Transport routing

    /**
     * @brief Sets the transport correlation id -- the token used to route/match a work item through the
     * transport layer (a (batch_id, slot) token in the courtier networked consumers).
     * @param id The transport correlation id to store on this work item
     */
    void setCorrelationId(const CORRELATION_ID_TYPE &id) noexcept { correlation_id_ = id; }
    /** @brief @return The transport correlation id stored on this work item (see setCorrelationId()) */
    CORRELATION_ID_TYPE getCorrelationId() const noexcept { return correlation_id_; }

    /**
     * @brief The stable, per-individual lineage id (D11). Minted once at construction, preserved across
     * every (re-)dispatch and serialized round-trip, fresh only on clone(). Unlike the per-dispatch
     * correlation id (which routes a single return to its slot), this identifies the INDIVIDUAL, so a late
     * return can be reunited with -- and de-duplicated against -- the live individual it belongs to even
     * after it has been resubmitted under a new correlation id.
     * @return This work item's 128-bit lineage id
     */
    SUBMISSION_UUID_TYPE getSubmissionUuid() const noexcept { return submission_uuid_.value; }
    /**
     * @brief Overwrites the lineage id. Used only to RESTORE a source's identity onto a retention clone
     * (cloneForRetention): a plain clone mints a fresh id, but a retained original must keep the lineage of
     * the item it stands in for. Not for general use -- the id is otherwise immutable after construction.
     * @param uuid The lineage id to stamp onto this work item
     */
    void setSubmissionUuid(const SUBMISSION_UUID_TYPE &uuid) noexcept { submission_uuid_.value = uuid; }

    /**
     * @brief Sets the courtier per-batch scheduling state. This is transient, server-side-only
     * bookkeeping (NOT serialized): it lets a networked consumer track, on the item itself, whether
     * the slot is awaiting a client / in flight / done within one dispatch round.
     * @param s The new per-batch dispatch/scheduling state for this work item
     */
    void setDispatchState(dispatchState s) noexcept { dispatch_state_ = s; }
    /** @brief @return The current per-batch dispatch/scheduling state (see setDispatchState()) */
    dispatchState getDispatchState() const noexcept { return dispatch_state_; }

    /***************************************************************************/
    // Timing

    /** @brief @return A tuple of (pre-processing, processing, post-processing) times in seconds */
    std::tuple<double, double, double> getProcessingTimes() const {
        return std::make_tuple(pre_processing_time_, processing_time_, post_processing_time_);
    }

    /***************************************************************************/
    // Error handling

    /**
     * @brief Retrieves and clears the stored exceptions, resetting the processing status.
     * @param ps The desired new processing status to set after extracting the stored exceptions
     * @return The stored error descriptions that were present before clearing
     */
    std::string get_and_clear_exceptions(processingStatus ps = processingStatus::UNPROCESSED) {
        std::string stored_exceptions = stored_error_descriptions_;
        this->set_processing_status(ps);
        return stored_exceptions;
    }

    /** @brief @return The accumulated error descriptions stored during processing */
    std::string getStoredErrorDescriptions() const { return stored_error_descriptions_; }

    /***************************************************************************/
    // OA scratch transfer (server-side return reconciliation)

    /**
     * @brief Re-attaches the optimization-algorithm scratch of @p original onto this work item.
     *
     * The OA-owned scratch (a geneva concept: the per-individual personality object + per-group adaption
     * POD state) is deliberately omitted on the wire, so a work item that has made a networked round-trip
     * comes back WITHOUT it. When a networked consumer reconciles such a return against the
     * originally-submitted item it still holds, it calls this to graft the scratch back from that
     * original, so the live population element keeps its evolved OA state across the round-trip. The base
     * carries no scratch, so this is a no-op unless a derived class (geneva's individual) overrides it.
     * @param original The originally-submitted item supplying the scratch to graft back
     */
    void graftOaScratchFrom(const GProcessable &original) { this->graftOaScratchFrom_(original); }

    /***************************************************************************/
    // In-place, pointer-preserving return reconciliation (server-side).

    /**
     * @brief Absorbs the computed results + processing lifecycle of a returned item @p src INTO this
     * work item, keeping this item's own heap address and lineage id.
     *
     * This is the pointer-preserving counterpart of "the returned object becomes the slot": rather than
     * overwriting a population slot's owning pointer with the deserialized return (which would free the
     * original object and change its address), a networked consumer keeps the originally-submitted object
     * and absorbs only what the worker computed. Address stability lets other parts of the system (e.g. a
     * per-individual prefetch overlapping evaluation) hold a snapshot of population addresses across a
     * submission. The lifecycle (status / errors / timing / routing counters / correlation id) is copied
     * while THIS item keeps its own stable lineage id; a result-bearing derived class additionally copies
     * its result store, and -- as documented on the geneva override -- deliberately keeps its own input
     * data (results-only return) and OA scratch.
     *
     * @param src The returned, evaluated item whose results + lifecycle are absorbed into this one
     */
    void absorbResultsFrom(const GProcessable &src) { this->absorbResultsFrom_(src); }

    /**
     * @brief Whether this item arrived without its input data (a results-only return), so the server
     * must graft the input data back on via graftInputDataFrom() before using the item. Default
     * false; a work-item type supporting the lightweight return form overrides the hook.
     * @return true iff the input data was omitted on the wire and must be grafted
     */
    bool inputDataOmitted() const { return this->inputDataOmitted_(); }

    /**
     * @brief Grafts the input data of @p original onto this (results-only) item. A no-op in the
     * base; a work-item type supporting the lightweight return form overrides the hook.
     * @param original The originally-submitted item supplying the input data
     */
    void graftInputDataFrom(const GProcessable &original) { this->graftInputDataFrom_(original); }

    /**
     * @brief Replaces this work item's content in place with a deep copy of @p src, keeping this item's
     * heap address (a fresh lineage id is the caller's responsibility -- a refill is a new individual).
     *
     * Used by the clone-on-partial-return refill to substitute a viable sibling into a failed slot without
     * relocating it (so a concurrent address snapshot stays valid). The base has no content and returns
     * false ("not supported"); a result-bearing derived class overrides it to perform the in-place copy
     * and returns true. A consumer that gets false falls back to clone-and-replace (correct for the
     * non-optimization demo item types, which need no address stability).
     *
     * @param src The source item to deep-copy into this one
     * @return true if the in-place content copy was performed; false if unsupported (caller should replace)
     */
    [[nodiscard]] bool loadContentFrom(const GProcessable &src) { return this->loadContentFrom_(src); }

protected:
    /***************************************************************************/
    /**
     * @brief Hook: clears the (result-bearing derived class's) stored results when the processing
     * status is reset. The base has no result store, so the default is a no-op; GProcessingContainerT
     * overrides it to clear its result vector.
     */
    virtual void clearStoredResults_() { /* no result store in the base */ }

    /**
     * @brief Hook: re-attaches the OA-owned scratch from @p original (see graftOaScratchFrom()). The base
     * carries no scratch, so the default is a no-op; geneva's GOptimizableEntity overrides it to deep-copy
     * the scratch back.
     * @param original The originally-submitted item supplying the scratch to graft back
     */
    virtual void graftOaScratchFrom_([[maybe_unused]] const GProcessable &original) {
        /* no OA scratch in the base */
    }

    /**
     * @brief Hook behind absorbResultsFrom(): copies the non-generic processing lifecycle (status,
     * errors, timing, routing counters, correlation id) from @p src into this item while keeping this
     * item's own lineage id (LineageId's copy-assignment keeps the target's value). A result-bearing
     * derived class overrides this to additionally copy its result store (and to keep genome / scratch).
     * @param src The returned item whose lifecycle state is absorbed
     */
    virtual void absorbResultsFrom_(const GProcessable &src) { GProcessable::operator=(src); }

    /** @brief Hook behind inputDataOmitted(): whether this item is a results-only return. Default false.
     *  @return false in the base */
    virtual bool inputDataOmitted_() const { return false; }

    /** @brief Hook behind graftInputDataFrom(): grafts @p original's input data onto this item. Default no-op.
     *  @param original The originally-submitted item supplying the input data (unused in the default) */
    virtual void graftInputDataFrom_([[maybe_unused]] const GProcessable &original) { /* nothing */ }

    /**
     * @brief Hook behind loadContentFrom(): performs an in-place deep copy of @p src into this item.
     * The base carries no content and cannot, so it returns false; a result-bearing derived class
     * overrides it to copy its full content and returns true.
     * @param src The source item to copy in place
     * @return true if the in-place copy was performed; false in the base (unsupported)
     */
    virtual bool loadContentFrom_([[maybe_unused]] const GProcessable &src) { return false; }

    /**
     * @brief Lets derived classes flag a custom error condition. Sets the ERROR_FLAGGED status and
     * appends the (non-empty) message to the stored error descriptions.
     * @param error_info An error description (must not be empty; appended to any existing descriptions)
     */
    void force_set_error(const std::string &error_info) {
        if(error_info.empty()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GProcessable::force_set_error(): Error info is empty" << '\n'
            );
        }
        // There may already be information stored in this variable. Hence we attach via +=
        stored_error_descriptions_ += error_info;
        processing_status_ = processingStatus::ERROR_FLAGGED;
    }

    /***************************************************************************/
    // Data -- protected so the result-bearing derived class's process()/etc. can read/write it.

    /// Stable, per-individual lineage id (D11). A self-minting value type whose copy/assign/serialize
    /// semantics implement "fresh on clone, kept on load, preserved on the wire" -- see detail::LineageId.
    detail::LineageId submission_uuid_;

    CORRELATION_ID_TYPE correlation_id_ = CORRELATION_ID_TYPE();

    /// Transient, server-side-only per-batch scheduling state for the networked consumers.
    /// Deliberately NOT part of serialize() (the wire/clone never needs it; see dispatchState).
    dispatchState dispatch_state_ = dispatchState::NONE;

    double pre_processing_time_ = 0.;  ///< Time needed for pre-processing (seconds)
    double processing_time_ = 0.;      ///< Time needed for the actual processing step (seconds)
    double post_processing_time_ = 0.; ///< Time needed for post-processing (seconds)

    std::string
        stored_error_descriptions_; ///< Stores exceptions that may have occurred during processing
    processingStatus processing_status_ =
        processingStatus::UNPROCESSED; ///< By default no processing is initiated
};

/******************************************************************************/

} /* namespace Gem::Courtier */
