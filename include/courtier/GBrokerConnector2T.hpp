/**
 * @file GBrokerConnector2T.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */

// Standard headers go here

#include <sstream>
#include <algorithm>
#include <vector>
#include <utility>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/greg_serialize.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility/enable_if.hpp>

#ifndef GBROKERCONNECTOR2T_HPP_
#define GBROKERCONNECTOR2T_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GPODExpectationChecksT.hpp"
#include "common/GMathHelperFunctionsT.hpp"
#include "common/GParserBuilder.hpp"
#include "common/GThreadPool.hpp"
#include "courtier/GBufferPortT.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GSubmissionContainerT.hpp"

namespace Gem {
namespace Courtier {

/******************************************************************************/
/**
 * This class centralizes some functionality and data that is needed to perform
 * serial or parallel execution for a set of work items. Its main purpose is to
 * avoid duplication of code. Derived classes may deal with different types of
 * parallel execution, including connection to a broker and multi-threaded
 * execution. The serial mode is meant for debugging purposes only.
 */
template <typename processable_type>
class GBaseExecutorT {
   // Make sure processable_type is derived from the submission container
   BOOST_MPL_ASSERT((boost::is_base_of<GSubmissionContainerT<processable_type>, processable_type>));

public:
   ///////////////////////////////////////////////////////////////////////

   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;

      // ar & BOOST_SERIALIZATION_NVP(doLogging_); // nothing
   }

   ///////////////////////////////////////////////////////////////////////

   /***************************************************************************/
   /**
    * The default constructor
    */
   GBaseExecutorT()
      : submission_counter_(SUBMISSIONCOUNTERTYPE(0))
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The copy constructor. No local data to be copied.
    * submission_counter_ is just a temporary which always
    * starts counting at 0.
    *
    * @param cp A copy of another GBrokerConnector object
    */
   GBaseExecutorT(const GBaseExecutorT<processable_type>& cp)
      : submission_counter_(SUBMISSIONCOUNTERTYPE(0))
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
    * A standard assignment operator for GBaseExecutorT<processable_type> objects,
    *
    * @param cp A copy of another GBaseExecutorT<processable_type> object
    * @return A constant reference to this object
    */
   const GBaseExecutorT<processable_type>& operator=(const GBaseExecutorT<processable_type>& cp) {
      GBaseExecutorT<processable_type>::load(&cp);
      return *this;
   }

   /***************************************************************************/
   /**
    * Loads the data of another GBaseExecutorT object
    *
    * @param cp A constant pointer to another GBaseExecutorT object
    */
   void load(GBaseExecutorT<processable_type> const * const cp) {
      // nothing
   }

   /***************************************************************************/
   /**
    * Checks for equality with another GBaseExecutorT<processable_type> object
    *
    * @param  cp A constant reference to another GBaseExecutorT<processable_type> object
    * @return A boolean indicating whether both objects are equal
    */
   bool operator==(const GBaseExecutorT<processable_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GBaseExecutorT<processable_type>::operator==","cp", CE_SILENT);
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
      // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GBaseExecutorT<processable_type>::operator!=","cp", CE_SILENT);
   }

   /***************************************************************************/
   /**
    * Checks whether a given expectation for the relationship between this object and another object
    * is fulfilled.
    *
    * @param cp A constant reference to another GBrokerConnector
    * @param e The expected outcome of the comparison
    * @param limit The maximum deviation for floating point values (important for similarity checks)
    * @param caller An identifier for the calling entity
    * @param y_name An identifier for the object that should be compared to this one
    * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
    * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
    */
   boost::optional<std::string> checkRelationshipWith(
         const GBaseExecutorT<processable_type>& cp
         , const Gem::Common::expectation& e
         , const double& limit
         , const std::string& caller
         , const std::string& y_name
         , const bool& withMessages
   ) const {
      using namespace Gem::Common;

      // Will hold possible deviations from the expectation, including explanations
      std::vector<boost::optional<std::string> > deviations;

      // Check the local local data
      // deviations.push_back(checkExpectation(withMessages, "GBaseExecutorT<processable_type>", doLogging_, cp.doLogging_, "doLogging_", "cp.doLogging_", e , limit));

      return evaluateDiscrepancies("GBaseExecutorT<processable_type>", caller, deviations, e);
   }

