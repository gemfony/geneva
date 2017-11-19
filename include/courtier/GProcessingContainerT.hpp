/**
 * @file GProcessingContainerT.hpp
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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <tuple>
#include <chrono>
#include <type_traits>
#include <exception>

// Boost headers go here
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/variant.hpp>
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
#include <boost/uuid/uuid_serialize.hpp>
#include <boost/exception/diagnostic_information.hpp>

#ifndef GSUBMISSIONCONTAINERBASE_HPP_
#define GSUBMISSIONCONTAINERBASE_HPP_

// Geneva headers go here
#include "common/GSerializeTupleT.hpp"
#include "common/GSerializableFunctionObjectT.hpp"
#include "common/GExceptions.hpp"
#include "common/GErrorStreamer.hpp"
#include "courtier/GCourtierEnums.hpp"

namespace Gem {
namespace Courtier {

/******************************************************************************/
/**
 * This class can serve as a base class for items to be submitted through the broker. You need to
 * re-implement the purely virtual functions in derived classes. Note that it is mandatory for
 * derived classes to be serializable and to trigger serialization of this class.
 *
 * @tparam processable_type The type of the class derived from GProcessingContainerT
 * @tparam processing_result_type The result type of the process_ call
 */
template<
	typename processable_type
	, typename processing_result_type
	, class = typename std::enable_if<!std::is_void<processing_result_type>::value>::type
	, class = typename std::enable_if< std::is_pod <processing_result_type>::value>::type
>
class GProcessingContainerT
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & BOOST_SERIALIZATION_NVP(m_submission_counter)
		 & BOOST_SERIALIZATION_NVP(m_submission_position)
		 & BOOST_SERIALIZATION_NVP(m_bufferport_id)
		 & BOOST_SERIALIZATION_NVP(m_preProcessingDisabled)
		 & BOOST_SERIALIZATION_NVP(m_postProcessingDisabled)
		 & BOOST_SERIALIZATION_NVP(m_pre_processor_ptr)
		 & BOOST_SERIALIZATION_NVP(m_post_processor_ptr)
		 & BOOST_SERIALIZATION_NVP(m_processing_was_successful)
		 & BOOST_SERIALIZATION_NVP(m_stored_result)
		 & BOOST_SERIALIZATION_NVP(m_stored_exceptions)
		 & BOOST_SERIALIZATION_NVP(m_has_exceptions);

		 // Note that we do not serialize the temporary timings
	 }

	 ///////////////////////////////////////////////////////////////////////

