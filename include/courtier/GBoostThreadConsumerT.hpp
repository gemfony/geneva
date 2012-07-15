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

// Geneva headers go here

#include "common/GThreadGroup.hpp"
#include "common/GHelperFunctions.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/GConsumer.hpp"

namespace Gem {
namespace Courtier {

const boost::uint16_t DEFAULTGBTCMAXTHREADS = 4;

/***************************************************************/
/**
 * A derivative of GConsumer, that processes items in separate threads.
 * Objects of this class can exist alongside a networked consumer, as the broker
 * accepts more than one consumer. You can thus use this class to aid networked
 * optimization, if the server has spare CPU cores that would otherwise run idle.
 */
template <class processable_type>
class GBoostThreadConsumerT
	:public Gem::Courtier::GConsumer
{
public:
	/***************************************************************/
	/**
	 * The default constructor. Nothing special here.
	 */
	GBoostThreadConsumerT()
		: Gem::Courtier::GConsumer()
		, maxThreads_(boost::numeric_cast<std::size_t>(Gem::Common::getNHardwareThreads(DEFAULTGBTCMAXTHREADS)))
		, stop_(false)
		, broker_(GBROKER(processable_type))
	{ /* nothing */ }

	/***************************************************************/
	/**
	* Standard destructor. Nothing - our threads receive the stop
	* signal from the broker and shouldn't exist at this point anymore.
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
			maxThreads_ = boost::numeric_cast<std::size_t>(Gem::Common::getNHardwareThreads(DEFAULTGBTCMAXTHREADS));
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
	* Starts the worker threads. This function will not block.
	* Termination of the threads is triggered by a call to GConsumer::shutdown().
	*/
	void async_startProcessing() {
		gtg_.create_threads(boost::bind(&GBoostThreadConsumerT<processable_type>::processItems,this), maxThreads_);
	}

	/***************************************************************/
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
	}

	/*********************************************************************/
	/**
	* A unique identifier for a given consumer
	*
	* @return A unique identifier for a given consumer
	*/
	virtual std::string getConsumerName() const {
	  return std::string("GBoostThreadConsumerT");
	}

private:
	GBoostThreadConsumerT(const GBoostThreadConsumerT<processable_type>&); ///< Intentionally left undefined
	const GBoostThreadConsumerT<processable_type>& operator=(const GBoostThreadConsumerT<processable_type>&); ///< Intentionally left undefined

	std::size_t maxThreads_; ///< The maximum number of allowed threads in the pool
	Gem::Common::GThreadGroup gtg_; ///< Holds the processing threads
	mutable boost::shared_mutex stopMutex_;
	mutable bool stop_; ///< Set to true if we are expected to stop
	boost::shared_ptr<GBrokerT<processable_type> > broker_; ///< A shortcut to the broker so we do not have to go through the singleton

	/***************************************************************/
	/**
	* The function that gets new items from the broker, processes them
	* and returns them when finished. As this function is the main
	* execution point of a thread, we need to catch all exceptions.
	*/
	void processItems(){
		try{
			boost::shared_ptr<processable_type> p;
			Gem::Common::PORTIDTYPE id;
			boost::posix_time::time_duration timeout(boost::posix_time::milliseconds(10));

			while(true){
				// Have we been asked to stop ?
				{
					boost::shared_lock<boost::shared_mutex> lock(stopMutex_);
					if(stop_) {
						lock.unlock();
						break;
					}
				}

				// If we didn't get a valid item, start again with the while loop
				if(!broker_->get(id, p, timeout)) {
					continue;
				}

#ifdef DEBUG
				// Check that we indeed got a valid item
				if(!p) { // We didn't get a valid item after all
					raiseException(
						"In GBoostThreadConsumerT<processable_type>::startProcessing(): Error!" << std::endl
						<< "Got empty item when it shouldn't be empty!" << std::endl
					);
				}
#endif /* DEBUG */

				// Do the actual work
				p->process();

				// Return the item to the broker. The item will be discarded
				// if the requested target queue cannot be found.
				try {
					while(!broker_->put(id, p, timeout)){
						// Terminate if we have been asked to stop
						boost::shared_lock<boost::shared_mutex> lock(stopMutex_);
						if(stop_) {
							lock.unlock();
							break;
						}
					}
				} catch (Gem::Courtier::buffer_not_present&) {
					continue;
				}
			}
		}
		catch(boost::thread_interrupted&){
			// Terminate
			return;
		}
		catch(std::exception& e) {
			std::ostringstream error;
			error << "In GBoostThreadConsumerT<processable_type>::processItems(): Caught std::exception with message" << std::endl
			      << e.what() << std::endl;
			std::cerr << error.str();
			std::terminate();
		}
		catch(boost::exception& e){
			std::ostringstream error;
		    error << "In GBoostThreadConsumerT<processable_type>::processItems(): Caught boost::exception with message" << std::endl;
		    std::cerr << error.str();
		    std::terminate();
		}
		catch(...) {
			std::ostringstream error;
			error << "In GBoostThreadConsumerT<processable_type>::processItems(): Caught unknown exception." << std::endl;
			std::cerr << error.str();
			std::terminate();
		}
	}
};

/***************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GBOOSTTHREADCONSUMERT_HPP_ */
