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

#ifndef GSUBMISSIONCONTAINERBASE_HPP_
#define GSUBMISSIONCONTAINERBASE_HPP_

// Geneva headers go here
#include "common/GSerializeTupleT.hpp"
#include "common/GSerializableFunctionObjectT.hpp"
#include "common/GCommonInterfaceT.hpp"
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
	: public Gem::Common::GCommonInterfaceT<GProcessingContainerT<submission_type>>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GCommonInterfaceT_GProcessingContainerT_T", boost::serialization::base_object<Gem::Common::GCommonInterfaceT<GProcessingContainerT<submission_type>>>(*this))
		 & BOOST_SERIALIZATION_NVP(m_id)
		 & BOOST_SERIALIZATION_NVP(m_mayBePreProcessed)
		 & BOOST_SERIALIZATION_NVP(m_mayBePostProcessed)
		 & BOOST_SERIALIZATION_NVP(m_pre_processor_ptr)
		 & BOOST_SERIALIZATION_NVP(m_post_processor_ptr);
	 }

	 ///////////////////////////////////////////////////////////////////////

public:
	 typedef submission_type payload_type;

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
		 : m_id(cp.m_id)
	 	 , m_mayBePreProcessed(cp.m_mayBePreProcessed)
	 	 , m_mayBePostProcessed(cp.m_mayBePostProcessed)
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
	  * Checks for equality with another GProcessingContainerT<submission_type> object
	  *
	  * @param  cp A constant reference to another GProcessingContainerT<submission_type> object
	  * @return A boolean indicating whether both objects are equal
	  */
	 bool operator==(const GProcessingContainerT<submission_type>& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			 return true;
		 } catch(g_expectation_violation&) {
			 return false;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Checks for inequality with another GProcessingContainerT<submission_type> object
	  *
	  * @param  cp A constant reference to another GProcessingContainerT<submission_type> object
	  * @return A boolean indicating whether both objects are inequal
	  */
	 bool operator!=(const GProcessingContainerT<submission_type>& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			 return true;
		 } catch(g_expectation_violation&) {
			 return false;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Returns the name of this class
	  */
	 virtual std::string name() const override {
		 return std::string("GProcessingContainerT<submission_type>");
	 }

	 /***************************************************************************/
	 /**
	  * Checks for compliance with expectations with respect to another object
	  * of the same type
	  *
	  * @param cp A constant reference to another GProcessingContainerT<submission_type> object
	  * @param e The expected outcome of the comparison
	  * @param limit The maximum deviation for floating point values (important for similarity checks)
	  */
	 virtual void compare(
		 const GProcessingContainerT<submission_type> &cp
		 , const Gem::Common::expectation &e
		 , const double &limit
	 ) const override {
		 using namespace Gem::Common;

		 // Check that we are dealing with a GProcessingContainerT<submission_type> reference independent of this object and convert the pointer
		 const GProcessingContainerT<submission_type> *p_load
			 = Gem::Common::g_convert_and_compare<GProcessingContainerT<submission_type>, GProcessingContainerT<submission_type>>(cp, this);

		 GToken token("GProcessingContainerT<submission_type>", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GCommonInterfaceT<GProcessingContainerT<submission_type>>>(IDENTITY(*this, *p_load), token);

		 // ... and then our local data
		 compare_t(IDENTITY(m_id, p_load->m_id), token);
		 compare_t(IDENTITY(m_mayBePreProcessed, p_load->m_mayBePreProcessed), token);
		 compare_t(IDENTITY(m_mayBePostProcessed, p_load->m_mayBePostProcessed), token);
		 compare_t(IDENTITY(m_pre_processor_ptr, p_load->m_pre_processor_ptr), token);
		 compare_t(IDENTITY(m_post_processor_ptr, p_load->m_post_processor_ptr), token);

		 // React on deviations from the expectation
		 token.evaluate();
	 }

	 /***************************************************************************/
	 /**
	  * Allows derived classes to specify the tasks to be performed for this object
	  */
	 bool process() {
		 try {
			 if (this->mayBePreProcessed() && !this->preProcess_()) return false;
			 if (!this->process_()) return false;
			 if (this->mayBePostProcessed() && !this->postProcess_()) return false;
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
	  * Allows the courtier library to associate an id with the container
	  *
	  * @param id An id that allows the broker connector to identify this object
	  */
	 void setCourtierId(const std::tuple<Gem::Courtier::ID_TYPE_1, Gem::Courtier::ID_TYPE_2> &id) {
		 m_id = id;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the courtier-id associated with this container
	  *
	  * @return An id that allows the broker connector to identify this object
	  */
	 std::tuple<Gem::Courtier::ID_TYPE_1, Gem::Courtier::ID_TYPE_2> getCourtierId() const {
		 return m_id;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to check whether any user-defined pre-processing before the process()-
	  * step may occur. This may alter the individual's data.
	  */
	 bool mayBePreProcessed() const {
		 return m_mayBePreProcessed;
	 }

	 /***************************************************************************/
	 /**
	  * Calling this function will enable pre-processing of this work item
	  * a single time. It will usually be set upon submitting a work item to the broker.
	  * The flag will be reset once pre-processing has been done. Permission needs
	  * to be set upon every submission.
	  */
	 void allowPreProcessing() {
		 m_mayBePreProcessed = true;
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
		 return m_mayBePostProcessed;
	 }

	 /***************************************************************************/
	 /**
	  * Calling this function will enable postprocessing of this work item
	  * a single time. It will usually be set upon submitting a work item to the broker.
	  * The flag will be reset once post-processing has been done. Permission needs
	  * to be set upon every submission.
	  */
	 void allowPostProcessing() {
		 m_mayBePostProcessed = true;
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

protected:
	 /***************************************************************************/
	 /**
	  * Loads the data of another GProcessingContainerT<submission_type> object
	  */
	 virtual void load_(const GProcessingContainerT<submission_type> *cp) override {
		 // Check that we are dealing with a GProcessingContainerT<submission_type> reference independent of this object and convert the pointer
		 const GProcessingContainerT<submission_type> *p_load = Gem::Common::g_convert_and_compare<GProcessingContainerT<submission_type>, GProcessingContainerT<submission_type>>(cp, this);

		 // Load local data
		 m_id = p_load->m_id;
		 m_mayBePreProcessed = p_load->m_mayBePreProcessed;
		 m_mayBePostProcessed = p_load->m_mayBePostProcessed;
		 Gem::Common::copyCloneableSmartPointer(p_load->m_pre_processor_ptr, m_pre_processor_ptr);
		 Gem::Common::copyCloneableSmartPointer(p_load->m_post_processor_ptr, m_post_processor_ptr);
	 }

	 /***************************************************************************/
	 /** @brief Creates a deep clone of this object */
	 virtual G_API_COURTIER GProcessingContainerT<submission_type> * clone_() const override = 0;
	 /** @brief Allows derived classes to specify the tasks to be performed for this object */
	 virtual G_API_COURTIER bool process_() BASE = 0;

private:
	 /***************************************************************************/
	 /**
	  * Specifies tasks to be performed before the process_ call. Note: This function
	  * will reset the m_mayBePreProcessed-flag.
  	  */
	 bool preProcess_() {
		 if(m_mayBePreProcessed && m_pre_processor_ptr) {
			 submission_type& p = dynamic_cast<submission_type&>(*this);
			 (*m_pre_processor_ptr)(p);
			 m_mayBePreProcessed = false;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Specifies tasks to be performed after the process_ call. Note: This function
	  * will reset the m_mayBePostProcessed-flag.
  	  */
	 bool postProcess_() {
		 if(m_mayBePostProcessed && m_post_processor_ptr) {
			 submission_type& p = dynamic_cast<submission_type&>(*this);
			 (*m_post_processor_ptr)(p);
			 m_mayBePostProcessed = false;
		 }
	 }

	 /***************************************************************************/
	 // Data

	 std::tuple<Gem::Courtier::ID_TYPE_1, Gem::Courtier::ID_TYPE_2> m_id; ///< A two-part id that can be assigned to this container object

	 bool m_mayBePreProcessed  = false; ///< Indicates whether user-defined pre-processing may occur
	 bool m_mayBePostProcessed = false; ///< Indicates whether user-defined post-processing may occur

	 std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<submission_type>> m_pre_processor_ptr; ///< Actions to be performed before processing
	 std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<submission_type>> m_post_processor_ptr; ///< Actions to be performed after processing
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
