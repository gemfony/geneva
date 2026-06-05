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

#include <algorithm>
#include <chrono>
#include <cmath>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

// Boost headers go here
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

// Geneva headers go here
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "common/GParserBuilder.hpp"
#include "common/GPlotDesigner.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "common/GThreadPool.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/GBufferPortT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GCourtierHelperFunctions.hpp"
#include "courtier/GProcessingContainerT.hpp"

namespace Gem::Courtier {

/******************************************************************************/
/**
 * Status information for the workOn function and helper functions
 */
struct executor_status_t {
    bool is_complete = false; ///< Indicates whether a complete set of current items was obtained
    bool has_errors =
        false; ///< Indicates whether there were errors during processing of current items
};

/******************************************************************************/
/**
 * This class centralizes some functionality and data that is needed to perform
 * serial, parallel local or networked execution for a set of work items. Its main
 * purpose is to avoid duplication of code. Derived classes may deal with different
 * types of parallel execution, including connection to a broker and multi-threaded
 * execution. The serial mode is meant for debugging purposes only. The class
 * follows the Gemfony conventions for loading / cloning and comparison with
 * other objects. The main function "workOn" returns a struct which indicates whether
 * all submitted items have returned ("is_complete") and whether there were errors
 * ("has_errors"). Return items may have errors, i.e. it is possible that a set
 * of work items is complate, but has errors.
 */
template <typename processable_type>
class GBaseExecutorT : public Gem::Common::GCommonInterfaceT<GBaseExecutorT<processable_type>> {
    // Make sure processable_type adheres to the GProcessingContainerT interface
    static_assert(
        std::is_base_of_v<
            Gem::Courtier::
                GProcessingContainerT<processable_type, typename processable_type::result_type>,
            processable_type>,
        "GBaseExecutorT: processable_type does not adhere to the GProcessingContainerT<> interface"
    );

public:
    /////////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_NVP(max_resubmissions_);
    }

    /////////////////////////////////////////////////////////////////////////////

    /***************************************************************************/
    /**
	  * Copy constructor. Many data items are not copied, as we want them
	  * in pristine condition for a new object.
	  *
	  * @param cp A copy of another GBrokerConnector object
	  */
    GBaseExecutorT(const GBaseExecutorT<processable_type> &cp)
      : Gem::Common::GCommonInterfaceT<GBaseExecutorT<processable_type>>(cp)
      , max_resubmissions_(cp.max_resubmissions_) { /* nothing */
    }

    /***************************************************************************/
    /**
     * Copy constructor. Many data items are not moved, as we want them
     * in pristine condition for a new object.
     *
     * @param cp A copy of another GBrokerConnector object
     */
    GBaseExecutorT(GBaseExecutorT<processable_type> &&cp) noexcept
      : Gem::Common::GCommonInterfaceT<GBaseExecutorT<processable_type>>(std::move(cp))
      , max_resubmissions_(cp.max_resubmissions_) {
        // Reset the other object
        cp.iteration_counter_ = static_cast<ITERATION_COUNTER_TYPE>(0);
        cp.expected_number_ = 0;
        cp.object_first_submission_time_ = std::chrono::high_resolution_clock::time_point{};
        cp.iteration_first_submission_time_ = std::chrono::high_resolution_clock::time_point{};
        cp.iteration_end_time_ = std::chrono::high_resolution_clock::time_point{};
        cp.iteration_running_.store(true);
        cp.cycle_first_submission_time_ = std::chrono::high_resolution_clock::time_point{};
        cp.approx_cycle_start_time_ = std::chrono::high_resolution_clock::time_point{};
        cp.cycle_end_time_ = std::chrono::high_resolution_clock::time_point{};
        cp.cycle_running_.store(true);
        cp.no_items_submitted_in_object_.store(true);
        cp.no_items_submitted_in_iteration_.store(true);
        cp.no_items_submitted_in_cycle_.store(true);
        cp.in_first_iteration_.store(true);
        cp.iteration_first_individual_position_ = 0;
        cp.max_resubmissions_ = DEFAULTMAXRESUBMISSIONS;
        cp.n_resubmissions_ = 0;
        cp.n_returned_last_ = 0;
        cp.n_not_returned_last_ = 0;
        cp.n_old_work_items_ = 0;
        cp.n_erroneous_items_ = 0;
        cp.old_work_items_cnt_.clear();
    }

    /***************************************************************************/
    // Some defaulted or deleted constructors, destructor and assignment operators

    GBaseExecutorT() = default;
    ~GBaseExecutorT() override = default;

    GBaseExecutorT<processable_type> &operator=(GBaseExecutorT<processable_type> const &) = delete;
    GBaseExecutorT<processable_type> &operator=(GBaseExecutorT<processable_type> &&) = delete;

    /***************************************************************************/
    /**
	  * General initialization function to be called prior to the first submission
	  */
    void init() {
        this->init_();
    }

    /***************************************************************************/
    /**
	  * General finalization function to be called after the last submission
	  */
    void finalize() {
        this->finalize_();
    }

    /***************************************************************************/
    /**
	  * Submits and retrieves a set of work items in cycles / iterations. Each
	  * iteration represents a cycle of work item submissions and (posssibly full) retrieval.
	  * Iterations may not overlap, i.e. this function may not be called on the same
	  * GExecutorT object while another call is still running. The function aims to
	  * prevent this situation by setting and releasing a lock during its run-time.
	  * Work items need to be derived from the GProcessingContainerT, and need to have been marked with
	  * either IGNORE (these will not be processed) or DO_PROCESS. After processing,
	  * they will have one of the flags IGNORE (if they were not due to be processed),
	  * EXCEPTION_CAUGHT (if some exception was raised during processing) or ERROR_FLAGGED
	  * (if the user code has marked a solution as unusable). After a timeout, items may
	  * still have the DO_PROCESS flag set (if items did not return in time). Note that
	  * items still marked as DO_PROCESS may well return in later iterations. It is thus
	  * also possible that returned items do not belong to the current submission cycle.
	  * They will be appended to the old_work_items_cnt_ vector. You might thus have to
	  * post-process such "old" work items and decide what to do with them. This vector
	  * will be cleared for each new iteration. Hence, if you are interested in old work
	  * items, you need to retrieve them before a new submission of work items. The return
	  * code "is_complete" means that there were responses for each submitted work item
	  * of the current iteration. The return code "has_errors" means that some or all of
	  * them have had errors during processing.
	  *
	  * Iterations will either be counted from 0 upwards or may be supplied by the user
	  * upon calling workOn. Where they are supplied by the user, the user needs to make
	  * sure to always supply an increasing (but not necessarily consecutive) id. Providing
	  * an id smaller or equal than the last id will result in an error. E.g., when calling
	  * workOn once in a loop, the user may pass the loop id. Calling workOn twice in
	  * a loop and passing the same id twice will result in an error. Once external
	  *
	  * @param workItems A vector with work items to be evaluated beyond the broker
	  * @param resubmitUnprocessed Indicates whether unprocessed items should be resubmitted
	  * @param externalIterationCounter An external iteration id assigned to this object (and a boolean indicating whether this value should be used)
	  * @return A struct of booleans indicating whether all items were processed successfully and whether there were errors
	  */
    executor_status_t workOn(
        std::vector<std::shared_ptr<processable_type>> &workItems,
        bool resubmitUnprocessed = false,
        const std::tuple<ITERATION_COUNTER_TYPE, bool> &externalIterationCounter =
            std::tuple<ITERATION_COUNTER_TYPE, bool>(0, false),
        [[maybe_unused]] const std::string & caller = std::string()
    ) {
        //------------------------------------------------------------------------------------------
        // Make sure only one instance of this function can be called at the same time. If locking
        // the mutex fails, some other call to this function is still alive, which is a severe error
        std::unique_lock<std::mutex> workon_lock(concurrent_workon_mutex_, std::defer_lock);
        if(not workon_lock.try_lock()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GBaseExeuctorT<processable_type>::workOn(): Another call to this function "
                   "still seems"
                << '\n'
                << "to be active which is a severe error."
            );
        }

        //------------------------------------------------------------------------------------------
        // Assign an external iteration id to the iteration counter, if requested by the user
        bool useExternalId = std::get<1>(externalIterationCounter);
        if(useExternalId) {
            // Extract the external iteration counter
            ITERATION_COUNTER_TYPE externalId = std::get<0>(externalIterationCounter);
            // This call will throw, if externalId is < iteration_counter_
            this->set_external_iteration_counter(externalId);
        }

        //------------------------------------------------------------------------------------------
        // Initialization of a new iteration (possibly involving more than one submission of
        // selected work items).
        this->iterationInit(workItems);

        //------------------------------------------------------------------------------------------
        // The main business logic of item submission

        n_resubmissions_ = 0;
        executor_status_t status{false /* is_complete */, false /* has_errors */};
        do {
            //-----------------------
            // Initialization of a new "run" or resubmission
            this->cycleInit(workItems);

            //-----------------------
            // Submission and retrieval

            // Submit all work items.
            expected_number_ = this->submitAllWorkItems(workItems);

            // Wait for work items to complete. This function needs to
            // be re-implemented in derived classes.
            auto current_status = waitForReturn(workItems, old_work_items_cnt_);

            // There may not be errors during resubmission, so we need to save the "error state"
            if(current_status.is_complete) {
                status.is_complete = true;
            }
            if(current_status.has_errors) {
                status.has_errors = true;
            }

            // Perform necessary cleanup work for an iteration
            this->cycleFinalize(workItems);

            //-----------------------
            // Check whether we want to break the loop

            // Leave if we are complete or if we haven't been asked to resubmit unprocessed
            // items (even if we do not have a complete set of work items). Also leave
            // if we have reached the maximum number of resubmissions or
            // max_resubmissions_ was explicitly set to 0.
            if(
                status.is_complete // nothing left to do
                || not resubmitUnprocessed ||
                max_resubmissions_ == 0 // user is happy with unprocessed items
                || (++n_resubmissions_ >=
                    max_resubmissions_) // we have tried to resubmit items but did not succeed
            ) {
                // Leave the loop
                break;
            }

            //-----------------------
        }
        while(true);

        //------------------------------------------------------------------------------------------
        // Finalization of this iteration (possibly involving more than one submission of
        // selected work items)
        this->iterationFinalize(workItems);

        //------------------------------------------------------------------------------------------

        // Give feedback to the audience (may be overloaded in derived classes)
        this->visualize_performance();

        // Update the iteration counter. An "iteration" represents all work needed
        // to process a set of work items from the workItems vector. This may involve
        // resubmissions.
        iteration_counter_++;

