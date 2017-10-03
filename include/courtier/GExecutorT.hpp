/**
 * @file GExecutorT.hpp
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

// Boost headers go here
#include <boost/lexical_cast.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>

#ifndef GEXECUTOR_HPP_
#define GEXECUTOR_HPP_


// Geneva headers go here
#include "common/GCommonInterfaceT.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GMathHelperFunctionsT.hpp"
#include "common/GHelperFunctionsT.hpp"
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

/**
 * Invariants of this hierarchy:
 * - for consumers indicating that they are capable of full return, the process()
 *   function must always return a valid result (even if the user-code crashes)
 */

// TODO: Even in the case "capable of full return" we need to take into account
// that work items may return unprocessed. E.g. the processing code itself
// might throw.
// TODO: Take care of marking some classes abstract
// TODO: Take care of making some of theses classes serializable
// TODO: In GOptimizationAlgorithmT2(cpp/hpp): serialize-exports instantiations with processable_type == GParameterSet
// TODO: Some variables are still not listet in all Gemfony-API functions

/******************************************************************************/
/**
 * This class centralizes some functionality and data that is needed to perform
 * serial or parallel execution for a set of work items. Its main purpose is to
 * avoid duplication of code. Derived classes may deal with different types of
 * parallel execution, including connection to a broker and multi-threaded
 * execution. The serial mode is meant for debugging purposes only. The class
 * follows the Gemfony conventions for loading / cloning and comparison with
 * other objects.
 */
