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

      ar
      & BOOST_SERIALIZATION_NVP(doLogging_);
   }

   ///////////////////////////////////////////////////////////////////////

   /***************************************************************************/
   /**
    * The default constructor
    */
   GBaseExecutorT()
   : submission_counter_(SUBMISSIONCOUNTERTYPE(0))
   , doLogging_(false)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The standard copy constructor
    *
    * @param cp A copy of another GBrokerConnector object
    */
   GBaseExecutorT(const GBaseExecutorT<processable_type>& cp)
   : submission_counter_(SUBMISSIONCOUNTERTYPE(0))
   , doLogging_(cp.doLogging_)
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
    * Loads the data of another GBrokerConnector object
    *
    * @param cp A constant pointer to another GBrokerConnector object
    */
   void load(GBaseExecutorT<processable_type> const * const cp) {
      doLogging_ = cp->doLogging_;
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
      deviations.push_back(checkExpectation(withMessages, "GBaseExecutorT<processable_type>", doLogging_, cp.doLogging_, "doLogging_", "cp.doLogging_", e , limit));

      return evaluateDiscrepancies("GBaseExecutorT<processable_type>", caller, deviations, e);
   }

   /***************************************************************************/
   /**
    * Submits and retrieves a set of work items. After the work has been performed,
    * the items contained in the workItems vector may have been changed. Note that,
    * depending on the submission mode, it is not guaranteed by this function that all
    * submitted items are still contained in the vector after the call. It is also possible
    * that returned items do not belong to the current submission cycle. You might thus have
    * to post-process the vector. Note that it is impossible to submit items that are not
    * derived from processable_type.
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
         , const submissionReturnMode& srm
         , typename boost::enable_if<boost::is_base_of<processable_type, work_item> >::type* dummy = 0
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

      // Set the start time of the new iteration so we calculate a correct
      // return time for the first individual, regardless of whether older
      // individuals have returned first.
      iterationStartTime_ = boost::posix_time::microsec_clock::local_time();

      // Submit all items
      this->submitAllWorkItems(workItems, start, end);

      switch(srm) {
         case ACCEPTOLDERITEMS:
            complete = waitForIncompleteReturn();
            break;

         case REJECTOLDERITEMS:
            complete = waitForFullReturnWithTimeout();
            break;

         case EXPECTFULLRETURN:
            complete = waitForFullReturn();
            break;

         default:
         {
            glogger
            << "In GBaseExecutorT<>::workOn(): Received invalid submissionReturnMode: " << srm << std::endl
            << GEXCEPTION;
            return false; // Make the compiler happy
         }
         break;
      }

      return complete;
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
      return std::string();
   }

   /***************************************************************************/
   /**
    * Retrieves the current value of the submission counter
    *
    * @return The current value of the submission counter
    */
   SUBMISSIONCOUNTERTYPE getSubmissionId() const {
      return submission_counter_;
   }

protected:
   /***************************************************************************/
   /** @brief Wait for at least partial return of work items, until a time-out occurs */
   virtual bool waitForIncompleteReturn() = 0;
   /** @brief Wait for Return of the same iterations work items, until a time-out occurs; resubmit if necessary */
   // TODO: Fallback auf waitForFullReturn wenn Broker "capable of full return"
   virtual bool waitForFullReturnWithTimeout() = 0;
   // TODO: Fallback auf waitForFullReturn wenn Broker "capable of full return"
   /** @brief Waits until all items have returned, ignoring any timeout */
   virtual bool waitForFullReturn() = 0;
   /** @brief Submits a single work item */
   virtual void submit(boost::shared_ptr<processable_type>) = 0;

   /***************************************************************************/
   /**
    * Submission of all work items in the list
    */
   virtual void submitAllWorkItems (
         std::vector<boost::shared_ptr<processable_type> >& workItems
         , const std::size_t& start
         , const std::size_t& end
   ) {
      typename std::vector<boost::shared_ptr<processable_type> >::iterator it;
      POSITIONTYPE pos_cnt = start;
      for(it=workItems.begin()+start; it!=workItems.begin()+end; ++it) {
         (*it)->setCourtierId(boost::make_tuple<SUBMISSIONCOUNTERTYPE,POSITIONTYPE>(submission_counter_, pos_cnt++));
         this->submit(*it);
      }

      // Update the submission counter
      submission_counter_++;
   }

   /***************************************************************************/
   /**
    * Adds a logging result to the logging vector
    */
   void addLogData(boost::tuple<SUBMISSIONCOUNTERTYPE, boost::uint32_t> logData) {
      arrivalTimes_.push_back(logData);
   }

   /***************************************************************************/
   /**
    * Retrieves the start time of the iteration
    *
    * @return The start time of the iteration
    */
   boost::posix_time::ptime getIterationStartTime() const {
      return  iterationStartTime_;
   }

private:
   /***************************************************************************/
   // Local data

   SUBMISSIONCOUNTERTYPE submission_counter_; ///< Counts the number of submissions initiated by this object. NOTE: not serialized!

   bool doLogging_; ///< Specifies whether arrival times of work items should be logged
   std::vector<boost::tuple<SUBMISSIONCOUNTERTYPE, boost::uint32_t> > arrivalTimes_; ///< Holds the actual arrival times of current work items. Note: Neither serialized nor copied

   boost::posix_time::ptime iterationStartTime_; ///< Temporary that holds the start time for the retrieval of items in a given iteration
};

/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GBROKERCONNECTOR2T_HPP_ */
