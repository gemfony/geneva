/**
 * @file GThreadSafeQueueTest.cpp
 */

/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

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
		std::unique_ptr<testTask> task = Gem::Common::g_make_unique<testTask>();
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
