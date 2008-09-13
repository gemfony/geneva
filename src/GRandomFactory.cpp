/**
 * @file GRandomFactory.cpp
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

#include "GRandomFactory.hpp"

namespace Gem {
namespace Util {

/****************************************************************************/
/**
 * Synchronization of access to boost::date_time functions.
 */
boost::mutex randomseed_mutex;

/*************************************************************************/
/**
 * The standard constructor, which seeds the random number generator and
 * creates a predefined number of threads.
 */
GRandomFactory::GRandomFactory() throw() :
	g01_(DEFAULTFACTORYBUFFERSIZE),
	seed_(GRandomFactory::GSeed()),
	n01Threads_(DEFAULT01PRODUCERTHREADS)
{
	startProducerThreads();
}

/*************************************************************************/
/**
 * A constructor that creates a user-specified number of [0,1[ threads and
 * gauss threads. It seeds the random number generator and starts the
 * producer01 thread. Note that we enforce a minimum number of threads.
 */
GRandomFactory::GRandomFactory(const boost::uint16_t& n01Threads) throw() :
	g01_(DEFAULTFACTORYBUFFERSIZE),
	seed_(GRandomFactory::GSeed()),
	n01Threads_(n01Threads ? n01Threads : 1)
{
	startProducerThreads();
}

/*************************************************************************/
/**
 * The destructor. All threads are given the interrupt signal. Then
 * we wait for them to join us.
 */
GRandomFactory::~GRandomFactory() throw() {
	std::cout << "Terminating threads ..." << std::endl;

	producer_threads_01_.interrupt_all(); // doesn't throw
	producer_threads_01_.join_all();

	std::cout << "GrandomFactory has terminated" << std::endl;
}

/*************************************************************************/
/**
 * Sets the number of producer threads for this factory.
 *
 * @param n01Threads
 */
void GRandomFactory::setNProducerThreads(const boost::uint16_t& n01Threads)
{
	if (n01Threads > n01Threads_) { // start new 01 threads
		for (boost::uint16_t i = n01Threads_; i < n01Threads; i++) {
			producer_threads_01_.create_thread(boost::bind(&GRandomFactory::producer01, this, seed_++));
		}
	} else if (n01Threads < n01Threads_) { // We need to remove threads
		if (n01Threads == 0)
			return; // We need at least 1 thread

		producer_threads_01_.remove_last(n01Threads_ - n01Threads);
	}

	n01Threads_ = n01Threads;
}

/*************************************************************************/
/**
 * When objects need new [0,1[ random numbers, they call this function. Note
 * that calling threads are responsible for catching the boost::thread_interrupted
 * exception.
 *
 * @return A packet of new [0,1[ random numbers
 */
boost::shared_array<double> GRandomFactory::new01Container() {
	boost::shared_array<double> result; // empty

	try {
		g01_.pop_back(&result, boost::posix_time::milliseconds(
				DEFAULTFACTORYPUTWAIT));
	} catch (Gem::Util::gem_util_condition_time_out&) {
		// nothing - our way of signaling a time out
		// is to return an empty boost::shared_ptr
	}

	return result;
}

/*************************************************************************/
/**
 * This static function returns a seed based on the current time.
 *
 * Comments on some boost mailing lists (late 2005) seem to indicate that
 * the date_time library's functions are not re-entrant when using gcc and
 * its libraries. It was not possible to determine whether this is still
 * the case, hence we protect calls to date_time with a mutex.
 *
 * @return A seed based on the current time
 */
boost::uint32_t GRandomFactory::GSeed(){
	boost::mutex::scoped_lock lk(randomseed_mutex);

	std::cout << "Called GSeed" << std::endl;

	boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::local_time();
    return (uint32_t)t1.time_of_day().total_milliseconds();
}

/*************************************************************************/
/**
 * This function starts the threads needed for the production of random numbers
 */
void GRandomFactory::startProducerThreads() throw() {
	for (boost::uint16_t i = 0; i < n01Threads_; i++) {
		// thread() doesn't throw, and no exceptions are listed in the documentation
		// for the create_thread() function, so we assume it doesn't throw.
		producer_threads_01_.create_thread(boost::bind(&GRandomFactory::producer01, this, seed_++));
	}
}

/*************************************************************************/
/**
 * The production of [0,1[ random numbers takes place here. As this function
 * is called in a thread, it may not throw under any circumstance. Exceptions
 * could otherwise go unnoticed. Hence this function has a possibly confusing
 * setup.
 *
 * @param seed The seed for our local random number generator
 */
void GRandomFactory::producer01(const boost::uint32_t& seed) throw() {
	try {
		boost::lagged_fibonacci607 lf(seed);

		while (true) {
			double *p_raw = new double[DEFAULTARRAYSIZE];

			for (std::size_t i = 0; i < DEFAULTARRAYSIZE; i++) {
#ifdef DEBUG
				double value = lf();
				assert(value>=0. && value<1.);
				p_raw[i]=value;
#else
				p_raw[i] = lf();
#endif /* DEBUG */
			}

			boost::shared_array<double> p(p_raw);
			g01_.push_front(p);
		}
	} catch (boost::thread_interrupted&) { // Not an error
		return; // We're done
	} catch (std::bad_alloc& e) {
		std::ostringstream error;
		error << "In GRandomFactory::producer01(): Error!" << std::endl
				<< "Caught std::bad_alloc exception with message"
				<< std::endl << e.what() << std::endl;

		LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

		std::terminate();
	} catch (std::invalid_argument& e) {
		std::ostringstream error;
		error << "In GRandomFactory::producer01(): Error!" << std::endl
				<< "Caught std::invalid_argument exception with message"
				<< std::endl << e.what() << std::endl;

		LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

		std::terminate();
	} catch (boost::thread_resource_error&) {
		std::ostringstream error;
		error << "In GRandomFactory::producer01(): Error!" << std::endl
				<< "Caught boost::thread_resource_error exception which"
				<< std::endl
				<< "likely indicates that a mutex could not be locked."
				<< std::endl;

		LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

		// Terminate the process
		std::terminate();
	} catch (...) {
		std::ostringstream error;
		error << "In GRandomFactory::producer01(): Error!" << std::endl
				<< "Caught unkown exception." << std::endl;

		LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

		// Terminate the process
		std::terminate();
	}

	std::cout << "producer thread has ended ..." << std::endl;
}

/*************************************************************************/

} /* namespace Util */
} /* namespace Gem */
