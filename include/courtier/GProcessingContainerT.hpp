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
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
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
#include <chrono>
#include <type_traits>
#include <exception>
#include <functional>

// Boost headers go here
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <boost/exception/diagnostic_information.hpp>

// Geneva headers go here
#include "common/GSerializeTupleT.hpp"
#include "common/GSerializableFunctionObjectT.hpp"
#include "common/GExceptions.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GCourtierHelperFunctions.hpp"

namespace Gem::Courtier {

/******************************************************************************/
// An exception to be thrown if an exception was thrown during processing
class g_processing_exception : public gemfony_exception { using gemfony_exception::gemfony_exception; };

/******************************************************************************/
/**
 * This class can serve as a base class for items to be submitted through the broker. You need to
 * re-implement the purely virtual functions in derived classes. Note that it is mandatory for
 * derived classes to be serializable and to trigger serialization of this class.
 *
 * @tparam processable_type The type of the class derived from GProcessingContainerT
 * @tparam processing_result_type The result type of the process_ call; should be copyable
 */
template<
	typename processable_type
	, typename processing_result_type
	, class = typename std::enable_if<not std::is_void<processing_result_type>::value>::type
>
class GProcessingContainerT
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & BOOST_SERIALIZATION_NVP(m_iteration_counter)
		 & BOOST_SERIALIZATION_NVP(m_resubmission_counter)
		 & BOOST_SERIALIZATION_NVP(m_collection_position)
		 & BOOST_SERIALIZATION_NVP(m_bufferport_id)
		 & BOOST_SERIALIZATION_NVP(m_preProcessingDisabled)
		 & BOOST_SERIALIZATION_NVP(m_postProcessingDisabled)
		 & BOOST_SERIALIZATION_NVP(m_pre_processor_ptr)
		 & BOOST_SERIALIZATION_NVP(m_post_processor_ptr)
		 & BOOST_SERIALIZATION_NVP(m_pre_processing_time)
		 & BOOST_SERIALIZATION_NVP(m_processing_time)
		 & BOOST_SERIALIZATION_NVP(m_post_processing_time)
		 & BOOST_SERIALIZATION_NVP(m_bufferport_raw_retrieval_time)
		 & BOOST_SERIALIZATION_NVP(m_bufferport_raw_submission_time)
		 & BOOST_SERIALIZATION_NVP(m_bufferport_proc_retrieval_time)
		 & BOOST_SERIALIZATION_NVP(m_bufferport_proc_submission_time)
		 & BOOST_SERIALIZATION_NVP(m_stored_results_cnt)
		 & BOOST_SERIALIZATION_NVP(m_stored_error_descriptions)
		 & BOOST_SERIALIZATION_NVP(m_processing_status)
		 & BOOST_SERIALIZATION_NVP(m_evaluation_id);
	 }

	 ///////////////////////////////////////////////////////////////////////