        // Note: unprocessed items will be returned to the caller who needs to deal with them
        return status;
    }

    /***************************************************************************/
    /**
     * Code to be executed before the start of an iteration (i.e. a call to workOn)
     */
    void iterationInit(std::vector<std::shared_ptr<processable_type>> &workItems) {
        this->iterationInit_(workItems);
    }

    /***************************************************************************/
    /**
     * Code to be executed before the start of an iteration (i.e. a call to workOn)
     */
    void iterationFinalize(std::vector<std::shared_ptr<processable_type>> &workItems) {
        this->iterationFinalize_(workItems);
    }

    /***************************************************************************/
    /**
	  * Retrieves a copy of the old work items vector. Calling this function will
	  * clear the vector!
	  *
	  * @return A copy of the old work items vector
	  */
    std::vector<std::shared_ptr<processable_type>> getOldWorkItems() {
        return std::move(old_work_items_cnt_);
    }

    /***************************************************************************/
    /**
	  * Specifies how often work items should be resubmitted in the case a full return
	  * of work items is expected.
	  *
	  * @param maxResubmissions The maximum number of allowed resubmissions
	  */
    void setMaxResubmissions(std::size_t maxResubmissions) {
        max_resubmissions_ = maxResubmissions;
    }

    /***************************************************************************/
    /**
	  * Returns the maximum number of allowed resubmissions
	  *
	  * @return The maximum number of allowed resubmissions
	  */
    std::size_t getMaxResubmissions() const {
        return max_resubmissions_;
    }

    /***************************************************************************/
    /**
	  * Retrieve the number of individuals returned during the last iteration
	  */
    std::size_t getNReturnedLast() const noexcept {
        return n_returned_last_;
    }

    /***************************************************************************/
    /**
	  * Retrieve the number of individuals NOT returned during the last iteration
	  */
    std::size_t getNNotReturnedLast() const noexcept {
        return n_not_returned_last_;
    }

    /***************************************************************************/
    /**
 	  * Retrieves the current number of old work items in this iteration
 	  */
    std::size_t getNOldWorkItems() const noexcept {
        return n_old_work_items_;
    }

    /***************************************************************************/
    /**
	  * Retrieves the number of work items with errors in this iteration
	  */
    std::size_t getNErroneousWorkItems() const noexcept {
        return n_erroneous_items_;
    }

    /***************************************************************************/
    /**
	  * Retrieve the time of the very first submission in this object
	  */
    std::chrono::high_resolution_clock::time_point getObjectFirstSubmissionTime() const {
        return object_first_submission_time_;
    }

    /***************************************************************************/
    /**
	  * Retrieve the time of the very first submission in the current iteration
	  */
    std::chrono::high_resolution_clock::time_point getIterationFirstSubmissionTime() const {
        return iteration_first_submission_time_;
    }

    /***************************************************************************/
    /**
	  * Retrieve the time of the very first submission in the current resubmission
	  */
    std::chrono::high_resolution_clock::time_point getCycleFirstSubmissionTime() const {
        return cycle_first_submission_time_;
    }

    /***************************************************************************/
    /**
	  * Retrieve the approximate time of the start of the cycle
	  */
    std::chrono::high_resolution_clock::time_point getApproxCycleStartTime() const {
        return approx_cycle_start_time_;
    }

    /***************************************************************************/
    /**
	  * Retrieve the end time of the last cycle
	  */
    std::chrono::high_resolution_clock::time_point getCycleEndTime() const {
#ifdef DEBUG
        // Cross check that the cycle has indeed ended
        if(cycle_running_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GBaseExecutorT<processable_type>::getCycleEndTime():" << '\n'
                << "There still seems to be an active cycle while the end" << '\n'
                << "time of the cycle is retrieved" << '\n'
            );
        }
#endif

        return cycle_end_time_;
    }

    /***************************************************************************/
    /**
	  * Retrieve the end time of the last iteration
	  */
    std::chrono::high_resolution_clock::time_point getIterationEndTime() const {
#ifdef DEBUG
        // Cross check that the cycle has indeed ended
        if(iteration_running_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GBaseExecutorT<processable_type>::getIterationEndTime():" << '\n'
                << "There still seems to be an active iteration while the end" << '\n'
                << "time of the iteration is retrieved" << '\n'
            );
        }
#endif

        return iteration_end_time_;
    }

