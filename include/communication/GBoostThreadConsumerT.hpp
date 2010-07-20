/**
 * @file GBoostThreadConsumerT.hpp
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

// Standard headers go here

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/version.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
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


#include "GBrokerT.hpp"
#include "GConsumer.hpp"
#include "GThreadGroup.hpp"

namespace Gem {
namespace Communication {

const boost::uint16_t DEFAULTGBTCMAXTHREADS = 4;

/***************************************************************/
/**
 * A derivative of GConsumer, that processes items in separate threads.
 * Objects of this class can exist alongside a networked consumer, as the broker
 * accepts more than one consumer. You can thus use this class to aid networked
 * optimization, if the server has spare CPU cores that would otherwise run idle.
 */
template <class processable_object>
class GBoostThreadConsumerT
	:public Gem::Communication::GConsumer
{
public:
	/***************************************************************/
	/**
	 * The default constructor. Nothing special here.
	 */
	GBoostThreadConsumerT()
		: Gem::Communication::GConsumer()
		, maxThreads_(DEFAULTGBTCMAXTHREADS)
		, stop_(false)
	{ /* nothing */ }

	/***************************************************************/
	/**
	* Standard destructor. Nothing - our thread is interrupted by the
	* broker, and GConsumer<processable_object>::process() catches the exception.
	*/
	~GBoostThreadConsumerT()
	{ /* nothing */ }

	/***************************************************************/
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
	std::size_t getMaxThreads(void) const  {
		return maxThreads_;
	}

	/***************************************************************/
	/**
	* Starts the worker threads and then waits for their termination.
	* Termination of the threads is triggered by a call to GConsumer::shutdown().
	*/
	void process() {
		gtg_.create_threads(boost::bind(&GBoostThreadConsumerT::processItems,this), maxThreads_);
		gtg_.join_all();
	}

	/***************************************************************/
	/**
	* Finalization code. Sends all threads an interrupt signal.
	* process() then waits for them to join.
	*/
	void shutdown() {
		boost::mutex::scoped_lock stopLock(stopMutex_);
		stop_=true;
	}


private:
	GBoostThreadConsumerT(const GBoostThreadConsumerT&); ///< Intentionally left undefined
	const GBoostThreadConsumerT& operator=(const GBoostThreadConsumerT&); ///< Intentionally left undefined

	std::size_t maxThreads_; ///< The maxumum number of allowed threads in the pool
	Gem::Common::GThreadGroup gtg_; ///< Holds the processing threads

	boost::mutex stopMutex_;
	bool stop_; ///< Set to true if we are expected to stop

	/***************************************************************/
	/**
	* The function that gets new items from the broker, processes them
	* and returns them when finished. Note that we explicitly disallow lazy
	* evaluation, so we are sure that value calculation takes place in this
	* class. As this function is the main execution point of a thread, we
	* need to catch all exceptions.
	*/
	void processItems(){
		try{
			boost::shared_ptr<processable_object> p;
			Gem::Communication::PORTIDTYPE id;
			boost::posix_time::time_duration timeout(boost::posix_time::milliseconds(10));

			while(true){
				// Have we been asked to stop ?
				{
					boost::mutex::scoped_lock stopLock(stopMutex_);
					if(stop_) break;
				}

				try{
					id = GBROKER(boost::shared_ptr<processable_object>)->get(p, timeout);
				}
				catch(Gem::Common::condition_time_out &) { continue; }

				if(p){
					p->process();

					try{
						GBROKER(boost::shared_ptr<processable_object>)->put(id, p, timeout);
					}
					catch(Gem::Common::condition_time_out &) { continue; }
				}
			}
		}
		catch(boost::thread_interrupted&){
			// Terminate
			return;
		}
		catch(std::exception& e) {
			std::ostringstream error;
			error << "In GBoostThreadConsumerT::processItems(): Caught std::exception with message" << std::endl
			      << e.what() << std::endl;
			std::cerr << error.str();
			std::terminate();
		}
		catch(boost::exception& e){
			std::ostringstream error;
		    error << "In GBoostThreadConsumerT::processItems(): Caught boost::exception with message" << std::endl;
		    std::cerr << error.str();
		    std::terminate();
		}
		catch(...) {
			std::ostringstream error;
			error << "In GBoostThreadConsumerT::processItems(): Caught unknown exception." << std::endl;
			std::cerr << error.str();
			std::terminate();
		}
	}

};

/***************************************************************/

} /* namespace Communication */
} /* namespace Gem */

#endif /* GBOOSTTHREADCONSUMERT_HPP_ */
