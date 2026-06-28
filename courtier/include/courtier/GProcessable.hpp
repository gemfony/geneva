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
#include <chrono>
#include <string>
#include <tuple>

// Boost headers
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

// Geneva headers
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "courtier/GCourtierEnums.hpp"          // processingStatus, dispatchState, the counter typedefs
#include "courtier/GCourtierHelperFunctions.hpp" // psToStr

namespace Gem::Courtier {

/******************************************************************************/
/**
 * @brief The non-generic processing lifecycle of a courtier work item.
 *
 * GProcessable holds everything about a work item that is INDEPENDENT of what it computes: its
 * processing status (the UNPROCESSED / DO_PROCESS / PROCESSED / EXCEPTION_CAUGHT / ERROR_FLAGGED
 * state machine), its accumulated error descriptions, the transport routing/bookkeeping counters
 * (iteration / resubmission / collection-position / correlation id), the transient per-batch
 * dispatch-scheduling state, and the broker/processing timing. It carries NO result store and is NOT
 * templated on a result type, so the courtier transport and consumer machinery can reason about a work
 * item's lifecycle without knowing what it evaluates to.
 *
 * The result store, the actual process() orchestration and the (typed) pre-/post-processors live one
 * layer down, on GProcessingContainerT<processable_type, processing_result_type>, which derives from
 * this class. The only coupling between the status machine and the result store -- clearing stored
 * results when the status is reset -- is bridged by the virtual clearStoredResults_() hook, which the
 * result-bearing derived class overrides.
 */
class GProcessable {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /**
     * @brief Serialises the non-generic lifecycle state (status, errors, routing counters, timing).
     * The transient dispatch-scheduling state (dispatch_state_) is deliberately NOT serialised.
     *
     * @tparam Archive The Boost.Serialization archive type
     * @param ar The archive to (de)serialise the lifecycle state with
     * @param version The (unused) serialization version number
     */
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        ar &BOOST_SERIALIZATION_NVP(iteration_counter_) &
            BOOST_SERIALIZATION_NVP(resubmission_counter_) &
            BOOST_SERIALIZATION_NVP(collection_position_) &
            BOOST_SERIALIZATION_NVP(correlation_id_) &
            BOOST_SERIALIZATION_NVP(pre_processing_time_) &
            BOOST_SERIALIZATION_NVP(processing_time_) &
            BOOST_SERIALIZATION_NVP(post_processing_time_) &
            BOOST_SERIALIZATION_NVP(broker_raw_retrieval_time_) &
            BOOST_SERIALIZATION_NVP(broker_raw_submission_time_) &
            BOOST_SERIALIZATION_NVP(broker_proc_retrieval_time_) &
            BOOST_SERIALIZATION_NVP(broker_proc_submission_time_) &
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
    // Transport routing / bookkeeping counters

    /** @brief Sets the iteration counter. @param counter The iteration counter value to store */
    void setIterationCounter(const ITERATION_COUNTER_TYPE &counter) noexcept {
        iteration_counter_ = counter;
    }
    /** @brief @return The iteration counter stored on this work item */
    ITERATION_COUNTER_TYPE getIterationCounter() const noexcept { return iteration_counter_; }

    /** @brief Sets the resubmission counter. @param resubmission_counter The value to store */
    void setResubmissionCounter(const RESUBMISSION_COUNTER_TYPE &resubmission_counter) noexcept {
        resubmission_counter_ = resubmission_counter;
    }
    /** @brief @return The resubmission counter stored on this work item */
    RESUBMISSION_COUNTER_TYPE getResubmissionCounter() const noexcept {
        return resubmission_counter_;
    }

    /** @brief Sets the position inside the submitted collection. @param pos The position to store */
    void setCollectionPosition(const COLLECTION_POSITION_TYPE &pos) noexcept {
        collection_position_ = pos;
    }
    /** @brief @return The position of this work item within its submitted collection */
    COLLECTION_POSITION_TYPE getCollectionPosition() const noexcept { return collection_position_; }

    /**
     * @brief Sets the transport correlation id -- the token used to route/match a work item through the
     * transport layer (a (batch_id, slot) token in the courtier networked consumers).
     * @param id The transport correlation id to store on this work item
     */
    void setCorrelationId(const CORRELATION_ID_TYPE &id) noexcept { correlation_id_ = id; }
    /** @brief @return The transport correlation id stored on this work item (see setCorrelationId()) */
    CORRELATION_ID_TYPE getCorrelationId() const noexcept { return correlation_id_; }

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

