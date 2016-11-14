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

#ifndef GSUBMISSIONCONTAINERBASE_HPP_
#define GSUBMISSIONCONTAINERBASE_HPP_

// Geneva headers go here
#include "common/GSerializeTupleT.hpp"
#include "common/GSerializableFunctionObjectT.hpp"
#include "courtier/GCourtierEnums.hpp"

namespace Gem {
namespace Courtier {

/******************************************************************************/
/**
 * This class can serve as a base class for items to be submitted through the broker. You need to
 * re-implement the purely virtual functions in derived classes. Note that it is mandatory for
 * derived classes to be serializable and to trigger serialization of this class.
 */
template<typename submission_type>
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
		 & BOOST_SERIALIZATION_NVP(m_pre_processing_time)
		 & BOOST_SERIALIZATION_NVP(m_processing_time)
		 & BOOST_SERIALIZATION_NVP(m_post_processing_time);
	 }

	 ///////////////////////////////////////////////////////////////////////

public:
	 using payload_type = submission_type;

	 /***************************************************************************/
	 /**
	  * The default constructor
	  */
	 GProcessingContainerT()
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The copy constructor
	  *
	  * @param cp A copy of another GSubmissionContainer object
	  */
	 GProcessingContainerT(const GProcessingContainerT<submission_type> &cp)
		 : m_submission_counter(cp.m_submission_counter)
		 , m_submission_position(cp.m_submission_position)
		 , m_bufferport_id(cp.m_bufferport_id)
	 	 , m_preProcessingDisabled(cp.m_preProcessingDisabled)
 		 , m_postProcessingDisabled(cp.m_postProcessingDisabled)
	 {
		 Gem::Common::copyCloneableSmartPointer(cp.m_pre_processor_ptr, m_pre_processor_ptr);
		 Gem::Common::copyCloneableSmartPointer(cp.m_post_processor_ptr, m_post_processor_ptr);
	 }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GProcessingContainerT()
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Perform the actual processing steps. E.g. in optimization algorithms,
	  * post-processing allows to run a sub-optimization. The amount of time
	  * needed for processing is done for logging purposes.
	  *
	  * TODO: Try to continue in case of exceptions ?
	  */
	 bool process() {
		 try {
			 auto startTime = std::chrono::high_resolution_clock::now();
			 if (!this->preProcess_()) return false;
			 auto afterPreProcessing = std::chrono::high_resolution_clock::now();
			 if (!this->process_()) return false;
			 auto afterProcessing = std::chrono::high_resolution_clock::now();
			 if (!this->postProcess_()) return false;
			 auto afterPostProcessing = std::chrono::high_resolution_clock::now();

			 // Make a not of the time needed for each step
			 m_pre_processing_time = std::chrono::duration<double>(afterPreProcessing - startTime).count();
			 m_processing_time = std::chrono::duration<double>(afterProcessing - afterPreProcessing).count();
			 m_post_processing_time = std::chrono::duration<double>(afterPostProcessing - afterProcessing).count();
		 } catch(Gem::Common::gemfony_error_condition& e) {
			 glogger
				 << "In GProcessingContainerT<>::process(): Caught Gem::Common::gemfony_error_condition with message" << std::endl
				 << e.what() << std::endl
				 << GTERMINATION;
		 } catch(boost::exception&){
			 glogger
				 << "In GProcessingContainerT<>::process(): Caught boost::exception with message" << std::endl
				 << GTERMINATION;
		 } catch(std::exception& e){
			 glogger
				 << "In GProcessingContainerT<>::process(): Caught std::exception with message" << std::endl
				 << e.what() << std::endl
				 << GTERMINATION;
		 } catch(...){
			 glogger
				 << "GProcessingContainerT<>::process(): Caught unknown exception" << std::endl
				 << GTERMINATION;
		 }

		 return true;
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
	 virtual void loadConstantData(std::shared_ptr<submission_type>) BASE
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Allows to set the counter of a given submission
	  */
	 void setSubmissionCounter(const SUBMISSION_COUNTER_TYPE& counter) {
		 m_submission_counter = counter;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the counter of a given submission
	  */
	 SUBMISSION_COUNTER_TYPE getSubmissionCounter() const {
		 return m_submission_counter;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the position inside of a given submission
	  */
	 void setSubmissionPosition(const SUBMISSION_POSITION_TYPE& pos) {
		 m_submission_position = pos;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the position inside of a given submission
	  */
	 SUBMISSION_POSITION_TYPE getSubmissionPosition() const {
		 return m_submission_position;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the id inside of the originating buffer
	  */
	 void setBufferId(const BUFFERPORT_ID_TYPE& id) {
		 m_bufferport_id = id;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the id of the originating buffer
	  */
	 BUFFERPORT_ID_TYPE getBufferId() const {
		 return m_bufferport_id;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to check whether any user-defined pre-processing before the process()-
	  * step may occur. This may alter the individual's data.
	  */
	 bool mayBePreProcessed() const {
		 return !m_preProcessingDisabled;
	 }

	 /***************************************************************************/
	 /**
	  * Allow or prevent pre-processing (used by pre-processing algorithms to prevent
	  * recursive pre-processing). See e.g. GEvolutionaryAlgorithmPostOptimizerT. Once a veto
	  * exists, no pre-processing will occur until the veto is lifted.
	  */
	 void vetoPreProcessing(bool veto) {
		 m_preProcessingDisabled = veto;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to register a pre-processor object
	  */
	 void registerPreProcessor(std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<submission_type>> pre_processor_ptr) {
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
	 void registerPostProcessor(std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<submission_type>> post_processor_ptr) {
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
	  * Loads the data of another GProcessingContainerT<submission_type> object
	  */
	 void load_pc(const GProcessingContainerT<submission_type> *cp) {
		 // Check that we are dealing with a GProcessingContainerT<submission_type> reference independent of this object and convert the pointer
		 const GProcessingContainerT<submission_type> *p_load = Gem::Common::g_convert_and_compare<GProcessingContainerT<submission_type>, GProcessingContainerT<submission_type>>(cp, this);

		 // Load local data
		 m_submission_counter = p_load->m_submission_counter;
		 m_submission_position = p_load->m_submission_position;
		 m_bufferport_id = p_load->m_bufferport_id;
		 m_preProcessingDisabled = p_load->m_preProcessingDisabled;
		 m_postProcessingDisabled = p_load->m_postProcessingDisabled;
		 Gem::Common::copyCloneableSmartPointer(p_load->m_pre_processor_ptr, m_pre_processor_ptr);
		 Gem::Common::copyCloneableSmartPointer(p_load->m_post_processor_ptr, m_post_processor_ptr);
	 }

protected:
	 /***************************************************************************/
	 /** @brief Allows derived classes to specify the tasks to be performed for this object */
	 virtual G_API_COURTIER bool process_() BASE = 0;

private:
	 /***************************************************************************/
	 /**
	  * Specifies tasks to be performed before the process_ call. Note: This function
	  * will reset the m_mayBePreProcessed-flag.
  	  */
	 bool preProcess_() {
		 bool result = true;

		 if(this->mayBePreProcessed() && m_pre_processor_ptr) {
			 submission_type& p = dynamic_cast<submission_type&>(*this);
			 result = (*m_pre_processor_ptr)(p);
		 }

		 return result;
	 }

	 /***************************************************************************/
	 /**
	  * Specifies tasks to be performed after the process_ call. Note: This function
	  * will reset the m_mayBePostProcessed-flag.
  	  */
	 bool postProcess_() {
		 bool result = true;

		 if(this->mayBePostProcessed() && m_post_processor_ptr) {
			 submission_type& p = dynamic_cast<submission_type&>(*this);
			 result = (*m_post_processor_ptr)(p);
		 }

		 return result;
	 }

	 /***************************************************************************/
	 // Data

	 SUBMISSION_COUNTER_TYPE  m_submission_counter = 0;
	 SUBMISSION_POSITION_TYPE m_submission_position = 0;
	 BUFFERPORT_ID_TYPE m_bufferport_id = BUFFERPORT_ID_TYPE();

	 bool m_preProcessingDisabled = false; ///< Indicates whether pre-processing was diabled entirely
	 bool m_postProcessingDisabled = false; ///< Indicates whether pre-processing was diabled entirely

	 std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<submission_type>> m_pre_processor_ptr; ///< Actions to be performed before processing
	 std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<submission_type>> m_post_processor_ptr; ///< Actions to be performed after processing

	 double m_pre_processing_time = 0.; ///< The amount of time needed for pre-processing (in seconds)
	 double m_processing_time = 0.; ///< The amount of time needed for the actual processing step (in seconds)
	 double m_post_processing_time = 0.; ///< The amount of time needed for post-processing (in seconds)
};

/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

/******************************************************************************/
/** @brief Mark this class as abstract */
namespace boost {
namespace serialization {
template<typename submission_type>
struct is_abstract<Gem::Courtier::GProcessingContainerT<submission_type>> : public boost::true_type {
};
template<typename submission_type>
struct is_abstract<const Gem::Courtier::GProcessingContainerT<submission_type>> : public boost::true_type {
};
}
}

/******************************************************************************/

#endif /* GSUBMISSIONCONTAINERBASE_HPP_ */