template<typename processable_type>
class GBaseExecutorT
	: public Gem::Common::GCommonInterfaceT<GBaseExecutorT<processable_type>>
{
	 // Make sure processable_type adheres to the GProcessingContainerT interface
	 static_assert(
		 std::is_base_of<Gem::Courtier::GProcessingContainerT<processable_type>, processable_type>::value
		 , "GBaseExecutorT: processable_type does not adhere to the GProcessingContainerT interface"
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
	 /**
	  * The default constructor
	  */
	 GBaseExecutorT()
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The copy constructor. No local data to be copied.
	  * m_submission_counter is just a temporary which always
	  * starts counting at 0.
	  *
	  * @param cp A copy of another GBrokerConnector object
	  */
	 GBaseExecutorT(const GBaseExecutorT<processable_type> &cp)
	 	: Gem::Common::GCommonInterfaceT<GBaseExecutorT<processable_type>>(cp)
	   , m_maxResubmissions(cp.m_maxResubmissions)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The standard destructor. We have no object-wide dynamically allocated data, hence
	  * this function is empty.
	  */
	 virtual ~GBaseExecutorT()
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Checks for equality with another GBaseExecutorT<processable_type> object
	  *
	  * @param  cp A constant reference to another GBaseExecutorT<processable_type> object
	  * @return A boolean indicating whether both objects are equal
	  */
	 bool operator==(const GBaseExecutorT<processable_type>& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			 return true;
		 } catch (g_expectation_violation &) {
			 return false;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Checks for inequality with another GBaseExecutorT<processable_type> object
	  *
	  * @param  cp A constant reference to another GBaseExecutorT<processable_type> object
	  * @return A boolean indicating whether both objects are inequal
	  */
	 bool operator!=(const GBaseExecutorT<processable_type>& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			 return true;
		 } catch (g_expectation_violation &) {
			 return false;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Returns the name of this class
	  */
	 virtual std::string name() const override {
		 return std::string("GBaseExecutorT<processable_type>");
	 }

	 /***************************************************************************/
	 /**
	  * A standard assignment operator for GBaseExecutorT<processable_type> objects,
	  *
	  * @param cp A copy of another GBaseExecutorT<processable_type> object
	  * @return A constant reference to this object
	  */
	 const GBaseExecutorT<processable_type> &operator=(const GBaseExecutorT<processable_type> &cp) {
		 this->load_(&cp);
		 return *this;
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
	  * Submits and retrieves a set of work items. You need to supply a vector
	  * of booleans of the same length indicating which items need to be submitted.
	  * "true" stands for "submit", "false" leads to the corresponding work items
	  * being ignored. After the function returns, some or all of the work items
	  * will have been processed. You can find out about this by querying the workItemPos
	  * vector. Item positions that have been processed will be set to "false".
	  * Positions remaining "true" have not been processed (but might still return in
	  * later iterations). It is thus also possible that returned items do not belong
	  * to the current submission cycle. They will be appended to the oldWorkItems vector.
	  * You might thus have to post-process the work items. Note that it is impossible
	  * to submit items that are not derived from GProcessingContainerT<processable_type>.
	  * This function will not alter the size of the workItems vector. It does not
	  * guarantee that all work items have indeed been processed. You can find out
	  * via the workItemPos vector, which items were processed.
	  *
	  * TODO: Catch exceptions in this loop or in the entire function
	  *
	  * @param workItems A vector with work items to be evaluated beyond the broker
	  * @param workItemPos A vector of the item positions to be worked on
	  * @param oldWorkItems Will hold old work items after the job submission
	  * @param resubmitUnprocessed Indicates whether unprocessed items should be resubmitted
	  * @param caller Optionally holds information on the caller
	  * @return A boolean indicating whether all expected items have returned
	  */
	 bool workOn(
		 std::vector<std::shared_ptr<processable_type>>& workItems
		 , std::vector<bool>& workItemPos
		 , std::vector<std::shared_ptr<processable_type>>& oldWorkItems
		 , bool resubmitUnprocessed = false
		 , const std::string &caller = std::string()
	 ) {
		 bool completed = false;

		 // Set the start time of the new iteration
		 m_submissionStartTime = std::chrono::system_clock::now();

		 // Check that both vectors have the same size
		 Gem::Common::assert_container_sizes_match(
			 workItems
			 , workItemPos
			 , "GBaseExecutorT<processable_type>::workOn()"
		 );

		 // The expected number of work items from the current iteration is
		 // equal to the number of unprocessed items
		 m_expectedNumber = boost::numeric_cast<std::size_t>(std::count(workItemPos.begin(), workItemPos.end(), GBC_UNPROCESSED));
		 // Take care of a situation where no items have been submitted
		 if (m_expectedNumber == 0) {
			 return true;
		 }

		 // Make sure the vector of old work items is empty
		 oldWorkItems.clear();

		 // Perform necessary setup work for an iteration (a facility for derived classes)
		 this->iterationInit(workItems, workItemPos, oldWorkItems);

		 std::size_t nResubmissions = 0;
		 do {
			 // Submit all work items.
			 this->submitAllWorkItems(
				 workItems
				 , workItemPos
			 );

			 // Wait for work items to complete. This function needs to
			 // be re-implemented in derived classes.
			 completed = waitForReturn(
				 workItems
				 , workItemPos
				 , oldWorkItems
			 );

			 // We may leave if all items have returned
			 if(completed) break;

			 // We may leave if the user hasn't asked to resubmit unprocessed items
			 if(!resubmitUnprocessed) break;
		 } while(m_maxResubmissions>0?(++nResubmissions < m_maxResubmissions):true);
		 // Note: m_maxResubmissions will result in an endless loop of resubmissions

		 // Perform necessary cleanup work for an iteration (a facility for derived classes)
		 this->iterationFinalize(workItems, workItemPos, oldWorkItems);

		 // Find out about the number of returned items
		 m_n_notReturnedLast = boost::numeric_cast<std::size_t>(std::count(workItemPos.begin(), workItemPos.end(), GBC_UNPROCESSED));
		 m_n_returnedLast    = m_expectedNumber - m_n_notReturnedLast;
		 m_n_oldWorkItems    = oldWorkItems.size();

		 // Sort old work items according to their ids so they can be readily used by the caller
		 std::sort(
			 oldWorkItems.begin()
			 , oldWorkItems.end()
			 , [](std::shared_ptr<processable_type> x_ptr, std::shared_ptr<processable_type> y_ptr) -> bool {
				 using namespace boost;
				 return x_ptr->getSubmissionPosition() < y_ptr->getSubmissionPosition();
			 }
		 );

		 // Give feedback to the audience (may be overloaded in derived classes)
		 this->visualize_performance();

		 // Update the iteration counter
		 m_iteration_counter++;

		 // Note: unprocessed items will be returned to the caller, which needs to deal with them
		 return completed;
	 }

	 /***************************************************************************/
	 /**
	  * Gives access to the value of the current submission id
	  */
	 ITERATION_COUNTER_TYPE getSubmissionId() const {
		 return m_iteration_counter;
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
	  * General initialization function to be called prior to the first submission
	  */
	 virtual void init() BASE { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * General finalization function to be called after the last submission
	  */
	 virtual void finalize() BASE { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Retrieve the number of individuals returned during the last iteration
	  */
	 std::size_t getNReturned() const {
		 return m_n_returnedLast;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieve the number of individuals NOT returned during the last iteration
	  */
	 std::size_t getNNotReturned() const {
		 return m_n_notReturnedLast;
	 }

	 /***************************************************************************/
	 /**
 	  * Retrieves the current number of old work items in this iteration
 	  */
	 std::size_t getNOldWorkItems() const {
		 return m_n_oldWorkItems;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the current submission id
	  */
	 ITERATION_COUNTER_TYPE getCurrentSubmissionId() const {
		 return m_iteration_counter;
	 }

	 /***************************************************************************/
	 /**
	  * Searches for compliance with expectations with respect to another object
	  * of the same type
	  */
	 virtual void compare(
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

	 /***************************************************************************/

	 virtual void visualize_performance() BASE { /* nothing */ }

protected:
	 /***************************************************************************/
	 /**
	  * Loads the data of another object
	  */
	 virtual void load_(const GBaseExecutorT<processable_type>* cp) override {
		 // Check that we are dealing with a GDecorator reference independent of this object and convert the pointer
		 const GBaseExecutorT<processable_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		 // No parent class with loadable data

		 // Copy local data
		 m_maxResubmissions = p_load->m_maxResubmissions;
	 }

	 /***************************************************************************/
	 /** @brief Submits a single work item */
	 virtual void submit(std::shared_ptr<processable_type>) = 0;

	 /** @brief Waits for work items to return */
	 virtual bool waitForReturn(
		 std::vector<std::shared_ptr<processable_type>>&
		 , std::vector<bool>&
		 , std::vector<std::shared_ptr<processable_type>>&
	 ) = 0;

	 /***************************************************************************/
	 /**
	  * Allow to perform necessary setup work for an iteration. Derived classes
	  * should make sure this base function is called first when they overload
	  * this function.
	  */
	 virtual void iterationInit(
		 std::vector<std::shared_ptr<processable_type>>& workItems
		 , std::vector<bool> &workItemPos
		 , std::vector<std::shared_ptr<processable_type>>& oldWorkItems
	 ) BASE { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Allows to perform necessary cleanup work for an iteration. Derived classes
	  * should make sure this base function is called last when they overload
	  * this function.
	  */
	 virtual void iterationFinalize(
		 std::vector<std::shared_ptr<processable_type>>& workItems
		 , std::vector<bool> &workItemPos
		 , std::vector<std::shared_ptr<processable_type>>& oldWorkItems
	 ) BASE { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Submission of all work items in the list
	  *
	  * TODO: Take care of situations where submission may block
	  */
	 void submitAllWorkItems(
		 std::vector<std::shared_ptr<processable_type>>& workItems
		 , std::vector<bool> &workItemPos
	 ) {
		 // Submit work items
		 SUBMISSION_POSITION_TYPE pos_cnt = 0;
		 for(auto w_ptr: workItems) { // std::shared_ptr may be copied
			 if (GBC_UNPROCESSED == workItemPos[pos_cnt]) { // Is the item due to be submitted ? We only submit items that are marked as "unprocessed"
#ifdef DEBUG
				 if(!w_ptr) {
					 glogger
						 << "In GBaseExecutorT<processable_type>::submitAllWorkItems(): Error" << std::endl
						 << "Received empty work item in position "  << pos_cnt << std::endl
						 << "m_iteration_counter = " << m_iteration_counter << std::endl
						 << GEXCEPTION;
				 }
#endif

				 w_ptr->setSubmissionCounter(m_iteration_counter);
				 w_ptr->setSubmissionPosition(pos_cnt);

				 this->submit(w_ptr);
			 }
			 pos_cnt++;
		 }
	 }

	 /***************************************************************************/
	 // Local data
	 ITERATION_COUNTER_TYPE m_iteration_counter = ITERATION_COUNTER_TYPE(0); ///< Counts the number of submissions initiated for this object. Note: not serialized!

	 std::size_t m_expectedNumber = 0; ///< The number of work items to be submitted (and expected back)
	 std::chrono::system_clock::time_point m_submissionStartTime = std::chrono::system_clock::now(); ///< Temporary that holds the start time for the retrieval of items in a given iteration

	 std::size_t m_maxResubmissions = DEFAULTMAXRESUBMISSIONS; ///< The maximum number of re-submissions allowed if a full return of submitted items is attempted

	 std::size_t m_n_returnedLast = 0; ///< The number of individuals returned in the last iteration cycle
	 std::size_t m_n_notReturnedLast = 0; ///< The number of individuals MOT returned in the last iteration cycle
	 std::size_t m_n_oldWorkItems = 0; ///< The number of old work items returned in a given iteration
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
	 /**
	  * The default constructor
	  */
	 GSerialExecutorT()
		 : GBaseExecutorT<processable_type>()
	 { /* nothing */ }

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
	 /**
	  * The destructor
	  */
	 virtual ~GSerialExecutorT()
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Checks for equality with another GSerialExecutorT<processable_type> object
	  *
	  * @param  cp A constant reference to another GSerialExecutorT<processable_type> object
	  * @return A boolean indicating whether both objects are equal
	  */
	 bool operator==(const GSerialExecutorT<processable_type>& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			 return true;
		 } catch (g_expectation_violation &) {
			 return false;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Checks for inequality with another GSerialExecutorT<processable_type> object
	  *
	  * @param  cp A constant reference to another GSerialExecutorT<processable_type> object
	  * @return A boolean indicating whether both objects are inequal
	  */
	 bool operator!=(const GSerialExecutorT<processable_type>& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			 return true;
		 } catch (g_expectation_violation &) {
			 return false;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Returns the name of this class
	  */
	 virtual std::string name() const override {
		 return std::string("GSerialExecutorT<processable_type>");
	 }

	 /***************************************************************************/
	 /**
	  * A standard assignment operator for GSerialExecutorT<processable_type> objects,
	  *
	  * @param cp A copy of another GSerialExecutorT<processable_type> object
	  * @return A constant reference to this object
	  */
	 const GSerialExecutorT<processable_type> &operator=(const GSerialExecutorT<processable_type> &cp) {
		 GSerialExecutorT<processable_type>::load_(&cp);
		 return *this;
	 }

	 /***************************************************************************/
	 /**
	  * Adds local configuration options to a GParserBuilder object
	  *
	  * @param gpb The GParserBuilder object to which configuration options should be added
	  */
	 virtual void addConfigurationOptions(
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
	 virtual void compare(
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
	 virtual void load_(const GBaseExecutorT<processable_type> *cp) override {
		 const GSerialExecutorT<processable_type> *p_load = dynamic_cast<GSerialExecutorT<processable_type> const *const>(cp);

		 if (!cp) { // nullptr
			 glogger
				 << "In GSerialExecutorT<processable_type>::load_(): Conversion error!" << std::endl
				 << GEXCEPTION;
		 }

		 // Load our parent classes data
		 GBaseExecutorT<processable_type>::load_(p_load);
	 }

	 /***************************************************************************/
	 /**
	  * Creates a deep clone of this object.
	  */
	 virtual GBaseExecutorT<processable_type>* clone_() const override {
		 return new GSerialExecutorT<processable_type>(*this);
	 }

	 /***************************************************************************/
	 /**
	  * Submits a single work item. In the case of serial execution, all work
	  * is done inside of this function. We rely on the process() function which
	  * is guaranteed to be part of the processable_type interface (note that
	  * our base class stipulates that processable_type is a derivative of
	  * GProcessingContainerT<processable_type> .
	  *
	  * @param w The work item to be processed
	  */
	 virtual void submit(std::shared_ptr <processable_type> w) override {
		 w->process();
	 }

	 /***************************************************************************/
	 /**
	  * Waits for work items to return. Mostlty empty, as all work is done inside
	  * of the submit() function.
	  */
	 virtual bool waitForReturn(
		 std::vector<std::shared_ptr < processable_type>>& workItems
		 , std::vector<bool> &workItemPos
		 , std::vector<std::shared_ptr<processable_type>>& oldWorkItems
	 ) override {
		 // Mark all positions as "processed"
		 for(auto item: workItemPos) {
			 item = GBC_PROCESSED;
		 }

		 return true;
	 }
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
	  * The default constructor
	  */
	 GMTExecutorT()
		 : GBaseExecutorT<processable_type>()
	 	 , m_n_threads(boost::numeric_cast<std::uint16_t>(Gem::Common::getNHardwareThreads()))
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initialization with the number of threads
	  */
	 GMTExecutorT(std::uint16_t nThreads)
		 : GBaseExecutorT<processable_type>()
		 , m_n_threads(nThreads)
	 { /* nothing */ }

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
	 /**
	  * The destructor
	  */
	 virtual ~GMTExecutorT()
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Checks for equality with another GMTExecutorT<processable_type> object
	  *
	  * @param  cp A constant reference to another GMTExecutorT<processable_type> object
	  * @return A boolean indicating whether both objects are equal
	  */
	 bool operator==(const GMTExecutorT<processable_type>& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			 return true;
		 } catch (g_expectation_violation &) {
			 return false;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Checks for inequality with another GMTExecutorT<processable_type> object
	  *
	  * @param  cp A constant reference to another GMTExecutorT<processable_type> object
	  * @return A boolean indicating whether both objects are inequal
	  */
	 bool operator!=(const GMTExecutorT<processable_type>& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			 return true;
		 } catch (g_expectation_violation &) {
			 return false;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Returns the name of this class
	  */
	 virtual std::string name() const override {
		 return std::string("GMTExecutorT<processable_type>");
	 }

	 /***************************************************************************/
	 /**
	  * A standard assignment operator for GMTExecutorT<processable_type> objects,
	  *
	  * @param cp A copy of another GMTExecutorT<processable_type> object
	  * @return A constant reference to this object
	  */
	 const GMTExecutorT<processable_type> &operator=(const GMTExecutorT<processable_type> &cp) {
		 GMTExecutorT<processable_type>::load_(&cp);
		 return *this;
	 }

	 /***************************************************************************/
	 /**
	  * Adds local configuration options to a GParserBuilder object
	  *
	  * @param gpb The GParserBuilder object to which configuration options should be added
	  */
	 virtual void addConfigurationOptions(
		 Gem::Common::GParserBuilder &gpb
	 ) override {
		 // Call our parent class's function
		 GBaseExecutorT<processable_type>::addConfigurationOptions(gpb);

		 // No local data
	 }

	 /***************************************************************************/
	 /**
	  * Sets the number of threads for the thread pool. If nThreads is set
	  * to 0, an attempt will be made to set the number of threads to the
	  * number of hardware threading units (e.g. number of cores or hyperthreading
	  * units).
	  *
	  * @param nThreads The number of threads the threadpool should use
	  */
	 void setNThreads(std::uint16_t nThreads) {
		 if (nThreads == 0) {
			 m_n_threads = boost::numeric_cast<std::uint16_t>(Gem::Common::getNHardwareThreads(Gem::Courtier::DEFAULTNSTDTHREADS));
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
	  * General initialization function to be called prior to the first submission
	  */
	 virtual void init() override {
		 // GBaseExecutorT<processable_type> sees exactly the environment it would when called from its own class
		 GBaseExecutorT<processable_type>::init();

		 // Initialize our thread pool
		 m_gtp_ptr.reset(new Gem::Common::GThreadPool(m_n_threads));
	 }

	 /***************************************************************************/
	 /**
	  * General finalization function to be called after the last submission
	  */
	 virtual void finalize() override {
		 // Check whether there were any errors during thread execution
		 if (m_gtp_ptr->hasErrors()) {
			 std::ostringstream oss;
			 std::vector<std::string> errors;
			 errors = m_gtp_ptr->getErrors();

			 for(auto error: errors) {
				 oss << error << std::endl;
			 }

			 glogger
				 << "In GMTExecutorT<processable_type>::finalize(): Warning!" << std::endl
			 	 << "There were errors during thread execution in GThreadPool:" << std::endl
				 << oss.str() << std::endl
				 << GWARNING;
		 }

		 // Terminate our thread pool
		 m_gtp_ptr.reset();

		 // GBaseExecutorT<processable_type> sees exactly the environment it would when called from its own class
		 GBaseExecutorT<processable_type>::finalize();
	 }

	 /***************************************************************************/
	 /**
	  * Searches for compliance with expectations with respect to another object
	  * of the same type
	  */
	 virtual void compare(
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
	 virtual void load_(const GBaseExecutorT<processable_type> *cp) override {
		 const GMTExecutorT<processable_type> *p_load = dynamic_cast<const GMTExecutorT<processable_type> *>(cp);

		 if (!cp) { // nullptr
			 glogger
				 << "In GMTExecutorT<processable_type>::load_(): Conversion error!" << std::endl
				 << GEXCEPTION;
		 }

		 // Load our parent classes data
		 GBaseExecutorT<processable_type>::load_(cp);

		 // Load our local data
		 m_n_threads = p_load->m_n_threads;
	 }


	 /***************************************************************************/
	 /**
	  * Creates a deep clone of this object.
	  */
	 virtual GBaseExecutorT<processable_type>* clone_() const override {
		 return new GMTExecutorT<processable_type>(*this);
	 }

	 /***************************************************************************/
	 /**
	  * Submits a single work item. As we are dealing with multi-threaded
	  * execution, we simply let a thread pool executed a lambda function
	  * which takes care of the processing.
	  *
	  * @param w The work item to be processed
	  */
	 virtual void submit(std::shared_ptr<processable_type> w) override {
#ifdef DEBUG
		 if (m_gtp_ptr && w) {
			 m_gtp_ptr->async_schedule([w]() -> bool { return w->process(); });
		 } else {
			 if (!m_gtp_ptr) {
				 glogger
					 << "In In GMTExecutorT<processable_type>::submit(): Error!" << std::endl
					 << "Threadpool pointer is empty" << std::endl
					 << GEXCEPTION;
			 } else if(!w) {
				 glogger
					 << "In In GMTExecutorT<processable_type>::submit(): Error!" << std::endl
					 << "work item pointer is empty" << std::endl
					 << GEXCEPTION;
			 }
		 }
#else
		 m_gtp_ptr->async_schedule([w]() -> bool { return w->process(); });
#endif /* DEBUG */
	 }

	 /***************************************************************************/
	 /**
	  * Waits for the thread pool to run empty.
	  */
	 virtual bool waitForReturn(
		 std::vector<std::shared_ptr < processable_type>>& workItems
		 , std::vector<bool> &workItemPos
		 , std::vector<std::shared_ptr<processable_type>>& oldWorkItems
	 ) override {
		 m_gtp_ptr->wait();

		 // Mark all positions as "processed"
		 for(auto item: workItemPos) {
			 item = GBC_PROCESSED;
		 }

		 return true;
	 }

private:
	 /***************************************************************************/

	 std::uint16_t m_n_threads; ///< The number of threads
	 std::shared_ptr<Gem::Common::GThreadPool> m_gtp_ptr; ///< Temporarily holds a thread pool
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class relays execution of work items to a broker, to which several
 * different consumers may be connected.
 */
template<typename processable_type>
class GBrokerExecutorT
	: public GBaseExecutorT<processable_type> {
	 ///////////////////////////////////////////////////////////////////////

	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GBaseExecutorT", boost::serialization::base_object<GBaseExecutorT<processable_type>>(*this))
		 & BOOST_SERIALIZATION_NVP(m_waitFactor)
		 & BOOST_SERIALIZATION_NVP(m_initialWaitFactor)
		 & BOOST_SERIALIZATION_NVP(m_capable_of_full_return)
		 & BOOST_SERIALIZATION_NVP(m_gpd)
		 & BOOST_SERIALIZATION_NVP(m_waiting_times_graph)
		 & BOOST_SERIALIZATION_NVP(m_returned_items_graph)
		 & BOOST_SERIALIZATION_NVP(m_waitFactorWarningEmitted)
		 & BOOST_SERIALIZATION_NVP(m_lastReturnTime)
		 & BOOST_SERIALIZATION_NVP(m_lastAverage)
		 & BOOST_SERIALIZATION_NVP(m_remainingTime)
		 & BOOST_SERIALIZATION_NVP(m_maxTimeout);
	 }

	 ///////////////////////////////////////////////////////////////////////

	 using GBufferPortT_ptr = std::shared_ptr<Gem::Courtier::GBufferPortT<std::shared_ptr<processable_type>>>;
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
		 , m_initialWaitFactor(cp.m_initialWaitFactor)
		 , m_capable_of_full_return(cp.m_capable_of_full_return)
		 , m_gpd("Maximum waiting times and returned items", 1, 2) // Intentionally not copied
		 , m_waitFactorWarningEmitted(cp.m_waitFactorWarningEmitted)
		 , m_lastReturnTime(cp.m_lastReturnTime)
		 , m_lastAverage(cp.m_lastAverage)
		 , m_remainingTime(cp.m_remainingTime)
		 , m_maxTimeout(cp.m_maxTimeout)
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
	  *
	  * TODO: This is a hack. GBrokerExecutorT from factory will otherwise
	  * overwrite the file.
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
	  * Checks for equality with another GBrokerExecutorT<processable_type> object
	  *
	  * @param  cp A constant reference to another GBrokerExecutorT<processable_type> object
	  * @return A boolean indicating whether both objects are equal
	  */
	 bool operator==(const GBrokerExecutorT<processable_type>& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			 return true;
		 } catch (g_expectation_violation &) {
			 return false;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Checks for inequality with another GBrokerExecutorT<processable_type> object
	  *
	  * @param  cp A constant reference to another GBrokerExecutorT<processable_type> object
	  * @return A boolean indicating whether both objects are inequal
	  */
	 bool operator!=(const GBrokerExecutorT<processable_type>& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			 return true;
		 } catch (g_expectation_violation &) {
			 return false;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Returns the name of this class
	  */
	 virtual std::string name() const override {
		 return std::string("GBrokerExecutorT<processable_type>");
	 }


	 /***************************************************************************/
	 /**
	  * A standard assignment operator for GBrokerExecutorT<processable_type> objects,
	  *
	  * @param cp A copy of another GBrokerExecutorT<processable_type> object
	  * @return A constant reference to this object
	  */
	 const GBrokerExecutorT<processable_type> &operator=(const GBrokerExecutorT<processable_type> &cp) {
		 GBrokerExecutorT<processable_type>::load_(&cp);
		 return *this;
	 }

	 /***************************************************************************/
	 /**
	  * Adds local configuration options to a GParserBuilder object
	  *
	  * @param gpb The GParserBuilder object to which configuration options should be added
	  */
	 virtual void addConfigurationOptions(
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

		 gpb.registerFileParameter<double>(
			 "initialWaitFactor" // The name of the variable
			 , DEFAULTINITIALBROKERWAITFACTOR2 // The default value
			 , [this](double w) {
				 this->setInitialWaitFactor(w);
			 }
		 )
			 << "A static double factor for timeouts in the first iteration." << std::endl
			 << "Set this to the inverse of the number of parallel processing" << std::endl
			 << "units being used.";
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
	  * Allows to set the initial wait factor to be applied to timeouts. A wait factor
	  * <= 0 is not allowed.
	  */
	 void setInitialWaitFactor(double initialWaitFactor) {
		 if(initialWaitFactor <= 0.) {
			 glogger
				 << "In GBrokerExecutorT<processable_type>::setInitialWaitFactor(): Error!" << std::endl
				 << "Invalid wait factor " << initialWaitFactor << " supplied. Must be > 0."
				 << GEXCEPTION;
		 }
		 m_initialWaitFactor = initialWaitFactor;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the wait factor variable
	  */
	 double getInitialWaitFactor() const {
		 return m_initialWaitFactor;
	 }

	 /***************************************************************************/
	 /**
	  * General initialization function to be called prior to the first submission
	  */
	 virtual void init() override {
		 // To be called prior to all other initialization code
		 GBaseExecutorT<processable_type>::init();

		 // Make sure we have a valid buffer port
		 if (!m_CurrentBufferPort) {
			 m_CurrentBufferPort
				 = GBufferPortT_ptr(new Gem::Courtier::GBufferPortT<std::shared_ptr<processable_type>>());
		 }

		 // Retrieve a connection to the broker
		 m_broker_ptr = GBROKER(processable_type);

		 // Add the buffer port to the broker
		 m_broker_ptr->enrol(m_CurrentBufferPort);

		 // Check the capabilities of consumsers enrolled with the broker.
		 // Note that this call may block until consumers have actually been enrolled.
		 m_capable_of_full_return = m_broker_ptr->capableOfFullReturn();

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
	 virtual void finalize() override {
		 // Get rid of the buffer port
		 m_CurrentBufferPort.reset();

		 // Likely unnecessary cleanup
		 m_capable_of_full_return = false;

		 // Disconnect from the broker
		 m_broker_ptr.reset();

		 // To be called after all other finalization code
		 GBaseExecutorT<processable_type>::finalize();
	 }

	 /***************************************************************************/
	 /**
	  * Searches for compliance with expectations with respect to another object
	  * of the same type
	  */
	 virtual void compare(
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
		 compare_t(IDENTITY(m_initialWaitFactor, p_load->m_initialWaitFactor), token);
		 compare_t(IDENTITY(m_capable_of_full_return, p_load->m_capable_of_full_return), token);
		 compare_t(IDENTITY(m_waitFactorWarningEmitted, p_load->m_waitFactorWarningEmitted), token);
		 compare_t(IDENTITY(m_lastReturnTime, p_load->m_lastReturnTime), token);
		 compare_t(IDENTITY(m_lastAverage, p_load->m_lastAverage), token);
		 compare_t(IDENTITY(m_remainingTime, p_load->m_remainingTime), token);
		 compare_t(IDENTITY(m_maxTimeout, p_load->m_maxTimeout), token);

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
	 virtual void load_(const GBaseExecutorT<processable_type> * cp) override {
		 const GBrokerExecutorT<processable_type> *p_load = dynamic_cast<const GBrokerExecutorT<processable_type> *>(cp);

		 if (!p_load) { // nullptr
			 glogger
				 << "In GBrokerExecutorT<processable_type>::load(): Conversion error!" << std::endl
				 << GEXCEPTION;
		 }

		 // Load our parent classes data
		 GBaseExecutorT<processable_type>::load_(cp);

		 // Local data
		 m_waitFactor = p_load->m_waitFactor;
		 m_initialWaitFactor = p_load->m_initialWaitFactor;
		 m_capable_of_full_return = p_load->m_capable_of_full_return;
		 m_waitFactorWarningEmitted = p_load->m_waitFactorWarningEmitted;
		 m_lastReturnTime = p_load->m_lastReturnTime;
		 m_lastAverage = p_load->m_lastAverage;
		 m_remainingTime = p_load->m_remainingTime;
		 m_maxTimeout = p_load->m_maxTimeout;
	 }

	 /***************************************************************************/
	 /**
	  * Creates a deep clone of this object.
	  */
	 virtual GBaseExecutorT<processable_type>* clone_() const override {
		 return new GBrokerExecutorT<processable_type>(*this);
	 }

	 /***************************************************************************/
	 /**
	  * Allows to perform necessary setup work for an iteration
	  */
	 virtual void iterationInit(
		 std::vector<std::shared_ptr<processable_type>>& workItems
		 , std::vector<bool> &workItemPos
		 , std::vector<std::shared_ptr<processable_type>>& oldWorkItems
	 ) override {
		 // Make sure the parent classes iterationInit function is executed first
		 // This function will also update the iteration start time
		 GBaseExecutorT<processable_type>::iterationInit(workItems, workItemPos, oldWorkItems);
	 }

	 /***************************************************************************/
	 /**
	  * Allows to perform necessary cleanup work for an iteration or do calculations
	  * for the next iteration.
	  */
	 virtual void iterationFinalize(
		 std::vector<std::shared_ptr<processable_type>>& workItems
		 , std::vector<bool> &workItemPos
		 , std::vector<std::shared_ptr<processable_type>>& oldWorkItems
	 ) override {
		 // Calculate average return times of work items.
		 m_lastAverage =
			 (GBaseExecutorT<processable_type>::m_n_returnedLast>0)
			 ? (m_lastReturnTime - GBaseExecutorT<processable_type>::m_submissionStartTime)/GBaseExecutorT<processable_type>::m_n_returnedLast
			 : (std::chrono::system_clock::now() - GBaseExecutorT<processable_type>::m_submissionStartTime)/GBaseExecutorT<processable_type>::m_expectedNumber; // this is an artificial number, as no items have returned

		 m_maxTimeout =
			 m_lastAverage
			 * boost::numeric_cast<double>(GBaseExecutorT<processable_type>::m_expectedNumber)
			 * m_waitFactor;
	 }

	 /***************************************************************************/
	 /**
	  * Waits for all items to return or possibly until a timeout has been reached.
	  */
	 virtual bool waitForReturn(
		 std::vector<std::shared_ptr<processable_type>>& workItems
		 , std::vector<bool> &workItemPos
		 , std::vector<std::shared_ptr<processable_type>>& oldWorkItems
	 ) override {
		 if(m_capable_of_full_return) {
			 // Wait until all work items have returned (possibly indefinitely)
			 return this->waitForFullReturn(workItems, workItemPos, oldWorkItems);
		 } else {
			 // Wait for a given amount of time, decided upon by the function.
			 // Items that have not returned in time may return in a later iteration
			 return this->waitForTimeOut(workItems, workItemPos, oldWorkItems);
		 }
	 }

private:
	 /***************************************************************************/
	 /**
	  * Submits a single work item.
	  *
	  * @param w The work item to be processed
	  */
	 virtual void submit(std::shared_ptr<processable_type> w_ptr) override {
#ifdef DEBUG
		 if(!w_ptr) {
			 glogger
				 << "In GBrokerExecutorT::submit(): Error!" << std::endl
				 << "Work item is empty" << std::endl
				 << GEXCEPTION;
		 }

		 if(!m_CurrentBufferPort) {
			 glogger
				 << "In GBrokerExecutorT::submit(): Error!" << std::endl
				 << "Current buffer port is empty when it shouldn't be" << std::endl
				 << GEXCEPTION;
		 }
#endif /* DEBUG */

		 // Store the id of the buffer port in the item
		 w_ptr->setBufferId(m_CurrentBufferPort->getUniqueTag());

		 // Perform the actual submission
		 m_CurrentBufferPort->push_raw(w_ptr);
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
		 m_CurrentBufferPort->pop_processed(w);

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
		 m_CurrentBufferPort->pop_processed(w_ptr, timeout);
		 return w_ptr;
	 }

	 /***************************************************************************/
	 /**
	  * Updates the maximum allowed timeframe for calculations
	  */
	 void reviseMaxTime(std::size_t nReturnedCurrent) {
		 // Are we called for the first time in the first iteration)
		 if(nReturnedCurrent==0) {
			 std::chrono::duration<double> currentElapsed
				 = std::chrono::system_clock::now() - GBaseExecutorT<processable_type>::m_submissionStartTime;

			 if (this->getCurrentSubmissionId() == ITERATION_COUNTER_TYPE(0)) {
				 // Calculate a timeout for subsequent retrievals in this iteration. In the first iteration and for the first item,
				 // this timeout is the number of remaining items times the return time needed for the first item times a custom
				 // wait factor for the first submission. This may be very long, but takes care of a situation where there is only
				 // a single worker.
				 m_maxTimeout =
					 currentElapsed
					 * boost::numeric_cast<double>(GBaseExecutorT<processable_type>::m_expectedNumber)
					 * m_initialWaitFactor;
			 } else { // Not the first work item
				 std::chrono::duration<double> currentAverage = currentElapsed / ((std::max)(nReturnedCurrent, std::size_t(1))); // Avoid division by 0
				 m_maxTimeout =
					 currentAverage
					 * GBaseExecutorT<processable_type>::m_expectedNumber
					 * m_waitFactor;
			 }
		 } else {
#ifdef DEBUG
			 if(!m_waitFactorWarningEmitted) {
				 if(m_waitFactor > 0. && m_waitFactor < 1.) {
					 glogger
						 << "In GBrokerExecutorT::waitForTimeOut(): Warning" << std::endl
						 << "It is suggested not to use a wait time < 1. Current value: " << m_waitFactor << std::endl
						 << GWARNING;
				 }
			 }
#endif
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Checks whether we have passed the maximum time frame. The function will
	  * also update the remaining time.
	  *
	  * @return A boolean indicating whether the maximum allowed time was passed
	  */
	 bool passedMaxTime(std::size_t nReturnedCurrent) {
		 std::chrono::duration<double> currentElapsed
			 = std::chrono::system_clock::now() - GBaseExecutorT<processable_type>::m_submissionStartTime;

		 if (currentElapsed > m_maxTimeout) {
			 m_remainingTime = std::chrono::duration<double>(0.);
			 return true;
		 } else {
			 m_remainingTime = m_maxTimeout - currentElapsed;
			 return false;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Waits until a timeout occurs and returns, either complete (true) or
	  * incomplete (false). The algorithm works like this:
	  *
	  * In iteration n==0:
	  * - We have initially no indication how much time each calculation takes.
	  *   Hence we wait for the first return and measure the time. We then make
	  *   a very conservative estimate for the time needed for further returns as
	  *   "number of remaining items times the time needed for the first item
	  *   times an initial wait factor". This takes care of the case that there is
	  *   only a single client worker.
	  * - This estimate is then continuously revised for each new return
	  *
	  * In iteration n>0:
	  * - The timeout is calculated from the average time needed for the work items
	  *   of the previous iteration, times a wait factor
	  *
	  * @param workItems The work items to be processed
	  * @param workItemPos Booleans for each work item indicating whether processing has happened
	  * @param oldWorkItems A collection of old work items that have returned after a timeout
	  * @return A boolean indicating whether a complete return of items was achieved
	  */
	 bool waitForTimeOut(
		 std::vector<std::shared_ptr<processable_type>>& workItems
		 , std::vector<bool> &workItemPos
		 , std::vector<std::shared_ptr<processable_type>>& oldWorkItems
	 ) {
		 //----------------------------------------------------------------
		 // If the wait factor is == 0, we fall back to
		 // the "complete return" submission return mode.
		 if(m_waitFactor == 0.) {
			 return waitForFullReturn(
				 workItems
				 , workItemPos
				 , oldWorkItems
			 );
		 }

		 //----------------------------------------------------------------

		 std::shared_ptr<processable_type> w;
		 std::size_t nReturnedCurrent = 0;

		 // Check if this is the first iteration. If so, wait (possibly indefinitely)
		 // for the first item to return so we can estimate a suitable timeout
		 if (0 == this->getSubmissionId()) { // Wait indefinitely for first item
			 // Retrieve a single work item
			 w = this->retrieve();

			 // It is a severe error if no item is received in the first iteration.
			 // Note that retrieve() will wait indefinitely and, once it returns,
			 // should carry a work item.
			 if (!w) {
				 glogger
					 << "In GBrokerExecutorT<>::waitForTimeOut(): Error!" << std::endl
					 << "First item received in first iteration is empty. We cannot continue!"
					 << GEXCEPTION;
			 }

			 if (
				 this->addWorkItemAndCheckCompleteness(
					 w
					 , nReturnedCurrent
					 , workItems
					 , workItemPos
					 , oldWorkItems
				 )
				 ) {
				 // This covers the rare case that a "collection" of a *single*
				 // work item was submitted.
				 return true;
			 } else {
				 reviseMaxTime(0);
			 }
		 }

		 //------------------------------------

		 // Loop until a timeout is reached or all current items have returned
		 while (true) {
			 // Check if we have passed the maximum allowed time frame.
			 // This function will also update the remaining time.
			 if(passedMaxTime(nReturnedCurrent)) {
				 return false; // No complete return as we have reached the timeout
			 }

			 // Obtain the next item
			 w = retrieve(m_remainingTime);

			 // Leave if this was the last item
			 if (this->addWorkItemAndCheckCompleteness(
				 w
				 , nReturnedCurrent
				 , workItems
				 , workItemPos
				 , oldWorkItems
			 )
				 ) {
				 break;
			 }

			 // Continuously revise the maxTimeout, if this is the first submission
			 if(w && 0 == this->getSubmissionId()) {
				 reviseMaxTime(nReturnedCurrent);
			 }
		 }

		 return true;
	 }

	 /***************************************************************************/
	 /**
	  * Waits (possibly indefinitely) until all items have returned. Note that this
	  * function may stall, if for whatever reason a work item does not return. If this
	  * is not acceptable, use either waitForTimeout() or waitForTimeoutAndResubmit()
	  * instead of this function. It is recommended to only use this function in
	  * environments that are considered safe in the sense that work items will practically
	  * always return. Local cluster environments will often fall into this category.
	  * There may not be returns from older iterations, which are attached to the end
	  * of the work item vector
	  */
	 bool waitForFullReturn(
		 std::vector<std::shared_ptr<processable_type>>& workItems
		 , std::vector<bool> &workItemPos
		 , std::vector<std::shared_ptr<processable_type>>& oldWorkItems
	 ) {
		 std::size_t nReturnedCurrent = 0;
		 while (!addWorkItemAndCheckCompleteness(
			 this->retrieve()
			 , nReturnedCurrent
			 , workItems
			 , workItemPos
			 , oldWorkItems
		 ));

		 return true;
	 }

	 /***************************************************************************/
	 /**
	  * Adds a work item to the corresponding vectors. This function assumes that
	  * the work item is valid, i.e. points to a valid object.
	  *
	  * @return A boolean indicating whether all work items of the current iteration were received
	  */
	 bool addWorkItemAndCheckCompleteness(
		 std::shared_ptr <processable_type> w_ptr
		 , std::size_t &nReturnedCurrent
		 , std::vector<std::shared_ptr<processable_type>>& workItems
		 , std::vector<bool> &workItemPos
		 , std::vector<std::shared_ptr<processable_type>>& oldWorkItems
	 ) {
		 // If we have been passed an empty item, simply continue the outer loop
		 if(!w_ptr) {
			 return false;
		 } else {
			 // Make the return time of the last item known
			 m_lastReturnTime = std::chrono::system_clock::now();
		 }

		 bool completed = false;
		 auto current_iteration = GBaseExecutorT<processable_type>::m_iteration_counter;
		 auto w_iteration = w_ptr->getSubmissionCounter();

		 if (current_iteration == w_iteration) {
			 // Mark the position of the work item in the workItemPos vector and cross-check
			 std::size_t w_pos = w_ptr->getSubmissionPosition();

			 if (w_pos >= workItems.size()) {
				 glogger
					 << "In GBrokerExecutorT<processable_type>::addWorkItemAndCheckCompleteness(): Error!" << std::endl
					 << "Received work item for position " << w_pos << " while" << std::endl
					 << "only a range [0" << ", " << workItems.size() << "[ was expected." << std::endl
					 << GEXCEPTION;
			 }

			 // Re-submitted items might return twice, so we only add them
			 // if a processed item hasn't been added yet.
			 if (GBC_UNPROCESSED == workItemPos.at(w_pos)) {
				 workItemPos.at(w_pos) = GBC_PROCESSED; // Successfully returned
				 workItems.at(w_pos) = w_ptr;
				 if (++nReturnedCurrent == GBaseExecutorT<processable_type>::m_expectedNumber) {
					 completed = true;
				 }
			 } // no else

		 } else { // It could be that a previous submission did not expect a full return. Hence older items may occur
			 oldWorkItems.push_back(w_ptr);
		 }

		 return completed;
	 }

	 /***************************************************************************/
	 /**
 	  * Allows to emit information at the end of an iteration
 	  * TODO: The content of this function is a hack. Submitted for current debugging purposes
 	  */
	 virtual void visualize_performance() override {
		 // Call our parent class'es report function
		 GBaseExecutorT<processable_type>::visualize_performance();

		 // Now do our own reporting
		 std::chrono::duration<double> currentElapsed
			 = std::chrono::system_clock::now() - GBaseExecutorT<processable_type>::m_submissionStartTime;
		 auto current_iteration = GBaseExecutorT<processable_type>::m_iteration_counter;

		 m_waiting_times_graph->add(boost::numeric_cast<double>(current_iteration), m_maxTimeout.count());
		 m_returned_items_graph->add(boost::numeric_cast<double>(current_iteration), boost::numeric_cast<double>(this->getNReturned()));

		 if (0 == GBaseExecutorT<processable_type>::m_iteration_counter) {
			 std::cout
				 << "Maximum waiting time in iteration " << current_iteration << ": " << m_maxTimeout.count()
				 << " s (" << currentElapsed.count() << ", "
				 << this->getNReturned() << " / " << GBaseExecutorT<processable_type>::m_expectedNumber << ", " << m_initialWaitFactor << ")" << std::endl;
		 } else {
			 std::cout
				 << "Maximum waiting time in iteration " << current_iteration << ": " << m_maxTimeout.count()
				 << " s (" << m_lastAverage.count() << ", "
				 << this->getNReturned() << " / " << GBaseExecutorT<processable_type>::m_expectedNumber << ", " << m_waitFactor << ")" << std::endl;
		 }
	 }

	 /***************************************************************************/
	 // Local data
	 double m_waitFactor = DEFAULTBROKERWAITFACTOR2; ///< A static factor to be applied to timeouts
	 double m_initialWaitFactor = DEFAULTINITIALBROKERWAITFACTOR2; ///< A static factor to be applied to timeouts in the first iteration

	 GBufferPortT_ptr m_CurrentBufferPort; ///< Holds a GBufferPortT object during the calculation. Note: It is neither serialized nor copied
	 GBroker_ptr m_broker_ptr; ///< A (possibly empty) pointer to a global broker
	 bool m_capable_of_full_return = false; ///< Indicates whether the broker may return results without losses

	 Gem::Common::GPlotDesigner m_gpd; ///< A wrapper for the plots
	 std::shared_ptr<Gem::Common::GGraph2D> m_waiting_times_graph;  ///< The maximum waiting time resulting from the wait factor
	 std::shared_ptr<Gem::Common::GGraph2D> m_returned_items_graph;  ///< The maximum waiting time resulting from the wait factor

	 bool m_waitFactorWarningEmitted = false; ///< Specifies whether a warning about a small waitFactor has already been emitted

	 std::chrono::system_clock::time_point m_lastReturnTime = std::chrono::system_clock::now(); ///< Temporary that holds the time of the return of the last item of an iteration
	 std::chrono::duration<double> m_lastAverage = std::chrono::duration<double>(0.); ///< The average time needed for the last submission
	 std::chrono::duration<double> m_remainingTime = std::chrono::duration<double>(0.); ///< The remaining time in the current iteration
	 std::chrono::duration<double> m_maxTimeout = std::chrono::duration<double>(0.); ///< The maximum amount of time allowed for the entire calculation
};


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GEXECUTOR_HPP_ */
