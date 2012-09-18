/**
 * @file GBoostThreadConsumerT.hpp
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

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cast.hpp>
#include <boost/cstdint.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GBOOSTTHREADCONSUMERT_HPP_
#define GBOOSTTHREADCONSUMERT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here

#include "common/GThreadGroup.hpp"
#include "common/GHelperFunctions.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/GConsumer.hpp"
#include "courtier/GSubmissionContainerT.hpp"

namespace Gem {
namespace Courtier {

const boost::uint16_t DEFAULTGBTCMAXTHREADS = 4;

/******************************************************************************/
/**
 * A derivative of GConsumer, that processes items in separate threads.
 * Objects of this class can exist alongside a networked consumer, as the broker
 * accepts more than one consumer. You can thus use this class to aid networked
 * optimization, if the server has spare CPU cores that would otherwise run idle.
 * The class makes use of the template arguments' process() function.
 */
template <class processable_type>
class GBoostThreadConsumerT
	:public Gem::Courtier::GConsumer
{
private:
	// Make sure processable_type adheres to the GSubmissionContainerT interface
	BOOST_MPL_ASSERT((boost::is_base_of<Gem::Courtier::GSubmissionContainerT<processable_type>, processable_type>));

protected:
   class GWorker; ///< Forward declaration
   class GDefaultWorker; ///< Forward declaration

public:
	/***************************************************************************/
	/**
	 * The default constructor. Nothing special here.
	 */
	GBoostThreadConsumerT()
		: Gem::Courtier::GConsumer()
		, maxThreads_(boost::numeric_cast<std::size_t>(Gem::Common::getNHardwareThreads(DEFAULTGBTCMAXTHREADS)))
		, stop_(false)
		, broker_(GBROKER(processable_type))
	   , workerTemplate_(new GDefaultWorker())
	{ /* nothing */ }

   /***************************************************************************/
	/**
	* Standard destructor. Nothing - our threads receive the stop
	* signal from the broker and shouldn't exist at this point anymore.
	*/
	virtual ~GBoostThreadConsumerT()
	{ /* nothing */ }

   /***************************************************************************/
	/**
	* Sets the maximum number of threads. Note that this function
	* will only have an effect before the threads have been started.
	* If maxThreads is set to 0, an attempt will be made to automatically
	* determine a suitable number of threads.
	*
	* @param maxThreads The maximum number of allowed threads
	*/
	void setMaxThreads(const std::size_t& maxThreads) {
		if(maxThreads == 0) {
			maxThreads_ = boost::numeric_cast<std::size_t>(Gem::Common::getNHardwareThreads(DEFAULTGBTCMAXTHREADS));
		}
		else {
			maxThreads_ = maxThreads;
		}
	}

   /***************************************************************************/
	/**
	* Retrieves the maximum number of allowed threads
	*
	* @return The maximum number of allowed threads
	*/
	std::size_t getMaxThreads(void) const  {
		return maxThreads_;
	}

   /***************************************************************************/
	/**
	* Finalization code. Sends all threads an interrupt signal.
	* process() then waits for them to join.
	*/
	void shutdown() {
		{
			boost::unique_lock<boost::shared_mutex> lock(stopMutex_);
			stop_=true;
			lock.unlock();
		}

		gtg_.join_all();
		workers_.clear();
	}

   /***************************************************************************/
	/**
	* A unique identifier for a given consumer
	*
	* @return A unique identifier for a given consumer
	*/
	virtual std::string getConsumerName() const {
	  return std::string("GBoostThreadConsumerT");
	}

   /***************************************************************************/
   /**
   * Starts the worker threads. This function will not block.
   * Termination of the threads is triggered by a call to GConsumer::shutdown().
   */
   void async_startProcessing() {
#ifdef DEBUG
      if(!workerTemplate_) { // Is a template empty ?
         raiseException(
            "In GBoostThreadConsumerT<processable_type>::async_startProcessing(): Error!" << std::endl
            << "workerTemplate_ is empty when it should not be empty" << std::endl
         );
      }
#endif /* DEBUG */

      for(std::size_t i=0; i<maxThreads_; i++) {
         boost::shared_ptr<GWorker> p_worker = workerTemplate_->clone();
         p_worker->setThreadId(i);
         gtg_.create_thread(boost::bind(&GBoostThreadConsumerT<processable_type>::GWorker::run, p_worker, this));

         workers_.push_back(p_worker);
      }
   }

protected:
   /***************************************************************************/
   /**
    * Allows to register a different worker template with this class. This facility
    * is meant to be used by derived classes only, hence this function is protected.
    */
   void registerWorkerTemplate(boost::shared_ptr<GWorker> workerTemplate) {
#ifdef DEBUG
      if(!workerTemplate_) { // Is a template empty ?
         raiseException(
            "In GBoostThreadConsumerT<processable_type>::registerWorkerTemplate(): Error!" << std::endl
            << "workerTemplate is empty when it should not be empty" << std::endl
         );
      }
#endif /* DEBUG */

      workerTemplate_ = workerTemplate;
   }

private:
	GBoostThreadConsumerT(const GBoostThreadConsumerT<processable_type>&); ///< Intentionally left undefined
	const GBoostThreadConsumerT<processable_type>& operator=(const GBoostThreadConsumerT<processable_type>&); ///< Intentionally left undefined

	std::size_t maxThreads_; ///< The maximum number of allowed threads in the pool
	Gem::Common::GThreadGroup gtg_; ///< Holds the processing threads
	mutable boost::shared_mutex stopMutex_;
	mutable bool stop_; ///< Set to true if we are expected to stop
	boost::shared_ptr<GBrokerT<processable_type> > broker_; ///< A shortcut to the broker so we do not have to go through the singleton
   std::vector<boost::shared_ptr<GWorker> > workers_; ///< Holds the worker objects
   boost::shared_ptr<GWorker> workerTemplate_; ///< All workers will be created as a clone of this worker

protected:
   /***************************************************************************/
   /**
    * A nested class that performs the actual work inside of a thread. It is
    * meant as a means to  Classes derived from GBoostThreadConsumerT
    * may use their own derivative from this class and store complex
    * information associated with the execution inside of the worker threads.
    * Note that a GWorker(-derivative) must be copy-constructible and implement
    * the clone() function.
    */
   class GWorker{
   public:
      /************************************************************************/
      /**
       * The default constructor
       */
      GWorker()
         : thread_id_(0)
      { /* nothing */ }

      /************************************************************************/
      /**
       * The copy constructor. We do not copy the thread id, as it is set by
       * async_startprocessing().
       */
      GWorker(const GWorker& cp)
         : thread_id_(0)
      { /* nothing */ }

      /************************************************************************/
      /**
       * The destructor
       */
      virtual ~GWorker()
      { /* nothing */ }

      /************************************************************************/
      /**
       * The main entry point for the execution
       *
       * @param outer A pointer to the surrounding class, so we do get access to the mutex
       */
      void run(GBoostThreadConsumerT<processable_type> * outer) {
         try{
            boost::shared_ptr<processable_type> p;
            Gem::Common::PORTIDTYPE id;
            boost::posix_time::time_duration timeout(boost::posix_time::milliseconds(10));

            while(true){
               // Have we been asked to stop ?
               {
                  boost::shared_lock<boost::shared_mutex> lock(outer->stopMutex_);
                  if(outer->stop_) {
                     lock.unlock();
                     break;
                  }
               }

               // If we didn't get a valid item, start again with the while loop
               if(!outer->broker_->get(id, p, timeout)) {
                  continue;
               }

   #ifdef DEBUG
               // Check that we indeed got a valid item
               if(!p) { // We didn't get a valid item after all
                  raiseException(
                     "In GBoostThreadConsumerT<processable_type>::GWorker::operator(): Error!" << std::endl
                     << "Got empty item when it shouldn't be empty!" << std::endl
                  );
               }
   #endif /* DEBUG */

               // Initiate the actual processing
               this->process(p);

               // Return the item to the broker. The item will be discarded
               // if the requested target queue cannot be found.
               try {
                  while(!outer->broker_->put(id, p, timeout)){
                     // Terminate if we have been asked to stop
                     boost::shared_lock<boost::shared_mutex> lock(outer->stopMutex_);
                     if(outer->stop_) {
                        lock.unlock();
                        break;
                     }
                  }
               } catch (Gem::Courtier::buffer_not_present&) {
                  continue;
               }
            }
         }
         catch(boost::thread_interrupted&){ // Normal termination
            return;
         }
         catch(std::exception& e) {
            std::ostringstream error;
            error << "In GBoostThreadConsumerT<processable_type>::GWorker::operator():" << std::endl
                  << "Caught std::exception with message" << std::endl
                  << e.what() << std::endl;
            std::cerr << error.str();
            std::terminate();
         }
         catch(boost::exception& e){
            std::ostringstream error;
             error << "In GBoostThreadConsumerT<processable_type>::GWorker::operator():" << std::endl
                   << "Caught boost::exception with message" << std::endl;
             std::cerr << error.str();
             std::terminate();
         }
         catch(...) {
            std::ostringstream error;
            error << "In GBoostThreadConsumerT<processable_type>::GWorker::operator():" << std::endl
                  << "Caught unknown exception." << std::endl;
            std::cerr << error.str();
            std::terminate();
         }
      }

      /************************************************************************/
      // Some purely virtual functions

      /** @brief Creation of deep clones of this object('s derivatives) */
      virtual boost::shared_ptr<GWorker> clone() const = 0;
      /** @brief Actual per-item work is done here -- Implement this in derived classes */
      virtual void process(boost::shared_ptr<processable_type> p) = 0;

      /************************************************************************/
      /**
       * Set the thread id
       */
      void setThreadId(const std::size_t thread_id) {
         thread_id_ = thread_id;
      }

      /************************************************************************/
      /**
       * Retrieve this class'es id
       */
      std::size_t getThreadId() const {
         return thread_id_;
      }

   private:
      /************************************************************************/

      std::size_t thread_id_; ///< The id of the thread running this class'es operator()
   };

   /***************************************************************************/
   /**
    * The default derivative of GWorker that is used when no other worker has
    * been registered with our outer class.
    */
   class GDefaultWorker
      : public GBoostThreadConsumerT<processable_type>::GWorker
   {
   public:
      /************************************************************************/
      /**
       * The default constructor
       */
      GDefaultWorker() : GWorker()
      { /* nothing */ }

      /************************************************************************/
      /**
       * The copy constructor.
       */
      GDefaultWorker(const GDefaultWorker& cp) : GWorker(cp)
      { /* nothing */ }

      /************************************************************************/
      /**
       * The destructor
       */
      virtual ~GDefaultWorker()
      { /* nothing */ }

      /************************************************************************/
      /**
       * Create a deep clone of this object, camouflaged as a GWorker
       */
      virtual boost::shared_ptr<GWorker> clone() const {
         return boost::shared_ptr<GDefaultWorker>(new GDefaultWorker(*this));
      }

      /************************************************************************/
      /**
       * Actual per-item work is done here. Overload this function if you want
       * to do something different here.
       */
      virtual void process(boost::shared_ptr<processable_type> p) {
         // Do the actual work
   #ifdef DEBUG
         if(p) p->process();
         else {
            raiseException(
                  "In GBoostThreadConsumerT<processable_type>::GWorker::process(): Error!" << std::endl
                  << "Received empty pointer for processable object" << std::endl
            );
         }
   #else
         p->process();
   #endif /* DEBUG */
      }

      /************************************************************************/
   };
};

/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GBOOSTTHREADCONSUMERT_HPP_ */
