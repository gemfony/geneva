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
   bool workOn(
         std::vector<boost::shared_ptr<processable_type> >& workItems
         , const std::size_t& start
         , const std::size_t& end
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

      // Set the start time of the new iteration
      iterationStartTime_ = boost::posix_time::microsec_clock::local_time();

      // Allows derived classes to perform necessary setup work for an iteration
      this->iterationSetup();

      // Submit all items
      this->submitAllWorkItems(workItems, start, end);

      // Wait for work items to return. This function needs to
      // be re-implemented in derived classes.
      complete = waitForReturn(workItems, start, end);

      // Allows derived classes to perform necessary cleanup work for an iteration
      this->iterationCleanup();

      // Update the submission counter
      submission_counter_++;

      // Let the audience know
      return complete;
   }

   /***************************************************************************/
   /**
    * Adds local configuration options to a GParserBuilder object
    *
    * @param gpb The GParserBuilder object to which configuration options should be added
    * @param showOrigin Makes the function indicate the origin of parameters in comments
    */
   virtual void addConfigurationOptions (
      Gem::Common::GParserBuilder& gpb
      , const bool& showOrigin
   ) {
      /* no local data. hence empty */
   }

protected:
   /***************************************************************************/

   /** @brief Submits a single work item */
   virtual void submit(boost::shared_ptr<processable_type>) = 0;
   /** @brief Waits for work items to return */
   virtual bool waitForReturn(
      std::vector<boost::shared_ptr<processable_type> >&
      , const std::size_t&
      , const std::size_t&
   ) = 0;

   /** @brief Allows derived classes to perform necessary setup work for an iteration */
   virtual void iterationSetup() { /* nothing */ }
   /** @brief Allows derived classes to perform necessary cleanup work for an iteration */
   virtual void iterationCleanup() { /* nothing */ }

   /***************************************************************************/
   /**
    * Submission of all work items in the list
    */
   void submitAllWorkItems (
      std::vector<boost::shared_ptr<processable_type> >& workItems
      , const std::size_t& start
      , const std::size_t& end
   ) {
      // Submit work items
      typename std::vector<boost::shared_ptr<processable_type> >::iterator it;
      POSITIONTYPE pos_cnt = start;
      for(it=workItems.begin()+start; it!=workItems.begin()+end; ++it) {
         (*it)->setCourtierId(boost::make_tuple<SUBMISSIONCOUNTERTYPE,POSITIONTYPE>(submission_counter_, pos_cnt++));
         this->submit(*it);
      }
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

   /***************************************************************************/
   /**
    * Adds local configuration options to a GParserBuilder object
    *
    * @param gpb The GParserBuilder object to which configuration options should be added
    * @param showOrigin Makes the function indicate the origin of parameters in comments
    */
   virtual void addConfigurationOptions (
      Gem::Common::GParserBuilder& gpb
      , const bool& showOrigin
   ) {
      // Call our parent class's function
      GBaseExecutorT<processable_type>::addConfigurationOptions(gpb, showOrigin);

      // No local data
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
   virtual bool waitForReturn(
      std::vector<boost::shared_ptr<processable_type> >&
      , const std::size_t&
      , const std::size_t&
   ) {
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

   /***************************************************************************/
   /**
    * Adds local configuration options to a GParserBuilder object
    *
    * @param gpb The GParserBuilder object to which configuration options should be added
    * @param showOrigin Makes the function indicate the origin of parameters in comments
    */
   virtual void addConfigurationOptions (
      Gem::Common::GParserBuilder& gpb
      , const bool& showOrigin
   ) {
      // Call our parent class's function
      GBaseExecutorT<processable_type>::addConfigurationOptions(gpb, showOrigin);

      // No local data
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
   virtual bool waitForReturn(
      std::vector<boost::shared_ptr<processable_type> >&
      , const std::size_t&
      , const std::size_t&
   ) {
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
class GBrokerConnector2T
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
   GBrokerConnector2T()
   : GBaseExecutorT<processable_type>()
   , srm_(DEFAULTSRM)
   , maxResubmissions_(DEFAULTMAXRESUBMISSIONS)
   , doLogging_(false)
   , CurrentBufferPort_(new Gem::Courtier::GBufferPortT<boost::shared_ptr<processable_type> >())
   {
      GBROKER(processable_type)->enrol(CurrentBufferPort_);
   }

   /***************************************************************************/
   /**
    * The copy constructor
    *
    * @param cp A copy of another GBrokerConnector object
    */
   GBrokerConnector2T(const GBrokerConnector2T<processable_type>& cp)
   : GBaseExecutorT<processable_type>(cp)
   , srm_(cp.srm_)
   , maxResubmissions_(cp.maxResubmissions_)
   , doLogging_(cp.doLogging_)
   , CurrentBufferPort_(new Gem::Courtier::GBufferPortT<boost::shared_ptr<processable_type> >())
   {
      GBROKER(processable_type)->enrol(CurrentBufferPort_);
   }

   /***************************************************************************/
   /**
    * The destructor
    */
   virtual ~GBrokerConnector2T()
   {
      CurrentBufferPort_.reset();
   }

   /***************************************************************************/
   /**
    * A standard assignment operator for GBrokerConnector2T<processable_type> objects,
    *
    * @param cp A copy of another GBrokerConnector2T<processable_type> object
    * @return A constant reference to this object
    */
   const GBrokerConnector2T<processable_type>& operator=(const GBrokerConnector2T<processable_type>& cp) {
      GBrokerConnector2T<processable_type>::load(&cp);
      return *this;
   }

   /***************************************************************************/
   /**
    * Loads the data of another GBrokerConnector2T object
    *
    * @param cp A constant pointer to another GBrokerConnector2T object
    */
   void load(GBrokerConnector2T<processable_type> const * const cp) {
      // Load our parent classes data
      GBaseExecutorT<processable_type>::load(cp);

      // Local data
      srm_ = cp->srm_;
      maxResubmissions_ = cp->maxResubmissions_;
      doLogging_ = cp->doLogging_;
   }

   /***************************************************************************/
   /**
    * Checks for equality with another GBrokerConnector2T<processable_type> object
    *
    * @param  cp A constant reference to another GBrokerConnector2T<processable_type> object
    * @return A boolean indicating whether both objects are equal
    */
   bool operator==(const GBrokerConnector2T<processable_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GBrokerConnector2T<processable_type>::operator==","cp", CE_SILENT);
   }

   /***************************************************************************/
   /**
    * Checks for inequality with another GBrokerConnector2T<processable_type> object
    *
    * @param  cp A constant reference to another GBrokerConnector2T<processable_type> object
    * @return A boolean indicating whether both objects are inequal
    */
   bool operator!=(const GBrokerConnector2T<processable_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GBrokerConnector2T<processable_type>::operator!=","cp", CE_SILENT);
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
         const GBrokerConnector2T<processable_type>& cp
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
      deviations.push_back(GBaseExecutorT<processable_type>::checkRelationShipWith(cp, e, limit, "GBrokerConnector2T<processable_type>", y_name, withMessages));

      // Check local data
      deviations.push_back(checkExpectation(withMessages, "GBrokerConnector2T<processable_type>", srm_, cp.srm_, "srm_", "cp.srm_", e , limit));
      deviations.push_back(checkExpectation(withMessages, "GBrokerConnector2T<processable_type>", maxResubmissions_, cp.maxResubmissions_, "maxResubmissions_", "cp.maxResubmissions_", e , limit));
      deviations.push_back(checkExpectation(withMessages, "GBrokerConnector2T<processable_type>", doLogging_, cp.doLogging_, "doLogging_", "cp.doLogging_", e , limit));

      return evaluateDiscrepancies("GBrokerConnector2T<processable_type>", caller, deviations, e);
   }

   /***************************************************************************/
   /**
    * Adds local configuration options to a GParserBuilder object
    *
    * @param gpb The GParserBuilder object to which configuration options should be added
    * @param showOrigin Makes the function indicate the origin of parameters in comments
    */
   virtual void addConfigurationOptions (
      Gem::Common::GParserBuilder& gpb
      , const bool& showOrigin
   ) {
      std::string comment;
      std::string comment1;
      std::string comment2;

      // Call our parent class's function
      GBaseExecutorT<processable_type>::addConfigurationOptions(gpb, showOrigin);

      // Add local data

      comment = ""; // Reset the comment string
      comment += "Indicates whether GBrokerConnector2T<processable_type> should;";
      comment += "accept an incomplete return of work items (0), resubmit missing work;";
      comment += "items in order to achieve a full set (1) or wait indefinitely;";
      comment += "for the return of work items (2).";
      if(showOrigin) comment += "[GBrokerConnector2T<processable_type>]";
      gpb.registerFileParameter<submissionReturnMode>(
         "submissionReturnMode" // The name of the variable
         , DEFAULTSRM // The default value
         , boost::bind(
            &GBrokerConnector2T<processable_type>::setSubmissionReturnMode
            , this
            , _1
           )
         , Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
         , comment
      );

      comment = ""; // Reset the comment string
      comment += "The amount of resubmissions allowed if a full return of work;";
      comment += "items was expected but only a subset has returned;";
      if(showOrigin) comment += "[GBrokerConnector2T<processable_type>]";
      gpb.registerFileParameter<std::size_t>(
         "maxResubmissions" // The name of the variable
         , DEFAULTMAXRESUBMISSIONS // The default value
         , boost::bind(
            &GBrokerConnector2T<processable_type>::setMaxResubmissions
            , this
            , _1
           )
         , Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
         , comment
      );

      comment = ""; // Reset the comment string
      comment += "Activates (1) or de-activates (0) logging;";
      comment += "iteration's first timeout;";
      if(showOrigin) comment += "[GBrokerConnector2T<processable_type>]";
      gpb.registerFileParameter<bool>(
         "doLogging" // The name of the variable
         , false // The default value
         , boost::bind(
            &GBrokerConnector2T<processable_type>::doLogging
            , this
            , _1
           )
         , Gem::Common::VAR_IS_SECONDARY // Alternative: VAR_IS_ESSENTIAL
         , comment
      );
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
    * Allows to perform necessary setup work for an iteration
    */
   virtual void iterationSetup() {
      // We want to be able to calculate proper turn-around times for individuals in logging mode
      if(doLogging_) {
         iterationStartTimes_.push_back(
            boost::tuple<SUBMISSIONCOUNTERTYPE, boost::posix_time::ptime> (
               GBaseExecutorT<processable_type>::submission_counter_
               , GBaseExecutorT<processable_type>::iterationStartTime_
            )
         );
      }
   }

   /***************************************************************************/
   /**
    * Wait for at least partial return of work items, until a time-out occurs.
    * Incomplete sets of returned items must be expected.
    */
   bool waitForTimeout(
      std::vector<boost::shared_ptr<processable_type> >& workItems
      , const std::size_t& start
      , const std::size_t& end
   ) {
      bool complete = false;

      // The expected number of work items from the current iteration
      std::size_t expectedNumber = end-start;
      std::size_t nReturnedCurrent = 0;

      // Remove the submitted items, we do not need them anymore
      workItems.erase(workItems.begin()+start, workItems.begin()+end);

      // An indication of whether all items have returned already
      std::vector<bool> returnedItemPos(workItems.size());
      // Holds a true for items that have returned or haven't been submitted
      Gem::Common::assignVecConst(returnedItemPos, true);
      // Initialize positions of items that have been submitted with false
      for(std::size_t i=0; i<expectedNumber; i++) {
         returnedItemPos[start+i] = false;
      }

      // Note the current iteration for easy reference
      SUBMISSIONCOUNTERTYPE current_iteration = GBaseExecutorT<processable_type>::submission_counter_;

      boost::posix_time::time_duration currentElapsedTime; // Will hold the elapsed time since the start of this iteration
      boost::posix_time::time_duration remainingTime; // Will hold the remaining time until the timeout is reached

      // If this is the very first submission we need to wait for
      // the first work item to return and measure the time in order to set a time out
      if(SUBMISSIONCOUNTERTYPE(0) == current_iteration) {
         // Retrieve the very first work item. We do not enact a timeout here but wait endlessly
      }

      while(
         !complete
         && currentElapsedTime < currentTimeOut_
      ) {

      }

      return complete;
   }

   /***************************************************************************/
   /**
    * Wait for return of the same iterations work items, until a time-out occurs;
    * resubmit if necessary. Note that incomplete sets of work items may still occur,
    * if not all items have returned after the maximum number of resubmissions.
    */
   bool waitForTimeoutAndResubmit(
      std::vector<boost::shared_ptr<processable_type> >& workItems
      , const std::size_t& start
      , const std::size_t& end
   ) {
      bool complete = false;

      // TODO

      return complete;
   }

   /***************************************************************************/
   /**
    * Waits until all items have returned, ignoring any timeout. Note that this
    * function may stall, if for whatever reason a work item does not return. If this
    * is not acceptable, use either waitForTimeout() or waitForTimeoutAndResubmit()
    * instead of this function. It is recommended to only use this function in
    * environments that are considered safe in the sense that work items will practically
    * always return. Local cluster environments will often fall into this category.
    * There may not be any returns from older iterations, as the algorithm waits
    * endlessly for the return of current items.
    */
   bool waitForFullReturn(
      std::vector<boost::shared_ptr<processable_type> >& workItems
      , const std::size_t& start
      , const std::size_t& end
   ) {
      bool complete = false;

      // The expected number of work items from the current iteration
      std::size_t expectedNumber = end-start;
      std::size_t nReturnedCurrent = 0;

      // Remove the submitted items, we do not need them anymore
      workItems.erase(workItems.begin()+start, workItems.begin()+end);

      // An indication of whether all items have returned already
      std::vector<bool> returnedItemPos(workItems.size());
      // Holds a true for items that have returned or haven't been submitted
      Gem::Common::assignVecConst(returnedItemPos, true);
      // Initialize positions of items that have been submitted with false
      for(std::size_t i=0; i<expectedNumber; i++) {
         returnedItemPos[start+i] = false;
      }

      // Note the current iteration for easy reference
      SUBMISSIONCOUNTERTYPE current_iteration = GBaseExecutorT<processable_type>::submission_counter_;

      while(!complete) {
         // This will block if no item can currently be retrieved
         boost::shared_ptr<processable_type> w = this->retrieve();

#ifdef DEBUG
         if(!w) {
            glogger
            << "In GBrokerConnector2T<processable_type>::waitForFullReturn(): Error!" << std::endl
            << "Received empty work item" << std::endl
            << GEXCEPTION;
         }
#endif /* DEBUG */

         SUBMISSIONCOUNTERTYPE w_iteration = boost::get<0>(w->getCourtierId());

         if(current_iteration_ == w_iteration) {
            // Mark the position of the work item in the returnedItemPos vector and cross-check
            std::size_t w_pos = boost::get<1>(w->getCourtierId());

            if(w_pos<start || w_pos >= end) {
               glogger
               << "In GBrokerConnector2T<processable_type>::waitForFullReturn(): Error!" << std::endl
               << "Received work item for position " << w_pos << " while" << std::endl
               << "only a range [" << begin << ", " << end << ") was expected." << std::endl
               << GEXCEPTION;
            }

            if(false == returnedItemPos.at(w_pos)) {
               returnedItemPos.at(w_pos) = true;
            } else { // This should not happen
               glogger
               << "In GBrokerConnector2T<processable_type>::waitForFullReturn(): Error!" << std::endl
               << "Received work item which is marked as having already returned" << std::endl
               << GEXCEPTION;
            }

            // Add the item to the workItems vector at the start of the range
            workItems.insert(workItems.begin()+start, p);
            nReturnedCurrent++;
         } else { // We do not allow returns from older iterations in this mode
            glogger
            << "In GBrokerConnector2T<processable_type>::waitForFullReturn(): Error!" << std::endl
            << "Received work item from iteration " << w_iteration << std::endl
            << "while we are currently in iteration " << current_iteration << std::endl
            << GEXCEPTION;
         }

         if(nReturnedCurrent == expectedNumber) {
            complete = true;
         }
      }

      // Sort the work item vector according to the item position, so it is in pristine condition
      std::sort(workItems.begin()+start, workItems.begin()+end, courtierPosComp<processable_type>());

      return complete;
   }

   /***************************************************************************/
   /**
    * Waits for all items to return or possibly until a timeout has been reached.
    */
   virtual bool waitForReturn(
      std::vector<boost::shared_ptr<processable_type> >& workItems
      , const std::size_t& start
      , const std::size_t& end
   ) {
      bool result = false;

      switch(srm_) {
         case INCOMPLETERETURN:
            result = waitForTimeout(workItems, start, end);
            break;

         case RESUBMISSIONAFTERTIMEOUT:
            result = waitForTimeoutAndResubmit(workItems, start, end);
            break;

         case EXPECTFULLRETURN:
            result = waitForFullReturn(workItems, start, end);
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
    * Submits a single work item.
    *
    * @param w The work item to be processed
    */
   virtual void submit(boost::shared_ptr<processable_type> w) {
      CurrentBufferPort_->push_front_orig(w);
   }

   /***************************************************************************/
   /**
    * Retrieves an item from the broker, waiting indefinitely for returns
    *
    * @return The obtained processed work item
    */
   boost::shared_ptr<processable_type> retrieve() {
      // Holds the retrieved item
      boost::shared_ptr<processable_type> w;
      CurrentBufferPort_->pop_back_processed(w);

      // Perform any necessary logging work
      log(w);

      return w;
   }

private:
   /***************************************************************************/
   /**
    * Performs necessary logging work for each received work item, if requested
    */
   void log(boost::shared_ptr<processable_type> w) {
      // Make a note of the arrival times in logging mode
      if(doLogging_) {
         boost::tuple<SUBMISSIONCOUNTERTYPE,POSITIONTYPE> courtier_id = w->getCourtierId();
         logData_.push_back(
            boost::tuple<SUBMISSIONCOUNTERTYPE, SUBMISSIONCOUNTERTYPE, boost::uint32_t>(
               boost::get<0>(courtier_id)
               , GBaseExecutorT<processable_type>::submission_counter_
               , boost::posix_time::microsec_clock::local_time()
            )
         );
      }
   }

   /***************************************************************************/
   /**
    * A simple comparison operator that helps to sort individuals according to their
    * position in the population. Smaller position numbers will end up in front.
    */
   struct courtierPosComp {
      bool operator()(boost::shared_ptr<processable_type> x, boost::shared_ptr<processable_type> y) {
         using namespace boost;
         return boost::get<1>(x->getCourtierId()) < boost::get<1>(y->getCourtierId());
      }
   };

   /***************************************************************************/
   // Local data

   submissionReturnMode srm_; ///< Indicates how (long) the object shall wait for returns
   std::size_t maxResubmissions_; ///< The maximum number of re-submissions allowed if a full return of submitted items is attempted

   bool doLogging_; ///< Specifies whether arrival times of work items should be logged
   std::vector<boost::tuple<SUBMISSIONCOUNTERTYPE, SUBMISSIONCOUNTERTYPE, boost::posix_time::ptime> > logData_; ///< Holds the sending and receiving iteration as well as the time needed for completion
   std::vector<boost::tuple<SUBMISSIONCOUNTERTYPE, boost::posix_time::ptime> > iterationStartTimes_; ///< Holds the start times of given iterations, if logging is activated

   GBufferPortT_ptr CurrentBufferPort_; ///< Holds a GBufferPortT object during the calculation. Note: It is neither serialized nor copied
};


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GBROKERCONNECTOR2T_HPP_ */
