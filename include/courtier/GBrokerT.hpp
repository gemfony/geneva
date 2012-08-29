/**
 * @file GBrokerT.hpp
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
#include <vector>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <limits>
#include <stdexcept>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/utility.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/thread.hpp>
#include <boost/limits.hpp>

#ifndef GBROKERT_HPP_
#define GBROKERT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GBoundedBufferWithIdT.hpp"
#include "common/GSingletonT.hpp"
#include "common/GThreadGroup.hpp"
#include "courtier/GConsumer.hpp"
#include "courtier/GBufferPortT.hpp"
#include "courtier/GCourtierEnums.hpp"

namespace Gem {
namespace Courtier {

/***********************************************************************************/
/** @brief Class to be thrown as a message in the case of a time-out in GBuffer */
class buffer_not_present: public std::exception {};

/**************************************************************************************/

/** @brief The maximum allowed port id. Note that, if we have no 64 bit integer types,
 * we will only be able to count up to roughly 4 billion. PORTIDTYPE is defined in
 * GBoundedBufferWithIdT.hpp, based on whether BOOST_HAS_LONG_LONG is defined or not. */
const Gem::Common::PORTIDTYPE MAXPORTID = boost::numeric::bounds<Gem::Common::PORTIDTYPE>::highest()-1;

/**************************************************************************************/
/**
 * This class acts as the main interface between producers and consumers.
 */