    /** @brief @return The time point at which this item was retrieved from the raw queue */
    std::chrono::high_resolution_clock::time_point getRawRetrievalTime() const {
        return broker_raw_retrieval_time_;
    }
    /** @brief @return The time point at which this item was submitted to the raw queue */
    std::chrono::high_resolution_clock::time_point getRawSubmissionTime() const {
        return broker_raw_submission_time_;
    }
    /** @brief @return The time point at which this item was retrieved from the processed queue */
    std::chrono::high_resolution_clock::time_point getProcRetrievalTime() const {
        return broker_proc_retrieval_time_;
    }
    /** @brief @return The time point at which this item was submitted to the processed queue */
    std::chrono::high_resolution_clock::time_point getProcSubmissionTime() const {
        return broker_proc_submission_time_;
    }

    /** @brief @return A tuple of (pre-processing, processing, post-processing) times in seconds */
    std::tuple<double, double, double> getProcessingTimes() const {
        return std::make_tuple(pre_processing_time_, processing_time_, post_processing_time_);
    }

    /** @brief Marks the time when the item was added to a GBufferPortT raw queue */
    void markRawSubmissionTime() {
        broker_raw_submission_time_ = std::chrono::high_resolution_clock::now();
    }
    /** @brief Marks the time when the item was retrieved from a GBufferPortT raw queue */
    void markRawRetrievalTime() {
        broker_raw_retrieval_time_ = std::chrono::high_resolution_clock::now();
    }
    /** @brief Marks the time when the item was submitted to a GBufferPortT processed queue */
    void markProcSubmissionTime() {
        broker_proc_submission_time_ = std::chrono::high_resolution_clock::now();
    }
    /** @brief Marks the time when the item was retrieved from a GBufferPortT processed queue */
    void markProcRetrievalTime() {
        broker_proc_retrieval_time_ = std::chrono::high_resolution_clock::now();
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

protected:
    /***************************************************************************/
    /**
     * @brief Hook: clears the (result-bearing derived class's) stored results when the processing
     * status is reset. The base has no result store, so the default is a no-op; GProcessingContainerT
     * overrides it to clear its result vector.
     */
    virtual void clearStoredResults_() { /* no result store in the base */ }

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

    ITERATION_COUNTER_TYPE iteration_counter_ = static_cast<ITERATION_COUNTER_TYPE>(0);
    RESUBMISSION_COUNTER_TYPE resubmission_counter_ = static_cast<RESUBMISSION_COUNTER_TYPE>(0);
    COLLECTION_POSITION_TYPE collection_position_ = static_cast<COLLECTION_POSITION_TYPE>(0);
    CORRELATION_ID_TYPE correlation_id_ = CORRELATION_ID_TYPE();

    /// Transient, server-side-only per-batch scheduling state for the networked consumers.
    /// Deliberately NOT part of serialize() (the wire/clone never needs it; see dispatchState).
    dispatchState dispatch_state_ = dispatchState::NONE;

    double pre_processing_time_ = 0.;  ///< Time needed for pre-processing (seconds)
    double processing_time_ = 0.;      ///< Time needed for the actual processing step (seconds)
    double post_processing_time_ = 0.; ///< Time needed for post-processing (seconds)

    std::chrono::high_resolution_clock::time_point
        broker_raw_retrieval_time_; ///< Time when the item was retrieved from the raw queue
    std::chrono::high_resolution_clock::time_point
        broker_raw_submission_time_; ///< Time when the item was submitted to the raw queue
    std::chrono::high_resolution_clock::time_point
        broker_proc_retrieval_time_; ///< Time when the item was retrieved from the processed queue
    std::chrono::high_resolution_clock::time_point
        broker_proc_submission_time_; ///< Time when the item was submitted to the processed queue

    std::string
        stored_error_descriptions_; ///< Stores exceptions that may have occurred during processing
    processingStatus processing_status_ =
        processingStatus::UNPROCESSED; ///< By default no processing is initiated
};

/******************************************************************************/

} /* namespace Gem::Courtier */
