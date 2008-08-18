/**
 * @file
 */

/* GBoostThreadConsumer.cpp
 *
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

#include "GBoostThreadConsumer.hpp"

namespace Gem {
namespace GenEvA {

/***************************************************************/
/**
 * The default constructor. Nothing special here.
 */
GBoostThreadConsumer::GBoostThreadConsumer()
	:GConsumer,
	 maxThreads_(DEFAULTGBTCMAXTHREADS)
{
	// Make ourselves known to the broker.
	GINDIVIDUALBROKER.enrol(dynamic_pointer_cast<GConsumer>(shared_from_this()));
}

/***************************************************************/
/**
 * Standard destructor. Nothing - our thread is interrupted by the
 * broker, and GConsumer<GIndividual>::process() catches the exception.
 */
GBoostThreadConsumer::~GBoostThreadConsumer()
{ /* nothing */ }

/***************************************************************/
/**
 * Starts the worker threads and then waits for their termination.
 * Termination of the threads is triggered by a call to GConsumer::shutdown().
 */
void GBoostThreadConsumer::process() {
	gtg_.create_threads(boost::bind(&GBoostThreadConsumer::processItems,this), maxThreads_);
	gtg_.join_all();
}

/***************************************************************/
/**
 * The function that gets new items from the broker, processes them
 * and returns them when finished. Note that we explicitly disallow lazy
 * evaluation, so we are sure that value calculation takes place in this
 * class. As this function is the main execution point of a thread, we
 * need to catch all exceptions.
 */
void GBoostThreadConsumer::processItems(){
	try{
		while(true){
			// Have we been asked to finish ?
			if(boost::this_thead::interruption_requested()) return;

			shared_ptr<GIndividual> p;
			PORTIDTYPE id = GINDIVIDUALBROKER.get(p);

			if(p){
				bool previous p->setAllowLazyEvaluation(false);
				if(p->getAttribute("command") == "evaluate") p->fitness();
				else if(p->getAttribute("command") == "mutate") p->mutate();
				p->setAllowLazyEvaluation(previous);
			}

			GINDIVIDUALBROKER.put(id,p);
		}
	}
	catch(boost::thread_interrupted&){
		// Terminate
		return;
	}
	catch(boost::exception& e){
		std::ostringstream error;
	    error << "In GBoostThreadConsumer::processItems(): Caught boost::exception with message" << std::endl
	   		  << e.diagnostic_information() << std::endl;

	    LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

  	    std::terminate();
	}
    catch(std::exception& e) {
		std::ostringstream error;
		error << "In GBoostThreadConsumer::processItems(): Caught std::exception with message" << std::endl
	  	      << e.what() << std::endl;

		LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

		std::terminate();
	}
	catch(...) {
		std::ostringstream error;
		error << "In GBoostThreadConsumer::processItems(): Caught unknown exception." << std::endl;

		LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

		std::terminate();
	}
}

/***************************************************************/
/**
 * Finalization code. Sends all threads an interrupt signal.
 * process() then waits for them to join.
 */
void GBoostThreadConsumer::shutdown() {
	gtg_.interrupt_all();
}

/***************************************************************/
/**
 * Sets the maximum number of threads. Note that this function
 * will only have an effect before the threads have been started.
 *
 * @param maxThreads The maximum number of allowed threads
 */
void GBoostThreadConsumer::setMaxThreads(boost::uint16_t maxThreads) {
	maxThreads_ = maxThreads;
}

/***************************************************************/
/**
 * Retrieves the maximum number of allowed threads
 *
 * @return The maximum number of allowed threads
 */
uint16_t GBoostThreadConsumer::getMaxThreads(void) const throw() {
	return maxThreads_;
}

/***************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