protected:
    /***************************************************************************/
    /**
	  * Loads the data of another object
	  */
    void load_(const GBaseExecutorT<processable_type> *cp) override {
        // Check that we are dealing with a GDecorator reference independent of this object and convert the pointer
        const auto p_load_ptr = Gem::Common::g_convert_and_compare(cp, this);

        // No parent class with loadable data

        // Copy local data
        max_resubmissions_ = p_load_ptr->max_resubmissions_;
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GBaseExecutorT<processable_type>>(
        GBaseExecutorT<processable_type> const &,
        GBaseExecutorT<processable_type> const &,
        Gem::Common::GToken &
    );

    /***************************************************************************/
    /**
     * Searches for compliance with expectations with respect to another object
     * of the same type
     */
    void compare_(
        const GBaseExecutorT<processable_type> &cp // the other object
        ,
        const Gem::Common::expectation &e // the expectation for this object, e.g. equality
        ,
        const double & /*limit*/ // the limit for allowed deviations of floating point types
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GDecorator reference independent of this object and convert the pointer
        const GBaseExecutorT<processable_type> *p_load =
            Gem::Common::g_convert_and_compare(cp, this);

        GToken token("GBaseExecutorT<processable_type>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<GCommonInterfaceT<GBaseExecutorT<processable_type>>>(
            *this,
            *p_load,
            token
        );

        // ... and then our local data
        compare_t(IDENTITY(this->max_resubmissions_, p_load->max_resubmissions_), token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /**
	  * General initialization function to be called prior to the first submission
	  */
    virtual void init_() {
        no_items_submitted_in_object_ = true;
    }

    /***************************************************************************/
    /**
	  * General finalization function to be called after the last submission
	  */
    virtual void finalize_() { /* nothing */
    }

    /***************************************************************************/
    /**
	  * General initialization function to be called prior to the first submission
	  */
    virtual void iterationInit_(std::vector<std::shared_ptr<processable_type>> &workItems) {
        // Make it known that a new iteration is about to start
        iteration_running_ = true;
        // Make it known that no items have been submitted yet in this iteration
        no_items_submitted_in_iteration_ = true;

        // Clear old work items not cleared by the caller after the last iteration
        old_work_items_cnt_.clear();

        // Reset some counters and flags
        n_returned_last_ = 0;
        n_not_returned_last_ = 0;
        n_old_work_items_ = 0;
        n_erroneous_items_ = 0;
    }

    /***************************************************************************/
    /**
     * Adds local configuration options to a GParserBuilder object
     *
     * @param gpb The GParserBuilder object to which configuration options should be added
     */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) override {
        // Add local options
        gpb.registerFileParameter<std::size_t>(
            "max_resubmissions" // The name of the variable
            ,
            DEFAULTMAXRESUBMISSIONS // The default value
            ,
            [this](std::size_t r) { this->setMaxResubmissions(r); }
        ) << "The amount of resubmissions allowed if a full return of work"
          << '\n'
          << "items was expected but only a subset has returned";
    }

    /***************************************************************************/
    /**
	  * General initialization function to be called prior to the first submission
	  */
    virtual void iterationFinalize_(std::vector<std::shared_ptr<processable_type>> &workItems) {
        // Sort remaining old work items according to their position
        std::sort(
            old_work_items_cnt_.begin(),
            old_work_items_cnt_.end(),
            [](std::shared_ptr<processable_type> x_ptr,
               std::shared_ptr<processable_type> y_ptr) -> bool {
                using namespace boost;
                return x_ptr->getCollectionPosition() <= y_ptr->getCollectionPosition();
            }
        );
        // Remove duplicates
        old_work_items_cnt_.erase(
            std::unique(
                old_work_items_cnt_.begin(),
                old_work_items_cnt_.end(),
                [](std::shared_ptr<processable_type> x_ptr,
                   std::shared_ptr<processable_type> y_ptr) -> bool {
                    return x_ptr->getCollectionPosition() == y_ptr->getCollectionPosition();
                }
            ) // Returns the begin() of the duplicates moved to the end of the vector
            ,
            old_work_items_cnt_.end()
        );
        // Remove unprocessed or erroneous items and count the remaining items
        n_old_work_items_ =
            this->cleanItemsWithoutFlag(old_work_items_cnt_, processingStatus::PROCESSED);

        // Find out about the number of items that have not returned (yet) from the last submission.
        // These are equivalent to the items that still have the DO_PROCESS flag set.
        n_not_returned_last_ = this->countItemsWithStatus(workItems, processingStatus::DO_PROCESS);
        // The number of actually returned items is equal to the number of expected items minus the number of not returned items
        n_returned_last_ = expected_number_ - n_not_returned_last_;
        // Count the number of work items with errors. We need to count two flags
        n_erroneous_items_ = this->countItemsWithStatus(workItems, processingStatus::ERROR_FLAGGED);
        n_erroneous_items_ +=
            this->countItemsWithStatus(workItems, processingStatus::EXCEPTION_CAUGHT);

        // Make it known that the first iteration has ended (if this is the first iteration)
        if(in_first_iteration_) {
            in_first_iteration_ = false;
        }

        // Mark the end of processing in this iteration
        iteration_end_time_ = this->now();

        // Make it known that the iteration has ended
        iteration_running_ = false;
    }

    /***************************************************************************/
    /**
	  * Allow to perform necessary setup work for an iteration. Derived classes
	  * should make sure this base function is called first when they overload
	  * this function.
	  */
    virtual void cycleInit_(std::vector<std::shared_ptr<processable_type>> &workItems) {
        // No items have so far been submitted in current cycle
        no_items_submitted_in_cycle_ = true;
        // Make it known that a new cycle is about to start
        cycle_running_ = true;
    }

    /***************************************************************************/
    /**
	  * Non-virtual wrapper for cycleInit_()
	  */
    void cycleInit(std::vector<std::shared_ptr<processable_type>> &workItems) {
        this->cycleInit_(workItems);
    }

    /***************************************************************************/
    /**
	  * Allows to perform necessary cleanup work for an iteration. Derived classes
	  * should make sure this base function is called last when they overload
	  * this function.
	  */
    virtual void cycleFinalize_(std::vector<std::shared_ptr<processable_type>> &workItems) {
        // Mark the end of processing in this cycle
        cycle_end_time_ = this->now();
        // Make it known that the cycle has ended
        cycle_running_ = false;
    }

    /***************************************************************************/
    /**
	  * Non-virtual wrapper for cycleFinalize_()
	  */
    void cycleFinalize(std::vector<std::shared_ptr<processable_type>> &workItems) {
        this->cycleFinalize_(workItems);
    }

    /***************************************************************************/
    /**
	  * Submission of all work items in the list
	  */
    std::size_t submitAllWorkItems(std::vector<std::shared_ptr<processable_type>> &workItems) {
        // Submit work items
        COLLECTION_POSITION_TYPE pos_cnt = 0;
        std::size_t nSubmittedItems = 0;
        bool got_first_processable_item_id = false;
        for(auto const &w_ptr : workItems) {
#ifdef DEBUG
            if(not w_ptr) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GBaseExecutorT<processable_type>::submitAllWorkItems():" << '\n'
                    << "Received empty work item in position " << pos_cnt << '\n'
                    << "iteration_counter_ = " << iteration_counter_ << '\n'
                );
            }
#endif

            // Is the item due to be submitted ?
            processingStatus ps = w_ptr->getProcessingStatus();
            if(processingStatus::DO_PROCESS == ps) {
                // Update some internal variables
                w_ptr->setIterationCounter(iteration_counter_);
                w_ptr->setCollectionPosition(pos_cnt);
                w_ptr->setResubmissionCounter(n_resubmissions_);

                // Do the actual submission
                this->submit(w_ptr);

                // Assign the id of the first processable item in this iteration. This is
                // so we can identify the first individual in the first iteration.
                if(not got_first_processable_item_id) {
                    got_first_processable_item_id = true;
                    iteration_first_individual_position_ = pos_cnt;
                }

                // Make sure we use the same timings in the following
                auto current_time = this->now();

                // Determine the time of the very first submission
                if(no_items_submitted_in_object_) {
                    no_items_submitted_in_object_ = false;
                    object_first_submission_time_ = current_time;
                }

                if(no_items_submitted_in_iteration_) {
                    no_items_submitted_in_iteration_ = false;
                    iteration_first_submission_time_ = current_time;
                }

                // Determine the time of the first submission in the current iteration
                if(no_items_submitted_in_cycle_) {
                    no_items_submitted_in_cycle_ = false;
                    cycle_first_submission_time_ = current_time;
                }

                // Update the submission counter
                nSubmittedItems++;
            }
            else if(processingStatus::DO_IGNORE != ps && processingStatus::PROCESSED != ps) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GBaseExecutorT<processable_type>::submitAllWorkItems(): Error"
                    << '\n'
                    << "processing status is neither DO_PROCESS nor DO_IGNORE. We got " << ps
                    << '\n'
                );
            }

            pos_cnt++;
        }

        // Set the start time of the new cycle. How this time is determined depends
        // on the actual executor. NOTE that the following call may block, if a start time cannot
        // yet be determined.
        if(this->inFirstIteration() && this->inFirstCycle()) {
            approx_cycle_start_time_ = this->determineInitialCycleStartTime();
        }
        else {
            approx_cycle_start_time_ = cycle_first_submission_time_;
        }

        // Let the audience know how many items were submitted
        return nSubmittedItems;
    }

    /***************************************************************************/
    /**
	  * Returns the current iteration as used for the tagging of work items
	  * @return The id of the current submission cycle
	  */
    ITERATION_COUNTER_TYPE get_iteration_counter() const noexcept {
        return iteration_counter_;
    }

    /***************************************************************************/
    /**
	  * Retrieves the expected number of work items in the current iteration
	  */
    std::size_t getExpectedNumber() const noexcept {
        return expected_number_;
    }

    /***************************************************************************/
    /**
	  * Retrieves the time when an iteration has started
	  */
    std::chrono::high_resolution_clock::time_point getIterationStartTime() const {
        return this->iteration_first_submission_time_;
    }

    /***************************************************************************/
    /**
	  * A little helper function to make the code easier to read
	  */
    std::chrono::high_resolution_clock::time_point now() const {
        return std::chrono::high_resolution_clock::now();
    }

    /***************************************************************************/
    /** @brief Submits a single work item */
    virtual void submit(std::shared_ptr<processable_type>) = 0;

    /***************************************************************************/
    /**
	  * Waits for work items to return and checks for completeness
	  * (i.e. all items have returned and there were no exceptions).
	  *
	  * @param workItems A vector with work items to be evaluated beyond the broker
	  * @return A struct of booleans indicating whether all items were processed successfully and whether there were errors
	  */
    virtual executor_status_t waitForReturn(
        std::vector<std::shared_ptr<processable_type>> &workItems,
        std::vector<std::shared_ptr<processable_type>> &
    ) = 0;

    /***************************************************************************/
    /**
	  * Count the number of work items in a batch with a specific flag
	  */
    std::size_t countItemsWithStatus(
        const std::vector<std::shared_ptr<processable_type>> &workItems,
        const processingStatus &ps
    ) {
        return Gem::Common::narrow<std::size_t>(std::count_if(
            workItems.begin(),
            workItems.end(),
            [ps](std::shared_ptr<processable_type> p) {
                if(ps == p->getProcessingStatus()) {
                    return true;
                }
                return false;
            }
        ));
    }

    /***************************************************************************/
    /**
	  * Checks for the status with respect to errors and processing status
	  * of a collecion of work items
	  */
    executor_status_t
    checkExecutionState(const std::vector<std::shared_ptr<processable_type>> &workItems) {
        bool is_complete = true;
        bool has_errors = false;

        // First check that there are no remaining items with the DO_PROCESS status
        auto nUnprocessed = this->countItemsWithStatus(workItems, processingStatus::DO_PROCESS);
        if(nUnprocessed > 0) {
            is_complete = false;
        }

        // Now check for error states
        auto nErrorState = this->countItemsWithStatus(workItems, processingStatus::ERROR_FLAGGED);
        nErrorState += this->countItemsWithStatus(workItems, processingStatus::EXCEPTION_CAUGHT);
        if(nErrorState > 0) {
            has_errors = true;
        }

        return executor_status_t{is_complete, has_errors};
    }

    /***************************************************************************/
    /**
	  * Removes work items without a given flag from the vector and return
	  * the number of remaining items
	  */
    std::size_t cleanItemsWithoutFlag(
        std::vector<std::shared_ptr<processable_type>> &items_cnt,
        const processingStatus &desired_ps
    ) {
        // Remove unprocessed items from the list of old work items
        Gem::Common::erase_if(
            items_cnt,
            [this, desired_ps](std::shared_ptr<processable_type> item_ptr) {
                auto ps = item_ptr->getProcessingStatus();
                if(ps != desired_ps) {
#ifdef DEBUG
                    // Some logging, as this condition should be very rare and might indicate a more general problem.
                    glogger << "In GBaseExeuctorT<processable_type>::cleanItemsWithoutFlag():"
                            << '\n'
                            << "Removing work item in submission " << this->get_iteration_counter()
                            << '\n'
                            << "because it does not have the desired status " << desired_ps
                            << '\n'
                            << "Found status " << ps << " instead." << '\n'
                            << GLOGGING;
#endif

                    // Erase
                    return true;
                }

                // Do not erase
                return false;
            }
        );

        // Return the number of remaining items
        return items_cnt.size();
    }

    /***************************************************************************/
    /**
	  * Checks if any work items have already been submitted in the object
	  */
    bool checkItemsSubmittedInObject() const noexcept {
        return not no_items_submitted_in_object_;
    }

    /***************************************************************************/
    /**
	  * Checks if any work items have already been submitted in the current iteration
	  */
    bool checkItemsSubmittedInCycle() const noexcept {
        return not no_items_submitted_in_cycle_;
    }

    /***************************************************************************/
    /**
	  * Checks if this is the first iteration
	  */
    bool inFirstIteration() const noexcept {
        return in_first_iteration_;
    }

    /***************************************************************************/
    /**
	  * Checks if this is the first cycle of an iteration
	  */
    bool inFirstCycle() const noexcept {
        return (0 == n_resubmissions_);
    }

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override {
        return false;
    };
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override { /* nothing */ };
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override { /* nothing */ };

private:
    /***************************************************************************/
    /**
	  * Returns the name of this class
	  */
    std::string name_() const override {
        return std::string("GBaseExecutorT<processable_type>");
    }

    /***************************************************************************/
    /** @brief Determination of the time when execution of the initial cycle has started */
    virtual std::chrono::high_resolution_clock::time_point
    determineInitialCycleStartTime() const = 0;

    /***************************************************************************/
    /** @brief Graphical progress feedback */
    virtual void visualize_performance() = 0;

    /***************************************************************************/
    /**
	  * Internal function to set the iteration counter. The function will throw
	  * if the assigned iteration is < the internally determined iteration
	  */
    void set_external_iteration_counter(const ITERATION_COUNTER_TYPE &external_iteration_counter) {
        if(external_iteration_counter < iteration_counter_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GBaseExeuctorT<processable_type>::set_external_iteration_counter():"
                << '\n'
                << "Tried to set external iteration counter to value " << external_iteration_counter
                << " ," << '\n'
                << "while internal counter is at " << iteration_counter_ << " ." << '\n'
                << "The internal counter needs to be <= the external counter when being set"
                << '\n'
            );
        }

        iteration_counter_ = external_iteration_counter;
    }

    /***************************************************************************/
    // Data

    /* @brief Counts the number of submissions initiated for this object; may also be set by the user */
    ITERATION_COUNTER_TYPE iteration_counter_ = static_cast<ITERATION_COUNTER_TYPE>(0);

    std::size_t expected_number_ =
        0; ///< The number of work items to be submitted (and expected back)

    /** brief The timepoint of the very first submission (or possibly retrieval from the queue */
    std::chrono::high_resolution_clock::time_point object_first_submission_time_;
    /** @brief Temporary that holds the start time for the retrieval of items in a given iteration */
    std::chrono::high_resolution_clock::time_point iteration_first_submission_time_;
    /** @brief Temporary that holds the start time for the retrieval of items in a given iteration */
    std::chrono::high_resolution_clock::time_point iteration_end_time_;
    /** @brief Indicates whether an iteration is currently being processed */
    std::atomic<bool> iteration_running_{true};
    /** @brief Temporary that holds the start time for the retrieval of items in a given cycle */
    std::chrono::high_resolution_clock::time_point cycle_first_submission_time_;
    /** @brief The approximate time of the start of processing in an iteration */
    std::chrono::high_resolution_clock::time_point approx_cycle_start_time_;
    /** @brief Temporary that holds the start time for the retrieval of items in a given cycle */
    std::chrono::high_resolution_clock::time_point cycle_end_time_;
    /** @brief Indicates whether a cycle is currently being processed */
    std::atomic<bool> cycle_running_{true};

    std::atomic<bool> no_items_submitted_in_object_{
        true
    }; ///< Indicates whether items have already been submitted
    std::atomic<bool> no_items_submitted_in_iteration_{
        true
    }; ///< Indicates whether items have already been submitted in an iteration
    std::atomic<bool> no_items_submitted_in_cycle_{
        true
    }; ///< Indicates whether items have already been submitted in a cycle
    std::atomic<bool> in_first_iteration_{
        true
    }; ///< Allows to check whether we are in the first iteration (will be set to false in cycleFinalize_()

    std::size_t iteration_first_individual_position_ =
        0; ///< The position of the first item to be processed in the workItems vector

    /** @brief The maximum number of re-submissions allowed if a full return of submitted items is attempted */
    std::size_t max_resubmissions_ = DEFAULTMAXRESUBMISSIONS;
    std::size_t n_resubmissions_ = 0; ///< A temporary counter of the current resubmission

    std::size_t n_returned_last_ =
        0; ///< The number of individuals returned in the last iteration cycle
    std::size_t n_not_returned_last_ =
        0; ///< The number of individuals NOT returned in the last iteration cycle
    std::size_t n_old_work_items_ =
        0; ///< The number of old work items returned in a given iteration
    std::size_t n_erroneous_items_ =
        0; ///< The number of work items with errors in the current iteration

    std::vector<std::shared_ptr<processable_type>>
        old_work_items_cnt_; ///< Temporarily holds old work items of the current iteration

    std::mutex
        concurrent_workon_mutex_; ///< Makes sure the workOn function is only called once at the same time on this object
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class executes work items serially. It is mostly meant for debugging
 * purposes
 */
