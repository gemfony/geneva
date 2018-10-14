/**
 * @file
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here

#include <cmath>
#include <sstream>
#include <algorithm>
#include <vector>
#include <utility>
#include <functional>
#include <memory>
#include <type_traits>
#include <tuple>
#include <chrono>
#include <exception>
#include <thread>
#include <mutex>

// Boost headers go here
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>

// Geneva headers go here
#include "common/GCommonInterfaceT.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GParserBuilder.hpp"
#include "common/GPlotDesigner.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "common/GThreadPool.hpp"
#include "courtier/GBufferPortT.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GProcessingContainerT.hpp"
#include "courtier/GCourtierHelperFunctions.hpp"

namespace Gem {
namespace Courtier {

/******************************************************************************/
/**
 * Status information for the workOn function and helper functions
 */
struct executor_status_t {
	 bool is_complete = false; ///< Indicates whether a complete set of current items was obtained
	 bool has_errors  = false; ///< Indicates whether there were errors during processing of current items
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
template<typename processable_type>
class GBaseExecutorT
	: public Gem::Common::GCommonInterfaceT<GBaseExecutorT<processable_type>>
{
	 // Make sure processable_type adheres to the GProcessingContainerT interface
	 static_assert(
		 std::is_base_of<Gem::Courtier::GProcessingContainerT<processable_type, typename processable_type::result_type>, processable_type>::value
		 , "GBaseExecutorT: processable_type does not adhere to the GProcessingContainerT<> interface"
	 );

public:
	 /////////////////////////////////////////////////////////////////////////////

	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GCommonInterfaceT_GBaseExecutotT"
						, boost::serialization::base_object<Gem::Common::GCommonInterfaceT<GBaseExecutorT<processable_type>>>(*this))
		 & BOOST_SERIALIZATION_NVP(m_maxResubmissions);
	 }

	 /////////////////////////////////////////////////////////////////////////////

	 /***************************************************************************/
	 /** @brief The default constructor */
	 GBaseExecutorT() = default;

	 /***************************************************************************/
	 /**
	  * The copy constructor. Several data items are not copied, as we want them
	  * in pristine condition for a new object.
	  *
	  * @param cp A copy of another GBrokerConnector object
	  */
	 GBaseExecutorT(const GBaseExecutorT<processable_type> &cp)
		 : Gem::Common::GCommonInterfaceT<GBaseExecutorT<processable_type>>(cp)
		 , m_maxResubmissions(cp.m_maxResubmissions)
	 { /* nothing */ }

	 /***************************************************************************/
	 /** @brief The standard destructor */
	 virtual ~GBaseExecutorT() = default;

	 /***************************************************************************/
	 /**
	  * Returns the name of this class
	  */
	 std::string name() const override {
		 return std::string("GBaseExecutorT<processable_type>");
	 }

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
	  * They will be appended to the m_old_work_items_vec vector. You might thus have to
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
	  * @param caller Optionally holds information on the caller
	  * @return A struct of booleans indicating whether all items were processed successfully and whether there were errors
	  */
	 executor_status_t workOn(
		 std::vector<std::shared_ptr<processable_type>>& workItems
		 , bool resubmitUnprocessed = false
		 , const std::tuple<ITERATION_COUNTER_TYPE, bool>& externalIterationCounter = std::tuple<ITERATION_COUNTER_TYPE, bool>(0, false)
		 , const std::string &caller = std::string()
	 ) {
		 //------------------------------------------------------------------------------------------
		 // Make sure only one instance of this function can be called at the same time. If locking
		 // the mutex fails, some other call to this function is still alive, which is a severe error
		 std::unique_lock<std::mutex> workon_lock(m_concurrent_workon_mutex, std::defer_lock);
		 if(not workon_lock.try_lock()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GBaseExeuctorT<processable_type>::workOn(): Another call to this function still seems" << std::endl
					 << "to be active which is a severe error."
			 );
		 }

		 //------------------------------------------------------------------------------------------
		 // Assign an external iteration id to the iteration counter, if requested by the user
		 bool useExternalId = std::get<1>(externalIterationCounter);
		 if(useExternalId) {
			 // Extract the external iteration counter
			 ITERATION_COUNTER_TYPE externalId = std::get<0>(externalIterationCounter);
			 // This call will throw, if externalId is < m_iteration_counter
			 this->set_external_iteration_counter(externalId);
		 }

		 //------------------------------------------------------------------------------------------
		 // Initialization of a new iteration (possibly involving more than one submission of
		 // selected work items).
		 this->iterationInit(workItems);

		 //------------------------------------------------------------------------------------------
		 // The main business logic of item submission

		 m_nResubmissions = 0;
		 executor_status_t status {false /* is_complete */, false /* has_errors */};
		 do {
			 //-----------------------
			 // Initialization of a new "run" or resubmission
			 this->cycleInit(workItems);

			 //-----------------------
			 // Submission and retrieval

			 // Submit all work items.
			 m_expectedNumber = this->submitAllWorkItems(workItems);

			 // Wait for work items to complete. This function needs to
			 // be re-implemented in derived classes.
			 auto current_status = waitForReturn(
				 workItems
				 , m_old_work_items_vec
			 );

			 // There may not be errors during resubmission, so we need to save the "error state"
			 if (current_status.is_complete) status.is_complete = true;
			 if (current_status.has_errors) status.has_errors = true;

			 // Perform necessary cleanup work for an iteration
			 this->cycleFinalize(workItems);

			 //-----------------------
			 // Check whether we want to break the loop

			 // Leave if we are complete or if we haven't been asked to resubmit unprocessed
			 // items (even if we do not have a complete set of work items). Also leave
			 // if we have reached the maximum number of resubmissions or
			 // m_maxResubmissions was explicitly set to 0.
			 if (
				 status.is_complete  // nothing left to do
				 || not resubmitUnprocessed || (resubmitUnprocessed && m_maxResubmissions == 0) // user is happy with unprocessed items
				 || (++m_nResubmissions >= m_maxResubmissions) // we have tried to resubmit items but did not succeed
			 ) {
				 // Leave the loop
				 break;
			 }

			 //-----------------------
		 } while(true);

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
		 m_iteration_counter++;

		 // Note: unprocessed items will be returned to the caller who needs to deal with them
		 return status;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves a copy of the old work items vector. Calling this function will
	  * clear the vector!
	  *
	  * @return A copy of the old work items vector
	  */
	 std::vector<std::shared_ptr<processable_type>> getOldWorkItems() {
		 return std::move(m_old_work_items_vec);
	 }

	 /***************************************************************************/
	 /**
	  * Adds local configuration options to a GParserBuilder object
	  *
	  * @param gpb The GParserBuilder object to which configuration options should be added
	  */
	 virtual void addConfigurationOptions(
		 Gem::Common::GParserBuilder &gpb
	 ) BASE {
		 gpb.registerFileParameter<std::size_t>(
			 "maxResubmissions" // The name of the variable
			 , DEFAULTMAXRESUBMISSIONS // The default value
			 , [this](std::size_t r) {
				 this->setMaxResubmissions(r);
			 }
		 )
			 << "The amount of resubmissions allowed if a full return of work" << std::endl
			 << "items was expected but only a subset has returned";
	 }

	 /***************************************************************************/
	 /**
	  * Specifies how often work items should be resubmitted in the case a full return
	  * of work items is expected.
	  *
	  * @param maxResubmissions The maximum number of allowed resubmissions
	  */
	 void setMaxResubmissions(std::size_t maxResubmissions) {
		 m_maxResubmissions = maxResubmissions;
	 }

	 /***************************************************************************/
	 /**
	  * Returns the maximum number of allowed resubmissions
	  *
	  * @return The maximum number of allowed resubmissions
	  */
	 std::size_t getMaxResubmissions() const {
		 return m_maxResubmissions;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieve the number of individuals returned during the last iteration
	  */
	 std::size_t getNReturnedLast() const noexcept {
		 return m_n_returnedLast;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieve the number of individuals NOT returned during the last iteration
	  */
	 std::size_t getNNotReturnedLast() const noexcept {
		 return m_n_notReturnedLast;
	 }

	 /***************************************************************************/
	 /**
 	  * Retrieves the current number of old work items in this iteration
 	  */
	 std::size_t getNOldWorkItems() const noexcept {
		 return m_n_oldWorkItems;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the number of work items with errors in this iteration
	  */
	 std::size_t getNErroneousWorkItems() const noexcept {
		 return m_n_erroneousItems;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieve the time of the very first submission in this object
	  */
	 std::chrono::high_resolution_clock::time_point getObjectFirstSubmissionTime() const {
		 return m_object_first_submission_time;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieve the time of the very first submission in the current iteration
	  */
	 std::chrono::high_resolution_clock::time_point getIterationFirstSubmissionTime() const {
		 return m_iteration_first_submission_time;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieve the time of the very first submission in the current resubmission
	  */
	 std::chrono::high_resolution_clock::time_point getCycleFirstSubmissionTime() const {
		 return m_cycle_first_submission_time;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieve the approximate time of the start of the cycle
	  */
	 std::chrono::high_resolution_clock::time_point getApproxCycleStartTime() const {
		 return m_approx_cycle_start_time;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieve the end time of the last cycle
	  */
	 std::chrono::high_resolution_clock::time_point getCycleEndTime() const {
#ifdef DEBUG
		 // Cross check that the cycle has indeed ended
		 if(m_cycle_running) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GBaseExecutorT<processable_type>::getCycleEndTime():" << std::endl
					 << "There still seems to be an active cycle while the end" << std::endl
					 << "time of the cycle is retrieved" << std::endl
			 );
		 }
#endif

		 return m_cycle_end_time;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieve the end time of the last iteration
	  */
	 std::chrono::high_resolution_clock::time_point getIterationEndTime() const {
#ifdef DEBUG
		 // Cross check that the cycle has indeed ended
		 if(m_iteration_running) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GBaseExecutorT<processable_type>::getIterationEndTime():" << std::endl
					 << "There still seems to be an active iteration while the end" << std::endl
					 << "time of the iteration is retrieved" << std::endl
			 );
		 }
#endif

		 return m_iteration_end_time;
	 }

	 /***************************************************************************/
	 /**
	  * Searches for compliance with expectations with respect to another object
	  * of the same type
	  */
	 void compare(
		 const GBaseExecutorT<processable_type>& cp // the other object
		 , const Gem::Common::expectation& e // the expectation for this object, e.g. equality
		 , const double& limit // the limit for allowed deviations of floating point types
	 ) const override {
		 using namespace Gem::Common;

		 // Check that we are dealing with a GDecorator reference independent of this object and convert the pointer
		 const GBaseExecutorT<processable_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		 GToken token("GBaseExecutorT<processable_type>", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GCommonInterfaceT<GBaseExecutorT<processable_type>>>(IDENTITY(*this, *p_load), token);

		 // ... and then our local data
		 compare_t(IDENTITY(this->m_maxResubmissions,  p_load->m_maxResubmissions), token);

		 // React on deviations from the expectation
		 token.evaluate();
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Loads the data of another object
	  */
	 void load_(const GBaseExecutorT<processable_type>* cp) override {
		 // Check that we are dealing with a GDecorator reference independent of this object and convert the pointer
		 const auto p_load_ptr = Gem::Common::g_convert_and_compare(cp, this);

		 // No parent class with loadable data

		 // Copy local data
		 m_maxResubmissions = p_load_ptr->m_maxResubmissions;
	 }

	 /***************************************************************************/
	 /**
	  * General initialization function to be called prior to the first submission
	  */
	 virtual void init_() BASE {
		 m_no_items_submitted_in_object = true;
	 }

	 /***************************************************************************/
	 /**
	  * General finalization function to be called after the last submission
	  */
	 virtual void finalize_() BASE { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * General initialization function to be called prior to the first submission
	  */
	 virtual void iterationInit_(std::vector<std::shared_ptr<processable_type>>& workItems) BASE {
		 // Make it known that a new iteration is about to start
		 m_iteration_running = true;
		 // Make it known that no items have been submitted yet in this iteration
		 m_no_items_submitted_in_iteration = true;

		 // Clear old work items not cleared by the caller after the last iteration
		 m_old_work_items_vec.clear();

		 // Reset some counters and flags
		 m_n_returnedLast = 0;
		 m_n_notReturnedLast = 0;
		 m_n_oldWorkItems = 0;
		 m_n_erroneousItems = 0;
	 }

	 /***************************************************************************/
	 /**
	  * Code to be executed before the start of an iteration (i.e. a call to workOn)
	  */
	 void iterationInit(std::vector<std::shared_ptr<processable_type>>& workItems) {
		 this->iterationInit_(workItems);
	 }

	 /***************************************************************************/
	 /**
	  * General initialization function to be called prior to the first submission
	  */
	 virtual void iterationFinalize_(std::vector<std::shared_ptr<processable_type>>& workItems) BASE {
		 // Sort remaining old work items according to their position
		 std::sort(
			 m_old_work_items_vec.begin()
			 , m_old_work_items_vec.end()
			 , [](std::shared_ptr<processable_type> x_ptr, std::shared_ptr<processable_type> y_ptr) -> bool {
				 using namespace boost;
				 return x_ptr->getCollectionPosition() <= y_ptr->getCollectionPosition();
			 }
		 );
		 // Remove duplicates
		 m_old_work_items_vec.erase(
			 std::unique(
				 m_old_work_items_vec.begin()
				 , m_old_work_items_vec.end()
				 , [](std::shared_ptr<processable_type> x_ptr, std::shared_ptr<processable_type> y_ptr) -> bool {
					 return x_ptr->getCollectionPosition() == y_ptr->getCollectionPosition();
				 }
			 ) // Returns the begin() of the duplicates moved to the end of the vector
			 , m_old_work_items_vec.end()
		 );
		 // Remove unprocessed or erroneous items and count the remaining items
		 m_n_oldWorkItems    = this->cleanItemsWithoutFlag(m_old_work_items_vec, processingStatus::PROCESSED);

		 // Find out about the number of items that have not returned (yet) from the last submission.
		 // These are equivalent to the items that still have the DO_PROCESS flag set.
		 m_n_notReturnedLast = this->countItemsWithStatus(workItems, processingStatus::DO_PROCESS);
		 // The number of actually returned items is equal to the number of expected items minus the number of not returned items
		 m_n_returnedLast    = m_expectedNumber - m_n_notReturnedLast;
		 // Count the number of work items with errors. We need to count two flags
		 m_n_erroneousItems  = this->countItemsWithStatus(workItems, processingStatus::ERROR_FLAGGED);
		 m_n_erroneousItems += this->countItemsWithStatus(workItems, processingStatus::EXCEPTION_CAUGHT);

		 // Make it known that the first iteration has ended (if this is the first iteration)
		 if(m_in_first_iteration) {
			 m_in_first_iteration = false;
		 }

		 // Mark the end of processing in this iteration
		 m_iteration_end_time = this->now();

		 // Make it known that the iteration has ended
		 m_iteration_running = false;
	 }

	 /***************************************************************************/
	 /**
	  * Code to be executed before the start of an iteration (i.e. a call to workOn)
	  */
	 void iterationFinalize(std::vector<std::shared_ptr<processable_type>>& workItems) {
		 this->iterationFinalize_(workItems);
	 }

	 /***************************************************************************/
	 /**
	  * Allow to perform necessary setup work for an iteration. Derived classes
	  * should make sure this base function is called first when they overload
	  * this function.
	  */
	 virtual void cycleInit_(
		 std::vector<std::shared_ptr<processable_type>>& workItems
	 ) BASE {
		 // No items have so far been submitted in current cycle
		 m_no_items_submitted_in_cycle = true;
		 // Make it known that a new cycle is about to start
		 m_cycle_running = true;
	 }

	 /***************************************************************************/
	 /**
	  * Non-virtual wrapper for cycleInit_()
	  */
	 void cycleInit(
		 std::vector<std::shared_ptr<processable_type>>& workItems
	 ) {
		 this->cycleInit_(workItems);
	 }

	 /***************************************************************************/
	 /**
	  * Allows to perform necessary cleanup work for an iteration. Derived classes
	  * should make sure this base function is called last when they overload
	  * this function.
	  */
	 virtual void cycleFinalize_(
		 std::vector<std::shared_ptr<processable_type>>& workItems
	 ) BASE {
		 // Mark the end of processing in this cycle
		 m_cycle_end_time = this->now();
		 // Make it known that the cycle has ended
		 m_cycle_running = false;
	 }

	 /***************************************************************************/
	 /**
	  * Non-virtual wrapper for cycleFinalize_()
	  */
	 void cycleFinalize(
		 std::vector<std::shared_ptr<processable_type>>& workItems
	 ) {
		 this->cycleFinalize_(workItems);
	 }

	 /***************************************************************************/
	 /**
	  * Submission of all work items in the list
	  */
	 std::size_t submitAllWorkItems(std::vector<std::shared_ptr<processable_type>>& workItems) {
		 // Submit work items
		 COLLECTION_POSITION_TYPE pos_cnt = 0;
		 std::size_t nSubmittedItems = 0;
		 bool got_first_processable_item_id = false;
		 for(auto w_ptr: workItems) { // std::shared_ptr may be copied
#ifdef DEBUG
			 if(not w_ptr) {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG, time_and_place)
						 << "In GBaseExecutorT<processable_type>::submitAllWorkItems():" << std::endl
						 << "Received empty work item in position "  << pos_cnt << std::endl
						 << "m_iteration_counter = " << m_iteration_counter << std::endl
				 );
			 }
#endif

			 // Is the item due to be submitted ?
			 processingStatus ps = w_ptr->getProcessingStatus();
			 if(processingStatus::DO_PROCESS == ps) {
				 // Update some internal variables
				 w_ptr->setIterationCounter(m_iteration_counter);
				 w_ptr->setCollectionPosition(pos_cnt);
				 w_ptr->setResubmissionCounter(m_nResubmissions);

				 // Do the actual submission
				 this->submit(w_ptr);

				 // Assign the id of the first processable item in this iteration. This is
				 // so we can identify the first individual in the first iteration.
				 if(not got_first_processable_item_id) {
					 got_first_processable_item_id = true;
					 m_iteration_first_individual_position = pos_cnt;
				 }

				 // Make sure we use the same timings in the following
				 auto current_time = this->now();

				 // Determine the time of the very first submission
				 if(m_no_items_submitted_in_object) {
					 m_no_items_submitted_in_object = false;
					 m_object_first_submission_time = current_time;
				 }

				 if(m_no_items_submitted_in_iteration) {
					 m_no_items_submitted_in_iteration = false;
					 m_iteration_first_submission_time = current_time;
				 }

				 // Determine the time of the first submission in the current iteration
				 if(m_no_items_submitted_in_cycle) {
					 m_no_items_submitted_in_cycle = false;
					 m_cycle_first_submission_time = current_time;
				 }

				 // Update the submission counter
				 nSubmittedItems++;
			 } else if(processingStatus::DO_IGNORE != ps && processingStatus::PROCESSED != ps) {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG, time_and_place)
						 << "In GBaseExecutorT<processable_type>::submitAllWorkItems(): Error" << std::endl
						 << "processing status is neither DO_PROCESS nor DO_IGNORE. We got " << ps << std::endl
				 );
			 }

			 pos_cnt++;
		 }

		 // Set the start time of the new cycle. How this time is determined depends
		 // on the actual executor. NOTE that the following call may block, if a start time cannot
		 // yet be determined.
		 if(this->inFirstIteration() && this->inFirstCycle()) {
			 m_approx_cycle_start_time = this->determineInitialCycleStartTime();
		 } else {
			 m_approx_cycle_start_time = m_cycle_first_submission_time;
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
		 return m_iteration_counter;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the expected number of work items in the current iteration
	  */
	 std::size_t getExpectedNumber() const noexcept {
		 return m_expectedNumber;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the time when an iteration has started
	  */
	 std::chrono::high_resolution_clock::time_point getIterationStartTime() const {
		 return this->m_iteration_first_submission_time;
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
	 virtual void submit(
		 std::shared_ptr<processable_type>
	 ) BASE = 0;

	 /***************************************************************************/
	 /**
	  * Waits for work items to return and checks for completeness
	  * (i.e. all items have returned and there were no exceptions).
	  *
	  * @param workItems A vector with work items to be evaluated beyond the broker
	  * @param oldWorkItems A vector with work items that have returned after the threshold
	  * @return A struct of booleans indicating whether all items were processed successfully and whether there were errors
	  */
	 virtual executor_status_t waitForReturn(
		 std::vector<std::shared_ptr<processable_type>>& workItems
		 , std::vector<std::shared_ptr<processable_type>>&
	 ) BASE = 0;

	 /***************************************************************************/
	 /**
	  * Count the number of work items in a batch with a specific flag
	  */
	 std::size_t countItemsWithStatus(
		 const std::vector<std::shared_ptr<processable_type>>& workItems
		 , const processingStatus& ps
	 ) {
		 return boost::numeric_cast<std::size_t>(
			 std::count_if(
				 workItems.begin()
				 , workItems.end()
				 , [ps](std::shared_ptr<processable_type> p) {
					 if(ps == p->getProcessingStatus()) {
						 return true;
					 }
					 return false;
				 }
			 )
		 );
	 }

	 /***************************************************************************/
	 /**
	  * Checks for the status with respect to errors and processing status
	  * of a collecion of work items
	  */
	 executor_status_t checkExecutionState(
		 const std::vector<std::shared_ptr<processable_type>>& workItems
	 ) {
		 bool is_complete = true;
		 bool has_errors = false;

		 // First check that there are no remaining items with the DO_PROCESS status
		 auto nUnprocessed = this->countItemsWithStatus(
			 workItems
			 , processingStatus::DO_PROCESS
		 );
		 if(nUnprocessed > 0) is_complete = false;

		 // Now check for error states
		 auto nErrorState = this->countItemsWithStatus(
			 workItems
			 , processingStatus::ERROR_FLAGGED
		 );
		 nErrorState     += this->countItemsWithStatus(
			 workItems
			 , processingStatus::EXCEPTION_CAUGHT
		 );
		 if(nErrorState > 0) has_errors = true;

		 return executor_status_t{is_complete, has_errors};
	 }

	 /***************************************************************************/
	 /**
	  * Removes work items without a given flag from the vector and return
	  * the number of remaining items
	  */
	 std::size_t cleanItemsWithoutFlag(
		 std::vector<std::shared_ptr<processable_type>>& items_vec
		 , const processingStatus& desired_ps
	 ) {
		 // Remove unprocessed items from the list of old work items
		 Gem::Common::erase_if(
			 items_vec
			 , [this, desired_ps](std::shared_ptr<processable_type> item_ptr) {
				 auto ps = item_ptr->getProcessingStatus();
				 if(ps != desired_ps) {
#ifdef DEBUG
					 // Some logging, as this condition should be very rare and might indicate a more general problem.
					 glogger
						 << "In GBaseExeuctorT<processable_type>::cleanItemsWithoutFlag():" << std::endl
						 << "Removing work item in submission " << this->get_iteration_counter() << std::endl
						 << "because it does not have the desired status " << desired_ps << std::endl
						 << "Found status " << ps << " instead." << std::endl
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
		 return items_vec.size();
	 }

	 /***************************************************************************/
	 /**
	  * Checks if any work items have already been submitted in the object
	  */
	 bool checkItemsSubmittedInObject() const noexcept {
		 return not m_no_items_submitted_in_object;
	 }

	 /***************************************************************************/
	 /**
	  * Checks if any work items have already been submitted in the current iteration
	  */
	 bool checkItemsSubmittedInCycle() const noexcept {
		 return not m_no_items_submitted_in_cycle;
	 }

	 /***************************************************************************/
	 /**
	  * Checks if this is the first iteration
	  */
	 bool inFirstIteration() const noexcept {
		 return m_in_first_iteration;
	 }

	 /***************************************************************************/
	 /**
	  * Checks if this is the first cycle of an iteration
	  */
	 bool inFirstCycle() const noexcept {
		 return (0==m_nResubmissions);
	 }

private:
	 /***************************************************************************/
	 /** @brief Determination of the time when execution of the initial cycle has started */
	 virtual std::chrono::high_resolution_clock::time_point determineInitialCycleStartTime() const BASE = 0;

	 /***************************************************************************/
	 /** @brief Graphical progress feedback */
	 virtual void visualize_performance() BASE = 0;

	 /***************************************************************************/
	 /**
	  * Internal function to set the iteration counter. The function will throw
	  * if the assigned iteration is < the internally determined iteration
	  */
	 void set_external_iteration_counter(const ITERATION_COUNTER_TYPE& external_iteration_counter) {
		 if(external_iteration_counter < m_iteration_counter){
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GBaseExeuctorT<processable_type>::set_external_iteration_counter():" << std::endl
					 << "Tried to set external iteration counter to value " << external_iteration_counter << " ," << std::endl
					 << "while internal counter is at " << m_iteration_counter << " ." << std::endl
					 << "The internal counter needs to be <= the external counter when being set" << std::endl
			 );
		 }

		 m_iteration_counter = external_iteration_counter;
	 }

	 /***************************************************************************/
	 // Data

	 /* @brief Counts the number of submissions initiated for this object; may also be set by the user */
	 ITERATION_COUNTER_TYPE m_iteration_counter = ITERATION_COUNTER_TYPE(0);

	 std::size_t m_expectedNumber = 0; ///< The number of work items to be submitted (and expected back)

	 /** brief The timepoint of the very first submission (or possibly retrieval from the queue */
	 std::chrono::high_resolution_clock::time_point m_object_first_submission_time;
	 /** @brief Temporary that holds the start time for the retrieval of items in a given iteration */
	 std::chrono::high_resolution_clock::time_point m_iteration_first_submission_time;
	 /** @brief Temporary that holds the start time for the retrieval of items in a given iteration */
	 std::chrono::high_resolution_clock::time_point m_iteration_end_time;
	 /** @brief Indicates whether an iteration is currently being processed */
	 std::atomic<bool> m_iteration_running{true};
	 /** @brief Temporary that holds the start time for the retrieval of items in a given cycle */
	 std::chrono::high_resolution_clock::time_point m_cycle_first_submission_time;
	 /** @brief The approximate time of the start of processing in an iteration */
	 std::chrono::high_resolution_clock::time_point m_approx_cycle_start_time;
	 /** @brief Temporary that holds the start time for the retrieval of items in a given cycle */
	 std::chrono::high_resolution_clock::time_point m_cycle_end_time;
	 /** @brief Indicates whether a cycle is currently being processed */
	 std::atomic<bool> m_cycle_running{true};

	 std::atomic<bool> m_no_items_submitted_in_object{true}; ///< Indicates whether items have already been submitted
	 std::atomic<bool> m_no_items_submitted_in_iteration{true}; ///< Indicates whether items have already been submitted in an iteration
	 std::atomic<bool> m_no_items_submitted_in_cycle{true}; ///< Indicates whether items have already been submitted in a cycle
	 std::atomic<bool> m_in_first_iteration{true}; ///< Allows to check whether we are in the first iteration (will be set to false in cycleFinalize_()

	 std::size_t m_iteration_first_individual_position = 0; ///< The position of the first item to be processed in the workItems vector

	 /** @brief The maximum number of re-submissions allowed if a full return of submitted items is attempted */
	 std::size_t m_maxResubmissions = DEFAULTMAXRESUBMISSIONS;
	 std::size_t m_nResubmissions = 0; ///< A temporary counter of the current resubmission

	 std::size_t m_n_returnedLast = 0; ///< The number of individuals returned in the last iteration cycle
	 std::size_t m_n_notReturnedLast = 0; ///< The number of individuals NOT returned in the last iteration cycle
	 std::size_t m_n_oldWorkItems = 0; ///< The number of old work items returned in a given iteration
	 std::size_t m_n_erroneousItems = 0; ///< The number of work items with errors in the current iteration

	 std::vector<std::shared_ptr<processable_type>> m_old_work_items_vec; ///< Temporarily holds old work items of the current iteration

	 std::mutex m_concurrent_workon_mutex; ///< Makes sure the workOn function is only called once at the same time on this object
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class executes work items serially. It is mostly meant for debugging
 * purposes
 */
template<typename processable_type>
class GSerialExecutorT
	: public GBaseExecutorT<processable_type>
{
	 ///////////////////////////////////////////////////////////////////////

	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar & make_nvp("GBaseExecutorT", boost::serialization::base_object<GBaseExecutorT<processable_type>>(*this));
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
		 : GBaseExecutorT<processable_type>(cp)
	 { /* nothing */ }

	 /***************************************************************************/
	 /** @brief The destructor */
	 virtual ~GSerialExecutorT() = default;

	 /***************************************************************************/
	 /**
	  * Returns the name of this class
	  */
	 std::string name() const override {
		 return std::string("GSerialExecutorT<processable_type>");
	 }

	 /***************************************************************************/
	 /**
	  * Adds local configuration options to a GParserBuilder object
	  *
	  * @param gpb The GParserBuilder object to which configuration options should be added
	  */
	 void addConfigurationOptions(
		 Gem::Common::GParserBuilder &gpb
	 ) override {
		 // Call our parent class's function
		 GBaseExecutorT<processable_type>::addConfigurationOptions(gpb);

		 // No local data
	 }

	 /***************************************************************************/
	 /**
	  * Searches for compliance with expectations with respect to another object
	  * of the same type
	  */
	 void compare(
		 const GBaseExecutorT<processable_type>& cp // the other object
		 , const Gem::Common::expectation& e // the expectation for this object, e.g. equality
		 , const double& limit // the limit for allowed deviations of floating point types
	 ) const override {
		 using namespace Gem::Common;

		 // Check that we are dealing with a GSerialExecutorT reference independent of this object and convert the pointer
		 const GSerialExecutorT<processable_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		 GToken token("GSerialExecutorT<processable_type>", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GBaseExecutorT<processable_type>>(IDENTITY(*this, *p_load), token);

		 // ... no local data

		 // React on deviations from the expectation
		 token.evaluate();
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Loads the data of another GSerialExecutorT object
	  *
	  * @param cp A constant pointer to another GSerialExecutorT object
	  */
	 void load_(const GBaseExecutorT<processable_type> *cp) override {
		 const auto p_load_ptr = dynamic_cast<GSerialExecutorT<processable_type> const *const>(cp);

		 if (not cp) { // nullptr
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GSerialExecutorT<processable_type>::load_(): Conversion error!" << std::endl
			 );
		 }

		 // Load our parent classes data
		 GBaseExecutorT<processable_type>::load_(p_load_ptr);
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
	 void submit(
		 std::shared_ptr<processable_type> w_ptr
	 ) override {
		 try {
			 // process may throw ...
			 w_ptr->process();
		 } catch(const g_processing_exception& e) {
			 // This is an expected exception if processing has failed. We do nothing,
			 // it is up to the caller to decide what to do with processing errors, and
			 // these are also stored in the processing item. We do try to create a sort
			 // of stack trace by emitting a warning, though. Processing errors should be rare,
			 // so might hint at some problem.
			 glogger
				 << "In GSerialExecutorT<processable_type>::submit():" << std::endl
				 << "Caught a g_processing_exception exception while processing the work item" << std::endl
				 << "with the error message" << std::endl
				 << e.what() << std::endl
				 << "Exception information should have been stored in the" << std::endl
				 << "work item itself. Processing should have been marked as" << std::endl
				 << "unsuccessful in the work item. We leave it to the" << std::endl
				 << "submitter to deal with this." << std::endl
				 << GWARNING;
		 } catch(const std::exception& e) {
			 // All exceptions should be caught inside of the process() call. It is a
			 // severe error if we nevertheless catch an error here. We throw a corresponding
			 // gemfony exception.
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GSerialExecutorT<processable_type>::submit(): Caught a" << std::endl
					 << "std::exception in a place where we didn't expect any exceptions." << std::endl
					 << "Got message" << std::endl
					 << e.what() << std::endl
			 );
		 } catch(...) {
			 // All exceptions should be caught inside of the process() call. It is a
			 // severe error if we nevertheless catch an error here. We throw a corresponding
			 // gemfony exception.
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GSerialExecutorT<processable_type>::submit(): Caught an" << std::endl
					 << "unknown exception in a place where we didn't expect any exceptions" << std::endl
			 );
		 }
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
		 std::vector<std::shared_ptr<processable_type>>& workItems
		 , std::vector<std::shared_ptr<processable_type>>& oldWorkItems
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
	 void cycleInit_(
		 std::vector<std::shared_ptr<processable_type>>& workItems
	 ) override {
		 // Make sure the parent classes cycleInit_ function is executed first
		 // This function will also update the iteration start time
		 GBaseExecutorT<processable_type>::cycleInit_(workItems);
	 }

	 /***************************************************************************/
	 /**
	  * Allows to perform necessary cleanup work for a cycle or do calculations
	  * for the next cycle
	  */
	 void cycleFinalize_(
		 std::vector<std::shared_ptr<processable_type>>& workItems
	 ) override {
		 // Make sure the parent classes cycleFinalize_ function is executed last
		 GBaseExecutorT<processable_type>::cycleFinalize_(workItems);
	 }

	 /***************************************************************************/
	 /**
	  * Allow to perform necessary setup work for an iteration.
	  */
	 void iterationInit_(
		 std::vector<std::shared_ptr<processable_type>>& workItems
	 ) override {
		 // Make sure the parent classes iterationInit_ function is executed first
		 GBaseExecutorT<processable_type>::iterationInit_(workItems);
	 }

	 /***************************************************************************/
	 /**
	  * Allows to perform necessary cleanup work for an iteration or do calculations
	  * for the next iteration
	  */
	 void iterationFinalize_(
		 std::vector<std::shared_ptr<processable_type>>& workItems
	 ) override {
		 // Make sure the parent classes iterationFinalize_ function is executed last
		 GBaseExecutorT<processable_type>::iterationFinalize_(workItems);
	 }

private:
	 /***************************************************************************/
	 /**
	  * Creates a deep clone of this object.
	  */
	 GBaseExecutorT<processable_type>* clone_() const override {
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
	 void visualize_performance() override { /* nothing */ }

	 /***************************************************************************/
	 // No local data
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class executes a collection of work items in multiple threads
 */
template<typename processable_type>
class GMTExecutorT
	: public GBaseExecutorT<processable_type>
{
	 ///////////////////////////////////////////////////////////////////////

	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GBaseExecutorT", boost::serialization::base_object<GBaseExecutorT<processable_type>>(*this))
		 & BOOST_SERIALIZATION_NVP(m_n_threads);
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
		 , m_n_threads(nThreads > 0 ? nThreads : Gem::Courtier::DEFAULTNSTDTHREADS)
	 {
	 	 if(0 == nThreads) {
			 glogger
				 << "In GMTExecutorT::GMTExecutorT(std::uint16_t nThreads):" << std::endl
				 << "User requested nThreads == 0. nThreads was reset to the default "
				 << Gem::Courtier::DEFAULTNSTDTHREADS << std::endl
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
		 , m_n_threads(cp.m_n_threads)
	 { /* nothing */ }

	 /***************************************************************************/
	 /** @brief The destructor */
	 virtual ~GMTExecutorT() = default;

	 /***************************************************************************/
	 /**
	  * Returns the name of this class
	  */
	 std::string name() const override {
		 return std::string("GMTExecutorT<processable_type>");
	 }

	 /***************************************************************************/
	 /**
	  * Adds local configuration options to a GParserBuilder object
	  *
	  * @param gpb The GParserBuilder object to which configuration options should be added
	  */
	 void addConfigurationOptions(
		 Gem::Common::GParserBuilder &gpb
	 ) override {
		 // Call our parent class's function ...
		 GBaseExecutorT<processable_type>::addConfigurationOptions(gpb);

		 // ... then add local dara
		 gpb.registerFileParameter<std::uint16_t>(
			 "nProcessingThreads" // The name of the variable in the configuration file
			 , Gem::Courtier::DEFAULTNSTDTHREADS // The default value
			 , [this](std::uint16_t nt) { this->setNThreads(nt); }
		 )
			 << "The number of threads used to simultaneously process work items" << std::endl
			 << "0 means \"automatic\"";
	 }

	 /***************************************************************************/
	 /**
	  * Sets the number of threads for the thread pool. If nThreads is set
	  * to 0, a warning will be printed and the number of threads will be set to
	  * a default value.
	  *
	  * @param nThreads The number of threads the threadpool should use
	  */
	 void setNThreads(std::uint16_t nThreads) {
		 if (nThreads == 0) {
			 m_n_threads = Gem::Courtier::DEFAULTNSTDTHREADS;

			 glogger
				 << "In GMTExecutorT::setNThreads(std::uint16_t nThreads):" << std::endl
				 << "User requested nThreads == 0. nThreads was reset to the default "
				 << Gem::Courtier::DEFAULTNSTDTHREADS << std::endl
				 << GWARNING;
		 }
		 else {
			 m_n_threads = nThreads;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the number of threads this population uses.
	  *
	  * @return The maximum number of allowed threads
	  */
	 std::uint16_t getNThreads() const {
		 return m_n_threads;
	 }

	 /***************************************************************************/
	 /**
	  * Searches for compliance with expectations with respect to another object
	  * of the same type
	  */
	 void compare(
		 const GBaseExecutorT<processable_type>& cp // the other object
		 , const Gem::Common::expectation& e // the expectation for this object, e.g. equality
		 , const double& limit // the limit for allowed deviations of floating point types
	 ) const override {
		 using namespace Gem::Common;

		 // Check that we are dealing with a GSerialExecutorT reference independent of this object and convert the pointer
		 const GMTExecutorT<processable_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		 GToken token("GMTExecutorT<processable_type>", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GBaseExecutorT<processable_type>>(IDENTITY(*this, *p_load), token);

		 // ... and then our local data
		 compare_t(IDENTITY(m_n_threads, p_load->m_n_threads), token);

		 // React on deviations from the expectation
		 token.evaluate();
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

		 if (not cp) { // nullptr
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GMTExecutorT<processable_type>::load_(): Conversion error!" << std::endl
			 );
		 }

		 // Load our parent classes data
		 GBaseExecutorT<processable_type>::load_(cp);

		 // Load our local data
		 m_n_threads = p_load_ptr->m_n_threads;
	 }

	 /***************************************************************************/
	 /**
	  * General initialization function to be called prior to the first submission
	  */
	 void init_() override {
		 // GBaseExecutorT<processable_type> sees exactly the environment it would when called from its own class
		 GBaseExecutorT<processable_type>::init_();

		 // Cross-check
		 assert(m_n_threads > 0);

		 // Initialize our thread pool
		 m_gtp_ptr.reset(new Gem::Common::GThreadPool(m_n_threads));
	 }

	 /***************************************************************************/
	 /**
	  * General finalization function to be called after the last submission
	  */
	 void finalize_() override {
		 // Terminate our thread pool
		 m_gtp_ptr.reset();

		 // GBaseExecutorT<processable_type> sees exactly the environment it would when called from its own class
		 GBaseExecutorT<processable_type>::finalize_();
	 }

	 /***************************************************************************/
	 /**
	  * Allow to perform necessary setup work for an iteration.
	  */
	 void cycleInit_(
		 std::vector<std::shared_ptr<processable_type>>& workItems
	 ) override {
		 // Make sure the parent classes cycleInit function is executed first
		 // This function will also update the iteration start time
		 GBaseExecutorT<processable_type>::cycleInit_(workItems);

		 // We want an empty futures vector for a new submission cycle,
		 // so we do not deal with old errors.
		 m_future_vec.clear();
	 }

	 /***************************************************************************/
	 /**
	  * Allows to perform necessary cleanup work for an iteration or do calculations
	  * for the next iteration.
	  */
	 void cycleFinalize_(
		 std::vector<std::shared_ptr<processable_type>>& workItems
	 ) override {
		 // Make sure the parent classes cycleFinalize_ function is executed last
		 GBaseExecutorT<processable_type>::cycleFinalize_(workItems);
	 }

	 /***************************************************************************/
	 /**
	  * Allow to perform necessary setup work for an iteration.
	  */
	 void iterationInit_(
		 std::vector<std::shared_ptr<processable_type>>& workItems
	 ) override {
		 // Make sure the parent classes iterationInit_ function is executed first
		 GBaseExecutorT<processable_type>::iterationInit_(workItems);
	 }

	 /***************************************************************************/
	 /**
	  * Allows to perform necessary cleanup work for an iteration or do calculations
	  * for the next iteration
	  */
	 void iterationFinalize_(
		 std::vector<std::shared_ptr<processable_type>>& workItems
	 ) override {
		 // Make sure the parent classes iterationFinalize_ function is executed last
		 GBaseExecutorT<processable_type>::iterationFinalize_(workItems);
	 }

	 /***************************************************************************/
	 /**
	  * Submits a single work item. As we are dealing with multi-threaded
	  * execution, we simply let a thread pool executed a lambda function
	  * which takes care of the processing.
	  *
	  * @param w_ptr The work item to be processed
	  */
	 void submit(
		 std::shared_ptr<processable_type> w_ptr
	 ) override {
		 using result_type = typename processable_type::result_type;

		 if (m_gtp_ptr && w_ptr) { // Do we have a valid thread pool and a valid work item ?
			 // async_schedule emits a future, which is std::moved into the std::vector
			 m_future_vec.push_back(
				 m_gtp_ptr->async_schedule( [w_ptr](){ return w_ptr->process(); })
			 );
		 } else {
			 if (not m_gtp_ptr) {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG, time_and_place)
						 << "In In GMTExecutorT<processable_type>::submit(): Error!" << std::endl
						 << "Threadpool pointer is empty" << std::endl
				 );
			 } else if(not w_ptr) {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG, time_and_place)
						 << "In In GMTExecutorT<processable_type>::submit(): Error!" << std::endl
						 << "work item pointer is empty" << std::endl
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
		 std::vector<std::shared_ptr<processable_type>>& workItems
		 , std::vector<std::shared_ptr<processable_type>>& oldWorkItems
	 ) override {
		 using result_type = typename processable_type::result_type;

		 // Note: Old work items are cleared in the "workOn" function

		 // Find out about the errors that were found
		 for(auto &f: m_future_vec) {
			 // Retrieve the future and check for errors
			 try {
				 result_type r = f.get();
			 } catch(const g_processing_exception& e) {
#ifdef DEBUG
				 // This is an expected exception if processing has failed. We do nothing,
				 // it is up to the caller to decide what to do with processing errors, and
				 // these are also stored in the processing item. We do try to create a sort
				 // of stack trace by emitting a warning, though. Processing errors should be rare,
				 // so might hint at some problem.
				 glogger
					 << "In GMTExecutorT<processable_type>::waitForReturn():" << std::endl
					 << "Caught a g_processing_exception exception while retrieving a future" << std::endl
					 << "with the error message" << std::endl
					 << e.what() << std::endl
					 << "Exception information should have been stored in the" << std::endl
					 << "work item itself. Processing should have been marked as" << std::endl
					 << "unsuccessful in the work item. We leave it to the" << std::endl
					 << "caller to deal with this." << std::endl
					 << GWARNING;
#endif
			 } catch(const std::exception& e) {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG, time_and_place)
						 << "In GMTExecutorT<processable_type>::waitForReturn(): Caught an" << std::endl
						 << "caught std::exception in a place where we didn't expect any exceptions" << std::endl
						 << "Got error message:" << std::endl
						 << e.what() << std::endl
				 );
			 } catch(...) {
				 // All exceptions should be caught inside of the process() call (i.e. emanate
				 // from future.get() . It is a severe error if we nevertheless catch an error here.
				 // We throw a corresponding gemfony exception.
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG, time_and_place)
						 << "In GMTExecutorT<processable_type>::waitForReturn(): Caught an" << std::endl
						 << "unknown exception in a place where we didn't expect any exceptions" << std::endl
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
	  * Creates a deep clone of this object.
	  */
	 GBaseExecutorT<processable_type>* clone_() const override {
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
	 void visualize_performance() override { /* nothing */ }

	 /***************************************************************************/
	 // Data

	 std::uint16_t m_n_threads = Gem::Courtier::DEFAULTNSTDTHREADS; ///< The number of threads
	 std::shared_ptr<Gem::Common::GThreadPool> m_gtp_ptr; ///< Temporarily holds a thread pool

	 std::vector<std::future<typename processable_type::result_type>> m_future_vec; ///< Temporarily hold futures stored during the submit call
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class relays execution of work items to a broker, to which several
 * different consumers may be connected.
 */
template<typename processable_type>
class GBrokerExecutorT :
	public GBaseExecutorT<processable_type>
{
	 ///////////////////////////////////////////////////////////////////////

	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GBaseExecutorT", boost::serialization::base_object<GBaseExecutorT<processable_type>>(*this))
		 & BOOST_SERIALIZATION_NVP(m_waitFactor)
		 & BOOST_SERIALIZATION_NVP(m_minPartialReturnPercentage)
		 & BOOST_SERIALIZATION_NVP(m_capable_of_full_return)
		 & BOOST_SERIALIZATION_NVP(m_gpd)
		 & BOOST_SERIALIZATION_NVP(m_waiting_times_graph)
		 & BOOST_SERIALIZATION_NVP(m_returned_items_graph)
		 & BOOST_SERIALIZATION_NVP(m_waitFactorWarningEmitted);
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
		 , m_gpd("Maximum waiting times and returned items", 1, 2)
	 {
		 m_gpd.setCanvasDimensions(std::make_tuple<std::uint32_t,std::uint32_t>(1200,1600));

		 m_waiting_times_graph = std::shared_ptr<Gem::Common::GGraph2D>(new Gem::Common::GGraph2D());
		 m_waiting_times_graph->setXAxisLabel("Iteration");
		 m_waiting_times_graph->setYAxisLabel("Maximum waiting time [s]");
		 m_waiting_times_graph->setPlotMode(Gem::Common::graphPlotMode::CURVE);

		 m_returned_items_graph = std::shared_ptr<Gem::Common::GGraph2D>(new Gem::Common::GGraph2D());
		 m_returned_items_graph->setXAxisLabel("Iteration");
		 m_returned_items_graph->setYAxisLabel("Number of returned items");
		 m_returned_items_graph->setPlotMode(Gem::Common::graphPlotMode::CURVE);
	 }

	 /***************************************************************************/
	 /**
	  * The copy constructor
	  *
	  * @param cp A copy of another GBrokerConnector object
	  */
	 GBrokerExecutorT(const GBrokerExecutorT<processable_type> &cp)
		 : GBaseExecutorT<processable_type>(cp)
		 , m_waitFactor(cp.m_waitFactor)
		 , m_minPartialReturnPercentage(cp.m_minPartialReturnPercentage)
		 , m_capable_of_full_return(cp.m_capable_of_full_return)
		 , m_gpd("Maximum waiting times and returned items", 1, 2) // Intentionally not copied
		 , m_waitFactorWarningEmitted(cp.m_waitFactorWarningEmitted)
	 {
		 m_gpd.setCanvasDimensions(std::make_tuple<std::uint32_t,std::uint32_t>(1200,1600));

		 m_waiting_times_graph = std::shared_ptr<Gem::Common::GGraph2D>(new Gem::Common::GGraph2D());
		 m_waiting_times_graph->setXAxisLabel("Iteration");
		 m_waiting_times_graph->setYAxisLabel("Maximum waiting time [s]");
		 m_waiting_times_graph->setPlotMode(Gem::Common::graphPlotMode::CURVE);

		 m_returned_items_graph = std::shared_ptr<Gem::Common::GGraph2D>(new Gem::Common::GGraph2D());
		 m_returned_items_graph->setXAxisLabel("Iteration");
		 m_returned_items_graph->setYAxisLabel("Number of returned items");
		 m_returned_items_graph->setPlotMode(Gem::Common::graphPlotMode::CURVE);
	 }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GBrokerExecutorT()
	 {
		 // Register the plotter
		 m_gpd.registerPlotter(m_waiting_times_graph);
		 m_gpd.registerPlotter(m_returned_items_graph);

		 // Write out the result. This is a hack.
		 if(m_waiting_times_graph->currentSize() > 0.) {
			 m_gpd.writeToFile("maximumWaitingTimes.C");
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Returns the name of this class
	  */
	 std::string name() const override {
		 return std::string("GBrokerExecutorT<processable_type>");
	 }

	 /***************************************************************************/
	 /**
	  * Adds local configuration options to a GParserBuilder object
	  *
	  * @param gpb The GParserBuilder object to which configuration options should be added
	  */
	 void addConfigurationOptions(
		 Gem::Common::GParserBuilder &gpb
	 ) override {
		 // Call our parent class's function
		 GBaseExecutorT<processable_type>::addConfigurationOptions(gpb);

		 // Add local data

		 gpb.registerFileParameter<double>(
			 "waitFactor" // The name of the variable
			 , DEFAULTBROKERWAITFACTOR2 // The default value
			 , [this](double w) {
				 this->setWaitFactor(w);
			 }
		 )
			 << "A static double factor for timeouts" << std::endl
			 << "A wait factor <= 0 means \"no timeout\"." << std::endl
			 << "It is suggested to use values >= 1.";

		 gpb.registerFileParameter<std::uint16_t>(
			 "minPartialReturnPercentage" // The name of the variable
			 , DEFAULTEXECUTORPARTIALRETURNPERCENTAGE // The default value
			 , [this](std::uint16_t percentage) {
				 this->setMinPartialReturnPercentage(percentage);
			 }
		 )
			 << "Set to a value < 100 to allow execution to continue when" << std::endl
			 << "minPartialReturnPercentage percent of the expected work items"  << std::endl
			 << "have returned. Set to 0 to disable this option.";
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the wait factor to be applied to timeouts. A wait factor
	  * <= 0 indicates an indefinite waiting time.
	  */
	 void setWaitFactor(double waitFactor) {
		 m_waitFactor = waitFactor;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the wait factor variable
	  */
	 double getWaitFactor() const {
		 return m_waitFactor;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the percentage of items that must have returned
	  * before execution continues. 0 means: Option is disabled.
	  */
	 std::uint16_t getMinPartialReturnPercentage() const noexcept {
		 return m_minPartialReturnPercentage;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the percentage of items that must have returned
	  * before execution continues. Set to 0 to disable this option.
	  */
	 void setMinPartialReturnPercentage(std::uint16_t minPartialReturnPercentage) {
		 Gem::Common::checkRangeCompliance<std::uint16_t>(
			 minPartialReturnPercentage
			 , 0, 100
			 , "GBrokerExecutorT<>::setMinPartialReturnPercentage()"
		 );
		 m_minPartialReturnPercentage = minPartialReturnPercentage;
	 }

	 /***************************************************************************/
	 /**
	  * Searches for compliance with expectations with respect to another object
	  * of the same type
	  */
	 void compare(
		 const GBaseExecutorT<processable_type>& cp // the other object
		 , const Gem::Common::expectation& e // the expectation for this object, e.g. equality
		 , const double& limit // the limit for allowed deviations of floating point types
	 ) const override {
		 using namespace Gem::Common;

		 // Check that we are dealing with a GSerialExecutorT reference independent of this object and convert the pointer
		 const GBrokerExecutorT<processable_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		 GToken token("GBrokerExecutorT<processable_type>", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GBaseExecutorT<processable_type>>(IDENTITY(*this, *p_load), token);

		 // ... and then our local data
		 compare_t(IDENTITY(m_waitFactor, p_load->m_waitFactor), token);
		 compare_t(IDENTITY(m_minPartialReturnPercentage, p_load->m_minPartialReturnPercentage), token);
		 compare_t(IDENTITY(m_capable_of_full_return, p_load->m_capable_of_full_return), token);
		 compare_t(IDENTITY(m_waitFactorWarningEmitted, p_load->m_waitFactorWarningEmitted), token);

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
	 void load_(const GBaseExecutorT<processable_type> * cp) override {
		 const auto p_load_ptr = dynamic_cast<const GBrokerExecutorT<processable_type> *>(cp);

		 if (not p_load_ptr) { // nullptr
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GBrokerExecutorT<processable_type>::load(): Conversion error!" << std::endl
			 );
		 }

		 // Load our parent classes data
		 GBaseExecutorT<processable_type>::load_(cp);

		 // Local data
		 m_waitFactor = p_load_ptr->m_waitFactor;
		 m_minPartialReturnPercentage = p_load_ptr->m_minPartialReturnPercentage;
		 m_capable_of_full_return = p_load_ptr->m_capable_of_full_return;
		 m_waitFactorWarningEmitted = p_load_ptr->m_waitFactorWarningEmitted;
	 }

	 /***************************************************************************/
	 /**
	  * General initialization function to be called prior to the first submission
	  */
	 void init_() override {
		 // To be called prior to all other initialization code
		 GBaseExecutorT<processable_type>::init_();

		 // Make sure we have a valid buffer port
		 if (not m_current_buffer_port_ptr) {
			 m_current_buffer_port_ptr.reset(
				 new Gem::Courtier::GBufferPortT<processable_type>()
			 );
		 }

		 // Add the buffer port to the broker and check whether all consumers
		 // enrolled with the broker are capable of full return
		 m_capable_of_full_return
			 = GBROKER(processable_type)->enrol_buffer_port(m_current_buffer_port_ptr);

#ifdef DEBUG
		 if(m_capable_of_full_return) {
			 glogger
				 << "In GBrokerExecutorT<>::init():" << std::endl
				 << "Assuming that all consumers are capable of full return" << std::endl
				 << GLOGGING;
		 } else {
			 glogger
				 << "In GBrokerExecutorT<>::init():" << std::endl
				 << "At least one consumer is not capable of full return" << std::endl
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
		 m_current_buffer_port_ptr->producer_disconnect();

		 // Get rid of our copy of the buffer port
		 m_current_buffer_port_ptr.reset();

		 // Likely unnecessary cleanup
		 m_capable_of_full_return = false;

		 // To be called after all other finalization code
		 GBaseExecutorT<processable_type>::finalize_();
	 }

	 /***************************************************************************/
	 /**
	  * Allows to perform necessary setup work for a cycle
	  */
	 void cycleInit_(
		 std::vector<std::shared_ptr<processable_type>>& workItems
	 ) override {
		 // Make sure the parent classes cycleInit function is executed first
		 // This function will also update the iteration start time
		 GBaseExecutorT<processable_type>::cycleInit_(workItems);

		 // Reset the number of currently returned items
		 m_nReturnedCurrent = 0;

#ifdef DEBUG
		 // Check that the waitFactor has a suitable size
		 if(not m_waitFactorWarningEmitted) {
			 if(m_waitFactor > 0. && m_waitFactor < 1.) {
				 glogger
					 << "In GBrokerExecutorT::cycleInit_(): Warning" << std::endl
					 << "It is suggested not to use a wait time < 1. Current value: " << m_waitFactor << std::endl
					 << GWARNING;
			 }
			 m_waitFactorWarningEmitted = true;
		 }
#endif
	 }

	 /***************************************************************************/
	 /**
	  * Allows to perform necessary cleanup work for an iteration or do calculations
	  * for the next iteration.
	  */
	 void cycleFinalize_(
		 std::vector<std::shared_ptr<processable_type>>& workItems
	 ) override {
		 // Make sure the parent classes cycleFinalize_ function is executed last
		 GBaseExecutorT<processable_type>::cycleFinalize_(workItems);
	 }

	 /***************************************************************************/
	 /**
	  * Allow to perform necessary setup work for an iteration.
	  */
	 void iterationInit_(
		 std::vector<std::shared_ptr<processable_type>>& workItems
	 ) override {
		 // Make sure the parent classes iterationInit_ function is executed first
		 GBaseExecutorT<processable_type>::iterationInit_(workItems);
	 }

	 /***************************************************************************/
	 /**
	  * Allows to perform necessary cleanup work for an iteration or do calculations
	  * for the next iteration
	  */
	 void iterationFinalize_(
		 std::vector<std::shared_ptr<processable_type>>& workItems
	 ) override {
		 // Make sure the parent classes iterationFinalize_ function is executed last
		 GBaseExecutorT<processable_type>::iterationFinalize_(workItems);
	 }

	 /***************************************************************************/
	 /**
	  * Submits a single work item.
	  *
	  * @param w The work item to be processed
	  */
	 void submit(
		 std::shared_ptr<processable_type> w_ptr
	 ) override {
		 if(not w_ptr) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GBrokerExecutorT::submit(): Errornot " << std::endl
					 << "Work item is empty" << std::endl
			 );
		 }

		 if(not m_current_buffer_port_ptr) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GBrokerExecutorT::submit(): Error!" << std::endl
					 << "Current buffer port is empty when it shouldn't be" << std::endl
			 );
		 }

		 // Store the id of the buffer port in the item
		 w_ptr->setBufferId(m_current_buffer_port_ptr->getUniqueTag());

		 // Perform the actual submission
		 m_current_buffer_port_ptr->push_raw(w_ptr);
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
		 std::vector<std::shared_ptr<processable_type>>& workItems
		 , std::vector<std::shared_ptr<processable_type>>& oldWorkItems
	 ) override {
		 // Act depending on the capabilities of the consumer or user-preferences
		 return
			 (m_capable_of_full_return || m_waitFactor == 0.)?
			 this->waitForFullReturn(workItems, oldWorkItems):
			 this->waitForTimeOut(workItems, oldWorkItems);
	 }

private:
	 /***************************************************************************/
	 /**
	  * Creates a deep clone of this object.
	  */
	 GBaseExecutorT<processable_type>* clone_() const override {
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
		 m_current_buffer_port_ptr->pop_processed(w);

		 return w;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves an item from the broker, waiting up to a given amount of time.
	  * The call will return earlier, if an item could already be retrieved.
	  *
	  * @return The obtained processed work item
	  */
	 std::shared_ptr<processable_type> retrieve(
		 const std::chrono::duration<double> &timeout
	 ) {
		 // Holds the retrieved item, if any
		 std::shared_ptr<processable_type> w_ptr;
		 m_current_buffer_port_ptr->pop_processed(w_ptr, timeout);
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
		 std::vector<std::shared_ptr<processable_type>>& workItems
		 , std::vector<std::shared_ptr<processable_type>>& oldWorkItems
	 ) {
		 std::shared_ptr<processable_type> w_ptr;
		 m_nReturnedCurrent = 0;
		 executor_status_t status;

		 // Note: Old work items are cleared in the "workOn" function

		 // Start to retrieve individuals and sort them into our vectors,
		 // until a halt criterion is reached.
		 do {
			 // Get the next individual. If we didn't receive a valid
			 // item, go to the timeout check
			 if(not (w_ptr = this->getNextItem())) continue;

			 // Try to add the work item to the list and check for completeness
			 status = this->addWorkItemAndCheckCompleteness(
				 w_ptr
				 , workItems
				 , oldWorkItems
			 );

			 // No need to continue if all currently submitted work items have returned
			 if(status.is_complete) break;

			 // For succesfully processed items, update the internal timeout variables,
			 // so we know how much longer this cycle should run
			 if(w_ptr->is_processed()) {
				 this->updateTimeout(w_ptr);
			 }
		 } while(not halt());

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
		 std::vector<std::shared_ptr<processable_type>>& workItems
		 , std::vector<std::shared_ptr<processable_type>>& oldWorkItems
	 ) {
		 executor_status_t status;

		 do {
			 status = this->addWorkItemAndCheckCompleteness(
				 this->retrieve() // Get the next item, waiting indefinitely
				 , workItems
				 , oldWorkItems
			 );

			 // Break the loop if all items (or at least the minimum percentage) were received
			 if(status.is_complete || this->minPartialReturnRateReached()) break;
		 } while(true);

		 // Check for the processing flags and derive the is_complete and has_errors states
		 return this->checkExecutionState(workItems);
	 }

	 /***************************************************************************/
	 /**
	  * Updates the remaining time for this iteration
	  *
	  * @param w_ptr The last returned item
	  */
	 void updateTimeout(
		 std::shared_ptr<processable_type> w_ptr
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
		 if(0==m_nReturnedCurrent) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GBrokerExeuctorT<processable_type>::updateTimeout():" << std::endl
					 << "m_nReturnedCurrent is 0" << std::endl
			 );
		 }
#endif
		 std::chrono::duration<double> avgReturnTime = currentElapsed / boost::numeric_cast<double>(m_nReturnedCurrent);

		 // Retrieve the current maximum processing time
		 std::chrono::duration<double> maxProcessingTime((boost::accumulators::max)(m_acc_max));

		 //-----------------------------------------------
		 // The actual timeout calculation
		 m_maxTimeout = m_waitFactor * (avgReturnTime * this->getExpectedNumber() + maxProcessingTime);

		 //-----------------------------------------------
		 // Let the audience know in DEBUG mode
#if defined(DEBUG) && defined(VERBOSETIMEOUTS)
		 glogger
			 << m_nReturnedCurrent << "\t: Got timeout of " << m_maxTimeout.count() << std::endl
			 << GLOGGING;
#endif

		 //-----------------------------------------------
	 }

	 /***************************************************************************/
	 /**
	  * Checks whether a timeout was encountered.
	  */
	 bool timeout() {
		 std::chrono::duration<double> currentElapsed
			 = this->now() - this->getApproxCycleStartTime();

		 if(currentElapsed >= m_maxTimeout) {
#if defined(DEBUG) && defined(VERBOSETIMEOUTS)
			 glogger
			 << "Leaving after timeout of " << m_maxTimeout.count() << " was reached" << std::endl
			 << GLOGGING;
#endif

			 return true;
		 }
		 else return false;
	 }

	 /***************************************************************************/
	 /**
	  * Checks for halt criteria
	  */
	 bool halt() {
		 // Timeout checks and update of timeout variables
		 if(this->timeout()) return true;

		 // For some algorithms, a partial return rate suffices
		 if(this->minPartialReturnRateReached()) return true;

		 // We want to continue
		 return false;
	 }

	 /***************************************************************************/
	 /**
	  * Checks whether the minimum return rate was reached
	  */
	 bool minPartialReturnRateReached() {
		 // Leave if this check is disabled
		 if(0 == this->getMinPartialReturnPercentage()) return false;

		 // Avoid problems related to floating point accuracy
		 std::size_t expectedNumber = this->getExpectedNumber();
		 if(m_nReturnedCurrent == expectedNumber) return true;

		 // Check if we have reached the minimum percentage
		 double realPercentage = boost::numeric_cast<double>(m_nReturnedCurrent) / boost::numeric_cast<double>(expectedNumber);
		 if(realPercentage >= boost::numeric_cast<double>(this->getMinPartialReturnPercentage())) {
			 return true;
		 } else {
			 return false;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Get the remaining time for this iteration
	  */
	 std::chrono::duration<double> remainingTime() const {
		 // The time that has passed since the start of execution in this cycle
		 std::chrono::duration<double> currentElapsed = this->now() - this->getApproxCycleStartTime();
		 // Calculate the remaining time
		 std::chrono::duration<double> remainingTime = std::chrono::duration<double>(0.);
		 if(m_maxTimeout > currentElapsed) {
			 remainingTime = m_maxTimeout - currentElapsed;
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
				 if (not w_ptr) {
					 throw gemfony_exception(
						 g_error_streamer(
							 DO_LOG
							 , time_and_place
						 )
							 << "In GBrokerExecutorT<processable_type>::getNextItem(): Received empty first individual"
							 << std::endl
					 );
				 }

				 // We do accept unsuccessfully processed items, but emit
				 // a warning, as this should be a very rare case
				 if(w_ptr->is_processed()) {
					 break;
				 } else { // unprocessed or has an error
					 glogger
						 << "In GBrokerExecutorT<>::getNextItem():" << std::endl
						 << "Received \"first\" individual which is either" << std::endl
						 << "unprocessed or has errors. Got processing status of " << w_ptr->getProcessingStatus() << std::endl
						 << "but expected " << processingStatus::PROCESSED << " ." << std::endl
						 << "The item will be discarded. As this should be a rare occurance," << std::endl
						 << "we do emit a warning here." << std::endl
						 << GWARNING;
				 }
			 } while(true);
		 } else { // not the first individual in the first iteration
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
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG, time_and_place)
						 << "In GBrokerExeuctorT<processable_type>::getNextItem():" << std::endl
						 << "Retrieval from the raw queue seems to have happened after" << std::endl
						 << "the submission to the processed queue." << std::endl
				 );
			 }
#endif

			 // Calculate the processing time and update the accumulator
			 std::chrono::duration<double> currentProcessingTime = w_ptr->getProcSubmissionTime() - w_ptr->getRawRetrievalTime();
			 m_acc_max(currentProcessingTime.count());
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
		 if(m_first_retrieval) {
#ifdef DEBUG
			 if((not this->inFirstIteration() || not this->inFirstCycle() || 0<m_nReturnedCurrent)) {
				 throw gemfony_exception(
					 g_error_streamer(
						 DO_LOG
						 , time_and_place
					 )
						 << "In GBrokerExecutorT<processable_type>::firstRetrieval():" << std::endl
						 << "Got true==m_first_retrieval, while one of the preconditions isn't met:" << std::endl
						 << "this->inFirstIteration() : " << this->inFirstIteration() << std::endl
						 << "this->inFirstCycle()     :" << this->inFirstCycle() << std::endl
						 << "m_nReturnedCurrent       :" << m_nReturnedCurrent << " (we expect 0)" << std::endl
				 );
			 }
#endif

			 m_first_retrieval = false;
			 return true;
		 } else {
			 return false;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Checks whether any work items have returned yet This is identical
	  * to the first call to this function, so we simply check and set a boolean
	  * indicating whether this is the first item.
	  */
	 bool firstItem() const {
		 if(m_first_item) {
#ifdef DEBUG
			 if((not this->inFirstIteration() || 1 != m_nReturnedCurrent)) {
				 throw gemfony_exception(
					 g_error_streamer(
						 DO_LOG
						 , time_and_place
					 )
						 << "In GBrokerExecutorT<processable_type>::firstItem():" << std::endl
						 << "Got true==m_first_item, while one of the preconditions isn't met:" << std::endl
						 << "this->inFirstIteration() : " << this->inFirstIteration() << std::endl
						 << "m_nReturnedCurrent       :" << m_nReturnedCurrent << " (we expect 1)" << std::endl
				 );
			 }
#endif

			 m_first_item = false;
			 return true;
		 } else {
			 return false;
		 }
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
		 std::shared_ptr <processable_type> w_ptr
		 , std::vector<std::shared_ptr<processable_type>>& workItems
		 , std::vector<std::shared_ptr<processable_type>>& oldWorkItems
	 ) {
		 // If we have been passed an empty item, simply continue the outer loop
		 if(not w_ptr) {
			 return executor_status_t{false /* is_complete */, false /* has_errors */ };
		 }

		 bool complete = false;
		 bool has_errors = false;

		 auto current_submission_id = this->get_iteration_counter();
		 auto worker_submission_id = w_ptr->getIterationCounter();

		 // Did this work item originate in the current submission cycle ?
		 if (current_submission_id == worker_submission_id) {
			 // Extract the original position of the work item and cross-check
			 std::size_t worker_position = w_ptr->getCollectionPosition();

			 // Add the work item to the list in the desired position. Re-submitted items
			 // might return twice, so we only add them if a processed item hasn't been added yet.
			 // We also take care of the situation that w_ptr points to the same object as the one
			 // stored in the workItems vector. This may happen in the case of local submission,
			 // such as with multi-threaded or serial consumers. DO_PROCESS will have been replaced
			 // by PROCESSED in this case.
			 if (
				 workItems.at(worker_position) == w_ptr // Same item
				 || processingStatus::DO_PROCESS == workItems.at(worker_position)->getProcessingStatus()
				 ) {
				 // Note that also items with errors may be added here. It is up to
				 // the caller to decide what to do with such work items.
				 if(workItems.at(worker_position) != w_ptr) workItems.at(worker_position) = w_ptr;
				 if (++m_nReturnedCurrent==this->getExpectedNumber()) complete=true;
				 if (w_ptr->has_errors()) has_errors=true;
			 } // no else
		 } else { // Not a work item from the current submission cycle.
			 // Ignore old work items with errors
			 if (processingStatus::PROCESSED == w_ptr->getProcessingStatus()) {
				 oldWorkItems.push_back(w_ptr);
			 } else {
				 // This should be rare. As we throw away items here, we want to
				 // make a record as a frequent occurrance might indicate a problem
				 glogger
					 << "In GBrokerExecutorT<>::addWorkItemAndCheckCompleteness():" << std::endl
					 << "Received old work item from submission cycle " << worker_submission_id << " (now " << current_submission_id << ")" << std::endl
					 << "We will throw the item away as it has the status id " << w_ptr->getProcessingStatus() << std::endl
					 << "(expected processingStatus::PROCESSED / " << processingStatus::PROCESSED << ")" << std::endl
					 << (w_ptr->has_errors()?w_ptr->getStoredErrorDescriptions():"") << std::endl
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
		 std::chrono::duration<double> currentElapsed
			 = this->now() - this->getIterationStartTime();
		 auto current_iteration = this->get_iteration_counter();

		 m_waiting_times_graph->add(boost::numeric_cast<double>(current_iteration), m_maxTimeout.count());
		 m_returned_items_graph->add(boost::numeric_cast<double>(current_iteration), boost::numeric_cast<double>(this->getNReturnedLast()));
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
		 if(not m_current_buffer_port_ptr) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GBrokerExecutorT<processable_type>::determineInitialCycleStartTime():" << std::endl
					 << "No valid buffer port found" << std::endl
			 );
		 }
#endif

		 return m_current_buffer_port_ptr->getFirstRetrievalTime();
	 }

	 /***************************************************************************/
	 // Local data
	 double m_waitFactor = DEFAULTBROKERWAITFACTOR2; ///< A static factor to be applied to timeouts

	 std::uint16_t m_minPartialReturnPercentage = DEFAULTEXECUTORPARTIALRETURNPERCENTAGE; ///< Minimum percentage of returned items after which execution continues

	 GBufferPortT_ptr m_current_buffer_port_ptr; ///< Holds a GBufferPortT object during the calculation. Note: It is neither serialized nor copied

	 bool m_capable_of_full_return = false; ///< Indicates whether the broker may return results without losses

	 mutable bool m_first_retrieval = true; ///< Indicates whether any retrievals have taken place so far
	 mutable bool m_first_item = true; ///< Indicates whether we are dealing with the very first individual

	 Gem::Common::GPlotDesigner m_gpd; ///< A wrapper for the plots
	 std::shared_ptr<Gem::Common::GGraph2D> m_waiting_times_graph;  ///< The maximum waiting time resulting from the wait factor
	 std::shared_ptr<Gem::Common::GGraph2D> m_returned_items_graph;  ///< The maximum waiting time resulting from the wait factor

	 bool m_waitFactorWarningEmitted = false; ///< Specifies whether a warning about a small waitFactor has already been emitted

	 std::size_t m_nReturnedCurrent = 0; ///< Temporary that holds the number of returned work items duing a submission cycle (or a resubmission)

	 std::chrono::duration<double> m_maxTimeout = std::chrono::duration<double>(0.); ///< The maximum amount of time allowed for the entire calculation

	 /** @brief Holds the maximum return times of processed individuals */
	 boost::accumulators::accumulator_set<
		 double
		 , boost::accumulators::stats<boost::accumulators::tag::max>
	 > m_acc_max;
};


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

/******************************************************************************/
// Some code for Boost.Serialization

/******************************************************************************/
// Mark GBaseExecutorT<> as abstract. This is the content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
namespace serialization {
template<typename processable_type>
struct is_abstract<Gem::Courtier::GBaseExecutorT<processable_type>> : public boost::true_type {};
template<typename processable_type>
struct is_abstract<const Gem::Courtier::GBaseExecutorT<processable_type>> : public boost::true_type {};
}
}

/******************************************************************************/

