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

#include "GConsumer.hpp"
#include "GMemberBroker.hpp"

namespace GenEvA
{

  /***************************************************************/
  /**
   * The standard constructor. We want to be able to access command
   * line arguments in derived classes
   *
   * \param argc The argument count
   * \param argv The argument vector
   */
  GConsumer::GConsumer(void)
    :stopConditionReached_(false)
  {
    BROKER.enrol(this);
  }

  /***************************************************************/
  /**
   * The standard destructor. We have no local data, hence this
   * function is empty.
   */
  GConsumer::~GConsumer()
  { /* nothing */ }

  /***************************************************************/
  /**
   * Catches all exceptions, as this function is the main function
   * in a thread.
   */
  void GConsumer::process(void){
	  try{
		  this->customProcess(); // The actual business logic
	  }
	  catch(boost::thread_interrupted&){ // We have been asked to stop
		  return;
	  }
	  catch(boost::exception& e){
			 GLogStreamer gls;
			 gls << "In GConsumer::process(): Caught boost::exception with message" << endl
				 << e.diagnostic_information() << endl
				 << logLevel(CRITICAL);

			 abort();
	  }
	  catch(std::exception& e){
		 GLogStreamer gls;
		 gls << "In GConsumer::process(): Caught std::exception with message" << endl
			 << e.what() << endl
			 << logLevel(CRITICAL);

		 abort();
	  }
	  catch(...){
		 GLogStreamer gls;
		 gls << "In GConsumer::process(): Caught unknown exception." << endl
			 << logLevel(CRITICAL);

		 abort();
	  }
  }

  /***************************************************************/
  /**
   * This function sets the stop condition. It is protected, as we
   * do not want users to call this function. GMemberBroker is declared
   * friend to this class in order to be able to call it.
   */
  void GConsumer::setStopCondition(void){
    boost::mutex::scoped_lock stop_lk(stop_mutex_);
    stopConditionReached_ = true;
  }

  /***************************************************************/
  /**
   * Checks whether the stop condition was set.
   *
   * \return A boolean indicating whether the stop condition was set
   */
  bool GConsumer::stopConditionReached(void) const{
    return stopConditionReached_;
  }

  /***************************************************************/

} /* namespace GenEvA */
