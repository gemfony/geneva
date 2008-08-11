/**
 * @file
 */

/* GBroker.hpp
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

// Standard headers go here

#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <utility>

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

#ifndef GBROKER_HPP_
#define GBROKER_HPP_

// GenEvA headers go here

#include "GenevaExceptions.hpp"
#include "GBufferPort.hpp"
#include "GBoundedBuffer.hpp"
#include "GThreadGroup.hpp"
#include "GLogger.hpp"

namespace Gem {
namespace Util {

// Forward declaration
template<class carryer_type> class GBroker;

/**************************************************************************************/
/**
 * This class forms the basis of a hierarchy of classes that take
 * objects from GBroker and process them, either locally or remotely.
 * Derived classes such as the GAsioTCPConsumer form the single point
 * of contact for remote clients. We do not want this class and its
 * derivatives to be copyable, hence we derive it from the
 * boost::noncopyable class.
 */
template<class carryer_type>
class GConsumer
	: public boost::enable_shared_from_this<GConsumer<carryer_type> >,
	  boost::noncopyable
{
public:
	/**********************************************************************************/
	/**
	 * GBroker is declared friend so that it is the only class
	 * that can access the GConsumer::process() function
	 */
	friend class GBroker<carryer_type> ;

	/**********************************************************************************/
	/**
	 * The initialization of a consumer requires access to a broker
	 *
	 * @param gb The external broker the consumer should connect to
	 */
	GConsumer(GBroker<carryer_type>* gb) :gb_(gb)
	{
		if (gb_) {
			// GBroker expects a shared_ptr<GConsumer<carryer_type> >, so we
			// use Boost's enable_shared_from_this class to create one.
			gb->enrol(this->shared_from_this());
		}
	}

	/**********************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GConsumer()
	{ /* nothing */ }

protected:
	/**********************************************************************************/
	/** @brief To be called from GConsumer::process() */
	virtual void init() = 0;
	/** @brief The actual business logic */
	virtual void customProcess() = 0;
	/** @brief To be called from GConsumer::process() */
	virtual void finally() = 0;

	/**********************************************************************************/
	/**
	 * A wrapper around customProcess needed to catch exceptions
	 */
	void process() {
		  try{
			  this->init(); // Initialize the object
			  this->customProcess(); // The actual business logic
			  this->finally(); // Final actions
		  }
		  catch(boost::thread_interrupted&){ // We have been asked to stop
			  return;
		  }
		  catch(boost::exception& e){
			  std::ostringstream error;
			  error << "In GConsumer::process(): Caught boost::exception with message" << std::endl
				    << e.diagnostic_information() << std::endl;

			  LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
			  std::terminate();
		  }
		  catch(std::exception& e){
			  std::ostringstream error;
			  error << "In GConsumer::process(): Caught std::exception with message" << std::endl
				    << e.what() << std::endl;

			  LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
			  std::terminate();
		  }
		  catch(...){
			  std::ostringstream error;
			  error << "In GConsumer::process(): Caught unknown exception." << std::endl;

			  LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
			  std::terminate();
		  }
	}

	/**********************************************************************************/
	const GBroker<carryer_type> *gb_;

private:
	GConsumer(); ///< Intentionally left undefined
	GConsumer(const GConsumer<carryer_type>&); ///< Intentionally left undefined
	const GConsumer& operator=(const GConsumer<carryer_type>&); ///< Intentionally left undefined
};

/**************************************************************************************/
////////////////////////////////////////////////////////////////////////////////////////
/**************************************************************************************/

const boost::uint32_t MAXPORTID = 1000000000; ///< The maximum allowed port id

/**************************************************************************************/
/**
 * This class acts as the main interface between producers and consumers.
 */
template<class carryer_type>
class GBroker: boost::noncopyable {
	typedef	typename boost::shared_ptr<carryer_type> carryer_type_ptr;
	typedef typename boost::shared_ptr<GBoundedBufferWithId<carryer_type_ptr> > GBoundedBufferWithId_Ptr;
	typedef typename std::list<GBoundedBufferWithId_Ptr> BufferPtrList;
	typedef typename std::map<boost::uint32_t, GBoundedBufferWithId_Ptr> BufferPtrMap;

public:
	/**********************************************************************************/
	/**
	 * The default constructor. We lock access to the buffer port collection until
	 * at least one producer has been registered. When this is the case, we unlock
	 * the mutex
	 */
	GBroker(void)
		:lastId_(0),
		 currentGetPosition_(RawBuffers_.begin()),
		 buffersPresentRaw_(false),
		 buffersPresentProcessed_(false)
	{ /* nothing */}

	/**********************************************************************************/
	/**
	 * The standard destructor. Notifies all threads that they should stop, then waits
	 * for their termination.
	 */
	virtual ~GBroker()
	{
		consumerThreads_.interrupt_all();
		consumerThreads_.join_all();
	}