public:
	 using payload_type = processable_type;
	 using result_type  = processing_result_type;

	 /***************************************************************************/
	 /**
	  * The default constructor
	  */
	 GProcessingContainerT() = default;

	 /***************************************************************************/
	 /**
	  * The copy constructor
	  *
	  * @param cp A copy of another GSubmissionContainer object
	  */
	 GProcessingContainerT(const GProcessingContainerT<processable_type, processing_result_type> &cp)
		 : m_submission_counter(cp.m_submission_counter)
			, m_submission_position(cp.m_submission_position)
			, m_bufferport_id(cp.m_bufferport_id)
			, m_preProcessingDisabled(cp.m_preProcessingDisabled)
			, m_postProcessingDisabled(cp.m_postProcessingDisabled)
	 	   , m_processing_was_successful(cp.m_processing_was_successful)
	 	   , m_stored_result(cp.m_stored_result)
	 	   , m_stored_exceptions(cp.m_stored_exceptions)
	 	   , m_has_exceptions(cp.m_has_exceptions)
	 {
		 Gem::Common::copyCloneableSmartPointer(cp.m_pre_processor_ptr, m_pre_processor_ptr);
		 Gem::Common::copyCloneableSmartPointer(cp.m_post_processor_ptr, m_post_processor_ptr);
	 }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GProcessingContainerT() = default;

	 /***************************************************************************/
	 /**
	  * Perform the actual processing steps. E.g. in optimization algorithms,
	  * post-processing allows to run a sub-optimization. The amount of time
	  * needed for processing is measured for logging purposes. Where one of the
	  * processing functions throws an exception, the function will store the
	  * necessary exception information locally and rethrow the exception.
	  *
	  * @return The result of the processing calls
	  */
	 result_type process() {
		 std::ostringstream exceptions_stream;

		 // Reset the check for successful execution and exceptions. This is a new run
		 m_processing_was_successful = false;
		 m_has_exceptions = false;

		 try {
			 // Perform the actual processing
			 auto startTime = std::chrono::high_resolution_clock::now();
			 this->preProcess_();
			 auto afterPreProcessing = std::chrono::high_resolution_clock::now();
			 this->process_();
			 auto afterProcessing = std::chrono::high_resolution_clock::now();
			 this->postProcess_();
			 auto afterPostProcessing = std::chrono::high_resolution_clock::now();

			 // Make a note of the time needed for each step
			 m_pre_processing_time = std::chrono::duration<double>(afterPreProcessing - startTime).count();
			 m_processing_time = std::chrono::duration<double>(afterProcessing - afterPreProcessing).count();
			 m_post_processing_time = std::chrono::duration<double>(afterPostProcessing - afterProcessing).count();

			 m_processing_was_successful = true;
		 } catch(boost::exception& e) {
			 m_has_exceptions = true;
			 exceptions_stream
				 << "In GProcessingContainerT<>::process():" << std::endl
				 << "Processing has thrown a boost exception with message" << std::endl
				 << boost::diagnostic_information(e) << std::endl
				 << "We will rethrow this exception" << std::endl;
		 } catch(std::exception& e) {
			 m_has_exceptions = true;
			 exceptions_stream
				 << "In GProcessingContainerT<processable_type>::process():" << std::endl
				 << "Processing has thrown an exception with message" << std::endl
				 << e.what() << std::endl
				 << "We will rethrow this exception" << std::endl;
		 } catch(...) {
			 m_has_exceptions = true;
			 exceptions_stream
				 << "In GProcessingContainerT<processable_type>::process():" << std::endl
				 << "Processing has thrown an unknown exception." << std::endl;
		 }

		 if(this->hasExceptions()) {
			 // Let the audience know
			 glogger
				 << exceptions_stream.str()
				 << GWARNING;

			 // Do some cleanup
			 m_pre_processing_time = 0.;
			 m_processing_time = 0.;
			 m_post_processing_time = 0.;

			 // Mark the processing as invalid and store the exceptions for later reference
			 m_processing_was_successful = false;
			 m_stored_exceptions = exceptions_stream.str();

			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << exceptions_stream.str()
			 );
		 }

		 // This part of the code should never be reached if an exception was thrown
		 m_stored_result = this->get_processing_result();
		 return m_stored_result;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieval of the stored result
	  */
	 result_type getStoredResult() const noexcept {
		 return m_stored_result;
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
	  * @param cD_ptr A pointer to the object whose data should be loaded
	  */
	 virtual void loadConstantData(std::shared_ptr<processable_type>) BASE
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Checks if processing was successful
	  *
	  * @return A boolean indicating whether processing was successful
	  */
	 bool processing_was_successful() const noexcept {
		 return m_processing_was_successful;
	 }

	 /***************************************************************************/
	 /**
	  * Checks if processing was successful
	  *
	  * @return A boolean indicating whether processing was successful
	  */
	 bool processing_was_unsuccessful() const noexcept {
		 return !m_processing_was_successful;
	 }

	 /***************************************************************************/
	 /**
	  * Enforces processing to be marked as successful
	  */
	 void force_mark_processing_as_successful() noexcept {
		 m_processing_was_successful = true;
	 };

	 /***************************************************************************/
	 /**
	  * Enforces processing to be marked as unsuccessful
	  */
	 void force_mark_processing_as_unsuccessful() noexcept {
		 m_processing_was_successful = false;
	 };

	 /***************************************************************************/
	 /**
	  * Allows to set the counter of a given submission
	  */
	 void setSubmissionCounter(const ITERATION_COUNTER_TYPE& counter)  noexcept {
		 m_submission_counter = counter;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the counter of a given submission
	  */
	 ITERATION_COUNTER_TYPE getSubmissionCounter() const noexcept {
		 return m_submission_counter;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the position inside of a given submission
	  */
	 void setSubmissionPosition(const SUBMISSION_POSITION_TYPE& pos) noexcept {
		 m_submission_position = pos;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the position inside of a given submission
	  */
	 SUBMISSION_POSITION_TYPE getSubmissionPosition() const noexcept {
		 return m_submission_position;
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
	 BUFFERPORT_ID_TYPE getBufferId() const noexcept {
		 return m_bufferport_id;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to check whether any user-defined pre-processing before the process()-
	  * step may occur. This may alter the individual's data.
	  */
	 bool mayBePreProcessed() const noexcept {
		 return !m_preProcessingDisabled;
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
	 bool mayBePostProcessed() const {
		 return !m_postProcessingDisabled;
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
	 std::tuple<double,double,double> getProcessingTimes() const {
		 return std::make_tuple(m_pre_processing_time, m_processing_time, m_post_processing_time);
	 };

	 /***************************************************************************/
	 /**
	  * Checks whether exceptions are stored in this object
	  *
	  * @return A boolean indicating whether exceptions are stored in this class
	  */
	 bool hasExceptions() const noexcept {
		 return m_has_exceptions;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves and clears exceptions and the m_has_exceptions flag.
	  */
	 std::string get_and_clear_exceptions() {
		 std::string stored_exceptions = m_stored_exceptions;

		 m_stored_exceptions.clear();
		 m_has_exceptions = false;

		 return stored_exceptions;
	 }

	 /***************************************************************************/
	 /**
	  * Loads the data of another GProcessingContainerT<processable_type, result_type> object
	  */
	 void load_pc(const GProcessingContainerT<processable_type, result_type> *cp) {
		 // Check that we are dealing with a GProcessingContainerT<processable_type, result_type> reference independent of this object and convert the pointer
		 const GProcessingContainerT<processable_type, result_type> *p_load
			 = Gem::Common::g_convert_and_compare<GProcessingContainerT<processable_type, result_type>, GProcessingContainerT<processable_type, result_type>>(cp, this);

		 // Load local data
		 m_submission_counter = p_load->m_submission_counter;
		 m_submission_position = p_load->m_submission_position;
		 m_bufferport_id = p_load->m_bufferport_id;
		 m_preProcessingDisabled = p_load->m_preProcessingDisabled;
		 m_postProcessingDisabled = p_load->m_postProcessingDisabled;
		 m_processing_was_successful = p_load->m_processing_was_successful;
		 m_stored_result = p_load->m_stored_result;
		 m_stored_exceptions = p_load->m_stored_exceptions;
		 m_has_exceptions = p_load->m_has_exceptions;

		 Gem::Common::copyCloneableSmartPointer(p_load->m_pre_processor_ptr, m_pre_processor_ptr);
		 Gem::Common::copyCloneableSmartPointer(p_load->m_post_processor_ptr, m_post_processor_ptr);
	 }

protected:
	 /***************************************************************************/

	 /** @brief Allows derived classes to specify the tasks to be performed for this object */
	 virtual G_API_COURTIER void process_() BASE = 0;

	 /** @brief Allows derived classes to give an indication of the processing result (if any); may not throw. */
	 virtual G_API_COURTIER result_type get_processing_result() const noexcept BASE = 0;

private:
	 /***************************************************************************/
	 /**
	  * Specifies tasks to be performed before the process_ call. Note: This function
	  * will reset the m_mayBePreProcessed-flag.
  	  */
	 void preProcess_() {
		 if(this->mayBePreProcessed() && m_pre_processor_ptr) {
			 processable_type& p = dynamic_cast<processable_type&>(*this);
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
			 processable_type& p = dynamic_cast<processable_type&>(*this);
			 (*m_post_processor_ptr)(p);
		 }
	 }

	 /***************************************************************************/
	 // Data

	 ITERATION_COUNTER_TYPE m_submission_counter = 0;
	 SUBMISSION_POSITION_TYPE m_submission_position = 0;
	 BUFFERPORT_ID_TYPE m_bufferport_id = BUFFERPORT_ID_TYPE();

	 bool m_preProcessingDisabled = false; ///< Indicates whether pre-processing was diabled entirely
	 bool m_postProcessingDisabled = false; ///< Indicates whether pre-processing was diabled entirely

	 std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<processable_type>> m_pre_processor_ptr; ///< Actions to be performed before processing
	 std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<processable_type>> m_post_processor_ptr; ///< Actions to be performed after processing

	 double m_pre_processing_time = 0.; ///< The amount of time needed for pre-processing (in seconds)
	 double m_processing_time = 0.; ///< The amount of time needed for the actual processing step (in seconds)
	 double m_post_processing_time = 0.; ///< The amount of time needed for post-processing (in seconds)

	 bool m_processing_was_successful = false; ///< Indicates whether an error has occurred during processing

	 result_type m_stored_result = result_type(0); ///< Buffers results of the process() call

	 std::string m_stored_exceptions = ""; ///< Stores exceptions that may have occurred during processing
	 bool m_has_exceptions = false; ///< Indicates whether the object holds exception data
};

/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

/******************************************************************************/
/** @brief Mark this class as abstract */
namespace boost {
namespace serialization {
template<typename processable_type, typename result_type>
struct is_abstract<Gem::Courtier::GProcessingContainerT<processable_type, result_type>> : public boost::true_type {
};
template<typename processable_type, typename result_type>
struct is_abstract<const Gem::Courtier::GProcessingContainerT<processable_type, result_type>> : public boost::true_type {
};
}
}

/******************************************************************************/

#endif /* GSUBMISSIONCONTAINERBASE_HPP_ */