template <typename processable_type>
class GSerialExecutorT // NOLINT(cppcoreguidelines-special-member-functions)
  : public GBaseExecutorT<processable_type> {
    ///////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GBaseExecutorT",
            boost::serialization::base_object<GBaseExecutorT<processable_type>>(*this)
        );
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /** @brief The default constructor */
    GSerialExecutorT() = default;

    /***************************************************************************/
    /**
	  * The copy constructor
	  *
	  * @param cp A copy of another GBrokerConnector object
	  */
    GSerialExecutorT(const GSerialExecutorT<processable_type> &cp)
      : GBaseExecutorT<processable_type>(cp) { /* nothing */
    }

    /***************************************************************************/
    /** @brief The destructor */
    ~GSerialExecutorT() override = default;

protected:
    /***************************************************************************/
    /**
	  * Loads the data of another GSerialExecutorT object
	  *
	  * @param cp A constant pointer to another GSerialExecutorT object
	  */
    void load_(const GBaseExecutorT<processable_type> *cp) override {
        const auto p_load_ptr = dynamic_cast<GSerialExecutorT<processable_type> const *>(cp);

        if(not cp) { // nullptr
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GSerialExecutorT<processable_type>::load_(): Conversion error!" << '\n'
            );
        }

        // Load our parent classes data
        GBaseExecutorT<processable_type>::load_(p_load_ptr);
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GSerialExecutorT<processable_type>>(
        GSerialExecutorT<processable_type> const &,
        GSerialExecutorT<processable_type> const &,
        Gem::Common::GToken &
    );

    /***************************************************************************/
    /**
     * Searches for compliance with expectations with respect to another object
     * of the same type
     */
    void compare_(
        const GBaseExecutorT<processable_type> &cp // the other object
        ,
        const Gem::Common::expectation &e // the expectation for this object, e.g. equality
        ,
        const double & /*limit*/ // the limit for allowed deviations of floating point types
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GSerialExecutorT reference independent of this object and convert the pointer
        const GSerialExecutorT<processable_type> *p_load =
            Gem::Common::g_convert_and_compare(cp, this);

        GToken token("GSerialExecutorT<processable_type>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<GBaseExecutorT<processable_type>>(*this, *p_load, token);

        // ... no local data

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /**
	  * Submits a single work item. In the case of serial execution, all work
	  * is done inside of this function. We rely on the process() function which
	  * is guaranteed to be part of the processable_type interface (note that
	  * our base class stipulates that processable_type is a derivative of
	  * GProcessingContainerT<processable_type> .
	  *
	  * @param w_ptr The work item to be processed
	  */
    void submit(std::shared_ptr<processable_type> w_ptr) override {
        try {
            // process may throw ...
            w_ptr->process();
        }
        catch(
            const g_processing_exception &e
        ) { // NOLINT(bugprone-empty-catch) — expected; error stored in work item, caller decides fate
            // This is an expected exception if processing has failed. We do nothing,
            // it is up to the caller to decide what to do with processing errors, and
            // these are also stored in the processing item. We do try to create a sort
            // of stack trace by emitting a warning, though. Processing errors should be rare,
            // so might hint at some problem.
            glogger << "In GSerialExecutorT<processable_type>::submit():" << '\n'
                    << "Caught a g_processing_exception exception while processing the work item"
                    << '\n'
                    << "with the error message" << '\n'
                    << e.what() << '\n'
                    << "Exception information should have been stored in the" << '\n'
                    << "work item itself. Processing should have been marked as" << '\n'
                    << "unsuccessful in the work item. We leave it to the" << '\n'
                    << "submitter to deal with this." << '\n'
                    << GWARNING;
        }
        catch(const std::exception &e) {
            // All exceptions should be caught inside of the process() call. It is a
            // severe error if we nevertheless catch an error here. We throw a corresponding
            // gemfony exception.
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GSerialExecutorT<processable_type>::submit(): Caught a" << '\n'
                << "std::exception in a place where we didn't expect any exceptions." << '\n'
                << "Got message" << '\n'
                << e.what() << '\n'
            );
        }
        catch(...) {
            // All exceptions should be caught inside of the process() call. It is a
            // severe error if we nevertheless catch an error here. We throw a corresponding
            // gemfony exception.
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GSerialExecutorT<processable_type>::submit(): Caught an" << '\n'
                << "unknown exception in a place where we didn't expect any exceptions" << '\n'
            );
        }
    }

    /***************************************************************************/
    /**
     * Adds local configuration options to a GParserBuilder object
     *
     * @param gpb The GParserBuilder object to which configuration options should be added
     */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) override {
        // Call our parent class's function
        GBaseExecutorT<processable_type>::addConfigurationOptions_(gpb);

        // No local data
    }

    /***************************************************************************/
    /**
	  * Waits for work items to return and checks for completeness
	  * (i.e. all items have returned and there were no exceptions).
	  *
	  * @param workItems A vector with work items to be evaluated beyond the broker
	  * @param oldWorkItems A vector with work items that have returned after the threshold
	  * @return A struct of booleans indicating whether all items were processed successfully and whether there were errors
	  */
    executor_status_t waitForReturn(
        std::vector<std::shared_ptr<processable_type>> &workItems,
        std::vector<std::shared_ptr<processable_type>> &oldWorkItems
    ) override {
        // Note: Old work items are cleared in the "workOn" function

        // We are dealing with serial execution on the local computer, so we
        // do not need to wait for return. All we need to do is to check whether
        // we had a "complete return" and/or errors.
        return this->checkExecutionState(workItems);
    }

    /***************************************************************************/
    /**
	  * General initialization function to be called prior to the first submission
	  */
    void init_() override {
        // GBaseExecutorT<processable_type> sees exactly the environment it would when called from its own class
        GBaseExecutorT<processable_type>::init_();
    }

    /***************************************************************************/
    /**
	  * General finalization function to be called after the last submission
	  */
    void finalize_() override {
        // GBaseExecutorT<processable_type> sees exactly the environment it would when called from its own class
        GBaseExecutorT<processable_type>::finalize_();
    }

    /***************************************************************************/
    /**
	  * Allow to perform necessary setup work for a cycle.
	  */
    void cycleInit_(std::vector<std::shared_ptr<processable_type>> &workItems) override {
        // Make sure the parent classes cycleInit_ function is executed first
        // This function will also update the iteration start time
        GBaseExecutorT<processable_type>::cycleInit_(workItems);
    }

    /***************************************************************************/
    /**
	  * Allows to perform necessary cleanup work for a cycle or do calculations
	  * for the next cycle
	  */
    void cycleFinalize_(std::vector<std::shared_ptr<processable_type>> &workItems) override {
        // Make sure the parent classes cycleFinalize_ function is executed last
        GBaseExecutorT<processable_type>::cycleFinalize_(workItems);
    }

    /***************************************************************************/
    /**
	  * Allow to perform necessary setup work for an iteration.
	  */
    void iterationInit_(std::vector<std::shared_ptr<processable_type>> &workItems) override {
        // Make sure the parent classes iterationInit_ function is executed first
        GBaseExecutorT<processable_type>::iterationInit_(workItems);
    }

    /***************************************************************************/
    /**
	  * Allows to perform necessary cleanup work for an iteration or do calculations
	  * for the next iteration
	  */
    void iterationFinalize_(std::vector<std::shared_ptr<processable_type>> &workItems) override {
        // Make sure the parent classes iterationFinalize_ function is executed last
        GBaseExecutorT<processable_type>::iterationFinalize_(workItems);
    }

private:
    /***************************************************************************/
    /**
	  * Returns the name of this class
	  */
    std::string name_() const override {
        return std::string("GSerialExecutorT<processable_type>");
    }

    /***************************************************************************/
    /**
	  * Creates a deep clone of this object.
	  */
    GBaseExecutorT<processable_type> *clone_() const override {
        return new GSerialExecutorT<processable_type>(*this);
    }

    /***************************************************************************/
    /**
	  * Retrieval of the start time of the very first cycle
	  */
    std::chrono::high_resolution_clock::time_point determineInitialCycleStartTime() const override {
        return this->getObjectFirstSubmissionTime();
    }

    /***************************************************************************/
    /** @brief Graphical progress feedback */
    void visualize_performance() override { /* nothing */
    }

    /***************************************************************************/
    // No local data
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class executes a collection of work items in multiple threads
 */
template <typename processable_type>
class GMTExecutorT // NOLINT(cppcoreguidelines-special-member-functions)
  : public GBaseExecutorT<processable_type> {
    ///////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GBaseExecutorT",
            boost::serialization::base_object<GBaseExecutorT<processable_type>>(*this)
        ) & BOOST_SERIALIZATION_NVP(n_threads_);
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /**
	  * Initialization with the number of threads. Setting the number of threads
	  * to 0 will result in a warning, and the number of threads will be reset.
	  */
    explicit GMTExecutorT(std::uint16_t nThreads)
      : GBaseExecutorT<processable_type>()
      , n_threads_(nThreads > 0 ? nThreads : Gem::Courtier::DEFAULTNSTDTHREADS) {
        if(0 == nThreads) {
            glogger << "In GMTExecutorT::GMTExecutorT(std::uint16_t nThreads):" << '\n'
                    << "User requested nThreads == 0. nThreads was reset to the default "
                    << Gem::Courtier::DEFAULTNSTDTHREADS << '\n'
                    << GWARNING;
        }
    }

    /***************************************************************************/
    /**
	  * The copy constructor
	  *
	  * @param cp A copy of another GBrokerConnector object
	  */
    GMTExecutorT(const GMTExecutorT<processable_type> &cp)
      : GBaseExecutorT<processable_type>(cp)
      , n_threads_(cp.n_threads_) { /* nothing */
    }

    /***************************************************************************/
    /** @brief The destructor */
    ~GMTExecutorT() override = default;

    /***************************************************************************/
    /**
	  * Sets the number of threads for the thread pool. If nThreads is set
	  * to 0, a warning will be printed and the number of threads will be set to
	  * a default value.
	  *
	  * @param nThreads The number of threads the threadpool should use
	  */
    void setNThreads(std::uint16_t nThreads) {
        if(nThreads == 0) {
            n_threads_ = Gem::Courtier::DEFAULTNSTDTHREADS;

            glogger << "In GMTExecutorT::setNThreads(std::uint16_t nThreads):" << '\n'
                    << "User requested nThreads == 0. nThreads was reset to the default "
                    << Gem::Courtier::DEFAULTNSTDTHREADS << '\n'
                    << GWARNING;
        }
        else {
            n_threads_ = nThreads;
        }
    }

    /***************************************************************************/
    /**
	  * Retrieves the number of threads this population uses.
	  *
	  * @return The maximum number of allowed threads
	  */
    std::uint16_t getNThreads() const {
        return n_threads_;
    }

