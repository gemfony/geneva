/**
 * \file
 */

/**
 * Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "GMemberBroker.hpp"

namespace GenEvA
{

  /**********************************************************************************/
  /**
   * The default constructor. Nothing special here.
   */
  GMemberBroker::GMemberBroker(void)
    :noPopulationEnrolled_(true),
     waitingTimeSec_ (DEFAULTWAITINGTIMESEC),
     waitingTimeMSec_(DEFAULTWAITINGTIMEMSEC),
     halt_(false),
     stopped_(false),
     processingInProgress_(false)
  { /* nothing */	}

  /**********************************************************************************/
  /**
   * Standard destructor.
   */
  GMemberBroker::~GMemberBroker(){
    this->shutdown();
    gbpMap_.clear();
  }

  /**********************************************************************************/
  /**
   * Retrieves a shared_ptr to a GBiBuffer, using a key string.
   *
   * \param key A string identifying a GBiBuffer object
   * \return A shared_ptr to the GBiBuffer object corresponding to key
   */
  GBiBufferPtr GMemberBroker::at(const std::string& key){
    boost::mutex::scoped_lock gbpmap_lk(gbp_mutex_);

    // Check that we actually have this id. Return an empty
    // pointer if this is not the case.
    if(gbpMap_.find(key) == gbpMap_.end()) return GBiBufferPtr();

    return gbpMap_[key];
  }

  /**********************************************************************************/
  /**
   * Creates a new GBiBuffer object corresponding to a population object identified
   * by the string key. The population (usually a GTransferPopulation) can access the
   * port through the GMemberBroker::at(const string& key) function. Starts the consumer
   * threads if necessary.
   *
   * \param key Identifier of a population requesting the GMemberBroker services
   */
  void GMemberBroker::enrol(string key){
    { // scope for mutex
      boost::mutex::scoped_lock gbpmap_lk(gbp_mutex_); // block access

      // First we do the actual enrolling of the population.

      // check that "key" does not exist yet. Raise an error if this
      // is the case ... . This is a serious error.
      if(gbpMap_.find(key) != gbpMap_.end()){
	GException ge;
	ge << "In GMemberBroker::enrol() : Error!" << endl
	   << "Tried to add key that already exists: " << key << endl
	   << raiseException;
      }

      GBiBufferPtr p(new GBiBufferGMC());
      gbpMap_[key] = p; // add new member
    }

    // then we start the consumer thread, if still necessary
    if(noPopulationEnrolled_){
      boost::mutex::scoped_lock gcv_lk(gcv_mutex_);

      // We do not accept changes of the internal settings from now on
      processingInProgress_ = true;

      // Check that any consumers have been registered. It does not make
      // sense to continue after this point if this is not the case.
      if(gcv_.empty()){
	GException ge;
	ge << "In GMemberBroker::enrol(string key) : Error!" << endl
	   << "No consumer object has been registered. We cannot" << endl
	   << "continue without this." << endl
	   << raiseException;
      }

      // Start the processing threads of all consumers. Note that this means
      // that all consumers added after the first population has been registered
      // will be ignored. We use boost::bind to create the function object needed
      // by boost.thread on the fly.
      vector<GConsumer *>::iterator it;
      for(it=gcv_.begin(); it!=gcv_.end(); ++it){
	(*it)->init();
	consumerThreads_.create_thread(boost::bind(&GConsumer::process, *it));
      }

      // now the consumer threads should be running and we do not
      // need to start this again.
      noPopulationEnrolled_ = false;
    }
  }

  /**********************************************************************************/
  /**
   * Makes a consumer known to this class. Note that this function will only have
   * an effect when called before the first GTransferPopulation has been created,
   * as noPopulationEnrolled_ will be set to false afterwards.
   *
   * \param g A pointer to a GConsumer object
   */
  void GMemberBroker::enrol(GConsumer *gconsumer){
    boost::mutex::scoped_lock gcv_lk(gcv_mutex_);

    if(!gconsumer){
      GException ge;
      ge << "In GMemberBroker::enrol() : Error!" << endl
	 << "Bad pointer to GConsumer object given." << endl
	 << raiseException;
    }

    if(noPopulationEnrolled_) gcv_.push_back(gconsumer);
  }

  /**********************************************************************************/
  /**
   * Removes a GBiBuffer object with a given id from the list
   *
   * \param key Identifier of a population requesting its GBiBuffer to be removed
   */
  void GMemberBroker::signoff(string key){
    boost::mutex::scoped_lock gbpmap_lk(gbp_mutex_); // block access
    GBiBufferPtrMap::iterator pos;

    // check that "key" indeed exists. Raise an error if this
    // is not the case ... . This is a serious error.
    if((pos=gbpMap_.find(key)) == gbpMap_.end()){
      GException ge;
      ge << "In GMemberBroker::signoff() : Error!" << endl
	 << "Tried to remove key that does not exist: " << key << endl
	 << raiseException;
    }

    // remove the item at position pos
    gbpMap_.erase(pos);
  }


  /**********************************************************************************/
  /**
   * Retrieves a "raw" item from a GBiBuffer. This function checks all GBiBuffer
   * objects in sequence with a given timeout value. If none of them contained an item,
   * it sets the timeout variable to true. It returns true if GBiBuffer objects were
   * present or false if this was not the case or a halt condition was reached. If the
   * function returns true and the timeout variable is set to false, the item variable
   * should contain a suitable value. (note: crude logic. should be changed soon).
   *
   * \param item The actual item to be processed
   * \return A boolean value indicating whether GBiBuffer objects were present
   */
  bool GMemberBroker::get(GMemberCarrierPtr& item, bool &timeout){
    GBiBufferPtrMap::iterator pos;
    string key;
    bool first=true;

    timeout=false;

    // check that we actually have GBiBuffers.
    {
      boost::mutex::scoped_lock gbpmap_lk(gbp_mutex_);
      if(gbpMap_.empty()) return false;
    }

    bool success = false;
    while(!success){
      // Have we been told to stop ?
      if(GMemberBroker::stop()) return false;

      boost::mutex::scoped_lock gbpmap_lk(gbp_mutex_);

      if(first) {
    	  pos = gbpMap_.begin();
    	  first=false;
      }

      try{
    	  pos->second->pop_back_orig(&item, waitingTimeSec_, waitingTimeMSec_);
    	  success=true;
    	  key = pos->first;
      }
      catch(time_out& t){ /* nothing - try again */ }

      // We have reached the end of the list and did not find any items
      if(++pos == gbpMap_.end() && !success){
    	  timeout=true;
    	  break;
      }
    }

    // o.k., we've received a shared_ptr<GMemberCarrier> or ran
    // into a timeout. Set its id if this is no timeout.
    if(!timeout) item->setId(key);
    return true;
  }

  /**********************************************************************************/
  /**
   * Retrieves an item from the broker in text format. Marshalling
   * is done as part of this function.
   *
   * \param item The item to be retrieved
   * \param timeout Indicates whether a timeout occurred
   * \return A boolean value indicating whether GBiBuffer objects were present
   */
  bool GMemberBroker::get(std::string& item, bool& timeout){
    shared_ptr<GMemberCarrier> p;
    bool issuccessful = this->get(p,timeout);
    if(!issuccessful || timeout) return issuccessful;
    item = p->toString();
    return true;
  }

  /**********************************************************************************/
  /**
   * With this function, a processed item is put into the processed queue of the
   * GBiBuffer object identified by key.
   *
   * \param key An identifier for a GBiBuffer object
   * \param item A processed item
   * \return A boolean value indicating whether the operation was successful
   */
  bool GMemberBroker::put(const GMemberCarrierPtr& item){
    boost::mutex::scoped_lock gbpmap_lk(gbp_mutex_);

    string key = item->getId();

    // Do we have this key ?
    if(key=="" || (gbpMap_.find(key) == gbpMap_.end())) return false;

    // put the processed value back into the GBiBuffer
    gbpMap_[key]->push_front_processed(item);
    return true;
  }

  /**********************************************************************************/
  /**
   * Submits an item to the broker in text format. Unmarshalling is
   * done as part of the GMemberCarrier.
   *
   * \param item The item to be submitted to the broker
   * \return A boolean indicating whether the operation was successful
   */
  bool GMemberBroker::put(const string& item){
    shared_ptr<GMemberCarrier> p(new GMemberCarrier());
    p->fromString(item);
    return this->put(p);
  }

  /**********************************************************************************/
  /**
   * Sets the waiting time used for the GMemberBroker::get functions to a new value.
   *
   * \param sec second-part of the timeout
   * \param msec millisecond-part of the timeout
   */
  void GMemberBroker::setWaitingTime(uint16_t sec, uint16_t msec){
    boost::mutex::scoped_lock variable_lk(variable_mutex_);

    waitingTimeSec_  = sec;
    waitingTimeMSec_ = msec;
  }

  /**********************************************************************************/
  /**
   * Retrieves the second part of the timeout value of get functions
   *
   * \return timeout value of get functions in seconds
   */
  uint16_t GMemberBroker::getWaitingTimeSec(void) const {
    return waitingTimeSec_;
  }

  /**********************************************************************************/
  /**
   * Retrieves the millisecond part of the timeout value of get functions
   *
   * \return timeout value of get functions in milliseconds
   */
  uint16_t GMemberBroker::getWaitingTimeMSec(void) const {
    return waitingTimeMSec_;
  }

  /**********************************************************************************/
  /**
   * The function is called by others to find out whether a halt condition was reached.
   *
   * \return A boolean indicating whether a stop condition was reached
   */
  bool GMemberBroker::stop(void) const{
    return halt_;
  }

  /**********************************************************************************/
  /**
   * Apart from our own halt_ variable we also set a stop condition for
   * the consumer objects. We then wait for all threads to finish. Note
   * that this function will only have an effect when called for the
   * first time. The GMemberBroker cannot be used anymode after this
   * function was called.
   *
   * \todo Make the other functions of this class observe the stopped_ variable
   */
  void GMemberBroker::shutdown(void){
    boost::mutex::scoped_lock gcv_lk(gcv_mutex_);
    if(!stopped_){
      halt_ = true;
      vector<GConsumer *>::iterator it;

      // Set the stop condition for all consumers
      for(it=gcv_.begin(); it!= gcv_.end(); ++it) (*it)->setStopCondition();

      GLogStreamer gls;
      gls << "In GMemberBroker::~GMemberBroker() :" << endl
	  << "Waiting for consumer threads to finish" << endl
	  << logLevel(INFORMATIONAL);

      consumerThreads_.join_all();

      // Once the consumer threads have finished,
      // we take care to call their finalization routines.
      for(it=gcv_.begin(); it!= gcv_.end(); ++it) (*it)->finally();

      stopped_ = true;
    }
  }

  /**********************************************************************************/

} /* namespace GenEvA */
