/**
 * @file GBrokerConnector2T.hpp
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

#include <sstream>
#include <algorithm>
#include <vector>
#include <utility>

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
class GBaseExecutorT
{
   // Make sure processable_type is derived from the submission container
   BOOST_MPL_ASSERT((boost::is_base_of<GSubmissionContainerT<processable_type>, processable_type>));

public:
   /////////////////////////////////////////////////////////////////////////////

   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;
   }

   /////////////////////////////////////////////////////////////////////////////

   /***************************************************************************/
   /**
    * The default constructor
    */
   GBaseExecutorT()
      : submission_counter_(SUBMISSIONCOUNTERTYPE(0))
      , expectedNumber_(0)
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
      , expectedNumber_(0)
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
   virtual void load(GBaseExecutorT<processable_type> const * const cp) BASE {
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
      return !checkRelationshipWith_common(cp, CE_EQUALITY, 0.,"GBaseExecutorT<processable_type>::operator==","cp", CE_SILENT);
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
      return !checkRelationshipWith_common(cp, CE_INEQUALITY, 0.,"GBaseExecutorT<processable_type>::operator!=","cp", CE_SILENT);
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
   virtual boost::optional<std::string> checkRelationshipWith_common(
      const GBaseExecutorT<processable_type>& cp
      , const Gem::Common::expectation& e
      , const double& limit
      , const std::string& caller
      , const std::string& y_name
      , const bool& withMessages
   ) const BASE {
      using namespace Gem::Common;

      // Will hold possible deviations from the expectation, including explanations
      std::vector<boost::optional<std::string> > deviations;

      // Check the local local data
      // no local data

      return evaluateDiscrepancies("GBaseExecutorT<processable_type>", caller, deviations, e);
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
    * to submit items that are not derived from GSubmissionContainerT<processable_type>.
    * This function will not alter the size of the workItems vector. It does not
    * guarantee that all work items have indeed been processed. You can find out
    * via the workItemPos vector.
    *
    * @param workItems A vector with work items to be evaluated beyond the broker
    * @param workItemPos A vector of the item positions to be worked on
    * @param oldWorkItems Will hold old work items after the job submission
    * @param originator Optionally holds information on the caller
    * @return A boolean indicating whether all expected items have returned
    */
   bool workOn(
         std::vector<boost::shared_ptr<processable_type> >& workItems
         , std::vector<bool>& workItemPos
         , std::vector<boost::shared_ptr<processable_type> >& oldWorkItems
         , const std::string& originator = std::string()
   ) {
      bool complete=false;

#ifdef DEBUG
      if(!originator.empty()) {
         glogger
         << "In GBaseExecutorT<processable_type>::workOn(): Info" << std::endl
         << "workOn was called from " << originator << std::endl
         << GLOGGING;
      }
#endif /* DEBUG */

      // Check that both vectors have the same size
      if(workItems.empty() || workItems.size() != workItemPos.size()) {
         glogger
         << "In GBaseExecutorT<processable_type>::workOn(): Error!" << std::endl
         << "Received invalid sizes: " << workItems.size() << " / " << workItemPos.size() << std::endl
         << GEXCEPTION;
      }

      // The expected number of work items from the current iteration
      expectedNumber_ = std::count(workItemPos.begin(), workItemPos.end(), true);
      // Take care of a situation where no items have been submitted
      if(expectedNumber_ == 0) return true;

      // Make sure the vector of old work items is empty
      oldWorkItems.clear();

      // Allows to perform necessary setup work for an iteration
      this->iterationInit(workItems, workItemPos, oldWorkItems);

      // Submit all items
      this->submitAllWorkItems(workItems, workItemPos);

      // Wait for work items to return. This function needs to
      // be re-implemented in derived classes.
      complete = waitForReturn(workItems, workItemPos, oldWorkItems);

      // Allows to perform necessary cleanup work for an iteration
      this->iterationFinalize(workItems, workItemPos, oldWorkItems);

      // Update the submission counter
      submission_counter_++;

      return complete;
   }

   /***************************************************************************/
   /**
    * Submits a set of work items in a range. There may be unprocessed work
    * items. At your choice, these may be removed from the workItems vector
    * or will be left there.
    *
    * @param workItems A vector with work items to be evaluated beyond the broker
    * @param start The id of first item to be worked on in the vector
    * @param end The id beyond the last item to be worked on in the vector
    * @param oldWorkItems A vector holding work items from older iterations
    * @param removeUnprocessed If set to true, unprocessed work items will be removed
    * @param originator Optionally holds information on the caller
    * @return A boolean indicating whether all expected items have returned
    */
   bool workOn(
         std::vector<boost::shared_ptr<processable_type> >& workItems
         , const std::size_t& start
         , const std::size_t& end
         , std::vector<boost::shared_ptr<processable_type> >& oldWorkItems
         , const bool& removeUnprocessed = true
         , const std::string& originator = std::string()
   ) {
      bool complete=false;

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

      // Assemble a position vector
      std::vector<bool> workItemPos(workItems.size(), GBC_PROCESSED);
      for(std::size_t pos=start; pos!=end; pos++) {
         workItemPos[pos] = GBC_UNPROCESSED;
      }

      // Start the calculation
      complete = this->workOn(
            workItems
            , workItemPos
            , oldWorkItems
            , originator
      );

      // Remove unprocessed items, if necessary
      if(!complete && removeUnprocessed) {
         typename std::vector<boost::shared_ptr<processable_type> >::iterator item_it;
         std::vector<bool>::iterator pos_it;

         std::vector<boost::shared_ptr<processable_type> > workItems_tmp;
         for(
            item_it=workItems.begin()+start, pos_it=workItemPos.begin()+start
            ; item_it!=workItems.begin()+end
            ; ++item_it, ++pos_it
         ){
            // Attach processed items to the tmp vector
            if(GBC_PROCESSED == *pos_it) {
               workItems_tmp.push_back(*item_it);
            }
         }

         // Remove all work items in the range [start:end[
         workItems.erase(workItems.begin()+start, workItems.begin()+end);

         // Insert the items from the tmp vector in position "start"
         workItems.insert(workItems.begin()+start, workItems_tmp.begin(), workItems_tmp.end());

#ifdef DEBUG
         // Cross check that there are remaining work items
         if(workItems.empty()) {
            glogger
            << "In GBrokerConnector2T<>::workOn(items, begin, end, oldWorkItems): Error!" << std::endl
            << "workItems vector is empty" << std::endl
            << GEXCEPTION;
         }
#endif /* DEBUG*/
      }

      // Let the audience know
      return complete;
   }

   /***************************************************************************/
   /**
    * Submits and retrieves a set of work items in a range
    *
    * @param workItems A vector with work items to be evaluated beyond the broker
    * @param range A boost::tuple holding the boundaries of the submission range
    * @param oldWorkItems A vector holding work items from older iterations
    * @param removeUnprocessed If set to true, unprocessed work items will be removed
    * @param originator Optionally holds information on the caller
    * @return A boolean indicating whether all expected items have returned
    */
   bool workOn(
         std::vector<boost::shared_ptr<processable_type> >& workItems
         , const boost::tuple<std::size_t, std::size_t>& range
         , std::vector<boost::shared_ptr<processable_type> >& oldWorkItems
         , const bool& removeUnprocessed = true
         , const std::string& originator = std::string()
   ) {
      return this->workOn(
         workItems
         , boost::get<0>(range)
         , boost::get<1>(range)
         , oldWorkItems
         , removeUnprocessed
         , originator
      );
   }

   /***************************************************************************/
   /**
    * Submits all work items in an array
    *
    * @param workItems A vector with work items to be evaluated beyond the broker
    * @param oldWorkItems A vector holding work items from older iterations
    * @param removeUnprocessed If set to true, unprocessed work items will be removed
    * @param originator Optionally holds information on the caller
    * @return A boolean indicating whether all expected items have returned
    */
   bool workOn(
         std::vector<boost::shared_ptr<processable_type> >& workItems
         , std::vector<boost::shared_ptr<processable_type> >& oldWorkItems
         , const bool& removeUnprocessed = true
         , const std::string& originator = std::string()
   ) {
      return this->workOn(
         workItems
         , 0
         , workItems.size()
         , oldWorkItems
         , removeUnprocessed
         , originator
      );
   }


   /***************************************************************************/
   /**
    * Adds local configuration options to a GParserBuilder object
    *
    * @param gpb The GParserBuilder object to which configuration options should be added
    */
   virtual void addConfigurationOptions (
      Gem::Common::GParserBuilder& gpb
   ) BASE {
      /* no local data. hence empty */
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

protected:
   /***************************************************************************/
   /** @brief Submits a single work item */
   virtual void submit(boost::shared_ptr<processable_type>) = 0;
   /** @brief Waits for work items to return */
   virtual bool waitForReturn(
      std::vector<boost::shared_ptr<processable_type> >&
      , std::vector<bool>&
      , std::vector<boost::shared_ptr<processable_type> >&
   ) = 0;

   /***************************************************************************/
   /**
    * Allow to perform necessary setup work for an iteration. Derived classes
    * should make sure this base function is called first when they overload
    * this function.
    */
   virtual void iterationInit(
      std::vector<boost::shared_ptr<processable_type> >& workItems
      , std::vector<bool>& workItemPos
      , std::vector<boost::shared_ptr<processable_type> >& oldWorkItems
   ) BASE {
      // Set the start time of the new iteration
      iterationStartTime_ = boost::posix_time::microsec_clock::local_time();
   }

   /***************************************************************************/
   /**
    * Allows to perform necessary cleanup work for an iteration. Derived classes
    * should make sure this base function is called last when they overload
    * this function.
    */
   virtual void iterationFinalize(
      std::vector<boost::shared_ptr<processable_type> >& workItems
      , std::vector<bool>& workItemPos
      , std::vector<boost::shared_ptr<processable_type> >& oldWorkItems
   ) BASE {
      // Make a note of the time needed up to now
      boost::posix_time::time_duration iterationDuration =
            boost::posix_time::microsec_clock::local_time() - iterationStartTime_;

      // Find out about the number of returned items
      std::size_t notReturned = std::count(workItemPos.begin(), workItemPos.end(), true);
      std::size_t nReturned   = expectedNumber_ - notReturned;

      if(nReturned == 0) { // Check whether any work items have returned at all
         glogger
         << "In GBaseExecutorT<processable_type>::iterationFinalize(): Warning!" << std::endl
         << "No current items have returned" << std::endl
         << "Got " << oldWorkItems.size() << " older work items" << std::endl
         << GWARNING;
      } else { // Calculate average return times of work items
         lastAverage_ = iterationDuration/boost::numeric_cast<int>(nReturned);
      }

      // Sort old work items so they can be readily used by the caller
      std::sort(oldWorkItems.begin(), oldWorkItems.end(), courtierPosComp());
   }

   /***************************************************************************/
   /**
    * Submission of all work items in the list
    */
   void submitAllWorkItems (
      std::vector<boost::shared_ptr<processable_type> >& workItems
      , std::vector<bool>& workItemPos
   ) {
      // Submit work items
      typename std::vector<boost::shared_ptr<processable_type> >::iterator it;
      POSITIONTYPE pos_cnt = 0;
      for(it=workItems.begin(); it!=workItems.end(); ++it) {
         if(GBC_UNPROCESSED==workItemPos[pos_cnt]) { // is the item due to be submitted ? We only submit items that are marked as "unprocessed"
#ifdef DEBUG
            if(!(*it)) {
               glogger
               << "In GBaseExecutorT<processable_type>::submitAllWorkItems(): Error" << std::endl
               << "Received empty work item in position "  << pos_cnt << std::endl
               << "submission_counter_ = " << submission_counter_ << std::endl
               << GEXCEPTION;
            }
#endif
            (*it)->setCourtierId(boost::make_tuple<SUBMISSIONCOUNTERTYPE,POSITIONTYPE>(submission_counter_, pos_cnt));
            this->submit(*it);
         }
         pos_cnt++;
      }
   }

   /***************************************************************************/
   // Local data
   SUBMISSIONCOUNTERTYPE submission_counter_; ///< Counts the number of submissions initiated by this object. Note: not serialized!
   std::size_t expectedNumber_; ///< The number of work items to be submitted (and expected back)
   boost::posix_time::ptime iterationStartTime_; ///< Temporary that holds the start time for the retrieval of items in a given iteration
   boost::posix_time::time_duration lastAverage_; ///< The average time needed for the last submission

private:
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
   virtual void load(GBaseExecutorT<processable_type> const * const cp_base) OVERRIDE {
      GSerialExecutorT<processable_type> const * const cp = dynamic_cast<GSerialExecutorT<processable_type> const * const>(cp_base);

      if(!cp) { // NULL pointer
         glogger
         << "In GSerialExecutorT<processable_type>::load(): Conversion error!" << std::endl
         << GEXCEPTION;
      }

      // Load our parent classes data
      GBaseExecutorT<processable_type>::load(cp);
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
      return !checkRelationshipWith_common(cp, CE_EQUALITY, 0.,"GSerialExecutorT<processable_type>::operator==","cp", CE_SILENT);
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
      return !checkRelationshipWith_common(cp, CE_INEQUALITY, 0.,"GSerialExecutorT<processable_type>::operator!=","cp", CE_SILENT);
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


   virtual boost::optional<std::string> checkRelationshipWith_common(
      const GBaseExecutorT<processable_type>& cp_base
      , const Gem::Common::expectation& e
      , const double& limit
      , const std::string& caller
      , const std::string& y_name
      , const bool& withMessages
   ) const OVERRIDE {
      using namespace Gem::Common;

      try {
         const GSerialExecutorT<processable_type>& cp = dynamic_cast<const GSerialExecutorT<processable_type>&>(cp_base);

         // Will hold possible deviations from the expectation, including explanations
         std::vector<boost::optional<std::string> > deviations;

         // Check our parent classes data
         deviations.push_back(GBaseExecutorT<processable_type>::checkRelationShipWith(cp, e, limit, "GSerialExecutorT<processable_type>", y_name, withMessages));

         return evaluateDiscrepancies("GSerialExecutorT<processable_type>", caller, deviations, e);
      } catch(const std::bad_cast& exp) {
         glogger
         << "In GSerialExecutorT<processable_type>::checkRelationshipWith_common(): Conversion error!" << std::endl
         << "with message: " << std::endl
         << exp.what() << std::endl
         << GEXCEPTION;
      }

      // Make the compiler happy
      return boost::optional<std::string>();
   }

   /***************************************************************************/
   /**
    * Adds local configuration options to a GParserBuilder object
    *
    * @param gpb The GParserBuilder object to which configuration options should be added
    */
   virtual void addConfigurationOptions (
      Gem::Common::GParserBuilder& gpb
   ) OVERRIDE {
      // Call our parent class's function
      GBaseExecutorT<processable_type>::addConfigurationOptions(gpb);

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
   virtual void submit(boost::shared_ptr<processable_type> w) OVERRIDE {
      w->process();
   }

   /***************************************************************************/
   /**
    * Waits for work items to return. Mostlty empty, as all work is done inside
    * of the submit() function.
    */
   virtual bool waitForReturn(
      std::vector<boost::shared_ptr<processable_type> >& workItems
      , std::vector<bool>& workItemPos
      , std::vector<boost::shared_ptr<processable_type> >& oldWorkItems
   ) OVERRIDE {
      // Mark all positions as returned
      std::vector<bool>::iterator it;
      for(it=workItemPos.begin(); it!=workItemPos.end(); ++it) {
         *it = false;
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
   virtual void load(GBaseExecutorT<processable_type> const * const cp_base) OVERRIDE {
      GMTExecutorT<processable_type> const * const cp = dynamic_cast<GMTExecutorT<processable_type> const * const>(cp_base);

      if(!cp) { // NULL pointer
         glogger
         << "In GMTExecutorT<processable_type>::load(): Conversion error!" << std::endl
         << GEXCEPTION;
      }

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
      return !checkRelationshipWith_common(cp, CE_EQUALITY, 0.,"GMTExecutorT<processable_type>::operator==","cp", CE_SILENT);
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
      return !checkRelationshipWith_common(cp, CE_INEQUALITY, 0.,"GMTExecutorT<processable_type>::operator!=","cp", CE_SILENT);
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
   virtual boost::optional<std::string> checkRelationshipWith_common(
      const GBaseExecutorT<processable_type>& cp_base
      , const Gem::Common::expectation& e
      , const double& limit
      , const std::string& caller
      , const std::string& y_name
      , const bool& withMessages
   ) const OVERRIDE {
      using namespace Gem::Common;

      try {
         const GMTExecutorT<processable_type>& cp = dynamic_cast<const GMTExecutorT<processable_type>&>(cp_base);

         // Will hold possible deviations from the expectation, including explanations
         std::vector<boost::optional<std::string> > deviations;

         // Check our parent classes data
         deviations.push_back(GBaseExecutorT<processable_type>::checkRelationshipWith_common(cp, e, limit, "GMTExecutorT<processable_type>", y_name, withMessages));

         return evaluateDiscrepancies("GMTExecutorT<processable_type>", caller, deviations, e);
      } catch(const std::bad_cast& exp) {
         glogger
         << "In GMTExecutorT<processable_type>::checkRelationshipWith_common(): Conversion error!" << std::endl
         << "with message: " << std::endl
         << exp.what() << std::endl
         << GEXCEPTION;
      }

      // Make the compiler happy
      return boost::optional<std::string>();
   }

   /***************************************************************************/
   /**
    * Adds local configuration options to a GParserBuilder object
    *
    * @param gpb The GParserBuilder object to which configuration options should be added
    */
   virtual void addConfigurationOptions (
      Gem::Common::GParserBuilder& gpb
   ) OVERRIDE {
      // Call our parent class's function
      GBaseExecutorT<processable_type>::addConfigurationOptions(gpb);

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
   virtual void submit(boost::shared_ptr<processable_type> w) OVERRIDE {
      tp_.async_schedule(boost::function<bool()>(boost::bind(&processable_type::process, w)));
   }

   /***************************************************************************/
   /**
    * Waits for the thread pool to run empty.
    */
   virtual bool waitForReturn(
      std::vector<boost::shared_ptr<processable_type> >& workItems
      , std::vector<bool>& workItemPos
      , std::vector<boost::shared_ptr<processable_type> >& oldWorkItems
   ) OVERRIDE {
      tp_.wait();

      // Mark all positions as "returned"
      std::vector<bool>::iterator it;
      for(it=workItemPos.begin(); it!=workItemPos.end(); ++it) {
         *it = false;
      }

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

   typedef boost::shared_ptr<Gem::Courtier::GBufferPortT<boost::shared_ptr<processable_type> > > GBufferPortT_ptr;

public:
   /***************************************************************************/
   /**
    * The default constructor
    */
   GBrokerConnector2T()
      : GBaseExecutorT<processable_type>()
      , srm_(DEFAULTSRM)
      , maxResubmissions_(DEFAULTMAXRESUBMISSIONS)
      , waitFactor_(DEFAULTBROKERWAITFACTOR2)
      , doLogging_(false)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Initialization with a given submission return mode.
    *
    * @param srm The submission-return mode to be used
    */
   explicit GBrokerConnector2T(submissionReturnMode srm)
      : GBaseExecutorT<processable_type>()
      , srm_(srm)
      , maxResubmissions_(DEFAULTMAXRESUBMISSIONS)
      , waitFactor_(DEFAULTBROKERWAITFACTOR2)
      , doLogging_(false)
   { /* nothing */ }

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
      , waitFactor_(cp.waitFactor_)
      , doLogging_(cp.doLogging_)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The destructor
    */
   virtual ~GBrokerConnector2T()
   { /* nothing */ }

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
   virtual void load(GBaseExecutorT<processable_type> const * const cp_base) OVERRIDE {
      GBrokerConnector2T<processable_type> const * const cp = dynamic_cast<GBrokerConnector2T<processable_type> const * const>(cp_base);

      if(!cp) { // NULL pointer
         glogger
         << "In GBrokerConnector2T<processable_type>::load(): Conversion error!" << std::endl
         << GEXCEPTION;
      }

      // Load our parent classes data
      GBaseExecutorT<processable_type>::load(cp);

      // Local data
      srm_ = cp->srm_;
      maxResubmissions_ = cp->maxResubmissions_;
      waitFactor_ = cp->waitFactor_;
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
      return !checkRelationshipWith_common(cp, CE_EQUALITY, 0.,"GBrokerConnector2T<processable_type>::operator==","cp", CE_SILENT);
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
      return !checkRelationshipWith_common(cp, CE_INEQUALITY, 0.,"GBrokerConnector2T<processable_type>::operator!=","cp", CE_SILENT);
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
   virtual boost::optional<std::string> checkRelationshipWith_common(
      const GBaseExecutorT<processable_type>& cp_base
      , const Gem::Common::expectation& e
      , const double& limit
      , const std::string& caller
      , const std::string& y_name
      , const bool& withMessages
   ) const BASE {
      using namespace Gem::Common;

      try {
         const GBrokerConnector2T<processable_type>& cp = dynamic_cast<const GBrokerConnector2T<processable_type>&>(cp_base);

         // Will hold possible deviations from the expectation, including explanations
         std::vector<boost::optional<std::string> > deviations;

         // Check our parent classes data
         deviations.push_back(GBaseExecutorT<processable_type>::checkRelationshipWith_common(cp, e, limit, "GBrokerConnector2T<processable_type>", y_name, withMessages));

         // Check local data
         deviations.push_back(checkExpectation(withMessages, "GBrokerConnector2T<processable_type>", srm_, cp.srm_, "srm_", "cp.srm_", e , limit));
         deviations.push_back(checkExpectation(withMessages, "GBrokerConnector2T<processable_type>", maxResubmissions_, cp.maxResubmissions_, "maxResubmissions_", "cp.maxResubmissions_", e , limit));
         deviations.push_back(checkExpectation(withMessages, "GBrokerConnector2T<processable_type>", waitFactor_, cp.waitFactor_, "waitFactor_", "cp.waitFactor_", e , limit));
         deviations.push_back(checkExpectation(withMessages, "GBrokerConnector2T<processable_type>", doLogging_, cp.doLogging_, "doLogging_", "cp.doLogging_", e , limit));

         return evaluateDiscrepancies("GBrokerConnector2T<processable_type>", caller, deviations, e);
      } catch(const std::bad_cast& exp) {
         glogger
         << "In GBrokerConnector2T<processable_type>::checkRelationshipWith_common(): Conversion error!" << std::endl
         << "with message: " << std::endl
         << exp.what() << std::endl
         << GEXCEPTION;
      }

      // Make the compiler happy
      return boost::optional<std::string>();
   }

   /***************************************************************************/
   /**
    * Adds local configuration options to a GParserBuilder object
    *
    * @param gpb The GParserBuilder object to which configuration options should be added
    */
   virtual void addConfigurationOptions (
      Gem::Common::GParserBuilder& gpb
   ) OVERRIDE {
      std::string comment;

      // Call our parent class's function
      GBaseExecutorT<processable_type>::addConfigurationOptions(gpb);

      // Add local data

      comment = ""; // Reset the comment string
      comment += "A static factor to be applied to timeouts;";
      gpb.registerFileParameter<std::size_t>(
         "waitFactor" // The name of the variable
         , DEFAULTBROKERWAITFACTOR2 // The default value
         , boost::bind(
            &GBrokerConnector2T<processable_type>::setWaitFactor
            , this
            , _1
           )
         , Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
         , comment
      );

      comment = ""; // Reset the comment string
      comment += "The amount of resubmissions allowed if a full return of work;";
      comment += "items was expected but only a subset has returned;";
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
    * or will timeout and optionally resubmit unprocessed items.
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
    * Allows to set the wait factor to be applied to timeouts. Note that a wait
    * factor of 0 will be silently amended and become 1.
    */
   void setWaitFactor(std::size_t waitFactor) {
      if(0==waitFactor) waitFactor_=1;
      else waitFactor_ = waitFactor;
   }

   /***************************************************************************/
   /**
    * Allows to retrieve the wait factor variable
    */
   std::size_t getWaitFactor() const {
      return waitFactor_;
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
      if(!doLogging_ || logData_.empty() || iterationStartTimes_.empty()) {
         glogger
         << "In GBrokerConnector2T<processable_type>::getLoggingResults(): Error!" << std::endl
         << "Attempt to retrieve logging results when no logging seems to have taken place." << std::endl
         << "Returning an empty string."
         << GWARNING;

         return std::string();
      }

      return std::string(); // TODO
   }

   /***************************************************************************/
   /**
    * General initialization function to be called prior to the first submission
    */
   virtual void init() OVERRIDE {
      // To be called prior to all other initialization code
      GBaseExecutorT<processable_type>::init();

      // Make sure we have a valid buffer port
      if(!CurrentBufferPort_) {
         CurrentBufferPort_
           = GBufferPortT_ptr(new Gem::Courtier::GBufferPortT<boost::shared_ptr<processable_type> >());
      }

      // Add the buffer port to the broker
      GBROKER(processable_type)->enrol(CurrentBufferPort_);
   }

   /***************************************************************************/
   /**
    * General finalization function to be called after the last submission
    */
   virtual void finalize() OVERRIDE {
      // Get rid of the buffer port
      CurrentBufferPort_.reset();

      // To be called after all other finalization code
      GBaseExecutorT<processable_type>::finalize();
   }

protected:
   /***************************************************************************/
   /**
    * Allows to perform necessary setup work for an iteration
    */
   virtual void iterationInit(
      std::vector<boost::shared_ptr<processable_type> >& workItems
      , std::vector<bool>& workItemPos
      , std::vector<boost::shared_ptr<processable_type> >& oldWorkItems
   ) OVERRIDE {
      // Make sure the parent classes iterationInit function is executed first
      // This function will also update the iteration start time
      GBaseExecutorT<processable_type>::iterationInit(workItems, workItemPos, oldWorkItems);

      // We want to be able to calculate proper turn-around times for individuals in logging mode
      if(doLogging_) {
         iterationStartTimes_.push_back(
             GBaseExecutorT<processable_type>::iterationStartTime_
         );
      }
   }

   /***************************************************************************/
   /**
    * Waits for all items to return or possibly until a timeout has been reached.
    */
   virtual bool waitForReturn(
      std::vector<boost::shared_ptr<processable_type> >& workItems
      , std::vector<bool>& workItemPos
      , std::vector<boost::shared_ptr<processable_type> >& oldWorkItems
   ) {
      bool complete = false;

      switch(srm_) {
         //------------------------------------------------------------------
         // Wait for a given amount of time, decided upon by the function.
         // Items that have not returned in time may return in a later iteration
         case INCOMPLETERETURN:
            complete = this->waitForTimeOut(workItems, workItemPos, oldWorkItems);
            break;

         //------------------------------------------------------------------
         // Wait for a given amount of time, decided upon by the function.
         // If not all items have returned, re-submit work items up to a
         // predefined number of times
         case RESUBMISSIONAFTERTIMEOUT:
            complete = this->waitForTimeOutAndResubmit(workItems, workItemPos, oldWorkItems);
            break;

         //------------------------------------------------------------------
         // Wait indefinitely, until all work items have returned
         case EXPECTFULLRETURN:
            complete = this->waitForFullReturn(workItems, workItemPos, oldWorkItems);
            break;

         //------------------------------------------------------------------
         // Fallback
         default:
         {
            glogger
            << "In GBrokerConnector2T<>::waitForReturn(): Error!" << std::endl
            << "Encountered an invalid submission return mode: " << srm_ << std::endl
            << GEXCEPTION;
         }
         break;
         //------------------------------------------------------------------
      }

      return complete;
   }

private:
   /***************************************************************************/
   /**
    * Submits a single work item.
    *
    * @param w The work item to be processed
    */
   virtual void submit(boost::shared_ptr<processable_type> w) {
#ifdef DEBUG
      if(!w) {
         glogger
         << "In GBrokerConnector2T::submit(): Error!" << std::endl
         << "Work item is empty" << std::endl
         << GEXCEPTION;
      }

      if(!CurrentBufferPort_) {
         glogger
         << "In GBrokerConnector2T::submit(): Error!" << std::endl
         << "Current buffer port is empty when it shouldn't be" << std::endl
         << GEXCEPTION;
      }
#endif /* DEBUG */

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

   /***************************************************************************/
   /**
    * Retrieves an item from the broker, waiting up to a given amount of time.
    * The call will return earlier, if an item could already be retrieved.
    *
    * @return The obtained processed work item
    */
   boost::shared_ptr<processable_type> retrieve(
      const boost::posix_time::time_duration& timeout
   ) {
      // Holds the retrieved item, if any
      boost::shared_ptr<processable_type> w;

      if(CurrentBufferPort_->pop_back_processed_bool(w, timeout)) { // We have received a valid item
         // Perform any necessary logging work
         log(w);
      }

      return w;
   }

   /***************************************************************************/
   /**
    * Waits until a timeout occurs and returns, either complete (true) or
    * incomplete (false).
    */
   bool waitForTimeOut(
      std::vector<boost::shared_ptr<processable_type> >& workItems
      , std::vector<bool>& workItemPos
      , std::vector<boost::shared_ptr<processable_type> >& oldWorkItems
   ) {
      boost::shared_ptr<processable_type> w;

      // Note the current iteration for easy reference
      SUBMISSIONCOUNTERTYPE current_iteration = GBaseExecutorT<processable_type>::submission_counter_;

      std::size_t nReturnedCurrent = 0;
      boost::posix_time::time_duration currentElapsed;
      boost::posix_time::time_duration maxTimeout;
      boost::posix_time::time_duration remainingTime;

      // Check if this is the first iteration. If so, wait (possibly indefinitely)
      // for the first item to return so we can estimate a suitable timeout
      if(0 == current_iteration) { // Wait indefinitely for first item
         w = this->retrieve();
         if(w && this->addVerifiedWorkItemAndCheckComplete
               (
                  w
                  , nReturnedCurrent
                  , workItems
                  , workItemPos
                  , oldWorkItems
               )
         ) {
            return true;
         }

         currentElapsed = boost::posix_time::microsec_clock::local_time() - GBaseExecutorT<processable_type>::iterationStartTime_;
         maxTimeout = currentElapsed*(boost::numeric_cast<int>((GBaseExecutorT<processable_type>::expectedNumber_ - std::size_t(1))*waitFactor_));
      } else { // O.k., so we are dealing with an iteration > 0
         maxTimeout = GBaseExecutorT<processable_type>::lastAverage_*boost::numeric_cast<int>((GBaseExecutorT<processable_type>::expectedNumber_*waitFactor_));
      }

      while(true) { // Loop until a timeout is reached or all current items have returned
         currentElapsed = boost::posix_time::microsec_clock::local_time() - GBaseExecutorT<processable_type>::iterationStartTime_;
         if(currentElapsed > maxTimeout) {
            return false;
         } else {
            remainingTime = maxTimeout - currentElapsed;
         }

         w = retrieve(remainingTime);
         if(w && this->addVerifiedWorkItemAndCheckComplete
               (
                  w
                  , nReturnedCurrent
                  , workItems
                  , workItemPos
                  , oldWorkItems
               )
         ) {
            break;
         }
      }

      return true;
   }

   /***************************************************************************/
   /**
    * Waits until a timeout occurs, then resubmits missing items up to a maximum
    * number of times.
    */
   bool waitForTimeOutAndResubmit(
      std::vector<boost::shared_ptr<processable_type> >& workItems
      , std::vector<bool>& workItemPos
      , std::vector<boost::shared_ptr<processable_type> >& oldWorkItems
   ) {
      bool complete=false;
      std::size_t nResubmissions = 0;

      do {
         complete = waitForTimeOut(workItems, workItemPos, oldWorkItems);
      } while(!complete && ++nResubmissions < maxResubmissions_);

      return complete;
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
      std::vector<boost::shared_ptr<processable_type> >& workItems
      , std::vector<bool>& workItemPos
      , std::vector<boost::shared_ptr<processable_type> >& oldWorkItems
   ) {
      std::size_t nReturnedCurrent = 0;
      while(
            !addVerifiedWorkItemAndCheckComplete(
               this->retrieve()
               , nReturnedCurrent
               , workItems
               , workItemPos
               , oldWorkItems
            )
      );

      return true;
   }

   /***************************************************************************/
   /**
    * Adds a work item to the corresponding vectors. This function assumes that
    * the work item is valid, i.e. points to a valid object.
    *
    * @return A boolean indicating whether all work items of the current iteration were received
    */
   bool addVerifiedWorkItemAndCheckComplete (
      boost::shared_ptr<processable_type> w
      , std::size_t& nReturnedCurrent
      , std::vector<boost::shared_ptr<processable_type> >& workItems
      , std::vector<bool>& workItemPos
      , std::vector<boost::shared_ptr<processable_type> >& oldWorkItems
   ) {
      bool complete = false;
      SUBMISSIONCOUNTERTYPE current_iteration = GBaseExecutorT<processable_type>::submission_counter_;
      SUBMISSIONCOUNTERTYPE w_iteration = boost::get<0>(w->getCourtierId());

      if(current_iteration == w_iteration) {
         // Mark the position of the work item in the workItemPos vector and cross-check
         std::size_t w_pos = boost::get<1>(w->getCourtierId());

         if(w_pos >= workItems.size()) {
            glogger
            << "In GBrokerConnector2T<processable_type>::addVerifiedWorkItemAndCheckComplete(): Error!" << std::endl
            << "Received work item for position " << w_pos << " while" << std::endl
            << "only a range [0"  << ", " << workItems.size() << "[ was expected." << std::endl
            << GEXCEPTION;
         }

         if(GBC_UNPROCESSED == workItemPos.at(w_pos)) {
            workItemPos.at(w_pos) = GBC_PROCESSED; // Successfully returned
            workItems.at(w_pos) = w;
            if(++nReturnedCurrent == GBaseExecutorT<processable_type>::expectedNumber_) {
               complete = true;
            }
         } // no else. Re-submitted items might return twice

      } else { // It could be that a previous submission did not expect a full return. Hence older items may occur
         oldWorkItems.push_back(w);
      }

      return complete;
   }

   /***************************************************************************/
   /**
    * Performs necessary logging work for each received work item, if requested
    */
   void log(boost::shared_ptr<processable_type> w) {
      // Make a note of the arrival times in logging mode
      if(doLogging_) {
         boost::tuple<SUBMISSIONCOUNTERTYPE,POSITIONTYPE> courtier_id = w->getCourtierId();
         logData_.push_back(
            boost::tuple<SUBMISSIONCOUNTERTYPE, SUBMISSIONCOUNTERTYPE, boost::posix_time::ptime>(
               boost::get<0>(courtier_id)
               , GBaseExecutorT<processable_type>::submission_counter_
               , boost::posix_time::microsec_clock::local_time()
            )
         );
      }
   }

   /***************************************************************************/
   // Local data

   submissionReturnMode srm_; ///< Indicates how (long) the object shall wait for returns
   std::size_t maxResubmissions_; ///< The maximum number of re-submissions allowed if a full return of submitted items is attempted
   std::size_t waitFactor_; ///< A static factor to be applied to timeouts

   bool doLogging_; ///< Specifies whether arrival times of work items should be logged
   std::vector<boost::tuple<SUBMISSIONCOUNTERTYPE, SUBMISSIONCOUNTERTYPE, boost::posix_time::ptime> > logData_; ///< Holds the sending and receiving iteration as well as the time needed for completion
   std::vector<boost::posix_time::ptime> iterationStartTimes_; ///< Holds the start times of given iterations, if logging is activated

   GBufferPortT_ptr CurrentBufferPort_; ///< Holds a GBufferPortT object during the calculation. Note: It is neither serialized nor copied
};


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GBROKERCONNECTOR2T_HPP_ */
