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
   , tp_()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Initialization with the number of threads
    */
   GMTExecutorT(boost::uint16_t nThreads)
   : GBaseExecutorT<processable_type>()
   , tp_(nThreads)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The copy constructor
    *
    * @param cp A copy of another GBrokerConnector object
    */
   GMTExecutorT(const GMTExecutorT<processable_type>& cp)
   : GBaseExecutorT<processable_type>(cp)
   , tp_(cp.tp_.getNThreads())
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

      // Adapt our local thread pool
      tp_.setNThreads((cp->tp_).getNThreads());
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
      tp_.schedule(boost::function<bool()>(boost::bind(&processable_type::process, w)));
   }

   /***************************************************************************/
   /**
    * Waits for the thread pool to run empty.
    */
   virtual bool waitForReturn() {
      tp_.wait();
      return true;
   }

private:
   /***************************************************************************/

   Gem::Common::GThreadPool tp_; ///< Holds a thread pool
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class executes a collection of work items in multiple threads
 */
template <typename processable_type>
class GBrokerConnectorT
   : public GBaseExecutorT<processable_type>
{
   ///////////////////////////////////////////////////////////////////////

   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;

      ar
      & make_nvp("GBaseExecutorT", boost::serialization::base_object<GBaseExecutorT<processable_type> >(*this))
      & BOOST_SERIALIZATION_NVP(srm_)
      & BOOST_SERIALIZATION_NVP(maxResubmissions_)
      & BOOST_SERIALIZATION_NVP(doLogging_);
   }

   ///////////////////////////////////////////////////////////////////////

public:
   /***************************************************************************/
   /**
    * The default constructor
    */
   GBrokerConnectorT()
   : GBaseExecutorT<processable_type>()
   , srm_(DEFAULTSRM)
   , maxResubmissions_(DEFAULTMAXRESUBMISSIONS)
   , doLogging_(false)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The copy constructor
    *
    * @param cp A copy of another GBrokerConnector object
    */
   GBrokerConnectorT(const GBrokerConnectorT<processable_type>& cp)
   : GBaseExecutorT<processable_type>(cp)
   , srm_(cp.srm_)
   , maxResubmissions_(cp.maxResubmissions_)
   , doLogging_(cp.doLogging_)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The destructor
    */
   virtual ~GBrokerConnectorT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * A standard assignment operator for GBrokerConnectorT<processable_type> objects,
    *
    * @param cp A copy of another GBrokerConnectorT<processable_type> object
    * @return A constant reference to this object
    */
   const GBrokerConnectorT<processable_type>& operator=(const GBrokerConnectorT<processable_type>& cp) {
      GBrokerConnectorT<processable_type>::load(&cp);
      return *this;
   }

   /***************************************************************************/
   /**
    * Loads the data of another GBrokerConnectorT object
    *
    * @param cp A constant pointer to another GBrokerConnectorT object
    */
   void load(GBrokerConnectorT<processable_type> const * const cp) {
      // Load our parent classes data
      GBaseExecutorT<processable_type>::load(cp);

      // Local data
      srm_ = cp->srm_;
      maxResubmissions_ = cp->maxResubmissions_;
      doLogging_ = cp->doLogging_;
   }

   /***************************************************************************/
   /**
    * Checks for equality with another GBrokerConnectorT<processable_type> object
    *
    * @param  cp A constant reference to another GBrokerConnectorT<processable_type> object
    * @return A boolean indicating whether both objects are equal
    */
   bool operator==(const GBrokerConnectorT<processable_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GBrokerConnectorT<processable_type>::operator==","cp", CE_SILENT);
   }

   /***************************************************************************/
   /**
    * Checks for inequality with another GBrokerConnectorT<processable_type> object
    *
    * @param  cp A constant reference to another GBrokerConnectorT<processable_type> object
    * @return A boolean indicating whether both objects are inequal
    */
   bool operator!=(const GBrokerConnectorT<processable_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GBrokerConnectorT<processable_type>::operator!=","cp", CE_SILENT);
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
         const GBrokerConnectorT<processable_type>& cp
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
      deviations.push_back(GBaseExecutorT<processable_type>::checkRelationShipWith(cp, e, limit, "GBrokerConnectorT<processable_type>", y_name, withMessages));

      // Check local data
      deviations.push_back(checkExpectation(withMessages, "GBrokerConnectorT<processable_type>", srm_, cp.srm_, "srm_", "cp.srm_", e , limit));
      deviations.push_back(checkExpectation(withMessages, "GBrokerConnectorT<processable_type>", maxResubmissions_, cp.maxResubmissions_, "maxResubmissions_", "cp.maxResubmissions_", e , limit));
      deviations.push_back(checkExpectation(withMessages, "GBrokerConnectorT<processable_type>", doLogging_, cp.doLogging_, "doLogging_", "cp.doLogging_", e , limit));

      return evaluateDiscrepancies("GBrokerConnectorT<processable_type>", caller, deviations, e);
   }

   /***************************************************************************/
   /**
    * Allows to set the submission return mode. Depending on this setting,
    * the object will wait indefinitely for items of the current submission to return,
    * or will timeout and optionally resubit unprocessed items.
    */
   void setSubmissionReturnMode(submissionReturnMode srm) {
      srm_ = srm;
   }

   /***************************************************************************/
   /**
    * Allows to retrieve the current submission return mode
    */
   submissionReturnMode getSubmissionReturnMode() const {
      return srm_;
   }

   /***************************************************************************/
   /**
    * Specifies how often work items should be resubmitted in the case a full return
    * of work items is expected.
    *
    * @param maxResubmissions The maximum number of allowed resubmissions
    */
   void setMaxResubmissions(std::size_t maxResubmissions) {
     maxResubmissions_ = maxResubmissions;
   }

   /***************************************************************************/
   /**
    * Returns the maximum number of allowed resubmissions
    *
    * @return The maximum number of allowed resubmissions
    */
   std::size_t getMaxResubmissions() const {
     return maxResubmissions_;
   }


   /***************************************************************************/
   /**
    * Allows to specify whether logging of arrival times of processed items should be
    * done. Note that only arrival times of items of the current submission are logged.
    * This also allows to find out how many items did not return before the deadline.
    *
    * @param dl A boolean whether logging of arrival times of items should be done
    */
   void doLogging(bool dl = true) {
      doLogging_ = dl;
   }

   /***************************************************************************/
   /**
    * Allows to determine whether logging of arrival times has been activated.
    *
    * @return A boolean indicating whether logging of arrival times has been activated
    */
   bool loggingActivated() const {
      return doLogging_;
   }

   /***************************************************************************/
   /**
    * Allows to retrieve the logging results in the form of a ROOT histogram
    *
    * @return The logging results in the form of a ROOT histogram
    */
   std::string getLoggingResults() const {
      return std::string(); // TODO
   }

