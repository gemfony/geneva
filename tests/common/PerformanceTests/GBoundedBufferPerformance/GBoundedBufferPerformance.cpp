/**
 * @file GBoundedBufferPerformance.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
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
 * http://www.gemfony.eu .
 */

// Force the bounded buffer to emit data
#define GEM_COMMON_BENCHMARK_BOUNDED_BUFFER

// Standard headers go here
#include <functional>
#include <memory>

// Boost headers go here
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
GBoundedBufferT<std::shared_ptr<double>> buffer;

/**
 * A barrier on which all threads have to wait
 */
std::shared_ptr<boost::barrier> all_sync_ptr;

/**
 * A barrier on which only producers need to wait
 */
std::shared_ptr<boost::barrier> producer_sync_ptr;

/**
 * A barrier on which only consumers need to wait
 */
std::shared_ptr<boost::barrier> consumer_sync_ptr;

/**
 * A counter for dropped items inside of the producers
 */
std::vector<std::size_t> producerDroppedCounter;

/**
 * The sum of all producer submissions
 */
std::vector<double> producerSum;

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
 * A global variable that makes consumers stop
 */
bool consumerStop;

/**
 * The associated mutex (one writer, multiple readers)
 */
boost::shared_mutex consumerStopMutex;

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
	, bool startAtOnce
) {
	std::size_t id = 0;
	std::size_t nDropped = 0;
	std::vector<std::size_t> droppedIteration;
	double sum = 0, var = 0.;

	Gem::Hap::GRandomT<Gem::Hap::RANDOMPROXY> gr; // A random number proxy

	// Find out about the maximum random delay in microseconds
	long maxRandomDelayMS = boost::numeric_cast<long>(maxRandomDelay.total_microseconds());

	// Find out about the id of this producer
	{
		boost::mutex::scoped_lock lk(producerMutex);
		id = producerIdCounter++;
	}

	if(startAtOnce) all_sync_ptr->wait(); // Do not start before all threads have reached the wait()
	else producer_sync_ptr->wait(); // Just wait for all producers to reach this point

	if(maxRandomDelay.total_microseconds() == 0) {
		if(timeout.total_microseconds() == 0) {
			for(std::size_t i=0; i<nItems; i++) {
				var = gr.uniform_01<double>();
				sum += var;
				buffer.push_front(std::shared_ptr<double>(new double(var)));
			}
		} else { // We use a timeout for push_fronts
			for(std::size_t i=0; i<nItems; i++) {
				var = gr.uniform_01<double>();
				if(buffer.push_front_bool(std::shared_ptr<double>(new double(var)), timeout)) {
					sum += var;
				} else {
					nDropped++;
					droppedIteration.push_back(i);
				}
			}
		}
	} else { // We use a random delay in between submissions
		if(timeout.total_microseconds() == 0) {
			for(std::size_t i=0; i<nItems; i++) {
				var = gr.uniform_01<double>();
				sum += var;
				buffer.push_front(std::shared_ptr<double>(new double(var)));
				boost::this_thread::sleep(
					boost::posix_time::microseconds(
						boost::numeric_cast<boost::posix_time::time_duration::tick_type>(
							gr.uniform_int(long(0), maxRandomDelayMS)
						)
					)
				);
			}
		} else {
			for(std::size_t i=0; i<nItems; i++) {
				var = gr.uniform_01<double>();
				if(buffer.push_front_bool(std::shared_ptr<double>(new double(var)), timeout)) {
					sum += var;
				} else {
					nDropped++;
					droppedIteration.push_back(i);
				}
				boost::this_thread::sleep(
					boost::posix_time::microseconds(
						boost::numeric_cast<boost::posix_time::time_duration::tick_type>(
							gr.uniform_int(long(0), maxRandomDelayMS)
						)
					)
				);
			}
		}
	}

	// Update the drop counter and sums
	{ // Explicit scope
		boost::mutex::scoped_lock lk(producerMutex);
		producerDroppedCounter.at(id) = nDropped;
		producerSum.at(id) = sum;
		std::cout << "Producer " << id << " has produced a total sum of " << sum;
		if(nDropped > 0) {
			std::cout << " and has dropped " << nDropped << " of " << nItems << " items in iteration ";
			for(std::vector<std::size_t>::iterator it = droppedIteration.begin(); it!=droppedIteration.end(); ++it) {
				std::cout << *it << " ";
			}
		}
		std::cout << "." << std::endl;
	}
}