   /***************************************************************************/
   /**
    * Submits and retrieves a set of work items. After the work has been performed,
    * the items contained in the workItems vector may have been changed. Note that it is not guaranteed
    * by this function that all submitted items are still contained in the vector after the call. It is
    * also possible that returned items do not belong to the current submission cycle. You might thus have
    * to post-process the vector. Note that it is impossible to submit items that are not
    * derived from GSubmissionContainerT<processable_type>.
    *
    * @param workItems A vector with work items to be evaluated beyond the broker
    * @param start The id of first item to be worked on in the vector
    * @param end The id beyond the last item to be worked on in the vector
    * @return A boolean indicating whether all expected items have returned
    */
   template <typename work_item>
   bool workOn(
         std::vector<boost::shared_ptr<work_item> >& workItems
         , const std::size_t& start
         , const std::size_t& end
         , typename boost::enable_if<boost::is_base_of<GSubmissionContainerT<processable_type>, work_item> >::type* dummy = 0
   ) {
      bool complete=false;

#ifdef DEBUG
      // Do some error checking
      if(workItems.empty()) {
         glogger
         << "In GBaseExecutorT<processable_type>::workOn(): Error!" << std::endl
         << "workItems_ vector is empty." << std::endl
         << GEXCEPTION;
      }

      if(end <= start) {
         glogger
         << "In GBaseExecutorT<processable_type>::workOn(): Error!" << std::endl
         << "Invalid start or end-values: " << start << " / " << end << std::endl
         << GEXCEPTION;
      }

      if(end > workItems.size()) {
         glogger
         << "In GBaseExecutorT<processable_type>::workOn(): Error!" << std::endl
         << "Last id " << end << " exceeds size of vector " << workItems.size() << std::endl
         << GEXCEPTION;
      }
#endif /* DEBUG */

      // Submit all items
      this->submitAllWorkItems(workItems, start, end);

      // Wait for work items to return. This function needs to
      // be re-implemented in derived classes.
      return waitForReturn();
   }

protected:
   /***************************************************************************/

   /** @brief Submits a single work item */
   virtual void submit(boost::shared_ptr<processable_type>) = 0;
   /** @brief Waits for work items to return */
   virtual bool waitForReturn() = 0;

   /***************************************************************************/
   /**
    * Submission of all work items in the list
    */
   template <typename work_item>
   void submitAllWorkItems (
         std::vector<boost::shared_ptr<work_item> >& workItems
         , const std::size_t& start
         , const std::size_t& end
   ) {
      // Set the start time of the new iteration so we calculate a correct
      // return time for the first individual, regardless of whether older
      // individuals have returned first.
      iterationStartTime_ = boost::posix_time::microsec_clock::local_time();

      typename std::vector<boost::shared_ptr<work_item> >::iterator it;
      POSITIONTYPE pos_cnt = start;
      for(it=workItems.begin()+start; it!=workItems.begin()+end; ++it) {
         (*it)->setCourtierId(boost::make_tuple<SUBMISSIONCOUNTERTYPE,POSITIONTYPE>(submission_counter_, pos_cnt++));
         this->submit(*it);
      }

      // Update the submission counter
      submission_counter_++;
   }