protected:
    /***************************************************************************/
    /**
	  * Loads the data of another GMTExecutorT object
	  *
	  * @param cp A constant pointer to another GMTExecutorT object
	  */
    void load_(const GBaseExecutorT<processable_type> *cp) override {
        const auto p_load_ptr = dynamic_cast<const GMTExecutorT<processable_type> *>(cp);

        if(not cp) { // nullptr
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GMTExecutorT<processable_type>::load_(): Conversion error!" << '\n'
            );
        }

        // Load our parent classes data
        GBaseExecutorT<processable_type>::load_(cp);

        // Load our local data
        n_threads_ = p_load_ptr->n_threads_;
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GMTExecutorT<processable_type>>(
        GMTExecutorT<processable_type> const &,
        GMTExecutorT<processable_type> const &,
        Gem::Common::GToken &
    );

    /***************************************************************************/
    /**
     * Searches for compliance with expectations with respect to another object
     * of the same type
     */
    void compare_(
        const GBaseExecutorT<processable_type> &cp // the other object
        ,
        const Gem::Common::expectation &e // the expectation for this object, e.g. equality
        ,
        const double & /*limit*/ // the limit for allowed deviations of floating point types
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GSerialExecutorT reference independent of this object and convert the pointer
        const GMTExecutorT<processable_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

        GToken token("GMTExecutorT<processable_type>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<GBaseExecutorT<processable_type>>(*this, *p_load, token);

        // ... and then our local data
        compare_t(IDENTITY(n_threads_, p_load->n_threads_), token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /**
	  * General initialization function to be called prior to the first submission
	  */
    void init_() override {
        // GBaseExecutorT<processable_type> sees exactly the environment it would when called from its own class
        GBaseExecutorT<processable_type>::init_();

        // Cross-check
        assert(n_threads_ > 0);

        // Initialize our thread pool
        gtp_ptr_.reset(new Gem::Common::GThreadPool(n_threads_));
    }

    /***************************************************************************/
    /**
	  * General finalization function to be called after the last submission
	  */
    void finalize_() override {
        // Terminate our thread pool
        gtp_ptr_.reset();

        // GBaseExecutorT<processable_type> sees exactly the environment it would when called from its own class
        GBaseExecutorT<processable_type>::finalize_();
    }

    /***************************************************************************/
    /**
	  * Allow to perform necessary setup work for an iteration.
	  */
    void cycleInit_(std::vector<std::shared_ptr<processable_type>> &workItems) override {
        // Make sure the parent classes cycleInit function is executed first
        // This function will also update the iteration start time
        GBaseExecutorT<processable_type>::cycleInit_(workItems);

        // We want an empty futures vector for a new submission cycle,
        // so we do not deal with old errors.
        future_cnt_.clear();
    }

    /***************************************************************************/
    /**
	  * Allows to perform necessary cleanup work for an iteration or do calculations
	  * for the next iteration.
	  */
    void cycleFinalize_(std::vector<std::shared_ptr<processable_type>> &workItems) override {
        // Make sure the parent classes cycleFinalize_ function is executed last
        GBaseExecutorT<processable_type>::cycleFinalize_(workItems);
    }

    /***************************************************************************/
    /**
	  * Allow to perform necessary setup work for an iteration.
	  */
    void iterationInit_(std::vector<std::shared_ptr<processable_type>> &workItems) override {
        // Make sure the parent classes iterationInit_ function is executed first
        GBaseExecutorT<processable_type>::iterationInit_(workItems);
    }

    /***************************************************************************/
    /**
	  * Allows to perform necessary cleanup work for an iteration or do calculations
	  * for the next iteration
	  */
    void iterationFinalize_(std::vector<std::shared_ptr<processable_type>> &workItems) override {
        // Make sure the parent classes iterationFinalize_ function is executed last
        GBaseExecutorT<processable_type>::iterationFinalize_(workItems);
    }

    /***************************************************************************/
    /**
     * Adds local configuration options to a GParserBuilder object
     *
     * @param gpb The GParserBuilder object to which configuration options should be added
     */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) override {
        // Call our parent class's function ...
        GBaseExecutorT<processable_type>::addConfigurationOptions_(gpb);

        // ... then add local dara
        gpb.registerFileParameter<std::uint16_t>(
            "n_processing_threads" // The name of the variable in the configuration file
            ,
            Gem::Courtier::DEFAULTNSTDTHREADS // The default value
            ,
            [this](std::uint16_t nt) { this->setNThreads(nt); }
        ) << "The number of threads used to simultaneously process work items"
          << '\n'
          << "0 means \"automatic\"";
    }

    /***************************************************************************/
    /**
	  * Submits a single work item. As we are dealing with multi-threaded
	  * execution, we simply let a thread pool executed a lambda function
	  * which takes care of the processing.
	  *
	  * @param w_ptr The work item to be processed
	  */
    void submit(std::shared_ptr<processable_type> w_ptr) override {
        using result_type = typename processable_type::result_type;

        if(gtp_ptr_ && w_ptr) { // Do we have a valid thread pool and a valid work item ?
            // async_schedule emits a future, which is std::moved into the std::vector
            future_cnt_.push_back(gtp_ptr_->async_schedule([w_ptr]() {
                return w_ptr->process();
            }));
        }
        else {
            if(not gtp_ptr_) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In In GMTExecutorT<processable_type>::submit(): Error!" << '\n'
                    << "Threadpool pointer is empty" << '\n'
                );
            }
            if(not w_ptr) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In In GMTExecutorT<processable_type>::submit(): Error!" << '\n'
                    << "work item pointer is empty" << '\n'
                );
            }
        }
    }

    /***************************************************************************/
    /**
	  * Waits for the thread pool to run empty and checks for completeness
	  * (i.e. all items have returned and there were no exceptions).
	  *
	  * @param workItems A vector with work items to be evaluated beyond the broker
	  * @param oldWorkItems A vector with work items that have returned after the threshold
	  * @return A struct of booleans indicating whether all items were processed successfully and whether there were errors
	  */
    executor_status_t waitForReturn(
        std::vector<std::shared_ptr<processable_type>> &workItems,
        std::vector<std::shared_ptr<processable_type>> &oldWorkItems
    ) override {
        using result_type = typename processable_type::result_type;

        // Note: Old work items are cleared in the "workOn" function

        // Find out about the errors that were found
        for(auto &f : future_cnt_) {
            // Retrieve the future and check for errors
            try {
                result_type r = f.get();
#ifdef DEBUG
            }
            catch(const g_processing_exception &e) {
                // This is an expected exception if processing has failed. We do nothing,
                // it is up to the caller to decide what to do with processing errors, and
                // these are also stored in the processing item. We do try to create a sort
                // of stack trace by emitting a warning, though. Processing errors should be rare,
                // so might hint at some problem.
                glogger << "In GMTExecutorT<processable_type>::waitForReturn():" << '\n'
                        << "Caught a g_processing_exception exception while retrieving a future"
                        << '\n'
                        << "with the error message" << '\n'
                        << e.what() << '\n'
                        << "Exception information should have been stored in the" << '\n'
                        << "work item itself. Processing should have been marked as" << '\n'
                        << "unsuccessful in the work item. We leave it to the" << '\n'
                        << "caller to deal with this." << '\n'
                        << GWARNING;
#endif
            }
            catch(const std::exception &e) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GMTExecutorT<processable_type>::waitForReturn():" << '\n'
                    << "caught std::exception in a place where we didn't expect any exceptions"
                    << '\n'
                    << "Got error message:" << '\n'
                    << e.what() << '\n'
                );
            }
            catch(...) {
                // All exceptions should be caught inside of the process() call (i.e. emanate
                // from future.get() . It is a severe error if we nevertheless catch an error here.
                // We throw a corresponding gemfony exception.
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GMTExecutorT<processable_type>::waitForReturn(): Caught an" << '\n'
                    << "unknown exception in a place where we didn't expect any exceptions"
                    << '\n'
                );
            }
        }

        // We are dealing with multi-threaded execution on the local computer, so we
        // do not need to wait for return. All we need to do is to check whether
        // we had a "complete return" and/or errors.
        return this->checkExecutionState(workItems);
    }

private:
    /***************************************************************************/
    /**
	  * The default constructor -- only needed for (de-)serialization purposes
	  */
    GMTExecutorT() = default;

    /***************************************************************************/
    /**
	  * Returns the name of this class
	  */
    std::string name_() const override {
        return std::string("GMTExecutorT<processable_type>");
    }

    /***************************************************************************/
    /**
	  * Creates a deep clone of this object.
	  */
    GBaseExecutorT<processable_type> *clone_() const override {
        return new GMTExecutorT<processable_type>(*this);
    }

    /***************************************************************************/
    /**
	  * Retrieval of the start time
	  */
    std::chrono::high_resolution_clock::time_point determineInitialCycleStartTime() const override {
        return this->getObjectFirstSubmissionTime();
    }

    /***************************************************************************/
    /** @brief Graphical progress feedback */
    void visualize_performance() override { /* nothing */
    }

    /***************************************************************************/
    // Data

    std::uint16_t n_threads_ = Gem::Courtier::DEFAULTNSTDTHREADS; ///< The number of threads
    std::shared_ptr<Gem::Common::GThreadPool> gtp_ptr_; ///< Temporarily holds a thread pool

    std::vector<std::future<typename processable_type::result_type>>
        future_cnt_; ///< Temporarily hold futures stored during the submit call
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class relays execution of work items to a broker, to which several
 * different consumers may be connected.
 */
template <typename processable_type>
class GBrokerExecutorT
  : // NOLINT(cppcoreguidelines-special-member-functions)
    public GBaseExecutorT<processable_type> {
    ///////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GBaseExecutorT",
            boost::serialization::base_object<GBaseExecutorT<processable_type>>(*this)
        ) & BOOST_SERIALIZATION_NVP(wait_factor_) &
            BOOST_SERIALIZATION_NVP(min_partial_return_percentage_) &
            BOOST_SERIALIZATION_NVP(capable_of_full_return_) & BOOST_SERIALIZATION_NVP(gpd_) &
            BOOST_SERIALIZATION_NVP(waiting_times_graph_) &
            BOOST_SERIALIZATION_NVP(returned_items_graph_) &
            BOOST_SERIALIZATION_NVP(wait_factor_warning_emitted_);
    }

    ///////////////////////////////////////////////////////////////////////

    using GBufferPortT_ptr = std::shared_ptr<Gem::Courtier::GBufferPortT<processable_type>>;
    using GBroker_ptr = std::shared_ptr<Gem::Courtier::GBrokerT<processable_type>>;