/*****************************************************************/
/**
 * This function consumes random numbers from the global buffer
 * and keeps track of failures
 *
 * @param timeout The timeout after which a new attempt for retrieval should be started
 * @param maxRandomDelay The maximum random delay in between two retrievals
 */
void consumer(
	boost::posix_time::time_duration timeout
	, boost::posix_time::time_duration maxRandomDelay
	, bool startAtOnce
) {
	std::size_t id = 0;
	std::size_t nStalled = 0;
	std::size_t iteration = 0;
	std::vector<std::size_t> droppedIteration;
	std::shared_ptr<double> item;
	double sum = 0.;
	bool stopExecution = false;
	bool maxTimeoutsReached = false;

	// Find out about the maximum random delay in microseconds
	long maxRandomDelayMS = boost::numeric_cast<long>(maxRandomDelay.total_microseconds());

	// Find out about the timeout in microseconds
	long timeoutMS = timeout.total_microseconds();
	if(timeoutMS == 0) {
		glogger
		<< "In consumer(): Error! Got timeout of 0" << std::endl
		<< GTERMINATION;
	}

	// Find out about the id of this producer
	{
		boost::mutex::scoped_lock lk(consumerMutex);
		id = consumerIdCounter++;
	}

	if(startAtOnce) all_sync_ptr->wait(); // Do not start before all threads have reached the wait()
	else consumer_sync_ptr->wait(); // Just wait for all consumers to reach this point

	if(maxRandomDelay.total_microseconds() == 0) {
		while(true) {
			if(buffer.pop_back_bool(item, timeout)) {
				sum += *item;
			} else {
				// We ran into a timeout; Check whether we've been asked to stop, otherwise count
				{
					boost::shared_lock<boost::shared_mutex> lock(consumerStopMutex);
					if(consumerStop) stopExecution = true;
				}

				if(stopExecution) break;
				else {
					droppedIteration.push_back(iteration);
					nStalled++;
				}
			}

			iteration++;
		}
	} else { // We use a random delay in-between submissions
		Gem::Hap::GRandomT<Gem::Hap::RANDOMPROXY> gr; // A random number generator

		while(true) {
			if(buffer.pop_back_bool(item, timeout)) {
				sum += *item;
			} else {
				// We ran into a timeout; Check whether we've been asked to stop, otherwise count
				{
					boost::shared_lock<boost::shared_mutex> lock(consumerStopMutex);
					if(consumerStop) stopExecution = true;
				}

				if(stopExecution) break;
				else {
					droppedIteration.push_back(iteration);
					nStalled++;
				}
			}
			boost::this_thread::sleep(
				boost::posix_time::microseconds(
					boost::numeric_cast<boost::posix_time::time_duration::tick_type>(
						gr.uniform_int(long(0), maxRandomDelayMS)
					)
				)
			);

			iteration++;
		}
	}

	// Update the counters
	{ // Explicit scope
		boost::mutex::scoped_lock lk(consumerMutex);
		consumerSum.at(id) = sum;
		consumerDroppedCounter.at(id) = nStalled;

		std::cout << "Consumer " << id << " has consumed a total sum of " << sum;
		if(nStalled > 0) {
			std::cout << " and could not retrieve items in " << nStalled << " cases in iteration(s) ";
			for(std::vector<std::size_t>::iterator it = droppedIteration.begin(); it!=droppedIteration.end(); ++it) {
				std::cout << *it << " ";
			}
			std::cout << "of " << iteration << " iterations";
		}
		std::cout << "." << std::endl;
	}
}

/*****************************************************************/
/**
 * This program tests the performance and reliability of the
 * GBoundedBufferT class by producing and consuming many double
 * values from within concurrent threads. The bounded buffer, as
 * the name suggests, acts as the buffer between producer and
 * consumer. This example both stress-tests the buffer and tests
 * its performance.
 */
