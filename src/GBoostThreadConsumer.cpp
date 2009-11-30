/**
 * @file GBoostThreadConsumer.cpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include "GBoostThreadConsumer.hpp"

namespace Gem {
namespace GenEvA {

/***************************************************************/
/**
 * The default constructor. Nothing special here.
 */
GBoostThreadConsumer::GBoostThreadConsumer()
	:Gem::Util::GConsumer(),
	 maxThreads_(DEFAULTGBTCMAXTHREADS),
	 stop_(false)
{ /* nothing */ }

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
		boost::shared_ptr<GIndividual> p;
		Gem::Util::PORTIDTYPE id;
		boost::posix_time::time_duration timeout(boost::posix_time::milliseconds(10));

		while(true){
			// Have we been asked to stop ?
			{
				boost::mutex::scoped_lock stopLock(stopMutex_);
				if(stop_) break;
			}

			try{
				id = GINDIVIDUALBROKER->get(p, timeout);
			}
			catch(Gem::Util::gem_util_condition_time_out &) { continue; }

			if(p){
				p->process();

				try{
					GINDIVIDUALBROKER->put(id, p, timeout);
				}
				catch(Gem::Util::gem_util_condition_time_out &) { continue; }
			}
		}
	}
	catch(boost::thread_interrupted&){
		// Terminate
		return;
	}
    catch(std::exception& e) {
		std::ostringstream error;
		error << "In GBoostThreadConsumer::processItems(): Caught std::exception with message" << std::endl
	  	      << e.what() << std::endl;
		std::cerr << error.str();
		std::terminate();
	}
	catch(boost::exception& e){
		std::ostringstream error;
	    error << "In GBoostThreadConsumer::processItems(): Caught boost::exception with message" << std::endl;
	    std::cerr << error.str();
  	    std::terminate();
	}
	catch(...) {
		std::ostringstream error;
		error << "In GBoostThreadConsumer::processItems(): Caught unknown exception." << std::endl;
		std::cerr << error.str();
		std::terminate();
	}
}

/***************************************************************/
/**
 * Finalization code. Sends all threads an interrupt signal.
 * process() then waits for them to join.
 */
void GBoostThreadConsumer::shutdown() {
	boost::mutex::scoped_lock stopLock(stopMutex_);
	stop_=true;
}

/***************************************************************/
/**
 * Sets the maximum number of threads. Note that this function
 * will only have an effect before the threads have been started.
 * If maxThreads is set to 0, an attempt will be made to automatically
 * determine a suitable number of threads.
 *
 * @param maxThreads The maximum number of allowed threads
 */
void GBoostThreadConsumer::setMaxThreads(const std::size_t& maxThreads) {
	if(maxThreads == 0) {
		std::size_t hardwareThreads = boost::thread::hardware_concurrency();

		if(hardwareThreads > 0) maxThreads_ = hardwareThreads;
		else maxThreads_ = DEFAULTGBTCMAXTHREADS;
	}
	else {
		maxThreads_ = maxThreads;
	}
}

/***************************************************************/
/**
 * Retrieves the maximum number of allowed threads
 *
 * @return The maximum number of allowed threads
 */
std::size_t GBoostThreadConsumer::getMaxThreads(void) const  {
	return maxThreads_;
}

/***************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