public:
    /***************************************************************************/
    /**
	  * The default constructor
	  */
    GBrokerExecutorT()
      : GBaseExecutorT<processable_type>()
      , gpd_("Maximum waiting times and returned items", 1, 2) {
        gpd_.setCanvasDimensions(std::make_tuple<std::uint32_t, std::uint32_t>(1200, 1600));

        waiting_times_graph_ = std::make_shared<Gem::Common::GGraph2D>();
        waiting_times_graph_->setXAxisLabel("Iteration");
        waiting_times_graph_->setYAxisLabel("Maximum waiting time [s]");
        waiting_times_graph_->setPlotMode(Gem::Common::graphPlotMode::CURVE);

        returned_items_graph_ =
            std::make_shared<Gem::Common::GGraph2D>();
        returned_items_graph_->setXAxisLabel("Iteration");
        returned_items_graph_->setYAxisLabel("Number of returned items");
        returned_items_graph_->setPlotMode(Gem::Common::graphPlotMode::CURVE);
    }

    /***************************************************************************/
    /**
	  * The copy constructor
	  *
	  * @param cp A copy of another GBrokerConnector object
	  */
    GBrokerExecutorT(const GBrokerExecutorT<processable_type> &cp)
      : GBaseExecutorT<processable_type>(cp)
      , wait_factor_(cp.wait_factor_)
      , min_partial_return_percentage_(cp.min_partial_return_percentage_)
      , capable_of_full_return_(cp.capable_of_full_return_)
      , gpd_("Maximum waiting times and returned items", 1, 2) // Intentionally not copied
      , wait_factor_warning_emitted_(cp.wait_factor_warning_emitted_) {
        gpd_.setCanvasDimensions(std::make_tuple<std::uint32_t, std::uint32_t>(1200, 1600));

        waiting_times_graph_ = std::make_shared<Gem::Common::GGraph2D>();
        waiting_times_graph_->setXAxisLabel("Iteration");
        waiting_times_graph_->setYAxisLabel("Maximum waiting time [s]");
        waiting_times_graph_->setPlotMode(Gem::Common::graphPlotMode::CURVE);

        returned_items_graph_ =
            std::make_shared<Gem::Common::GGraph2D>();
        returned_items_graph_->setXAxisLabel("Iteration");
        returned_items_graph_->setYAxisLabel("Number of returned items");
        returned_items_graph_->setPlotMode(Gem::Common::graphPlotMode::CURVE);
    }

    /***************************************************************************/
    /**
	  * The destructor
	  */
    ~GBrokerExecutorT() override {
        // Register the plotter
        gpd_.registerPlotter(waiting_times_graph_);
        gpd_.registerPlotter(returned_items_graph_);

        // Write out the result. This is a hack.
        if(waiting_times_graph_->currentSize() > 0.) {
            gpd_.writeToFile("maximumWaitingTimes.C");
        }
    }

    /***************************************************************************/
    /**
	  * Allows to set the wait factor to be applied to timeouts. A wait factor
	  * <= 0 indicates an indefinite waiting time.
	  */
    void setWaitFactor(double waitFactor) {
        wait_factor_ = waitFactor;
    }

    /***************************************************************************/
    /**
	  * Allows to retrieve the wait factor variable
	  */
    double getWaitFactor() const {
        return wait_factor_;
    }

    /***************************************************************************/
    /**
	  * Allows to retrieve the percentage of items that must have returned
	  * before execution continues. 0 means: Option is disabled.
	  */
    std::uint16_t getMinPartialReturnPercentage() const noexcept {
        return min_partial_return_percentage_;
    }

    /***************************************************************************/
    /**
	  * Allows to set the percentage of items that must have returned
	  * before execution continues. Set to 0 to disable this option.
	  */
    void setMinPartialReturnPercentage(std::uint16_t minPartialReturnPercentage) {
        Gem::Common::checkRangeCompliance<std::uint16_t>(
            minPartialReturnPercentage,
            0,
            100,
            "GBrokerExecutorT<>::setMinPartialReturnPercentage()"
        );
        min_partial_return_percentage_ = minPartialReturnPercentage;
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GBrokerExecutorT<processable_type>>(
        GBrokerExecutorT<processable_type> const &,
        GBrokerExecutorT<processable_type> const &,
        Gem::Common::GToken &
    );

    /***************************************************************************/
    /**
	  * Searches for compliance with expectations with respect to another object
	  * of the same type
	  */
    void compare_(
        const GBaseExecutorT<processable_type> &cp // the other object
        ,
        const Gem::Common::expectation &e // the expectation for this object, e.g. equality
        ,
        const double & /*limit*/ // the limit for allowed deviations of floating point types
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GSerialExecutorT reference independent of this object and convert the pointer
        const GBrokerExecutorT<processable_type> *p_load =
            Gem::Common::g_convert_and_compare(cp, this);

        GToken token("GBrokerExecutorT<processable_type>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<GBaseExecutorT<processable_type>>(*this, *p_load, token);

        // ... and then our local data
        compare_t(IDENTITY(wait_factor_, p_load->wait_factor_), token);
        compare_t(
            IDENTITY(min_partial_return_percentage_, p_load->min_partial_return_percentage_),
            token
        );
        compare_t(IDENTITY(capable_of_full_return_, p_load->capable_of_full_return_), token);
        compare_t(IDENTITY(wait_factor_warning_emitted_, p_load->wait_factor_warning_emitted_), token);

        // React on deviations from the expectation
        token.evaluate();
    }

protected:
    /***************************************************************************/
    /**
	  * Loads the data of another GBrokerExecutorT object
	  *
	  * @param cp A constant pointer to another GBrokerExecutorT object
	  */
    void load_(const GBaseExecutorT<processable_type> *cp) override {
        const auto p_load_ptr = dynamic_cast<const GBrokerExecutorT<processable_type> *>(cp);

        if(not p_load_ptr) { // nullptr
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GBrokerExecutorT<processable_type>::load(): Conversion error!" << '\n'
            );
        }

        // Load our parent classes data
        GBaseExecutorT<processable_type>::load_(cp);

        // Local data
        wait_factor_ = p_load_ptr->wait_factor_;
        min_partial_return_percentage_ = p_load_ptr->min_partial_return_percentage_;
        capable_of_full_return_ = p_load_ptr->capable_of_full_return_;
        wait_factor_warning_emitted_ = p_load_ptr->wait_factor_warning_emitted_;
    }

    /***************************************************************************/
    /**
	  * General initialization function to be called prior to the first submission
	  */
    void init_() override {
        // To be called prior to all other initialization code
        GBaseExecutorT<processable_type>::init_();

        // Make sure we have a valid buffer port
        if(not current_buffer_port_ptr_) {
            current_buffer_port_ptr_.reset(new Gem::Courtier::GBufferPortT<processable_type>());
        }

        // Add the buffer port to the broker and check whether all consumers
        // enrolled with the broker are capable of full return
        capable_of_full_return_ =
            broker<processable_type>()->enrol_buffer_port(current_buffer_port_ptr_);

#ifdef DEBUG
        if(capable_of_full_return_) {
            glogger << "In GBrokerExecutorT<>::init():" << '\n'
                    << "Assuming that all consumers are capable of full return" << '\n'
                    << GLOGGING;
        }
        else {
            glogger << "In GBrokerExecutorT<>::init():" << '\n'
                    << "At least one consumer is not capable of full return" << '\n'
                    << GLOGGING;
        }
#endif
    }

    /***************************************************************************/
    /**
	  * General finalization function to be called after the last submission
	  */
    void finalize_() override {
        // Make it known to the buffer port that we are disconnecting from it
        current_buffer_port_ptr_->producer_disconnect();

        // Get rid of our copy of the buffer port
        current_buffer_port_ptr_.reset();

        // Likely unnecessary cleanup
        capable_of_full_return_ = false;

        // To be called after all other finalization code
        GBaseExecutorT<processable_type>::finalize_();
    }

    /***************************************************************************/
    /**
	  * Allows to perform necessary setup work for a cycle
	  */
    void cycleInit_(std::vector<std::shared_ptr<processable_type>> &workItems) override {
        // Make sure the parent classes cycleInit function is executed first
        // This function will also update the iteration start time
        GBaseExecutorT<processable_type>::cycleInit_(workItems);

        // Reset the number of currently returned items
        n_returned_current_ = 0;

#ifdef DEBUG
        // Check that the waitFactor has a suitable size
        if(not wait_factor_warning_emitted_) {
            if(wait_factor_ > 0. && wait_factor_ < 1.) {
                glogger << "In GBrokerExecutorT::cycleInit_(): Warning" << '\n'
                        << "It is suggested not to use a wait time < 1. Current value: "
                        << wait_factor_ << '\n'
                        << GWARNING;
            }
            wait_factor_warning_emitted_ = true;
        }
#endif
    }

    /***************************************************************************/
    /**
	  * Allows to perform necessary cleanup work for an iteration or do calculations
	  * for the next iteration.
	  */
    void cycleFinalize_(std::vector<std::shared_ptr<processable_type>> &workItems) override {
        // Make sure the parent classes cycleFinalize_ function is executed last
        GBaseExecutorT<processable_type>::cycleFinalize_(workItems);
    }

    /***************************************************************************/
    /**
	  * Allow to perform necessary setup work for an iteration.
	  */
    void iterationInit_(std::vector<std::shared_ptr<processable_type>> &workItems) override {
        // Make sure the parent classes iterationInit_ function is executed first
        GBaseExecutorT<processable_type>::iterationInit_(workItems);
    }

    /***************************************************************************/
    /**
	  * Allows to perform necessary cleanup work for an iteration or do calculations
	  * for the next iteration
	  */
    void iterationFinalize_(std::vector<std::shared_ptr<processable_type>> &workItems) override {
        // Make sure the parent classes iterationFinalize_ function is executed last
        GBaseExecutorT<processable_type>::iterationFinalize_(workItems);
    }

    /***************************************************************************/
    /**
     * Adds local configuration options to a GParserBuilder object
     *
     * @param gpb The GParserBuilder object to which configuration options should be added
     */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) override {
        // Call our parent class's function
        GBaseExecutorT<processable_type>::addConfigurationOptions_(gpb);

        // Add local data

        gpb.registerFileParameter<double>(
            "wait_factor" // The name of the variable
            ,
            DEFAULTBROKERWAITFACTOR2 // The default value
            ,
            [this](double w) { this->setWaitFactor(w); }
        ) << "A static double factor for timeouts"
          << '\n'
          << "A wait factor <= 0 means \"no timeout\"." << '\n'
          << "It is suggested to use values >= 1.";

        gpb.registerFileParameter<std::uint16_t>(
            "min_partial_return_percentage" // The name of the variable
            ,
            DEFAULTEXECUTORPARTIALRETURNPERCENTAGE // The default value
            ,
            [this](std::uint16_t percentage) { this->setMinPartialReturnPercentage(percentage); }
        ) << "Set to a value < 100 to allow execution to continue when"
          << '\n'
          << "minPartialReturnPercentage percent of the expected work items" << '\n'
          << "have returned. Set to 0 to disable this option.";
    }

    /***************************************************************************/
    /**
	  * Submits a single work item.
	  *
	  * @param w_ptr The work item to be processed
	  */
    void submit(std::shared_ptr<processable_type> w_ptr) override {
        if(not w_ptr) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GBrokerExecutorT::submit(): Errornot " << '\n'
                << "Work item is empty" << '\n'
            );
        }

        if(not current_buffer_port_ptr_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GBrokerExecutorT::submit(): Error!" << '\n'
                << "Current buffer port is empty when it shouldn't be" << '\n'
            );
        }

        // Store the id of the buffer port in the item
        w_ptr->setBufferId(current_buffer_port_ptr_->getUniqueTag());

        // Perform the actual submission
        current_buffer_port_ptr_->push_raw(w_ptr);
    }

    /***************************************************************************/
    /**
	  * Waits for all items to return or possibly until a timeout has been reached.
	  * Checks for completeness (i.e. all items have returned and there were no exceptions).
	  *
	  * @param workItems A vector with work items to be evaluated beyond the broker
	  * @param oldWorkItems A vector with work items that have returned after the threshold
	  * @return A struct of booleans indicating whether all items were processed successfully and whether there were errors
	  */
    executor_status_t waitForReturn(
        std::vector<std::shared_ptr<processable_type>> &workItems,
        std::vector<std::shared_ptr<processable_type>> &oldWorkItems
    ) override {
        // Act depending on the capabilities of the consumer or user-preferences
        return (capable_of_full_return_ || wait_factor_ == 0.)
                 ? this->waitForFullReturn(workItems, oldWorkItems)
                 : this->waitForTimeOut(workItems, oldWorkItems);
    }

