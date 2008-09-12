/**
 * @file GBrokerT.hpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
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

// Standard headers go here

#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <limits>

// Boost headers go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/pool/detail/singleton.hpp>
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

// GenEvA headers go here

#include "GConsumer.hpp"
#include "GenevaExceptions.hpp"
#include "GBufferPortT.hpp"
#include "GBoundedBufferT.hpp"
#include "GThreadGroup.hpp"
#include "GLogger.hpp"

namespace Gem {
namespace Util {

/**************************************************************************************/

/** @brief The maximum allowed port id. Note that, if we have no 64 integer types,
 * we will only be able to count up to roughly 4 billion. PORTIDTYPE is defined in
 * GBufferPortT.hpp, based on whether BOOST_HAS_LONG_LONG is defined or not. */
const PORTIDTYPE MAXPORTID = std::numeric_limits<PORTIDTYPE>::max()-1;

/**************************************************************************************/
/**
 * This class acts as the main interface between producers and consumers.
 */
template<typename carryer_type>
class GBrokerT: boost::noncopyable {
	typedef typename boost::shared_ptr<GBoundedBufferWithIdT<carryer_type> > GBoundedBufferWithIdT_Ptr;
	typedef typename std::list<GBoundedBufferWithIdT_Ptr> BufferPtrList;
	typedef typename std::map<PORTIDTYPE, GBoundedBufferWithIdT_Ptr> BufferPtrMap;

public:
	/**********************************************************************************/
	/**
	 * The default constructor.
	 */
	GBrokerT(void)
		:lastId_(0),
		 currentGetPosition_(RawBuffers_.begin()),
		 buffersPresentRaw_(false),
		 buffersPresentProcessed_(false)
	{ /* nothing */}