public:
	 using payload_type = processable_type;
	 using result_type  = processing_result_type;

	 /***************************************************************************/
	 /**
	  * Initialization with the number of stored results
	  */
	 explicit GProcessingContainerT(std::size_t n_stored_results)
	 	: m_stored_results_cnt(n_stored_results, processing_result_type())
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The copy constructor
	  *
	  * @param cp A copy of another GSubmissionContainer object
	  */
	 explicit GProcessingContainerT(GProcessingContainerT<processable_type, processing_result_type> const &cp)
		 : m_iteration_counter(cp.m_iteration_counter)
		 , m_resubmission_counter(cp.m_resubmission_counter)
		 , m_collection_position(cp.m_collection_position)
		 , m_bufferport_id(cp.m_bufferport_id)
		 , m_preProcessingDisabled(cp.m_preProcessingDisabled)
		 , m_postProcessingDisabled(cp.m_postProcessingDisabled)
		 , m_pre_processing_time(cp.m_pre_processing_time)
		 , m_processing_time(cp.m_processing_time)
		 , m_post_processing_time(cp.m_post_processing_time)
		 , m_bufferport_raw_retrieval_time(cp.m_bufferport_raw_retrieval_time)
		 , m_bufferport_raw_submission_time(cp.m_bufferport_raw_submission_time)
		 , m_bufferport_proc_retrieval_time(cp.m_bufferport_proc_retrieval_time)
		 , m_bufferport_proc_submission_time(cp.m_bufferport_proc_submission_time)
	 	 , m_stored_results_cnt(cp.m_stored_results_cnt) // Note: processing_result_type must be copyable (e.g. it should not contain pointers)
		 , m_stored_error_descriptions(cp.m_stored_error_descriptions)
		 , m_processing_status(cp.m_processing_status)
		 , m_evaluation_id(cp.m_evaluation_id)
	 {
		 Gem::Common::copyCloneableSmartPointer(cp.m_pre_processor_ptr, m_pre_processor_ptr);
		 Gem::Common::copyCloneableSmartPointer(cp.m_post_processor_ptr, m_post_processor_ptr);
	 }

	 /***************************************************************************/
	 /**
	  * Assignment operator
	  */
	 GProcessingContainerT&
	 operator=(GProcessingContainerT<processable_type, processing_result_type> const& cp) {
	 	m_iteration_counter = cp.m_iteration_counter;
	 	m_resubmission_counter = cp.m_resubmission_counter;
	 	m_collection_position = cp.m_collection_position;
	 	m_bufferport_id = cp.m_bufferport_id;
	 	m_preProcessingDisabled = cp.m_preProcessingDisabled;
	 	m_postProcessingDisabled = cp.m_postProcessingDisabled;
	 	m_pre_processing_time = cp.m_pre_processing_time;
	 	m_processing_time = cp.m_processing_time;
	 	m_post_processing_time = cp.m_post_processing_time;
	 	m_bufferport_raw_retrieval_time = cp.m_bufferport_raw_retrieval_time;
	 	m_bufferport_raw_submission_time = cp.m_bufferport_raw_submission_time;
	 	m_bufferport_proc_retrieval_time = cp.m_bufferport_proc_retrieval_time;
	 	m_bufferport_proc_submission_time = cp.m_bufferport_proc_submission_time;
	 	m_stored_results_cnt = cp.m_stored_results_cnt; // Note: processing_result_type must be copyable (e.g. it should not contain pointers)
	 	m_stored_error_descriptions = cp.m_stored_error_descriptions;
	 	m_processing_status = cp.m_processing_status;
	 	m_evaluation_id = cp.m_evaluation_id;

	    Gem::Common::copyCloneableSmartPointer(cp.m_pre_processor_ptr, m_pre_processor_ptr);
	    Gem::Common::copyCloneableSmartPointer(cp.m_post_processor_ptr, m_post_processor_ptr);

	 	return *this;
	 }

	 /***************************************************************************/
	 // Defaulted or deleted constructors, destructor and move assignment operator

	 // Default constructor may be found in private section

	 // TODO: Make class movable
	 GProcessingContainerT(GProcessingContainerT<processable_type, processing_result_type> &&) = delete;
	 GProcessingContainerT<processable_type, processing_result_type>& operator=(GProcessingContainerT<processable_type, processing_result_type> &&) = delete;

     virtual ~GProcessingContainerT() BASE = default;

	 /***************************************************************************/
	 /**
	  * Sets the vector of stored results to a given collection and marks
	  * the object as processed
	  */
	 processing_result_type markAsProcessedWith(std::vector<processing_result_type> const & result_cnt) {
#ifdef DEBUG
		 // Check that we have been given a suitable new results vector
		 if(result_cnt.size() != m_stored_results_cnt.size()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GProcessingContainerT::markAsProcessedWith(): Vector dimensions" << std::endl
				 	 << "do not fit: " << result_cnt.size() << " / " << m_stored_results_cnt.size() << std::endl
			 );
		 }
