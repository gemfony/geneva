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
#include <boost/bind.hpp>
#include <boost/date_time.hpp>

// Geneva headers go here
#include "common/GThreadGroup.hpp"
#include "common/GBoundedBufferT.hpp"
#include "hap/GRandomT.hpp"

// Local headers
#include "GArgumentParser.hpp"

using namespace Gem::Common;
using namespace Gem::Common::Tests;

/*****************************************************************/
/**
 * The global thread-safe queue
 */
GBoundedBufferT<double> buffer;

/**
 * A barrier on which all threads have to wait
 */
boost::shared_ptr<boost::barrier> sync_ptr;

/**
 * A counter for dropped items inside of the producers
 */
std::vector<std::size_t> producerDroppedCounter;

/**
 * The sum of all producer submissions
 */
std::vector<double> producerSum;

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
		std::size_t nItems
		, boost::posix_time::time_duration timeout
		, boost::posix_time::time_duration maxRandomDelay
) {
	std::size_t id = 0;
	std::size_t nDropped = 0;

	Gem::Hap::GRandomT<Gem::Hap::RANDOMPROXY> gr; // A random number proxy

	// Find out about the maximum random delay in microseconds
	long maxRandomDelayMS = maxRandomDelay.total_microseconds();

	// Find out about the id of this producer
	{
		boost::mutex::scoped_lock lk(producerMutex);
		id = producerIdCounter++;
	}

	sync_ptr->wait(); // Do not start before all threads have reached the wait()

	if(maxRandomDelay.total_microseconds() == 0) {
		if(timeout.total_microseconds() == 0) {
			for(std::size_t i=0; i<nItems; i++) {
				buffer.push_front(gr.uniform_01<double>());
			}
		} else { // We use a timeout for push_fronts
			for(std::size_t i=0; i<nItems; i++) {
				if(!buffer.push_front_bool(gr.uniform_01<double>(), timeout)) {
					nDropped++;
				}
			}
		}
	} else { // We use a random delay in between submissions
		if(timeout.total_microseconds() == 0) {
			for(std::size_t i=0; i<nItems; i++) {
				buffer.push_front(gr.uniform_01<double>());
				boost::this_thread::sleep(
					boost::posix_time::microseconds(
						gr.uniform_int(long(0), maxRandomDelayMS)
					)
				);
			}
		} else {
			for(std::size_t i=0; i<nItems; i++) {
				if(!buffer.push_front_bool(gr.uniform_01<double>(), timeout)) {
					nDropped++;
				}
				boost::this_thread::sleep(
					boost::posix_time::microseconds(
						gr.uniform_int(long(0), maxRandomDelayMS)
					)
				);
			}
		}
	}

	// Update the drop counter and sums
	{ // Explicit scope
		boost::mutex::scoped_lock lk(producerMutex);
		producerDroppedCounter.at(id) = nDropped;

		std::cout << "Producer " << id << " dropped " << nDropped << " of " << nItems << " items" << std::endl;
	}
}

/*****************************************************************/
/**
 * This function consumes random numbers from the global buffer
 * and keeps track of failures
 *
 * @param timeout The timeout after which a new attempt for retrieval should be started
 * @param maxNumberOfTimeouts The maximum number of timeouts after which consummation should be stopped
 * @param maxRandomDelay The maximum random delay in between two retrievals
 */
void consumer(
		boost::posix_time::time_duration timeout
		, std::size_t maxNumberOfTimeouts
		, boost::posix_time::time_duration maxRandomDelay
) {
	std::size_t id = 0;
	std::size_t nDropped = 0;
	double item;
	double sum = 0.;
	bool maxTimeoutsReached = false;

	// Find out about the maximum random delay in microseconds
	long maxRandomDelayMS = maxRandomDelay.total_microseconds();

	// Find out about the timeout in microseconds
	long timeoutMS = timeout.total_microseconds();
	if(timeoutMS == 0) {
		std::cerr << "In consumer(): Error! Got timeout of 0" << std::endl;
		std::terminate();
	}

	// Find out about the id of this producer
	{
		boost::mutex::scoped_lock lk(consumerMutex);
		id = consumerIdCounter++;
	}

	sync_ptr->wait(); // Do not start before all threads have reached the wait()

	if(maxRandomDelay.total_microseconds() == 0) {
		while(true) {
			if(buffer.pop_back_bool(item, timeout)) {
				sum += item;
			} else {
				if(nDropped++ > maxNumberOfTimeouts) break;
			}
		}
	} else { // We use a random delay in between submissions
		Gem::Hap::GRandomT<Gem::Hap::RANDOMPROXY> gr; // A random number generator

		while(true) {
			if(buffer.pop_back_bool(item, timeout)) {
				sum += item;
			} else {
				if(nDropped++ > maxNumberOfTimeouts) {
					maxTimeoutsReached = true;
					break;
				}
			}
			boost::this_thread::sleep(
				boost::posix_time::microseconds(
					gr.uniform_int(long(0), maxRandomDelayMS)
				)
			);
		}
	}

	// Update the counters
	{ // Explicit scope
		boost::mutex::scoped_lock lk(consumerMutex);
		consumerSum.at(id) = sum;
		if(maxTimeoutsReached) {
			std::cout << "Consumer " << id << " has reached " << maxNumberOfTimeouts << " timeouts" << std::endl;
		}
	}
}

/*****************************************************************/
/**
 * This program tests the performance and reliability of the
 * GBoundedBufferT class by producing and consuming many double
 * values from within concurrent threads.
 */
int main(int argc, char**argv) {
	Gem::Common::GThreadGroup producer_gtg;
	Gem::Common::GThreadGroup consumer_gtg;

	std::string resultFile;
	std::size_t nProducers, nConsumers;
	std::size_t nItems;
	long timeoutMS;
	long maxRandomDelayMS;
	std::size_t maxNTimeouts;

	// Read the program options
	if(!parseCommandLine(
			argc, argv
			, resultFile
			, nProducers
			, nConsumers
			, nItems
			, timeoutMS
			, maxRandomDelayMS
			, maxNTimeouts
		)
	)
	{ exit(1); }

	// Initialize the counters and vectors
	producerIdCounter = 0;
	consumerIdCounter = 0;
	producerDroppedCounter.resize(nProducers);
	producerSum.resize(nProducers);
	consumerSum.resize(nConsumers);

	// Initialize the producer counters with 0s
	for(std::size_t i=0; i<nProducers; i++) {
		producerDroppedCounter[i] = 0;
		producerSum[i] = 0.;
	}
	// Initialize the consumer counters with 0
	for(std::size_t i=0; i<nConsumers; i++) {
		consumerSum[i] = 0.;
	}

	// Initialize the global barrier so all threads start at a predefined time
	sync_ptr = boost::shared_ptr<boost::barrier>(new boost::barrier(nProducers + nConsumers));

	// Start the threads
	producer_gtg.create_threads(
		boost::bind(
			producer
			, nItems
			, boost::posix_time::microseconds(timeoutMS)
			, boost::posix_time::microseconds(maxRandomDelayMS)
		)
		, nProducers
	);

	consumer_gtg.create_threads(
		boost::bind(
			consumer
			, boost::posix_time::microseconds(timeoutMS)
			, maxNTimeouts
			, boost::posix_time::microseconds(maxRandomDelayMS)
		)
		, nConsumers);

	// Wait for all threads to finish
	producer_gtg.join_all();
	consumer_gtg.join_all();
}
