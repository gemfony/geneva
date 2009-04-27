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

/*************************************************************************/
/**
 * Synchronization of access to boost::date_time functions.
 */
boost::mutex randomseed_mutex;

/*************************************************************************/
/**
 * Initialize of static data members
 */
boost::uint16_t Gem::Util::GRandomFactory::multiple_call_trap_ = 0;
boost::mutex Gem::Util::GRandomFactory::thread_creation_mutex_;

/*************************************************************************/
/**
 * The standard constructor, which seeds the random number generator and
 * creates a predefined number of threads.
 */
GRandomFactory::GRandomFactory()  :
	seed_(GRandomFactory::GSeed()),
	n01Threads_(DEFAULT01PRODUCERTHREADS),
	g01_ (boost::shared_ptr<Gem::Util::GBoundedBufferT<boost::shared_array<double> > >(new Gem::Util::GBoundedBufferT<boost::shared_array<double> > (DEFAULTFACTORYBUFFERSIZE)))
{
	{ // explicit scope ensures proper destruction of mutex
		boost::mutex::scoped_lock lk(thread_creation_mutex_);
		if(multiple_call_trap_ > 0) {
			std::ostringstream error;
			error << "Error in GRandomFactory::GRandomFactory(): class has been instantiated before." << std::endl;
			throw(Gem::GenEvA::geneva_error_condition(error.str()));
		}
		else {
			startProducerThreads();
			multiple_call_trap_++;
		}
	}
}

/*************************************************************************/
/**
 * The destructor. All threads are given the interrupt signal. Then
 * we wait for them to join us.
 */
GRandomFactory::~GRandomFactory() {
	producer_threads_01_.interrupt_all(); // doesn't throw
	producer_threads_01_.join_all();
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
		g01_->pop_back(&result, boost::posix_time::milliseconds(
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
	boost::posix_time::ptime t1;
    return (uint32_t)t1.time_of_day().total_milliseconds();
}

/*************************************************************************/
/**
 * This function starts the threads needed for the production of random numbers
 */
void GRandomFactory::startProducerThreads()  {
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
 * setup. Note that we do not use the global logger, as we do not want to risk
 * accessing a singleton past its possible destruction.
 *
 * @param seed The seed for our local random number generator
 */
void GRandomFactory::producer01(const boost::uint32_t& seed)  {
	try {
		boost::lagged_fibonacci607 lf(seed);

		while (true) {
			if(boost::this_thread::interruption_requested()) break;

			boost::shared_array<double> p(new double[DEFAULTARRAYSIZE]);

			 // Faster access during the fill procedure
			// Note that we own the only instance of this pointer at this point
			double *p_raw = p.get();

			for (std::size_t i = 0; i < DEFAULTARRAYSIZE; i++) {
#ifdef DEBUG
				double value = lf();
				assert(value>=0. && value<1.);
				p_raw[i]=value;
#else
				p_raw[i] = lf();
#endif /* DEBUG */
			}

			// Get a local copy of g01_, so its object does not get deleted involuntarily
			// when the singleton exits.
			boost::shared_ptr<Gem::Util::GBoundedBufferT<boost::shared_array<double> > > g01_cp = g01_;
			if(g01_cp) g01_cp->push_front(p);
		}
	} catch (boost::thread_interrupted&) { // Not an error
		return; // We're done
	} catch (std::bad_alloc& e) {
		std::cerr << "In GRandomFactory::producer01(): Error!" << std::endl
				  << "Caught std::bad_alloc exception with message"
				  << std::endl << e.what() << std::endl;

		std::terminate();
	} catch (std::invalid_argument& e) {
		std::cerr << "In GRandomFactory::producer01(): Error!" << std::endl
				  << "Caught std::invalid_argument exception with message"
				  << std::endl << e.what() << std::endl;

		std::terminate();
	} catch (boost::thread_resource_error&) {
		std::cerr << "In GRandomFactory::producer01(): Error!" << std::endl
				  << "Caught boost::thread_resource_error exception which"
				  << std::endl
				  << "likely indicates that a mutex could not be locked."
				  << std::endl;

		// Terminate the process
		std::terminate();
	} catch (...) {
		std::cerr << "In GRandomFactory::producer01(): Error!" << std::endl
				  << "Caught unkown exception." << std::endl;

		// Terminate the process
		std::terminate();
	}
}

/*************************************************************************/

} /* namespace Util */
} /* namespace Gem */
