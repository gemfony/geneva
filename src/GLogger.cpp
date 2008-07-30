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

#include "GLogger.hpp"

namespace GenEvA
{

  /***********************************************************************************/
  /**
   * A standard logger. By default we only log exceptions (level 0). Different log levels 
   * are defined in the file GLogStreamer.h .
   */
  GLogger::GLogger(void){
    addLogLevel(0);
  }

  /***********************************************************************************/
  /**
   * The standard destructor. Note that log targets are stored in shared_ptr objects,
   * so destruction of log targets takes place there. GLogger takes ownership of log
   * targets, so you do not need to delete them yourself.
   */
  GLogger::~GLogger(){
    logVector_.clear();
  }
    
  /***********************************************************************************/ 
  /**
   * Adds a log target to the GLogger class. Note that log targets are stored in 
   * shared_ptr objects, so you do not need to delete them yourself at the end of
   * the program.
   */
  void GLogger::addTarget(GBaseLogTarget *gblt){
    if(!gblt){
      std::cerr << "In GLogger::addTarget: Error!" << endl
		<< "GBaseLogTarget is empty." << endl;
      exit(1);
    }

    shared_ptr<GBaseLogTarget> p(gblt);
    logVector_.push_back(p);
  }

  /***********************************************************************************/
  /**
   * This function does the actual logging. As it protects itself with a mutex, it can
   * be called concurrently from different sources. If no logger is present yet, it will
   * add the console logger.
   */
  void GLogger::log(const string& msg, uint16_t level){
    // lets protect ourself, so we are the only logger.
    boost::mutex::scoped_lock slock(mutex_logger_);
	
	
    if(logVector_.size() == 0){
      std::clog << "In GLogger::log: Warning!" << endl
		<< "No log targets present. Will" << ""
		<< "add a console logger" << endl;
	    
      this->addTarget(new GConsoleLogger());
    }
	
    // Only log if the requested log level is in our list
    if(find(logLevel_.begin(), logLevel_.end(), level) != logLevel_.end()){
      vector<shared_ptr<GBaseLogTarget> >::iterator it;
      for(it=logVector_.begin(); it!=logVector_.end(); ++it) (*it)->log(msg);
		
      ///////////////////////////////////////////////////////////////////////
      // If this is an exception and we have been asked to terminate, 
      // act accordingly. This is for emergencies!
      if(level == 0){
	for(it=logVector_.begin(); it!=logVector_.end(); ++it) {
	  (*it)->log("Exception has been raised, handled by GLogger.\n");
	  (*it)->log("Checking exception handler ...\n");
	}
			
	if(exceptionHandler_){
	  for(it=logVector_.begin(); it!=logVector_.end(); ++it) {
	    (*it)->log("Found exception handler. Calling it ...\n");
	  }
				
	  exceptionHandler_(msg);
	}
	else{
	  for(it=logVector_.begin(); it!=logVector_.end(); ++it) {
	    (*it)->log("No handler found. Terminating ...\n");
	  }
				
	  exit(1);
	}
      }
      ///////////////////////////////////////////////////////////////////////
    }
  }

  /***********************************************************************************/
  /**
   * In this class log levels are added. It is assumed that set up of log levels is done
   * before any logging takes place.
   * 
   * \param val The log level to add to the list
   */

  // It would also be possible to sort here and run unify
  void GLogger::addLogLevel(uint16_t val){
    // Is the value already contained in the list ?
    if(find(logLevel_.begin(), logLevel_.end(), val) == logLevel_.end()){
      // no - let's add it to the list
      logLevel_.push_back(val);
      // sort the list
      sort(logLevel_.begin(), logLevel_.end());
    }
  }

  /***********************************************************************************/
  /** 
   * Adds log levels <= a given threshold
   * 
   * \param val The log level up to which logging should take place 
   */
  void GLogger::addLogLevelsUpTo(uint16_t val){
    for(uint16_t i=0; i<=val; i++) GLogger::addLogLevel(i);
  }

  /***********************************************************************************/
  /**
   * Here we can check whether any log targets are present.
   */
  bool GLogger::hasLogTargets(void) const{
    if(logVector_.size() == 0) return false;
	
    return true;
  }

  /***********************************************************************************/
  /**
   * Registers a user-defined exception handler. It will be called if this GLogger object
   * receives a message of severity EXCEPTION (0).
   */
  void GLogger::registerExceptionHandler(boost::function<void(const string&)> exceptionHandler){
    if(exceptionHandler){
      exceptionHandler_ = exceptionHandler;
    }
    else {
      cout << "Warning: exceptionHandler is empty." << endl 
	   << "Exceptions will not be handled according to your specifications!" << endl;
    }
  }

  /***********************************************************************************/

} /* namespace GenEvA */
