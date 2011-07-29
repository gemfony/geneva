/**
 * @file GBoundedBufferPerformance.hpp
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

// Boost headers go here
#include <boost/cstdint.hpp>
#include <boost/date_time.hpp>

// Geneva headers go here
#include "common/GBoundedBufferT.hpp"
#include "hap/GRandomT.hpp"

// Local headers
#include "GArgumentParser.hpp"

using namespace Gem::Common;
using namespace Gem::Common::Testing;

/*****************************************************************/
/**
 * The global thread-safe queue
 */
GBoundedBufferT<double> buffer;

/**
 * A counter for dropped items inside of the producers
 */
std::vector<std::size_t> producerDroppedCounter;

/**
 * A counter for dropped items inside of the consumers
 */
std::vector<std::size_t> consumerDroppedCounter;

/**
 * The sum of each comsumer reception
 */
std::vector<double> consumerSum;

/**
 * An id to be assigned to each producer
 */
std::size_t producerIdCounter;

/**
 * An id to be assigned to each consumer
 */
std::size_t consumerIdCounter;

/**
 * A mutex that protects "everything producer"
 */
boost::mutex producerMutex;

/**
 * A mutex that protects "everything consumer"
 */
boost::mutex consumerMutex;

/*****************************************************************/
/**
 * This function produces double random numbers and adds them to
 * a global queue
 *
 * @param nItems The amount of random numbers to be produced
 * @param timeout The timeout after which an item should be dropped
 * @param maxRandomDelay The maximum random delay in between two submissions
 */
void producer(
		const std::size_t& nItems
		, const boost::date_time& timeout
		, const boost::date_time& maxRandomDelay
) {
	std::size_t id = 0;
	std::size_t nDropped = 0;

	// A random number generator
	Gem::Hap::GRandomT<Gem::Hap::RANDOMPROXY> gr;

	// Find out about the maximum random delay in microseconds
	long maxRandomDelayMS = maxRandomDelay.total_microseconds();

	// Find out about the id of this producer
	{
		boost::mutex::scoped_lock lk(producerMutex);
		id = producerIdCounter++;
	}

	if(maxRandomDelay.total_microseconds() == 0) {
		if(timeout.total_microseconds() == 0) {
			for(std::size_t i=0; i<nItems; i++) {
				buffer.push_front(gr.uniform_01<double>());
			}
		} else { // We use a timeout for push_fronts
			for(std::size_t i=0; i<nItems; i++) {
				if(!push_front_bool(gr.uniform_01<double>(), timeout)) {
					nDropped++;
				}
			}
		}
	} else { // We use a random delay in between submissions
		if(timeout.total_microseconds() == 0) {
			for(std::size_t i=0; i<nItems; i++) {
				boost::this_thread::sleep(gr.uniform_int(long(0),maxRandomDelayMS));
				buffer.push_front(gr.uniform_01<double>());
			}
		} else {
			for(std::size_t i=0; i<nItems; i++) {
				boost::this_thread::sleep(gr.uniform_int(long(0),maxRandomDelayMS));
				if(!buffer.push_front_bool(gr.uniform_01<double>(), timeout)) {
					nDropped++;
				}
			}
		}
	}

	// Update the drop counter and sums
	{ // Explicit scope
		boost::mutex::scoped_lock lk(producerMutex);
		producerDroppedCounter.at(id) = nDropped;
	}
}

/*****************************************************************/
/**
 * This function consumes random numbers from the global buffer
 * and keeps track of failures
 *
 * @param nItems The maximum number of items after which the consumer should stop
 * @param timeout The timeout after which a new attempt for retrieval should be started
 * @param maxNumberOfTimeouts The maximum number of timeouts after which consummation should be stopped
 * @param maxRandomDelay The maximum random delay in between two retrievals
 */
void consumer(
		const std::size_t& nItems
		, const boost::date_time& timeout
		, const std::size_t& maxNumberOfTimeouts
		, const boost::date_time& maxRandomDelay
) {
	s td::size_t id = 0;
	std::size_t nDropped = 0;
	double *item;
	double sum = 0.;

	// A random number generator
	Gem::Hap::GRandomT<Gem::Hap::RANDOMPROXY> gr;

	// Find out about the maximum random delay in microseconds
	long maxRandomDelayMS = maxRandomDelay.total_microseconds();

	// Find out about the id of this producer
	{
		boost::mutex::scoped_lock lk(consumerMutex);
		id = consumerIdCounter++;
	}

	if(maxRandomDelay.total_microseconds() == 0) {
		if(timeout.total_microseconds() == 0) {
			for(std::size_t i=0; i<nItems; i++) {
				buffer.pop_back(item);
				sum += *item;
			}
		} else { // We use a timeout for push_fronts
			for(std::size_t i=0; i<nItems; i++) {
				if(!pop_back_bool(item, timeout)) {
					if(nDropped++ > maxNumberOfTimeouts) break;
				} else {
					sum += *item;
				}
			}
		}
	} else { // We use a random delay in between submissions
		if(timeout.total_microseconds() == 0) {
			for(std::size_t i=0; i<nItems; i++) {
				boost::this_thread::sleep(gr.uniform_int(long(0),maxRandomDelayMS));
				buffer.pop_back(item);
				sum += *item;
			}
		} else {
			for(std::size_t i=0; i<nItems; i++) {
				boost::this_thread::sleep(gr.uniform_int(long(0),maxRandomDelayMS));
				if(!pop_back_bool(item, timeout)) {
					if(nDropped++ > maxNumberOfTimeouts) break;
				} else {
					sum += *item;
				}
			}
		}
	}

	// Update the drop counter
	{ // Explicit scope
		boost::mutex::scoped_lock lk(consumerMutex);
		consumerDroppedCounter.at(id) = nDropped;
		consumerSum.at(id) = sum;
	}
}

/*****************************************************************/
/**
 * This program tests the performance and reliability of the
 * GBoundedBufferT class by producing and consuming many double
 * values from within concurrent threads.
 */
int main(int argc, char**argv) {
	// Read the program options

	// Initialize the counters
	producerIdCounter = 0;
	consumerIdCounter = 0;
	producerDroppedCounter.resize(nProducers);
	consumerDroppedCounter.resize(nConsumers);
	consumerSum.resize(nConsumers);

	// Initialize the counter with 0s
	for(std::size_t i=0; i<nProducers; i++) {
		producerDroppedCounter[i] = 0;
	}
}