#endif

		 // Transfer the new values
		 m_stored_results_cnt = result_cnt;

		 // Clear the error descriptions
		 m_stored_error_descriptions.clear();

		 // Mark as processed
		 m_processing_status = processingStatus::PROCESSED;

		 // This part of the code should never be reached if an exception was thrown
		 return this->m_stored_results_cnt.at(0);
	 }

	 /***************************************************************************/
	 /**
	  * Perform the actual processing steps. E.g. in optimization algorithms,
	  * post-processing allows to run a sub-optimization. The amount of time
	  * needed for processing is measured for logging purposes. Where one of the
	  * processing functions throws an exception, the function will store the
	  * necessary exception information locally and rethrow the exception.
	  * Note that user-defined processing- and post-processing functions need
	  * to make sure to set the results (be it main- or secondary results) of the
	  * process()-call. This function has no way to ensure that this is the case.
	  *
	  * @param ext_processor Injects an external function for the processing step
	  * @return The first result of the processing calls
	  */
	 processing_result_type process(
		 std::function<void(processable_type&)> ext_processor
	 		= std::function<void(processable_type&)>()
	 ) {
		 // This function should never be called if the processing status is not set to "DO_PROCESS"
		 if(processingStatus::DO_PROCESS != m_processing_status) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GProcessingContainerT::process(): Function called while m_processing_status was set to " << m_processing_status << std::endl
				 	 << "Expected " << processingStatus::DO_PROCESS << "(processingStatus::DO_PROCESS)" << std::endl
			 );
		 }

		 // Assign a new evaluation id
		 m_evaluation_id = std::string("eval_") + Gem::Common::to_string(boost::uuids::random_generator()());

		 // Clear the error descriptions
		 m_stored_error_descriptions.clear();

		 // "Nullify the result list.
		 this->clear_stored_results_vec();

		 std::ostringstream error_description_stream;
		 processing_result_type main_result;

		 try {
			 // Perform the actual processing
			 auto startTime = std::chrono::high_resolution_clock::now();
			 this->preProcess_();
			 auto afterPreProcessing = std::chrono::high_resolution_clock::now();

			 if(ext_processor) { // Have we been passed an external function for the processing step?
				 auto& p_ref = dynamic_cast<processable_type&>(*this);
				 ext_processor(p_ref);
			 } else { // No external function
				 this->process_();
			 }

			 auto afterProcessing = std::chrono::high_resolution_clock::now();
			 this->postProcess_();
			 auto afterPostProcessing = std::chrono::high_resolution_clock::now();

			 // Make a note of the time needed for each step
			 m_pre_processing_time = std::chrono::duration<double>(afterPreProcessing - startTime).count();
			 m_processing_time = std::chrono::duration<double>(afterProcessing - afterPreProcessing).count();
			 m_post_processing_time = std::chrono::duration<double>(afterPostProcessing - afterProcessing).count();

			 m_processing_status = processingStatus::PROCESSED;
		 } catch(boost::exception& e) {
			 // Let the audience know we had an error
			 m_processing_status = processingStatus::EXCEPTION_CAUGHT;
			 error_description_stream
				 << "In GProcessingContainerT<>::process():" << std::endl
				 << "Processing has thrown a boost exception with message" << std::endl
				 << boost::diagnostic_information(e) << std::endl
				 << "We will rethrow this exception" << std::endl;
		 } catch(std::exception& e) {
			 // Let the audience know we had an error
			 m_processing_status = processingStatus::EXCEPTION_CAUGHT;
			 error_description_stream
				 << "In GProcessingContainerT<processable_type>::process():" << std::endl
				 << "Processing has thrown an exception with message" << std::endl
				 << e.what() << std::endl
				 << "We will rethrow this exception" << std::endl;
		 } catch(...) {
			 // Let the audience know we had an error
			 m_processing_status = processingStatus::EXCEPTION_CAUGHT;
			 error_description_stream
				 << "In GProcessingContainerT<processable_type>::process():" << std::endl
				 << "Processing has thrown an unknown exception." << std::endl;
		 }

		 if(this->has_errors()) { // Either an exception was caught or the user has flagged an error
			 // Do some cleanup
			 m_pre_processing_time = 0.;
			 m_processing_time = 0.;
			 m_post_processing_time = 0.;

			 // "Nullify the result list.
			 this->clear_stored_results_vec();

			 // Store the exceptions for later reference
			 if(processingStatus::EXCEPTION_CAUGHT == m_processing_status) {
				 // Error information added by the user might already be stored in this variable. Hence we use +=
				 m_stored_error_descriptions += error_description_stream.str();
			 }

			 throw g_processing_exception( // Note: this is a specific exception to flag errors during processing
				 g_error_streamer(DO_LOG, time_and_place) << m_stored_error_descriptions
			 );
		 }

		 // This part of the code should never be reached if an exception was thrown
		 return this->m_stored_results_cnt.at(0);
	 }

	 /***************************************************************************/
	 /**
	  * Retrieval of the stored result. This function does not allow modifications
	  * of its return value. It will throw, if value retrieval is initiated for a work
	  * item which does not have the PROCESSED flag set.
	  *
	  * @param id The id of the stored result to be returned
	  * @return The stored result at position id in m_stored_results_vec
	  */
	 processing_result_type getStoredResult(std::size_t id = 0) const {
		 if(not this->is_processed()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GProcessingContainerT::getStoredResult(): Tried to" << std::endl
					 << "retrieve stored result while the PROCESSED flag was not set" << std::endl
			 );
		 }

		 return m_stored_results_cnt.at(id);
	 }


	 /******************************************************************************/
	 /**
	  * Retrieve the id assigned to the current evaluation. Note that there is no
	  * guaranty that the item has indeed been processed. This is id simply represents
	  * the processing id assigned at the beginning of the last process()-call.
	  */
	 [[nodiscard]] std::string getCurrentEvaluationID() const {
		 return m_evaluation_id;
	 }

	 /***************************************************************************/
	 /**
	  * Loads user-specified data. This function can be overloaded by derived classes. It
	  * is mainly intended to provide a mechanism to "deposit" an item at a remote site
	  * that holds otherwise constant data. That data then does not need to be serialized
	  * but can be loaded whenever a new work item arrives and has been de-serialized. Note
	  * that, if your individuals do not serialize important parts of an object, you need
	  * to make sure that constant data is loaded after reloading a checkpoint.
	  *
	  * @param cd_ptr A pointer to the object whose data should be loaded
	  */
	 void loadConstantData(std::shared_ptr<processable_type> cd_ptr) {
		 this->loadConstantData_(cd_ptr);
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the current processing status
	  */
	 [[nodiscard]] processingStatus getProcessingStatus() const noexcept {
		 return m_processing_status;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the current processing status as a string (mostly for
	  * debugging purposes).
	  */
	 [[nodiscard]] std::string getProcessingStatusAsStr() const noexcept {
		 return psToStr(m_processing_status);
	 }

	 /***************************************************************************/
	 /**
	  * Checks whether the processed flag was set for this item
	  *
	  * @return A boolean indicating whether the item was processed
	  */
	 [[nodiscard]] bool is_processed() const noexcept {
		 return (processingStatus::PROCESSED == this->getProcessingStatus());
	 }

	 /***************************************************************************/
	 /**
	  * Checks whether the IGNORED flag is set
	  */
	 [[nodiscard]] bool is_ignored() const noexcept {
		 return (processingStatus::DO_IGNORE == this->getProcessingStatus());
	 }

	 /***************************************************************************/
	 /**
	  * Checks whether the DO_PROCESS flag was  set for this item
	  *
	  * @return A boolean indicating whether the item is due for processing
	  */
	 [[nodiscard]] bool is_due_for_processing() const noexcept {
		 return (processingStatus::DO_PROCESS == this->getProcessingStatus());
	 }

	 /***************************************************************************/
	 /**
	  * Checks if there were errors during processing
	  *
	  * @return A boolean indicating whether there were errors during processing
	  */
	 [[nodiscard]] bool has_errors() const noexcept {
		 return
			 (processingStatus::EXCEPTION_CAUGHT == m_processing_status)
			 || (processingStatus::ERROR_FLAGGED == m_processing_status);
	 }

	 /***************************************************************************/
	 /**
	  * Allows to check whether an error was flagged by the user
	  */
	 [[nodiscard]] bool error_flagged_by_user() const noexcept {
		 return (processingStatus::ERROR_FLAGGED == m_processing_status);
	 }

	 /***************************************************************************/
	 /**
	  * Sets a given new processing state. Which new states are
	  * accepted depends on the current state:
	  * - IGNORE --> IGNORE, DO_PROCESS
	  * - DO_PROCESS --> DO_PROCESS, IGNORE
	  * - PROCESSED --> PROCESSED, IGNORE, DO_PROCESS
	  * - EXCEPTION_CAUGHT --> EXCEPTION_CAUGHT, IGNORE, DO_PROCESS
	  * - ERROR_FLAGGED --> ERROR_FLAGGED, IGNORE, DO_PROCESS
	  * Note that some target states may result in the erasure of existing
	  * information, such as past error messages. Setting a new processing state
	  * of "PROCESSED" via this function is not allowed and will result in an
	  * exception being thrown, unless this state is already set.
	  *
	  * @param target_ps The desired new processing status
	  */
	 void set_processing_status(processingStatus target_ps = processingStatus::DO_IGNORE) {
		 // Do nothing if the new state is equal to the old one
		 if(target_ps == m_processing_status) {
			 return;
		 }

		 // We do not accept setting a target state of PROCESSED via this function
		 if(target_ps == processingStatus::PROCESSED) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GProcessingContainerT<>::set_processing_status():" << std::endl
					 << "An attempt was made to set the processing state to PROCESSED" << std::endl
				 	 << "which is not allowed through this function." << std::endl
			 );
		 }

		 // We want to enforce specific targets depending on the current state
		 switch(m_processing_status) {
			 //------------------------------------------------------------------------------------

			 case processingStatus::DO_IGNORE:
				 if(target_ps == processingStatus::DO_PROCESS) {
					 // Store the new state
					 m_processing_status = target_ps;
				    // Clear any remaining error messages
					 m_stored_error_descriptions.clear();
					 // "Nullify" the result list.
					 this->clear_stored_results_vec();
				 } else {
					 throw gemfony_exception(
						 g_error_streamer(DO_LOG, time_and_place)
							 << "In GProcessingContainerT<>::set_processing_status():" << std::endl
							 << "Got invalid target processing status " << psToStr(target_ps) << std::endl
							 << "Expected a new state of DO_PROCESS for the" << std::endl
							 << "current state of " << psToStr(m_processing_status) << std::endl
					 );
				 }
				 break;

			 //------------------------------------------------------------------------------------

			 case processingStatus::DO_PROCESS:
				 if(target_ps == processingStatus::DO_IGNORE) {
					 // Store the new state
					 m_processing_status = target_ps;
					 // Clear any remaining error messages
					 m_stored_error_descriptions.clear();
					 // "Nullify" the result list.
					 this->clear_stored_results_vec();
				 } else {
					 throw gemfony_exception(
						 g_error_streamer(DO_LOG, time_and_place)
							 << "In GProcessingContainerT<>::set_processing_status():" << std::endl
							 << "Got invalid target processing status " << psToStr(target_ps) << std::endl
							 << "Expected a new state of DO_IGNORE for the" << std::endl
							 << "current state of " << psToStr(m_processing_status) << std::endl
					 );
				 }
				 break;

			 //------------------------------------------------------------------------------------

			 case processingStatus::PROCESSED:
				 if(target_ps == processingStatus::DO_IGNORE || target_ps == processingStatus::DO_PROCESS) {
					 // Store the new state
					 m_processing_status = target_ps;
					 // Clear any remaining error messages
					 m_stored_error_descriptions.clear();
					 // "Nullify" the result list.
					 this->clear_stored_results_vec();
				 } else {
					 throw gemfony_exception(
						 g_error_streamer(DO_LOG, time_and_place)
							 << "In GProcessingContainerT<>::set_processing_status():" << std::endl
							 << "Got invalid target processing status " << psToStr(target_ps) << std::endl
							 << "Expected a new state of DO_IGNORE or DO_PROCESS for the" << std::endl
							 << "current state of " << psToStr(m_processing_status) << std::endl
					 );
				 }
				 break;

			 //------------------------------------------------------------------------------------

			 case processingStatus::EXCEPTION_CAUGHT:
			 case processingStatus::ERROR_FLAGGED:
				 if(target_ps == processingStatus::DO_IGNORE || target_ps == processingStatus::DO_PROCESS) {
					 // Store the new state
					 m_processing_status = target_ps;
					 // Clear any remaining error messages
					 m_stored_error_descriptions.clear();
					 // "Nullify" the result list.
					 this->clear_stored_results_vec();
				 } else {
					 throw gemfony_exception(
						 g_error_streamer(DO_LOG, time_and_place)
							 << "In GProcessingContainerT<>::set_processing_status():" << std::endl
					 	    << "Got invalid target processing status " << psToStr(target_ps) << std::endl
						 	 << "Expected a new state of DO_IGNORE or DO_PROCESS for the" << std::endl
						 	 << "current state of " << psToStr(m_processing_status) << std::endl
					 );
				 }
				 break;

			 //------------------------------------------------------------------------------------
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Marks this item as being due for processing.
	  */
	 void mark_as_due_for_processing() {
		 m_processing_status = processingStatus::DO_PROCESS;
	 }

	 /***************************************************************************/
	 /**
	  * Sets the IGNORE flag for this work item so that it will not be processed.
	  */
	 void mark_as_ignorable() {
		 m_processing_status = processingStatus::DO_IGNORE;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the counter of a given iteration
	  */
	 void setIterationCounter(const ITERATION_COUNTER_TYPE &counter)  noexcept {
		 m_iteration_counter = counter;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the counter of a given iteration
	  */
	 [[nodiscard]] ITERATION_COUNTER_TYPE getIterationCounter() const noexcept {
		 return m_iteration_counter;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the counter of the current submission inside of an iteration
	  */
	 void setResubmissionCounter(const RESUBMISSION_COUNTER_TYPE &resubmission_counter) noexcept {
		 m_resubmission_counter = resubmission_counter;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the counter of the current submission inside of an iteration
	  */
	 [[nodiscard]] RESUBMISSION_COUNTER_TYPE getResubmissionCounter() const noexcept {
		 return m_resubmission_counter;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the position inside of a given collection submitted to the broker
	  */
	 void setCollectionPosition(const COLLECTION_POSITION_TYPE &pos) noexcept {
		 m_collection_position = pos;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the position inside of a given collection submitted to the broker
	  */
	 [[nodiscard]] COLLECTION_POSITION_TYPE getCollectionPosition() const noexcept {
		 return m_collection_position;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the id inside of the originating buffer
	  */
	 void setBufferId(const BUFFERPORT_ID_TYPE& id) noexcept {
		 m_bufferport_id = id;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the id of the originating buffer
	  */
	 [[nodiscard]] BUFFERPORT_ID_TYPE getBufferId() const noexcept {
		 return m_bufferport_id;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the timepoint when a work item was retrieved from the raw queue
	  */
	 [[nodiscard]] std::chrono::high_resolution_clock::time_point getRawRetrievalTime() const {
		 return m_bufferport_raw_retrieval_time;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the timepoint when a work item was submitted to the raw queue
	  */
	 [[nodiscard]] std::chrono::high_resolution_clock::time_point getRawSubmissionTime() const {
		 return m_bufferport_raw_submission_time;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the timepoint when a work item was retrieved from the processed queue
	  */
	 [[nodiscard]] std::chrono::high_resolution_clock::time_point getProcRetrievalTime() const {
		 return m_bufferport_proc_retrieval_time;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the timepoint when a work item was submitted to the processed queue
	  */
	 [[nodiscard]] std::chrono::high_resolution_clock::time_point getProcSubmissionTime() const {
		 return m_bufferport_proc_submission_time;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to check whether any user-defined pre-processing before the process()-
	  * step may occur. This may alter the individual's data.
	  */
	 [[nodiscard]] bool mayBePreProcessed() const noexcept {
		 return not m_preProcessingDisabled;
	 }

	 /***************************************************************************/
	 /**
	  * Allow or prevent pre-processing (used by pre-processing algorithms to prevent
	  * recursive pre-processing). See e.g. GEvolutionaryAlgorithmPostOptimizerT. Once a veto
	  * exists, no pre-processing will occur until the veto is lifted.
	  */
	 void vetoPreProcessing(bool veto) noexcept {
		 m_preProcessingDisabled = veto;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to register a pre-processor object
	  */
	 void registerPreProcessor(
		 std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<processable_type>> pre_processor_ptr
	 ) {
		 if(pre_processor_ptr) {
			 m_pre_processor_ptr = pre_processor_ptr;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Allows to check whether any user-defined post-processing after the process()-
	  * step may occur. This may be important if e.g. an optimization algorithm wants
	  * to submit evaluation work items to the broker which may then start an optimization
	  * run on the individual. This may alter the individual's data.
	  */
	 [[nodiscard]] bool mayBePostProcessed() const {
		 return not m_postProcessingDisabled;
	 }

	 /***************************************************************************/
	 /**
	  * Allow or prevent post-processing (used by post-processing algorithms to prevent
	  * recursive post-processing). See e.g. GEvolutionaryAlgorithmPostOptimizerT. Once a veto
	  * exists, no post-processing will occur until the veto is lifted.
	  */
	 void vetoPostProcessing(bool veto) {
		 m_postProcessingDisabled = veto;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to register a post-processor object
	  */
	 void registerPostProcessor(std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<processable_type>> post_processor_ptr) {
		 if(post_processor_ptr) {
			 m_post_processor_ptr = post_processor_ptr;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the processing time needed for the work item
	  */
	 [[nodiscard]] std::tuple<double,double,double> getProcessingTimes() const {
		 return std::make_tuple(m_pre_processing_time, m_processing_time, m_post_processing_time);
	 };

	 /***************************************************************************/
	 /**
	  * Retrieves and clears exceptions and the processing status.
	  *
	  * @param The desired new processing status
	  */
	 std::string get_and_clear_exceptions(processingStatus ps = processingStatus::DO_IGNORE) {
		 std::string stored_exceptions = m_stored_error_descriptions;
		 this->set_processing_status(ps);
		 return stored_exceptions;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to extract stored error descriptions
	  */
	 [[nodiscard]] std::string getStoredErrorDescriptions() const {
		 return m_stored_error_descriptions;
	 }

	 /***************************************************************************/
 	 /**
 	  * Marks the time when the item was added to a GBuffferPortT raw queue
 	  */
	 void markRawSubmissionTime() {
		 m_bufferport_raw_submission_time = std::chrono::high_resolution_clock::now();
	 }

	 /***************************************************************************/
 	 /**
 	  * Marks the time when the item was retrieved from a GBuffferPortT raw queue
 	  */
	 void markRawRetrievalTime() {
		 m_bufferport_raw_retrieval_time = std::chrono::high_resolution_clock::now();
	 }

	 /***************************************************************************/
	 /**
	  * Marks the time when the item was submitted to a GBuffferPortT processed queue
	  */
	 void markProcSubmissionTime() {
		 m_bufferport_proc_submission_time = std::chrono::high_resolution_clock::now();
	 }

	 /***************************************************************************/
	 /**
	  * Marks the time when the item was retrieved from a GBuffferPortT processed queue
	  */
	 void markProcRetrievalTime() {
		 m_bufferport_proc_retrieval_time = std::chrono::high_resolution_clock::now();
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the number of stored results
	  */
	 [[nodiscard]] std::size_t getNStoredResults() const {
		 return m_stored_results_cnt.size();
	 }

	 /***************************************************************************/
	 /**
	  * Loads the data of another GProcessingContainerT<processable_type, processing_result_type> object
	  */
	 void load_pc(const GProcessingContainerT<processable_type, processing_result_type> *cp) {
		 // Check that we are dealing with a GProcessingContainerT<processable_type, processing_result_type> reference independent of this object and convert the pointer
		 const GProcessingContainerT<processable_type, processing_result_type> *p_load
			 = Gem::Common::g_convert_and_compare<GProcessingContainerT<processable_type, processing_result_type>, GProcessingContainerT<processable_type, processing_result_type>>(cp, this);

		 // Load local data
		 m_iteration_counter = p_load->m_iteration_counter;
		 m_resubmission_counter = p_load->m_resubmission_counter;
		 m_collection_position = p_load->m_collection_position;
		 m_bufferport_id = p_load->m_bufferport_id;
		 m_preProcessingDisabled = p_load->m_preProcessingDisabled;
		 m_postProcessingDisabled = p_load->m_postProcessingDisabled;
		 m_pre_processing_time = p_load->m_pre_processing_time;
		 m_processing_time = p_load->m_processing_time;
		 m_post_processing_time = p_load->m_post_processing_time;
		 m_bufferport_raw_submission_time = p_load->m_bufferport_raw_submission_time;
		 m_bufferport_raw_retrieval_time = p_load->m_bufferport_raw_retrieval_time;
		 m_bufferport_proc_submission_time = p_load->m_bufferport_proc_submission_time;
		 m_bufferport_proc_retrieval_time = p_load->m_bufferport_proc_retrieval_time;
		 m_stored_results_cnt = p_load->m_stored_results_cnt; // note that this implies that processing_result_type is copyable --> e.g. it should not contain pointers
		 m_stored_error_descriptions = p_load->m_stored_error_descriptions;
		 m_processing_status = p_load->m_processing_status;
		 m_evaluation_id = p_load->m_evaluation_id;

		 Gem::Common::copyCloneableSmartPointer(p_load->m_pre_processor_ptr, m_pre_processor_ptr);
		 Gem::Common::copyCloneableSmartPointer(p_load->m_post_processor_ptr, m_post_processor_ptr);
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Retrieval of the stored result. This function allows modifications of its
	  * return value and is hence protected and only accessible by derived classes.
	  *
	  * @param id The id of the stored result to be returned
	  * @return The stored result at position id in m_stored_results_vec
	  */
	 processing_result_type& modifyStoredResult(std::size_t id = 0) {
		 return m_stored_results_cnt.at(id);
	 }

	 /***************************************************************************/
	 /**
	  * Allows derived classes to set the number of stored results. Note that this
	  * should happen prior to any operation with this object. Also note that this
	  * operation may invalidate other results already stored in this object.
	  *
	  * @param The number of stored results in this class
	  * @param new_val A value to be copied into new positions when the vector is increased
	  */
	 void setNStoredResults(
		 std::size_t n_stored_results
		 , processing_result_type new_val
	 ) {
		 m_stored_results_cnt.resize(n_stored_results, new_val);
	 }

	 /***************************************************************************/
	 /**
	  * Allows derived classes to set the number of stored results. Note that this
	  * should happen prior to any operation with this object. Also note that this
	  * operation may invalidate other results already stored in this object.
	  */
	 void setNStoredResults(
		 std::size_t n_stored_results
	 ) {
		 processing_result_type p;
		 this->setNStoredResults(n_stored_results, p);
	 }

	 /***************************************************************************/
	 /**
	  * Allows to register a result, using its id (i.e. position in the internal
	  * result storage. This function should be called from inside of the process_ call.
	  */
	 void registerResult(std::size_t id, const processing_result_type& r) {
		 m_stored_results_cnt.at(id) = r;
	 }

	 /***************************************************************************/
	 /**
	  * This function allows derived classes to specify custom error conditions by
	  * setting their own error messages. The function will also set the internal
	  * flags that indicate that an error has occurred and that processing was not
	  * successful. NOTE That the error description may not be empty.
	  *
	  * @param An error description
	  */
	 void force_set_error(
		 const std::string& error_info
	 ) {
		 if(error_info.empty()) {
			 throw gemfony_exception( // Note: this is a specific exception to flag errors during processing
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GProcessingContainerT::force_set_error(): Error info is empty" << std::endl
			 );
		 }

		 // There may already be information stored in this variable. Hence we attach the new information via +=
		 m_stored_error_descriptions += error_info;
		 m_processing_status = processingStatus::ERROR_FLAGGED;
	 }

	/***************************************************************************/
	/**
     * The default constructor. It is only needed for (de-)serialization purposes.
     * We want to enforce the specification of the number of evaluation criteria
     * in derived classes. Protected, so that a derived class can have a defaulted
     * default constructor.
     */
	GProcessingContainerT() = default;

private:
	 /***************************************************************************/
	 /**
	  * Little helper function to (re-)initialize the result storage vector
	  */
	 void clear_stored_results_vec() {
		 // "Nullify the result list. We cannot use range-based for here, as m_stored_results_cnt might hold booleans
		 for(auto it=m_stored_results_cnt.begin(); it!=m_stored_results_cnt.end(); ++it) {
			 *it = processing_result_type();
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Loads user-specified data. This function can be overloaded by derived classes. It
	  * is mainly intended to provide a mechanism to "deposit" an item at a remote site
	  * that holds otherwise constant data. That data then does not need to be serialized
	  * but can be loaded whenever a new work item arrives and has been de-serialized. Note
	  * that, if your work items do not serialize important parts of an object, you need
	  * to make sure that constant data is loaded after reloading a checkpoint.
	  *
	  * @param cD_ptr A pointer to the object whose data should be loaded
	  */
	 virtual void loadConstantData_(std::shared_ptr<processable_type>) BASE
	 { /* nothing */ }

	 /***************************************************************************/

	 /** @brief Allows derived classes to specify the tasks to be performed for this object */
	 virtual G_API_COURTIER void process_() BASE = 0;

	 /***************************************************************************/
	 /**
	  * Specifies tasks to be performed before the process_ call. Note: This function
	  * will reset the m_mayBePreProcessed-flag.
  	  */
	 void preProcess_() {
		 if(this->mayBePreProcessed() && m_pre_processor_ptr) {
			 auto& p = dynamic_cast<processable_type&>(*this);
			 (*m_pre_processor_ptr)(p);
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Specifies tasks to be performed after the process_ call. Note: This function
	  * will reset the m_mayBePostProcessed-flag.
  	  */
	 void postProcess_() {
		 if(this->mayBePostProcessed() && m_post_processor_ptr) {
			 auto& p = dynamic_cast<processable_type&>(*this);
			 (*m_post_processor_ptr)(p);
		 }
	 }

	 /***************************************************************************/
	 // Data

	 ITERATION_COUNTER_TYPE m_iteration_counter = ITERATION_COUNTER_TYPE(0);
	 RESUBMISSION_COUNTER_TYPE m_resubmission_counter = RESUBMISSION_COUNTER_TYPE(0);
	 COLLECTION_POSITION_TYPE m_collection_position = COLLECTION_POSITION_TYPE(0);
	 BUFFERPORT_ID_TYPE m_bufferport_id = BUFFERPORT_ID_TYPE();

	 bool m_preProcessingDisabled = false; ///< Indicates whether pre-processing was diabled entirely
	 bool m_postProcessingDisabled = false; ///< Indicates whether pre-processing was diabled entirely

	 std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<processable_type>> m_pre_processor_ptr; ///< Actions to be performed before processing
	 std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<processable_type>> m_post_processor_ptr; ///< Actions to be performed after processing

	 double m_pre_processing_time = 0.; ///< The amount of time needed for pre-processing (in seconds)
	 double m_processing_time = 0.; ///< The amount of time needed for the actual processing step (in seconds)
	 double m_post_processing_time = 0.; ///< The amount of time needed for post-processing (in seconds)

	 std::chrono::high_resolution_clock::time_point m_bufferport_raw_retrieval_time;   ///< Time when the item was retrieved from the raw queue
	 std::chrono::high_resolution_clock::time_point m_bufferport_raw_submission_time;  ///< Time when the item was submitted to the raw queue
	 std::chrono::high_resolution_clock::time_point m_bufferport_proc_retrieval_time;  ///< Time when the item was retrieved from the processed queue
	 std::chrono::high_resolution_clock::time_point m_bufferport_proc_submission_time; ///< Time when the item was submitted to the processed queue

	 std::vector<processing_result_type> m_stored_results_cnt = std::vector<processing_result_type>(1, processing_result_type()); ///< The results stored by this object

	 std::string m_stored_error_descriptions; ///< Stores exceptions that may have occurred during processing
	 processingStatus m_processing_status = processingStatus::DO_IGNORE; ///< By default no processing is initiated

	 std::string m_evaluation_id = "empty"; ///< A unique id that is assigned to an evaluation
};

/******************************************************************************/

} // namespace Gem::Courtier

/******************************************************************************/
/** @brief Mark this class as abstract */
namespace boost::serialization {
template<typename processable_type, typename processing_result_type>
struct is_abstract<Gem::Courtier::GProcessingContainerT<processable_type, processing_result_type>> : public boost::true_type {};
template<typename processable_type, typename processing_result_type>
struct is_abstract<const Gem::Courtier::GProcessingContainerT<processable_type, processing_result_type>> : public boost::true_type {};
} // namespace boost::serialization

/******************************************************************************/