private:
    /***************************************************************************/
    /**
	  * Returns the name of this class
	  */
    std::string name_() const override {
        return std::string("GBrokerExecutorT<processable_type>");
    }

    /***************************************************************************/
    /**
	  * Creates a deep clone of this object.
	  */
    GBaseExecutorT<processable_type> *clone_() const override {
        return new GBrokerExecutorT<processable_type>(*this);
    }

    /***************************************************************************/
    /**
	  * Retrieves an item from the broker, waiting indefinitely for returns
	  *
	  * @return The obtained processed work item
	  */
    std::shared_ptr<processable_type> retrieve() {
        // Holds the retrieved item
        std::shared_ptr<processable_type> w;
        current_buffer_port_ptr_->pop_processed(w);

        return w;
    }

    /***************************************************************************/
    /**
	  * Retrieves an item from the broker, waiting up to a given amount of time.
	  * The call will return earlier, if an item could already be retrieved.
	  *
	  * @return The obtained processed work item
	  */
    std::shared_ptr<processable_type> retrieve(const std::chrono::duration<double> &timeout) {
        // Holds the retrieved item, if any
        std::shared_ptr<processable_type> w_ptr;
        current_buffer_port_ptr_->pop_processed(w_ptr, timeout);
        return w_ptr;
    }

    /***************************************************************************/
    /**
	  * Returns a complete set of items or waits until a timeout occurs.
	  * Returns two status flags "is_complete" (== all items from the current
	  * submission have returned) and "has_errors" (== some or all of the submitted
	  * items had errors).
	  *
	  * @param workItems The work items to be processed
	  * @param oldWorkItems A collection of old work items that have returned after a timeout
	  * @return A struct of booleans indicating whether all items were processed successfully and whether there were errors
	  */
    executor_status_t waitForTimeOut(
        std::vector<std::shared_ptr<processable_type>> &workItems,
        std::vector<std::shared_ptr<processable_type>> &oldWorkItems
    ) {
        std::shared_ptr<processable_type> w_ptr;
        n_returned_current_ = 0;
        executor_status_t status;

        // Note: Old work items are cleared in the "workOn" function

        // Start to retrieve individuals and sort them into our vectors,
        // until a halt criterion is reached.
        do {
            // Get the next individual. If we didn't receive a valid
            // item, go to the timeout check
            if(not(w_ptr = this->getNextItem())) {
                continue; // NOLINT(bugprone-assignment-in-if-condition)
            }

            // Try to add the work item to the list and check for completeness
            status = this->addWorkItemAndCheckCompleteness(w_ptr, workItems, oldWorkItems);

            // No need to continue if all currently submitted work items have returned
            if(status.is_complete) {
                break;
            }

            // For succesfully processed items, update the internal timeout variables,
            // so we know how much longer this cycle should run
            if(w_ptr->is_processed()) {
                this->updateTimeout(w_ptr);
            }
        }
        while(not halt());

        // Check for the processing flags and derive the is_complete and has_errors states
        return this->checkExecutionState(workItems);
    };

    /***************************************************************************/
    /**
	  * Waits (possibly indefinitely) until all items have returned. Note that this
	  * function may stall, if for whatever reason a work item does not return. If this
	  * is not acceptable, use waitForTimeout() instead of this function. It is recommended
	  * to only use this function in environments that are considered safe in the sense
	  * that work items will always return. Local cluster environments may fall into this
	  * category (or the risk is considered low enough by the user to use this simpler
	  * function). It is possible to allow a partial return by setting the percentage
	  * of required returned items to an appropriate number. If all iterations run a
	  * "full return" strategy without this option, the oldWorkItems vector will remain empty.
	  * Otherwise there may be old returning items in the next iterations.
	  *
	  * @param workItems The work items to be processed
	  * @param oldWorkItems A collection of old work items that have returned after a timeout
	  * @return A struct of booleans indicating whether all items were processed successfully and whether there were errors
	  */
    executor_status_t waitForFullReturn(
        std::vector<std::shared_ptr<processable_type>> &workItems,
        std::vector<std::shared_ptr<processable_type>> &oldWorkItems
    ) {
        executor_status_t status;

        do {
            status = this->addWorkItemAndCheckCompleteness(
                this->retrieve() // Get the next item, waiting indefinitely
                ,
                workItems,
                oldWorkItems
            );

            // Break the loop if all items (or at least the minimum percentage) were received
            if(status.is_complete || this->minPartialReturnRateReached()) {
                break;
            }
        }
        while(true);

        // Check for the processing flags and derive the is_complete and has_errors states
        return this->checkExecutionState(workItems);
    }

    /***************************************************************************/
    /**
	  * Updates the remaining time for this iteration
	  */
    void updateTimeout(
        [[maybe_unused]] std::shared_ptr<processable_type> w_ptr
    ) {
        //-----------------------------------------------
        // We do not check for error conditions (particularly
        // w_ptr is empty or not processed), as this function
        // is only called for processed work items.

        //-----------------------------------------------
        // Extract some date for this work item

        // The current time
        auto now = this->now();

        // The time that has passed since the start of execution in this cycle
        std::chrono::duration<double> currentElapsed = now - this->getApproxCycleStartTime();
        // Safeguard against inaccurate cycleStartTime
        if(currentElapsed < std::chrono::duration<double>(0.)) {
            currentElapsed = std::chrono::duration<double>(0.1); // 0.1 s
        }

        // Calculate the average return time so far. This holds true also for the very first item
#ifdef DEBUG
        if(0 == n_returned_current_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GBrokerExeuctorT<processable_type>::updateTimeout():" << '\n'
                << "nReturnedCurrent_ is 0" << '\n'
            );
        }
#endif
        std::chrono::duration<double> avgReturnTime =
            currentElapsed / Gem::Common::narrow<double>(n_returned_current_);

        // Retrieve the current maximum processing time
        std::chrono::duration<double> maxProcessingTime(max_processing_time_);

        //-----------------------------------------------
        // The actual timeout calculation
        max_timeout_ =
            wait_factor_ * (avgReturnTime * this->getExpectedNumber() + maxProcessingTime);

        //-----------------------------------------------
        // Let the audience know in DEBUG mode
#if defined(DEBUG) && defined(VERBOSETIMEOUTS)
        glogger << nReturnedCurrent_ << "\t: Got timeout of " << maxTimeout_.count() << '\n'
                << GLOGGING;
#endif

        //-----------------------------------------------
    }

    /***************************************************************************/
    /**
	  * Checks whether a timeout was encountered.
	  */
    bool timeout() {
        std::chrono::duration<double> currentElapsed =
            this->now() - this->getApproxCycleStartTime();

        if(currentElapsed >= max_timeout_) {
#if defined(DEBUG) && defined(VERBOSETIMEOUTS)
            glogger << "Leaving after timeout of " << maxTimeout_.count() << " was reached"
                    << '\n'
                    << GLOGGING;
#endif

            return true;
        }
                    return false;
       
    }

    /***************************************************************************/
    /**
	  * Checks for halt criteria
	  */
    bool halt() {
        // Timeout checks and update of timeout variables
        if(this->timeout()) {
            return true;
        }

        // For some algorithms, a partial return rate suffices
        if(this->minPartialReturnRateReached()) {
            return true;
        }

        // We want to continue
        return false;
    }

    /***************************************************************************/
    /**
	  * Checks whether the minimum return rate was reached
	  */
    bool minPartialReturnRateReached() {
        // Leave if this check is disabled
        if(0 == this->getMinPartialReturnPercentage()) {
            return false;
        }

        // Avoid problems related to floating point accuracy
        std::size_t expectedNumber = this->getExpectedNumber();
        if(n_returned_current_ == expectedNumber) {
            return true;
        }

        // Check if we have reached the minimum percentage. getMinPartialReturnPercentage()
        // is a percentage in [0,100], so the returned fraction must be scaled to a percentage
        // before the comparison -- otherwise the fraction (<= 1.0) is compared against a value
        // up to 100 and the threshold can essentially never be reached (the feature was a no-op
        // for any setting other than 0 or, via the exact-equality branch above, 100).
        double realPercentage = 100. * Gem::Common::narrow<double>(n_returned_current_) /
                                Gem::Common::narrow<double>(expectedNumber);
        return (
            realPercentage >= Gem::Common::narrow<double>(this->getMinPartialReturnPercentage())
        );
    }

    /***************************************************************************/
    /**
	  * Get the remaining time for this iteration
	  */
    std::chrono::duration<double> remainingTime() const {
        // The time that has passed since the start of execution in this cycle
        std::chrono::duration<double> currentElapsed =
            this->now() - this->getApproxCycleStartTime();
        // Calculate the remaining time
        std::chrono::duration<double> remainingTime = std::chrono::duration<double>(0.);
        if(max_timeout_ > currentElapsed) {
            remainingTime = max_timeout_ - currentElapsed;
        }

        return remainingTime;
    }

    /***************************************************************************/
    /**
	  * Retrieves the next work item. Note that this function may block if,
	  * during the very first retrieval of work items, no (or no processed) item
	  * returns. A warning will be emitted for each retrieved item which has
	  * errors or is unprocessed during the first retrieval.
	  */
    std::shared_ptr<processable_type> getNextItem() {
        std::shared_ptr<processable_type> w_ptr;

        // Wait indefinitely for a processed item, if this is the very first individual retrieved.
        // We have no information on which we might base a timeout.
        if(this->firstRetrieval()) {
            do {
                // Wait indefinitely for the very first, successfully processed individual
                w_ptr = this->retrieve();

                // It is a severe error if we get an empty pointer here
                if(not w_ptr) {
                    throw geneva_exception(
                        g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                        << "In GBrokerExecutorT<processable_type>::getNextItem(): Received empty "
                           "first individual"
                        << '\n'
                    );
                }

                // We do accept unsuccessfully processed items, but emit
                // a warning, as this should be a very rare case
                if(w_ptr->is_processed()) {
                    break;
                }
                // unprocessed or has an error
                    glogger << "In GBrokerExecutorT<>::getNextItem():" << '\n'
                            << "Received \"first\" individual which is either" << '\n'
                            << "unprocessed or has errors. Got processing status of "
                            << w_ptr->getProcessingStatus() << '\n'
                            << "but expected " << processingStatus::PROCESSED << " ." << '\n'
                            << "The item will be discarded. As this should be a rare occurance,"
                            << '\n'
                            << "we do emit a warning here." << '\n'
                            << GWARNING;
               
            }
            while(true);
        }
        else { // not the first individual in the first iteration
            // Calculate the timeout
            std::chrono::duration<double> remainingTime = this->remainingTime();
            if(remainingTime != std::chrono::duration<double>(0.)) {
                // Obtain the next item, observing a timeout
                w_ptr = this->retrieve(remainingTime);
            }
        }

        // Update the maximum amount of time needed for processing a work item in the iteration. We use the
        // amount of time between retrieval from the raw queue and submission to the processed queue. We
        // euphemistically call this "processing time", even though it may contain transfer time,
        // times spent in queues, etc.
        if(w_ptr) {
#ifdef DEBUG
            if(w_ptr->getRawRetrievalTime() >= w_ptr->getProcSubmissionTime()) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GBrokerExeuctorT<processable_type>::getNextItem():" << '\n'
                    << "Retrieval from the raw queue seems to have happened after" << '\n'
                    << "the submission to the processed queue." << '\n'
                );
            }
