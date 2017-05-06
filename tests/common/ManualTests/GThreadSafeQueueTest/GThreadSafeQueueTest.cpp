/**
 * @file GThreadSafeQueueTest.cpp
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

// Standard headers go here
#include <cmath>
#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include <atomic>

// Boost headers go here

// Geneva headers go here
#include "common/GLogger.hpp"
#include "common/GThreadPool.hpp"
#include "common/GParserBuilder.hpp"
#include "common/GThreadSafeQueueT.hpp"
#include "hap/GRandomT.hpp"
#include "hap/GRandomDistributionsT.hpp"

using namespace Gem::Common;

/************************************************************************/
/**
 * A simple test task that sets a flag when processing has happened
 */
class testTask {
public:
	 /********************************************************************/
	 /**
	  * The default constructor
	  */
	 testTask()
		 : m_counterValue(0)
		 , m_operatorCalled(0)
	 { /* nothing */ }

	 /********************************************************************/
	 /**
	  * Checks whether processing has indeed taken place
	  */
	 bool processingOK() {
		 return 0 == m_counterValue && 2 == m_operatorCalled;
	 }

	 /********************************************************************/
	 /**
	  * Allows to check how often increment() has been called
	  */
	 std::int32_t getCounterValue() const {
		 return m_counterValue;
	 }

	 /********************************************************************/
	 /**
	  * Retrieves the number of operator calls
	  */
	 std::uint32_t getOperatorCalledValue() const {
		 return m_operatorCalled;
	 }

	 /********************************************************************/
	 /**
	  * Performs work on this object. This is the function to be executed
	  * inside of the threads
	  */
	 void process() {
		 this->increment();
		 this->decrement();
	 }

private:
	 /********************************************************************/
	 /**
	  * Increments the local counter
	  */
	 void increment() {
		 m_counterValue++;
		 m_operatorCalled++;
	 }

	 /********************************************************************/
	 /**
	  * Decrements the local counter
	  */
	 void decrement() {
		 m_counterValue--;
		 m_operatorCalled++;
	 }

	 /********************************************************************/
	 std::int32_t m_counterValue; ///< The internal value to be decremented or incremented
	 std::uint32_t m_operatorCalled; ///< This counter will be incremented whenever process() is called
};

/************************************************************************/
// Global objects

const std::size_t nProducerThreads = 4;
const std::size_t nWorkerThreads = 4;

const std::size_t n_work_items = 500;
const std::chrono::milliseconds timeout(500);

GThreadPool gtp_producers(nProducerThreads); ///< The global threadpool for producers
GThreadPool gtp_workers(nWorkerThreads); ///< The global threadpool for workers
GThreadSafeQueueT<testTask,10*n_work_items> g_tasks_raw; ///< The global threadpool for raw tasks

std::atomic<std::size_t> nProduced_raw{0};
const std::size_t produced_raw_max = n_work_items * nProducerThreads;
std::atomic<std::size_t> nProduced_up{0};
const std::size_t produced_up_max = n_work_items * nProducerThreads;
std::atomic<std::size_t> nProduced_sp{0};
const std::size_t produced_sp_max = n_work_items * nProducerThreads;

std::atomic<std::size_t> nProcessed{0};
const std::size_t processed_max = produced_raw_max + produced_up_max + produced_sp_max;

std::mutex cout_mutex;

/************************************************************************/
/**
 * Producer function -- submitted work items are "raw"
 */
void submit_raw_work_items() {
	std::size_t nProducedLocal = 0;
	do {
		if(g_tasks_raw.push_and_wait(testTask(), timeout)) {
			nProducedLocal++;
			nProduced_raw++;
		}
	} while(nProduced_raw <= produced_raw_max);

	std::unique_lock<std::mutex> cout_lock(cout_mutex);
	std::cout << "Thread " << std::this_thread::get_id() << " produced " << nProducedLocal << " items" << std::endl;
};

/************************************************************************/
/**
 * Producer function -- submitted work items are wrapped in a std::unique_ptr
 */
void submit_up_work_items() {
	std::size_t nProducedLocal = 0;
	do {
		std::unique_ptr<testTask> task = std::make_unique<testTask>();
		if(g_tasks_raw.push_and_wait(task, timeout)) {
			nProducedLocal++;
			nProduced_up++;
		}
	} while(nProduced_up <= produced_up_max);

	std::unique_lock<std::mutex> cout_lock(cout_mutex);
	std::cout << "Thread " << std::this_thread::get_id() << " produced " << nProducedLocal << " items" << std::endl;
};

/************************************************************************/
/**
 * Producer function -- submitted work items are wrapped in a std::shared_ptr
 */
void submit_sp_work_items() {
	std::size_t nProducedLocal = 0;
	do {
		std::shared_ptr<testTask> task = std::make_shared<testTask>();
		if(g_tasks_raw.push_and_wait(task, timeout)) {
			nProducedLocal++;
			nProduced_sp++;
		}
	} while(nProduced_sp <= produced_sp_max);

	std::unique_lock<std::mutex> cout_lock(cout_mutex);
	std::cout << "Thread " << std::this_thread::get_id() << " produced " << nProducedLocal << " items" << std::endl;
};

/************************************************************************/
/**
 * Consumer / worker
 */
void consume() {
	std::size_t nProcessedLocal = 0;
	do {
		bool item_is_available = false;
		std::shared_ptr<testTask> task_ptr = g_tasks_raw.pop_and_wait(item_is_available, timeout);
		if(task_ptr && item_is_available) {
			task_ptr->process();
			nProcessed++;
			nProcessedLocal++;
		}
	} while(nProcessed <= processed_max);

	std::unique_lock<std::mutex> cout_lock(cout_mutex);
	std::cout << "Thread " << std::this_thread::get_id() << " processed " << nProcessedLocal << " items" << std::endl;
}

/************************************************************************/

int main(int argc, char** argv) {
	// Submit worker
	for(std::size_t worker=0; worker<nWorkerThreads; worker++) {
		gtp_workers.async_schedule(consume);
	}

	// Submit "raw" producers
	for(std::size_t producer=0; producer<nProducerThreads; producer++) {
		gtp_producers.async_schedule(submit_raw_work_items);
	}
	// Wait for raw producers to finish
	{
		std::unique_lock<std::mutex> cout_lock(cout_mutex);
		std::cout << "Waiting for raw producers to finish" << std::endl;
	}
	gtp_producers.wait();

	// Submit "unique_ptr" producers
	for(std::size_t producer=0; producer<nProducerThreads; producer++) {
		gtp_producers.async_schedule(submit_up_work_items);
	}
	// Wait for unique_ptr producers to finish
	{
		std::unique_lock<std::mutex> cout_lock(cout_mutex);
		std::cout << "Waiting for unique_ptr to finish" << std::endl;
	}
	gtp_producers.wait();


	// Submit "shared_ptr" producers
	for(std::size_t producer=0; producer<nProducerThreads; producer++) {
		gtp_producers.async_schedule(submit_sp_work_items);
	}
	// Wait for shared_ptr producers to finish
	{
		std::unique_lock<std::mutex> cout_lock(cout_mutex);
		std::cout << "Waiting for shared_ptr to finish" << std::endl;
	}
	gtp_producers.wait();

	// Wait for worker threads to finish
	{
		std::unique_lock<std::mutex> cout_lock(cout_mutex);
		std::cout << "Waiting for workers to finish" << std::endl;
	}
	gtp_workers.wait();
}

/************************************************************************/