	/**********************************************************************************/
	/**
	 * The standard destructor. Notifies all consumers that they should stop, then waits
	 * for their threads to terminate.
	 */
	virtual ~GBrokerT()
	{
		consumerThreads_.join_all();
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
	void enrol(const boost::shared_ptr<GBufferPortT<carryer_type> >& gbp) {
		// Lock the access to our internal data
		boost::mutex::scoped_lock rawLock(RawBuffersMutex_);
		boost::mutex::scoped_lock processedLock(ProcessedBuffersMutex_);

		// Complain if the lastId_ is getting too large. lastId_ should
		// be replaced by a GUID/UUID, when it becomes available in Boost.
		// Note that, if this machine has no 64 bit integer types, we can
		// only count up to roughly 4 billion.
		if(lastId_ >= MAXPORTID){
			std::ostringstream error;
		    error << "In GBrokerT<T>::enrol(): lastId_ is getting too large: " << lastId_ << std::endl;
   		    LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

  		    std::terminate();
		}

		// Get new id for GBoundedBufferWithIdT classes and increment
		// the id afterwards for later use.
		PORTIDTYPE portId = lastId_++;

		// Retrieve the processed and original queues and tag them with
		// a suitable id. Increment the id for later use during other enrollments.
		boost::shared_ptr<GBoundedBufferWithIdT<carryer_type> > original = gbp->getOriginal();
		boost::shared_ptr<GBoundedBufferWithIdT<carryer_type> > processed = gbp->getProcessed();
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
		consumerCollection_.push_back(gc);
		consumerThreads_.create_thread(boost::bind(&GConsumer::process, gc));
	}

	/**********************************************************************************/
	/**
	 * Retrieves a "raw" item from a GBufferPortT. This function will block
	 * if no item can be retrieved.
	 *
	 * @param p Holds the retrieved "raw" item
	 * @return A key that uniquely identifies the origin of p
	 */
	PORTIDTYPE get(carryer_type& p) {
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
		currentBuffer->pop_back(&p);

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
	PORTIDTYPE get(carryer_type& p, const boost::posix_time::time_duration& timeout) {
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
		currentBuffer->pop_back(&p, timeout);

		// Return the id. The function is const, hence we should be
		// able to call it in a multi-threaded environment.
		return currentBuffer->getId();
	}

	/**********************************************************************************/
	/**
	 * Puts a processed item into the processed queue. Note that the item will simply
	 * be discarded if no target queue with the required id exists. The function will
	 * block otherwise, until it is again possible to submit the item.
	 *
	 * @param key A key that uniquely identifies the origin of p
	 * @param p Holds the "raw" item to be submitted to the processed queue
	 */
	void put(const PORTIDTYPE& id, const carryer_type& p) {
		GBoundedBufferWithIdT_Ptr currentBuffer;

		boost::mutex::scoped_lock processedLock(ProcessedBuffersMutex_);

		// Do not let execution start before the first buffer has been enrolled
		while(!buffersPresentProcessed_) readyToGoProcessed_.wait(processedLock);

		// Cross-check that the id is indeed available and retrieve the buffer
		if(ProcessedBuffers_.find(id) != ProcessedBuffers_.end())
		currentBuffer = ProcessedBuffers_[id];

		// Make the mutex available again, as the last call in this
		// function could block.
		processedLock.unlock();

		// Add p to the correct buffer, if it is a valid pointer
		if(currentBuffer) currentBuffer->push_front(p);
	}

	/**********************************************************************************/
	/**
	 * Puts a processed item into the processed queue, observing a timeout. Note that
	 * the item will simply be discarded if no target queue with the required id exists.
	 * An exception will be thrown when the timeout has been reached.
	 *
	 * @param key A key that uniquely identifies the origin of p
	 * @param p Holds the "raw" item to be submitted to the processed queue
	 * @param timeout Time after which the function should time out
	 */
	void put(const PORTIDTYPE& id, const carryer_type& p,
			 const boost::posix_time::time_duration& timeout)
	{
		GBoundedBufferWithIdT_Ptr currentBuffer;

		boost::mutex::scoped_lock processedLock(ProcessedBuffersMutex_);

		// Do not let execution start before the first buffer has been enrolled
		while(!buffersPresentProcessed_) readyToGoProcessed_.wait(processedLock);

		// Cross-check that the id is indeed available and retrieve the buffer
		if(ProcessedBuffers_.find(id) != ProcessedBuffers_.end())
		currentBuffer = ProcessedBuffers_[id];

		// Make the mutex available again, as the last call in this
		// function could block.
		processedLock.unlock();

		// Add p to the correct buffer, if it is a valid pointer
		if(currentBuffer) currentBuffer->push_front(p, timeout);
	}

private:
	/**********************************************************************************/
	GBrokerT(const GBrokerT<carryer_type>&); ///< Intentionally left undefined
	const GBrokerT& operator=(const GBrokerT<carryer_type>&); ///< Intentionally left undefined

	boost::mutex RawBuffersMutex_; ///< Regulates access to the RawBuffers_ collection
	boost::mutex ProcessedBuffersMutex_; ///< Regulates access to the ProcessedBuffers_ collection

	boost::condition_variable readyToGoRaw_; ///< The get function will block until this condition variable is set
	boost::condition_variable readyToGoProcessed_; ///< The put function will block until this condition variable is set

	BufferPtrList RawBuffers_; ///< Holds GBoundedBufferWithIdT objects with raw items
	BufferPtrMap ProcessedBuffers_; ///< Holds GBoundedBufferWithIdT objects for processed items

	PORTIDTYPE lastId_; ///< The last id we've assigned to a buffer
	typename BufferPtrList::iterator currentGetPosition_; ///< The current get position in the RawBuffers_ collection
	bool buffersPresentRaw_; ///< Set to true once the first "raw" bounded buffer has been enrolled
	bool buffersPresentProcessed_; ///< Set to true once the first "processed" bounded buffer has been enrolled

	GThreadGroup consumerThreads_; ///< Holds threads running GConsumer objects
	std::vector<boost::shared_ptr<GConsumer> > consumerCollection_;
};

/**************************************************************************************/

} /* namespace Util */
} /* namespace Gem */

#endif /* GBROKERT_HPP_ */