	/**********************************************************************************/
	/**
	 * This function is used by producers to register a new GBufferPort object
	 * with the broker. A GBufferPort object contains bounded buffers for raw (i.e.
	 * unprocessed) items and for processed items. A producer may at any time decide
	 * to drop a GBufferPort. This is simply done by letting the shared_ptr<GBufferPort>
	 * go out of scope. As the producer holds the only copy, the GBufferPort will then be
	 * deleted. A BufferPort contains two shared_ptr<GBoundedBufferWithId> objects. A shared_ptr
	 * to these objects is saved upon enrollment with the broker, so that letting the
	 * shared_ptr<GBufferPort> go out of scope will not drop the shared_ptr<GBoundedBufferWithId>
	 * objects immediately. This is important, as there may still be active connections
	 * with items being collected from or dropped into them by the consumers. It is the
	 * task of this function to remove the orphaned shared_ptr<GBoundedBufferWithId> objects.
	 * It thus needs to block access to the entire object during its operation. Note that one of
	 * the effects of this function is that the buffer collections will never run empty,
	 * once the first buffer has been registered.
	 *
	 * @param gbp A shared pointer to a new GBufferPort object
	 */
	void enrol(const shared_ptr<GBufferPort>& gbp) {
		// Lock the access to our internal data
		boost::mutex::scoped_lock rawLock(RawBuffersMutex_);
		boost::mutex::scoped_lock processedLock(ProcessedBuffersMutex_);

		// Calculate a valid id for GBoundedBufferWithId classes and increment
		// the id afterwards for later use.
		boost::uint32_t portId = lastId_++;

		// Roll-over - BAD, we rely on the fact that no "old" items return
		// when a new producer with the same id has already been registered.
		if(lastId_ >= MAXPORTID) lastId_ = 0;

		// Retrieve the processed and original queues and tag them with
		// a suitable id. Increment the id for later use during other enrollments.
		sharted_ptr<GBoundedBufferWithId<carryer_type> > original = gbp->getOriginal();
		sharted_ptr<GBoundedBufferWithId<carryer_type> > processed = gbp->getProcessed();
		original->setId(portId);
		processed->setId(portId);

		// Find orphaned items in the two collections and remove them.
		RawBuffers_.remove_if(boost::bind(&boost::shared_ptr<booleanClass>::unique,_1));

		for(BufferPtrMap::iterator it=ProcessedBuffers_.begin(); it!=ProcessedBuffers_.end();) {
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

		// If this was the first registered GBufferPort object, we need to notify any
		// available consumer objects
		if(!buffersPresentRaw_ || buffersPresentProcessed_) {
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
	void enrol(boost::shared_ptr<GConsumer<carryer_type> > gc) {
		consumerThreads_.createThread(boost::bind(&GConsumer<carryer_type>::process, gc));
	}

	/**********************************************************************************/
	/**
	 * Retrieves a "raw" item from a GBufferPort. This function will block
	 * if no item can be retrieved.
	 *
	 * @param p Holds the retrieved "raw" item
	 * @return A key that uniquely identifies the origin of p
	 */
	boost::uint32_t get(shared_ptr<carryer_type>& p) {
		GBoundedBufferWithId_Ptr currentBuffer;

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
	 * Puts a processed item into the processed queue. Note that the item will simply
	 * be discarded if no target queue with the required id exists. The function will
	 * block otherwise, until it is again possible to submit the item.
	 *
	 * @param key A key that uniquely identifies the origin of p
	 * @param p Holds the "raw" item to be submitted to the processed queue
	 */
	void put(const boost::uint32_t& id, const shared_ptr<carryer_type>& p) {
		GBoundedBufferWithId_Ptr currentBuffer;

		boost::mutex::scoped_lock processedLock(ProcessedBuffersMutex_);

		// Do not let execution start before the first buffer has been enrolled
		while(!buffersPresentProcessed_) readyToGoProcessed_.wait(processedLock);

		// Cross-check that the id is indeed available and retrieve the buffer
		if(ProcessedBuffers_.find(id) != ProcessedBuffers_.end())
		currentBuffer = ProcessedBuffers_[id].second;

		// Make the mutex available again, as the last call in this
		// function could block.
		processedLock.unlock();

		// Add p to the correct buffer, if it is a valid pointer
		if(currentBuffer) currentBuffer->push_front_processed(p);
	}

private:
	/**********************************************************************************/
	GBroker(const GBroker<carryer_type>&); ///< Intentionally left undefined
	const GBroker& operator=(const GBroker<carryer_type>&); ///< Intentionally left undefined

	boost::mutex RawBuffersMutex_; ///< Regulates access to the RawBuffers_ collection
	boost::mutex ProcessedBuffersMutex_; ///< Regulates access to the ProcessedBuffers_ collection

	boost::condition_variable readyToGoRaw_; ///< The get function will block until this condition variable is set
	boost::condition_variable readyToGoProcessed_; ///< The put function will block until this condition variable is set

	BufferPtrList RawBuffers_; ///< Holds GBoundedBufferWithId objects with raw items
	BufferPtrMap ProcessedBuffers_; ///< Holds GBoundedBufferWithId objects for processed items

	boost::uint32_t lastId_; ///< The last id we've assigned to a buffer
	BufferPtrList::iterator currentGetPosition_; ///< The current get position in the RawBuffers_ collection
	bool buffersPresentRaw_; ///< Set to true once the first "raw" bounded buffer has been enrolled
	bool buffersPresentProcessed_; ///< Set to true once the first "processed" bounded buffer has been enrolled

	GThreadGroup consumerThreads_; ///< Holds threads running GConsumer objects
};

/**************************************************************************************/

} /* namespace Util */
} /* namespace Gem */

#endif /* GBROKER_HPP_ */