#endif

            // Calculate the processing time and update the accumulator
            std::chrono::duration<double> currentProcessingTime =
                w_ptr->getProcSubmissionTime() - w_ptr->getRawRetrievalTime();
            max_processing_time_ = std::max(max_processing_time_, currentProcessingTime.count());
        }

        return w_ptr; // Will be empty if remainingTime is 0.
    }

    /***************************************************************************/
    /**
	  * Checks whether any calls to "retrieve() have been made yet This is identical
	  * to the first call to this function, so we simply check and set a boolean
	  * indicating whether this is the first retrieval.
	  */
    bool firstRetrieval() const {
        if(first_retrieval_) {
#ifdef DEBUG
            if((not this->inFirstIteration() || not this->inFirstCycle() ||
                0 < n_returned_current_)) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GBrokerExecutorT<processable_type>::firstRetrieval():" << '\n'
                    << "Got true==first_retrieval_, while one of the preconditions isn't met:"
                    << '\n'
                    << "this->inFirstIteration() : " << this->inFirstIteration() << '\n'
                    << "this->inFirstCycle()     :" << this->inFirstCycle() << '\n'
                    << "nReturnedCurrent_       :" << n_returned_current_ << " (we expect 0)"
                    << '\n'
                );
            }
#endif

            first_retrieval_ = false;
            return true;
        }
                    return false;
       
    }

    /***************************************************************************/
    /**
	  * Checks whether any work items have returned yet This is identical
	  * to the first call to this function, so we simply check and set a boolean
	  * indicating whether this is the first item.
	  */
    bool firstItem() const {
        if(first_item_) {
#ifdef DEBUG
            if((not this->inFirstIteration() || 1 != n_returned_current_)) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GBrokerExecutorT<processable_type>::firstItem():" << '\n'
                    << "Got true==first_item_, while one of the preconditions isn't met:"
                    << '\n'
                    << "this->inFirstIteration() : " << this->inFirstIteration() << '\n'
                    << "nReturnedCurrent_       :" << n_returned_current_ << " (we expect 1)"
                    << '\n'
                );
            }
#endif

            first_item_ = false;
            return true;
        }
                    return false;
       
    }

    /***************************************************************************/
    /**
	  * Adds a work item to the corresponding vectors. This function assumes that
	  * the work item is valid, i.e. points to a valid object.
	  *
	  * @param w_ptr The item to be added to the (old?) work items
	  * @param workItems The list of work items from the current iteration
	  * @param oldWorkItems The list of wirk items from previous iterations
	  * @return A struct indicating whether all work items of the current iteration were received and whether there were errors
	  */
    executor_status_t addWorkItemAndCheckCompleteness(
        std::shared_ptr<processable_type> w_ptr,
        std::vector<std::shared_ptr<processable_type>> &workItems,
        std::vector<std::shared_ptr<processable_type>> &oldWorkItems
    ) {
        // If we have been passed an empty item, simply continue the outer loop
        if(not w_ptr) {
            return executor_status_t{false /* is_complete */, false /* has_errors */};
        }

        bool complete = false;
        bool has_errors = false;

        auto current_submission_id = this->get_iteration_counter();
        auto worker_submission_id = w_ptr->getIterationCounter();

        // Did this work item originate in the current submission cycle ?
        if(current_submission_id == worker_submission_id) {
            // Extract the original position of the work item and cross-check
            std::size_t worker_position = w_ptr->getCollectionPosition();

            // Add the work item to the list in the desired position. Re-submitted items
            // might return twice, so we only add them if a processed item hasn't been added yet.
            // We also take care of the situation that w_ptr points to the same object as the one
            // stored in the workItems vector. This may happen in the case of local submission,
            // such as with multi-threaded or serial consumers. DO_PROCESS will have been replaced
            // by PROCESSED in this case.
            if(workItems.at(worker_position) == w_ptr // Same item
               || processingStatus::DO_PROCESS ==
                      workItems.at(worker_position)->getProcessingStatus()) {
                // Note that also items with errors may be added here. It is up to
                // the caller to decide what to do with such work items.
                if(workItems.at(worker_position) != w_ptr) {
                    workItems.at(worker_position) = w_ptr;
                }
                if(++n_returned_current_ == this->getExpectedNumber()) {
                    complete = true;
                }
                if(w_ptr->has_errors()) {
                    has_errors = true;
                }
            } // no else
        }
        else { // Not a work item from the current submission cycle.
            // Ignore old work items with errors
            if(processingStatus::PROCESSED == w_ptr->getProcessingStatus()) {
                oldWorkItems.push_back(w_ptr);
            }
            else {
                // This should be rare. As we throw away items here, we want to
                // make a record as a frequent occurrance might indicate a problem
                glogger << "In GBrokerExecutorT<>::addWorkItemAndCheckCompleteness():" << '\n'
                        << "Received old work item from submission cycle " << worker_submission_id
                        << " (now " << current_submission_id << ")" << '\n'
                        << "We will throw the item away as it has the status id "
                        << w_ptr->getProcessingStatus() << '\n'
                        << "(expected processingStatus::PROCESSED / " << processingStatus::PROCESSED
                        << ")" << '\n'
                        << (w_ptr->has_errors() ? w_ptr->getStoredErrorDescriptions() : "")
                        << '\n'
                        << GLOGGING;
            }
        }

        return executor_status_t{complete, has_errors};
    }

    /***************************************************************************/
    /**
 	  * Allows to emit information at the end of an iteration
 	  */
    void visualize_performance() override {
#ifdef DEBUG
        // Now do our own reporting
        std::chrono::duration<double> currentElapsed = this->now() - this->getIterationStartTime();
        auto current_iteration = this->get_iteration_counter();

        waiting_times_graph_->add(
            Gem::Common::narrow<double>(current_iteration),
            max_timeout_.count()
        );
        returned_items_graph_->add(
            Gem::Common::narrow<double>(current_iteration),
            Gem::Common::narrow<double>(this->getNReturnedLast())
        );
#endif
    }

    /***************************************************************************/
    /**
	  * Retrieval of the start time. For brokered execution this is defined as the
	  * time of the first retrieval of an item from the buffer port, as it may take
	  * some time until (networked) clients ask for work.
	  *
	  * @return The time of the first retrieval of a work item from the buffer port
	  */
    std::chrono::high_resolution_clock::time_point determineInitialCycleStartTime() const override {
#ifdef DEBUG
        // Check if we have a valid buffer port
        if(not current_buffer_port_ptr_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GBrokerExecutorT<processable_type>::determineInitialCycleStartTime():"
                << '\n'
                << "No valid buffer port found" << '\n'
            );
        }
#endif

        return current_buffer_port_ptr_->getFirstRetrievalTime();
    }

    /***************************************************************************/
    // Local data
    double wait_factor_ = DEFAULTBROKERWAITFACTOR2; ///< A static factor to be applied to timeouts

    std::uint16_t min_partial_return_percentage_ =
        DEFAULTEXECUTORPARTIALRETURNPERCENTAGE; ///< Minimum percentage of returned items after which execution continues

    GBufferPortT_ptr
        current_buffer_port_ptr_; ///< Holds a GBufferPortT object during the calculation. Note: It is neither serialized nor copied

    bool capable_of_full_return_ =
        false; ///< Indicates whether the broker may return results without losses

    mutable bool first_retrieval_ =
        true; ///< Indicates whether any retrievals have taken place so far
    mutable bool first_item_ =
        true; ///< Indicates whether we are dealing with the very first individual

    Gem::Common::GPlotDesigner gpd_; ///< A wrapper for the plots
    std::shared_ptr<Gem::Common::GGraph2D>
        waiting_times_graph_; ///< The maximum waiting time resulting from the wait factor
    std::shared_ptr<Gem::Common::GGraph2D>
        returned_items_graph_; ///< The maximum waiting time resulting from the wait factor

    bool wait_factor_warning_emitted_ =
        false; ///< Specifies whether a warning about a small waitFactor has already been emitted

    std::size_t n_returned_current_ =
        0; ///< Temporary that holds the number of returned work items duing a submission cycle (or a resubmission)

    std::chrono::duration<double> max_timeout_ = std::chrono::duration<double>(
        0.
    ); ///< The maximum amount of time allowed for the entire calculation

    double max_processing_time_ = 0.0; ///< Tracks maximum processing time of returned individuals (seconds)
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Courtier */

/******************************************************************************/
// Some code for Boost.Serialization

/******************************************************************************/
// Mark GBaseExecutorT<> as abstract. This is the content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) // NOLINT
namespace boost::serialization {
template <typename processable_type>
struct is_abstract<Gem::Courtier::GBaseExecutorT<processable_type>> : public boost::true_type {};
template <typename processable_type>
struct is_abstract<const Gem::Courtier::GBaseExecutorT<processable_type>>
  : public boost::true_type {};
} /* namespace boost::serialization */

/******************************************************************************/