protected:
   /***************************************************************************/
   /**
    * Wait for at least partial return of work items, until a time-out occurs
    */
   virtual bool waitForTimeout() {
      bool result = false;

      // TODO

      return result;
   }

   /***************************************************************************/
   /**
    * Wait for return of the same iterations work items, until a time-out occurs;
    * resubmit if necessary
    */
   virtual bool waitForTimeoutAndResubmit() {
      bool result = false;

      // TODO

      return result;
   }

   /***************************************************************************/
   /**
    * Waits until all items have returned, ignoring any timeout
    */
   virtual bool waitForFullReturn() {
      bool result = false;

      // TODO

      return result;
   }

   /***************************************************************************/
   /**
    * Waits for all items to return or possibly until a timeout has been reached.
    */
   virtual bool waitForReturn() {
      bool result = false;

      switch(srm_) {
         case INCOMPLETERETURN:
            result = waitForTimeout();
            break;

         case RESUBMISSIONAFTERTIMEOUT:
            result = waitForTimeoutAndResubmit();
            break;

         case EXPECTFULLRETURN:
            result = waitForFullReturn();
            break;

         default:
            {
               glogger
               << "In GBrokerConnector2T<>::waitForReturn(): Error!" << std::endl
               << "Encountered an invalid submission return mode: " << srm_ << std::endl
               << GEXCEPTION;
            }
            break;
      }

      return result;
   }

   /***************************************************************************/
   /**
    * Adds a logging result to the logging vector. The first parameter
    * represents the id of the iteration when the item was submitted, the
    * second parameter stands for the iteration when the item has returned
    * and the last item stands for the time between submission and return
    * of an item.
    */
   void addLogData(boost::tuple<SUBMISSIONCOUNTERTYPE, SUBMISSIONCOUNTERTYPE, boost::uint32_t> logData) {
      // TODO: How to find out about the submission time of an item ?

      arrivalTimes_.push_back(logData);
   }

   /***************************************************************************/
   /**
    * Submits a single work item.
    *
    * @param w The work item to be processed
    */
   virtual void submit(boost::shared_ptr<processable_type> w) {

   }

private:
   /***************************************************************************/
   // Local data

   submissionReturnMode srm_; ///< Indicates how (long) the object shall wait for returns
   std::size_t maxResubmissions_; ///< The maximum number of resubmissions allowed if a full return of submitted items is attempted

   bool doLogging_; ///< Specifies whether arrival times of work items should be logged
   std::vector<boost::tuple<SUBMISSIONCOUNTERTYPE, SUBMISSIONCOUNTERTYPE, boost::uint32_t> > arrivalTimes_; ///< Holds the actual arrival times of current work items. Note: Neither serialized nor copied
};


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GBROKERCONNECTOR2T_HPP_ */