template<typename carrier_type>
class GBrokerT
	:private boost::noncopyable
{
	typedef typename boost::shared_ptr<Gem::Common::GBoundedBufferWithIdT<boost::shared_ptr<carrier_type> > > GBoundedBufferWithIdT_Ptr;
	typedef typename std::list<GBoundedBufferWithIdT_Ptr> BufferPtrList;
	typedef typename std::map<Gem::Common::PORTIDTYPE, GBoundedBufferWithIdT_Ptr> BufferPtrMap;

public:
	/**********************************************************************************/
	/**
	 * The default constructor.
	 */
	GBrokerT()
		: finalized_(false)
		, lastId_(0)
		, currentGetPosition_(RawBuffers_.begin())
		, buffersPresentRaw_(false)
		, buffersPresentProcessed_(false)
	{ /* nothing */ }

	/**********************************************************************************/
	/**
	 * The standard destructor. Notifies all consumers that they should stop, then waits
	 * for their threads to terminate.
	 */
	virtual ~GBrokerT()
	{
		// Make sure the finalization code is executed
		// (if this hasn't happened already). Calling
		// finalize() multiple times is safe.
		finalize();
	}

	/**********************************************************************************/
	/**
	 * Initializes the broker. This function does nothing. Its only purpose is to control
	 * initialization of the factory in the singleton.
	 */
	void init() { /* nothing */ }

	/**********************************************************************************/
	/**
	 * Shuts the broker down, together with all consumers.
	 */
	void finalize() {
		// Only allow one finalization action to be carried out
		if(finalized_) return;

		// Shut down all consumers
		std::vector<boost::shared_ptr<GConsumer> >::iterator it;
		for(it=consumerCollection_.begin(); it!=consumerCollection_.end(); ++it) {
			(*it)->shutdown();
		}

		// Clear raw and processed buffers and the consumer lists
		RawBuffers_.clear(); // Somehow that gets rid of the Boost 1.46 crash
		ProcessedBuffers_.clear();
		consumerCollection_.clear();

		// Make sure this function does not execute code a second time
		finalized_ = true;
	}

	/**********************************************************************************/
	/**
	 * This function is used by producers to register a new GBufferPortT object
	 * with the broker. A GBufferPortT object contains bounded buffers for raw (i.e.
	 * unprocessed) items and for processed items. A producer may at any time decide
	 * to drop a GBufferPortT. This is simply done by letting the shared_ptr<GBufferPortT>
	 * go out of scope. As the producer holds the only copy, the GBufferPortT will then be
	 * deleted. A BufferPort contains two shared_ptr<GBoundedBufferWithIdT> objects. A shared_ptr
	 * to these objects is saved upon enrollment with the broker, so that letting the
	 * shared_ptr<GBufferPortT> go out of scope will not drop the shared_ptr<GBoundedBufferWithIdT>
	 * objects immediately. This is important, as there may still be active connections
	 * with items being collected from or dropped into them by the consumers. It is the
	 * task of this function to remove the orphaned shared_ptr<GBoundedBufferWithIdT> objects.
	 * It thus needs to block access to the entire object during its operation. Note that one of
	 * the effects of this function is that the buffer collections will never run empty,
	 * once the first buffer has been registered.
	 *
	 * @param gbp A shared pointer to a new GBufferPortT object
	 */
	void enrol(boost::shared_ptr<GBufferPortT<boost::shared_ptr<carrier_type> > > gbp) {
		// Lock the access to our internal data
		boost::mutex::scoped_lock rawLock(RawBuffersMutex_);
		boost::mutex::scoped_lock processedLock(ProcessedBuffersMutex_);

		// Complain if the lastId_ is getting too large. lastId_ should
		// be replaced by a GUID/UUID, when it becomes available in Boost.
		// Note that, if this machine has no 64 bit integer types, we can
		// only count up to roughly 4 billion. Note that we do not use
		// the global logger, so we do not have too many cross-references
		// between singletons.
		if(lastId_ >= MAXPORTID){
			std::cerr << "In GBrokerT<T>::enrol(): lastId_ is getting too large: " << lastId_ << std::endl;
  		    std::terminate();
		}

		// Get new id for GBoundedBufferWithIdT classes and increment
		// the id afterwards for later use.
		// TODO: This should later be done with Boost::UUID
		Gem::Common::PORTIDTYPE portId = lastId_++;

		// Retrieve the processed and original queues and tag them with
		// a suitable id. Increment the id for later use during other enrollments.
		boost::shared_ptr<Gem::Common::GBoundedBufferWithIdT<boost::shared_ptr<carrier_type> > > original = gbp->getOriginalQueue();
		boost::shared_ptr<Gem::Common::GBoundedBufferWithIdT<boost::shared_ptr<carrier_type> > > processed = gbp->getProcessedQueue();
		original->setId(portId);
		processed->setId(portId);

		// Find orphaned items in the two collections and remove them.
		RawBuffers_.remove_if(boost::bind(&GBoundedBufferWithIdT_Ptr::unique,_1));

		for(typename BufferPtrMap::iterator it=ProcessedBuffers_.begin(); it!=ProcessedBuffers_.end();) {
			if((it->second).unique()) { // Orphaned ? Get rid of it
				ProcessedBuffers_.erase(it++);
			}
			else ++it;
		}

		// Attach the new items to the lists
		RawBuffers_.push_back(original);
		ProcessedBuffers_[portId] = processed;

		// Fix the current get-pointer. We simply attach it to the start of the list
		currentGetPosition_= RawBuffers_.begin();

		// If this was the first registered GBufferPortT object, we need to notify any
		// available consumer objects. We only check one variable, as both are set
		// simultaneously.
		if(!buffersPresentRaw_) {
			buffersPresentRaw_ = true;
			buffersPresentProcessed_ = true;

			readyToGoRaw_.notify_all();
			readyToGoProcessed_.notify_all();
		}
	}

	/**********************************************************************************/
	/**
	 * Adds a new consumer to this class and starts its thread. Note that boost::bind
	 * knows how to handle a shared_ptr.
	 *
	 * @param gc A pointer to a GConsumer object
	 */
	void enrol(boost::shared_ptr<GConsumer> gc) {
		boost::mutex::scoped_lock consumerEnrolmentLock(consumerEnrolmentMutex_);

		// Do nothing if a consumer of this type has already been registered
		if(std::find(consumerTypesPresent_.begin(), consumerTypesPresent_.end(), gc->getConsumerName()) != consumerTypesPresent_.end()) {
			return;
		}

		// Archive the consumer and its name, then start its thread
		consumerCollection_.push_back(gc);
		consumerTypesPresent_.push_back(gc->getConsumerName());

		// Initiate processing in the consumer. This call will not block.
		gc->async_startProcessing();
	}

	/**********************************************************************************/
	/**
	 * Retrieves a "raw" item from a GBufferPortT. This function will block
	 * if no item can be retrieved.
	 *
	 * @param p Holds the retrieved "raw" item
	 * @return A key that uniquely identifies the origin of p
	 */
	Gem::Common::PORTIDTYPE get(boost::shared_ptr<carrier_type> & p) {
		GBoundedBufferWithIdT_Ptr currentBuffer;

		// Locks access to our internal data until we have a copy of a buffer.
		// This will prevent the buffer from being removed, as the use count
		// is increased. Also fixes the iterator.
		{
			boost::mutex::scoped_lock rawLock(RawBuffersMutex_);

			// Do not let execution start before the first buffer has been enrolled
			while(!buffersPresentRaw_) readyToGoRaw_.wait(rawLock);

			currentBuffer = *currentGetPosition_;
			if(++currentGetPosition_ == RawBuffers_.end()) currentGetPosition_ = RawBuffers_.begin();
		}

		// Retrieve the item. This function is thread-safe.
		currentBuffer->pop_back(p);

		// Return the id. The function is const, hence we should be
		// able to call it in a multi-threaded environment.
		return currentBuffer->getId();
	}

	/**********************************************************************************/
	/**
	 * Retrieves a "raw" item from a GBufferPortT, observing a timeout. Note that upon
	 * time-out an exception is thrown.
	 *
	 * @param p Holds the retrieved "raw" item
	 * @param timeout Time after which the function should time out
	 * @return A key that uniquely identifies the origin of p
	 */
	Gem::Common::PORTIDTYPE get(boost::shared_ptr<carrier_type> & p, boost::posix_time::time_duration timeout) {
		GBoundedBufferWithIdT_Ptr currentBuffer;

		// Locks access to our internal data until we have a copy of a buffer.
		// This will prevent the buffer from being removed, as the use count
		// is increased. Also fixes the iterator.
		{
			boost::mutex::scoped_lock rawLock(RawBuffersMutex_);

			// Do not let execution start before the first buffer has been enrolled
			while(!buffersPresentRaw_) readyToGoRaw_.wait(rawLock);

			currentBuffer = *currentGetPosition_;
			if(++currentGetPosition_ == RawBuffers_.end()) currentGetPosition_ = RawBuffers_.begin();
		}

		// Retrieve the item. This function is thread-safe.
		currentBuffer->pop_back(p, timeout);

		// Return the id. The function is const, hence we should be
		// able to call it in a multi-threaded environment.
		return currentBuffer->getId();
	}

	/**********************************************************************************/
	/**
	 * Retrieves a "raw" item from a GBufferPortT, observing a timeout. The function
	 * will indicate failure to retrieve a valid item by returning a boolean.
	 *
	 * @param id A key that identifies the origin of p
	 * @param p Holds the retrieved "raw" item
	 * @param timeout Time after which the function should time out
	 * @return A boolean that indicates whether the item retrieval was successful
	 */
	bool get(
			Gem::Common::PORTIDTYPE& id
			, boost::shared_ptr<carrier_type> & p
			, boost::posix_time::time_duration timeout
	) {
		GBoundedBufferWithIdT_Ptr currentBuffer;

		// Locks access to our internal data until we have a copy of a buffer.
		// This will prevent the buffer from being removed, as the use count
		// is increased. Also fixes the iterator.
		{
			boost::mutex::scoped_lock rawLock(RawBuffersMutex_);

			// Do not let execution start before the first buffer has been enrolled
			while(!buffersPresentRaw_) readyToGoRaw_.wait(rawLock);

			currentBuffer = *currentGetPosition_;
			id = currentBuffer->getId();
			if(++currentGetPosition_ == RawBuffers_.end()) {
				currentGetPosition_ = RawBuffers_.begin();
			}
		}

		// Retrieve the item. This function is thread-safe.
		return currentBuffer->pop_back_bool(p, timeout);
	}

	/**********************************************************************************/
	/**
	 * Puts a processed item into the processed queue. Note that the item will simply
	 * be discarded if no target queue with the required id exists. The function will
	 * block otherwise, until it is again possible to submit the item.
	 *
	 * @param id A key that uniquely identifies the origin of p
	 * @param p Holds the "raw" item to be submitted to the processed queue
	 */
	void put(
			Gem::Common::PORTIDTYPE id
			, boost::shared_ptr<carrier_type> p
	) {
		GBoundedBufferWithIdT_Ptr currentBuffer;

		boost::mutex::scoped_lock processedLock(ProcessedBuffersMutex_);

		// Do not let execution start before the first buffer has been enrolled
		while(!buffersPresentProcessed_) readyToGoProcessed_.wait(processedLock);

		// Cross-check that the id is indeed available and retrieve the buffer
		if(ProcessedBuffers_.find(id) != ProcessedBuffers_.end()) currentBuffer = ProcessedBuffers_[id];

		// Make the mutex available again, as the last call in this
		// function could block.
		processedLock.unlock();

		// Add p to the correct buffer, if it is a valid pointer.
		if(currentBuffer) currentBuffer->push_front(p);
	}

	/**********************************************************************************/
	/**
	 * Puts a processed item into the processed queue, observing a timeout. The function
	 * will throw a Gem::Courtier::buffer_not_present exception if the requested buffer
	 * isn't present. The function will return false if no item could be added to the buffer
	 * inside if the allowed time limits.
	 *
	 * @param id A key that uniquely identifies the origin of p
	 * @param p Holds the item to be submitted to the processed queue
	 * @param timeout Time after which the function should time out
	 * @param A boolean indicating whether the item could be added to the queue in time
	 */
	bool put(
			Gem::Common::PORTIDTYPE id
			, boost::shared_ptr<carrier_type> p
			, boost::posix_time::time_duration timeout
	) {
		GBoundedBufferWithIdT_Ptr currentBuffer;

		//-----------------------------------------------------------------------------
		// Make sure processing can start (impossible, before any buffer
		// port objects have been added)
		boost::mutex::scoped_lock processedLock(ProcessedBuffersMutex_);

		// Do not let execution start before the first buffer has been enrolled
		while(!buffersPresentProcessed_) readyToGoProcessed_.wait(processedLock);

		//-----------------------------------------------------------------------------
		// Cross-check that the id is indeed available and retrieve the buffer
		if(ProcessedBuffers_.find(id) != ProcessedBuffers_.end()) {
			currentBuffer = ProcessedBuffers_[id];
			// Make the mutex available again
			processedLock.unlock();
		} else {
			// Make the mutex available again
			std::cerr << "Did not find buffer with id " << id << std::endl;
			processedLock.unlock();

			throw Gem::Courtier::buffer_not_present();
			return false; // Make the compiler happy
		}

		//-----------------------------------------------------------------------------
		// Add p to the correct buffer, which we now assume to be valid. If this
		// cannot be done in time, let the audience know by returning false
		return currentBuffer->push_front_bool(p, timeout);

		//-----------------------------------------------------------------------------
	}

	/**********************************************************************************/
	/**
	 * Checks whether any consumers have been enrolled at the time of calling. As
	 * consumers are maintained inside of a thread group and consumers may be added
	 * asynchronously. this function can only give a hint.
	 *
	 * @return A boolean indicating whether any consumers are registered
	 */
	bool hasConsumers() const {
		return consumerCollection_.size()>0?true:false;
	}

private:
	/**********************************************************************************/
	GBrokerT(const GBrokerT<carrier_type>&); ///< Intentionally left undefined
	const GBrokerT& operator=(const GBrokerT<carrier_type>&); ///< Intentionally left undefined

	bool finalized_; ///< Indicates whether the finalization code has already been executed

	mutable boost::mutex RawBuffersMutex_; ///< Regulates access to the RawBuffers_ collection
	mutable boost::mutex ProcessedBuffersMutex_; ///< Regulates access to the ProcessedBuffers_ collection

	mutable boost::condition_variable readyToGoRaw_; ///< The get function will block until this condition variable is set
	mutable boost::condition_variable readyToGoProcessed_; ///< The put function will block until this condition variable is set

	BufferPtrList RawBuffers_; ///< Holds GBoundedBufferWithIdT objects with raw items
	BufferPtrMap ProcessedBuffers_; ///< Holds GBoundedBufferWithIdT objects for processed items

	Gem::Common::PORTIDTYPE lastId_; ///< The last id we've assigned to a buffer
	typename BufferPtrList::iterator currentGetPosition_; ///< The current get position in the RawBuffers_ collection
	bool buffersPresentRaw_; ///< Set to true once the first "raw" bounded buffer has been enrolled
	bool buffersPresentProcessed_; ///< Set to true once the first "processed" bounded buffer has been enrolled

	std::vector<boost::shared_ptr<GConsumer> > consumerCollection_; ///< Holds the actual consumers
	std::vector<std::string> consumerTypesPresent_; ///< Holds identifying strings for each consumer
	mutable boost::mutex consumerEnrolmentMutex_; ///< Protects the enrolment of consumers
};

/**************************************************************************************/
/**
 * We require GBrokerT<T> to be a singleton. This ensures that, for a given T, one
 * and only one Broker object exists that is constructed before main begins. All
 * external communication should refer to GBROKER(T).
 */
#define GBROKER(T)      Gem::Common::GSingletonT<Gem::Courtier::GBrokerT< T > >::Instance(0)
#define RESETGBROKER(T) Gem::Common::GSingletonT<Gem::Courtier::GBrokerT< T > >::Instance(1)

/**************************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GBROKERT_HPP_ */