   /***************************************************************************/
   // Local data
   SUBMISSIONCOUNTERTYPE submission_counter_; ///< Counts the number of submissions initiated by this object. Note: not serialized!
   boost::posix_time::ptime iterationStartTime_; ///< Temporary that holds the start time for the retrieval of items in a given iteration
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class executes work items serially. It is mostly meant for debugging
 * purposes
 */
template <typename processable_type>
class GSerialExecutorT
   : public GBaseExecutorT<processable_type>
{
   ///////////////////////////////////////////////////////////////////////

   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;

      ar
      & make_nvp("GBaseExecutorT", boost::serialization::base_object<GBaseExecutorT<processable_type> >(*this));
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
   GSerialExecutorT(const GSerialExecutorT<processable_type>& cp)
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
    * A standard assignment operator for GSerialExecutorT<processable_type> objects,
    *
    * @param cp A copy of another GSerialExecutorT<processable_type> object
    * @return A constant reference to this object
    */
   const GSerialExecutorT<processable_type>& operator=(const GSerialExecutorT<processable_type>& cp) {
      GSerialExecutorT<processable_type>::load(&cp);
      return *this;
   }

   /***************************************************************************/
   /**
    * Loads the data of another GSerialExecutorT object
    *
    * @param cp A constant pointer to another GSerialExecutorT object
    */
   void load(GSerialExecutorT<processable_type> const * const cp) {
      // Load our parent classes data
      GBaseExecutorT<processable_type>::load(cp);

      // No local data
   }

   /***************************************************************************/
   /**
    * Checks for equality with another GSerialExecutorT<processable_type> object
    *
    * @param  cp A constant reference to another GSerialExecutorT<processable_type> object
    * @return A boolean indicating whether both objects are equal
    */
   bool operator==(const GSerialExecutorT<processable_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GSerialExecutorT<processable_type>::operator==","cp", CE_SILENT);
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
      // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GSerialExecutorT<processable_type>::operator!=","cp", CE_SILENT);
   }

   /***************************************************************************/
   /**
    * Checks whether a given expectation for the relationship between this object and another object
    * is fulfilled.
    *
    * @param cp A constant reference to another GBrokerConnector
    * @param e The expected outcome of the comparison
    * @param limit The maximum deviation for floating point values (important for similarity checks)
    * @param caller An identifier for the calling entity
    * @param y_name An identifier for the object that should be compared to this one
    * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
    * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
    */
   boost::optional<std::string> checkRelationshipWith(
         const GSerialExecutorT<processable_type>& cp
         , const Gem::Common::expectation& e
         , const double& limit
         , const std::string& caller
         , const std::string& y_name
         , const bool& withMessages
   ) const {
      using namespace Gem::Common;

      // Will hold possible deviations from the expectation, including explanations
      std::vector<boost::optional<std::string> > deviations;

      // Check our parent classes data
      deviations.push_back(GBaseExecutorT<processable_type>::checkRelationShipWith(cp, e, limit, "GSerialExecutorT<processable_type>", y_name, withMessages));

      return evaluateDiscrepancies("GSerialExecutorT<processable_type>", caller, deviations, e);
   }

protected:
   /***************************************************************************/
   /**
    * Submits a single work item. In the case of serial execution, all work
    * is done inside of this function. We rely on the process() function which
    * is guaranteed to be part of the processable_type interface (note that
    * our base class stipulates that processable_type is a derivative of
    * GSubmissionContainerT<processable_type> .
    *
    * @param w The work item to be processed
    */
   virtual void submit(boost::shared_ptr<processable_type> w) {
      w->process();
   }

   /***************************************************************************/
   /**
    * Waits for work items to return. Empty, as all work is done inside of the
    * submit() function.
    */
   virtual bool waitForReturn() {
      return true;
   }
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class executes a collection of work items in multiple threads
 */
template <typename processable_type>
class GMTExecutorT
   : public GBaseExecutorT<processable_type>
{
   ///////////////////////////////////////////////////////////////////////

   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;

      ar
      & make_nvp("GBaseExecutorT", boost::serialization::base_object<GBaseExecutorT<processable_type> >(*this));
   }

   ///////////////////////////////////////////////////////////////////////

public:
   /***************************************************************************/
   /**
    * The default constructor
    */
   GMTExecutorT()
   : GBaseExecutorT<processable_type>()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Initialization with the number of threads
    */
   GMTExecutorT(boost::uint16_t nThreads)
   : GBaseExecutorT<processable_type>()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The copy constructor
    *
    * @param cp A copy of another GBrokerConnector object
    */
   GMTExecutorT(const GMTExecutorT<processable_type>& cp)
   : GBaseExecutorT<processable_type>(cp)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The destructor
    */
   virtual ~GMTExecutorT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * A standard assignment operator for GMTExecutorT<processable_type> objects,
    *
    * @param cp A copy of another GMTExecutorT<processable_type> object
    * @return A constant reference to this object
    */
   const GMTExecutorT<processable_type>& operator=(const GMTExecutorT<processable_type>& cp) {
      GMTExecutorT<processable_type>::load(&cp);
      return *this;
   }

   /***************************************************************************/
   /**
    * Loads the data of another GMTExecutorT object
    *
    * @param cp A constant pointer to another GMTExecutorT object
    */
   void load(GMTExecutorT<processable_type> const * const cp) {
      // Load our parent classes data
      GBaseExecutorT<processable_type>::load(cp);

      // No local data
   }

   /***************************************************************************/
   /**
    * Checks for equality with another GMTExecutorT<processable_type> object
    *
    * @param  cp A constant reference to another GMTExecutorT<processable_type> object
    * @return A boolean indicating whether both objects are equal
    */
   bool operator==(const GMTExecutorT<processable_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GMTExecutorT<processable_type>::operator==","cp", CE_SILENT);
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
      // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GMTExecutorT<processable_type>::operator!=","cp", CE_SILENT);
   }

   /***************************************************************************/
   /**
    * Checks whether a given expectation for the relationship between this object and another object
    * is fulfilled.
    *
    * @param cp A constant reference to another GBrokerConnector
    * @param e The expected outcome of the comparison
    * @param limit The maximum deviation for floating point values (important for similarity checks)
    * @param caller An identifier for the calling entity
    * @param y_name An identifier for the object that should be compared to this one
    * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
    * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
    */
   boost::optional<std::string> checkRelationshipWith(
         const GMTExecutorT<processable_type>& cp
         , const Gem::Common::expectation& e
         , const double& limit
         , const std::string& caller
         , const std::string& y_name
         , const bool& withMessages
   ) const {
      using namespace Gem::Common;

      // Will hold possible deviations from the expectation, including explanations
      std::vector<boost::optional<std::string> > deviations;

      // Check our parent classes data
      deviations.push_back(GBaseExecutorT<processable_type>::checkRelationShipWith(cp, e, limit, "GMTExecutorT<processable_type>", y_name, withMessages));

      return evaluateDiscrepancies("GMTExecutorT<processable_type>", caller, deviations, e);
   }

protected:
   /***************************************************************************/
   /**
    * Submits a single work item. As we are dealing with multi-threaded
    * execution, we simply push a worker into a thread pool.
    *
    * @param w The work item to be processed
    */
   virtual void submit(boost::shared_ptr<processable_type> w) {
      w->process();
   }

   /***************************************************************************/
   /**
    * Waits for the thread pool to run empty.
    */
   virtual bool waitForReturn() {
      return true;
   }

private:
   /***************************************************************************/

   boost::shared_ptr<Gem::Common::GThreadPool> tp_; ///< Temporarily holds a thread pool
};

/******************************************************************************/


} /* namespace Courtier */
} /* namespace Gem */

#endif /* GBROKERCONNECTOR2T_HPP_ */