int main(int argc, char**argv) {
	Gem::Common::GThreadGroup producer_gtg;
	Gem::Common::GThreadGroup consumer_gtg;

	std::string resultFile;
	std::size_t nProducers, nConsumers;
	long timeoutMS;
	long maxRandomDelayMS;
	std::size_t nItems = 0;
	long startDelayMS;
	bool startAtOnce;

	// Read the program options
	if(!parseCommandLine(
		argc, argv
		, resultFile
		, nProducers
		, nItems
		, nConsumers
		, timeoutMS
		, maxRandomDelayMS
		, startDelayMS
		, startAtOnce
	)
		)
	{ exit(1); }

	// Initialize the counters and vectors
	producerIdCounter = 0;
	consumerIdCounter = 0;
	producerDroppedCounter.resize(nProducers);
	consumerDroppedCounter.resize(nConsumers);
	producerSum.resize(nProducers);
	consumerSum.resize(nConsumers);

	// Prepare the termination criterion for consumers
	{
		boost::unique_lock<boost::shared_mutex> lock(consumerStopMutex);
		consumerStop = false;
	}

	// Initialize the producer counters with 0s
	for(std::size_t i=0; i<nProducers; i++) {
		producerDroppedCounter[i] = 0;
		producerSum[i] = 0.;
	}
	// Initialize the consumer counters with 0
	for(std::size_t i=0; i<nConsumers; i++) {
		consumerDroppedCounter[i] = 0;
		consumerSum[i] = 0.;
	}

	// Initialize the global barrier so all threads start at a predefined time
	all_sync_ptr = std::shared_ptr<boost::barrier>(new boost::barrier(boost::numeric_cast<unsigned int>(nProducers + nConsumers)));
	producer_sync_ptr = std::shared_ptr<boost::barrier>(new boost::barrier(boost::numeric_cast<unsigned int>(nProducers)));
	consumer_sync_ptr = std::shared_ptr<boost::barrier>(new boost::barrier(boost::numeric_cast<unsigned int>(nConsumers)));

	// Note the start time
	boost::posix_time::ptime startTime = boost::posix_time::microsec_clock::local_time();

	// Start the threads
	producer_gtg.create_threads(
		std::bind(
			producer
			, nItems
			, boost::posix_time::microseconds(boost::numeric_cast<boost::posix_time::time_duration::tick_type>(timeoutMS))
			, boost::posix_time::microseconds(boost::numeric_cast<boost::posix_time::time_duration::tick_type>(maxRandomDelayMS))
			, startAtOnce
		)
		, nProducers
	);

	if(!startAtOnce && startDelayMS > 0) {
		boost::this_thread::sleep(boost::posix_time::microseconds(boost::numeric_cast<boost::posix_time::time_duration::tick_type>(startDelayMS)));
	}

	consumer_gtg.create_threads(
		std::bind(
			consumer
			, boost::posix_time::microseconds(boost::numeric_cast<boost::posix_time::time_duration::tick_type>(timeoutMS))
			, boost::posix_time::microseconds(boost::numeric_cast<boost::posix_time::time_duration::tick_type>(maxRandomDelayMS))
			, startAtOnce
		)
		, nConsumers);

	// Wait for all threads to finish
	producer_gtg.join_all();

	// Tell the consumers to stop
	{
		boost::unique_lock<boost::shared_mutex> lock(consumerStopMutex);
		consumerStop = true;
	}

	consumer_gtg.join_all();

	// Note the termination time
	boost::posix_time::ptime endTime = boost::posix_time::microsec_clock::local_time();

	// Calculate the time it has taken to reach this point
	boost::posix_time::time_duration dur = endTime - startTime;

	// Find out the number of submissions per second
	double submissionsPerSecond = double(nProducers)*1000000.*double(nItems)/double(dur.total_microseconds());
	std::cout << "nItems = " << nProducers*nItems << ";" << " submissions/s = " << submissionsPerSecond << std::endl;
}
