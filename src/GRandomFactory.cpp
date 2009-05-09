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
 * Declaration (and initialization) of some global mutexes and data items
 */
boost::mutex randomseed_mutex;
boost::mutex global_seed_mutex_;
boost::uint32_t globalSeed = 0;  ///< A global seed that is incremented for each new invocation of GSeed()
bool seedInitialized = false;  ///< Specifies whether an initial seed has already been set

/*************************************************************************/
/**
 * Initialize of static data members
 */
boost::uint16_t Gem::Util::GRandomFactory::multiple_call_trap_ = 0;
boost::mutex Gem::Util::GRandomFactory::factory_creation_mutex_;

/*************************************************************************/
/**
 * The standard constructor, which seeds the random number generator and
 * creates a predefined number of threads.
 */
GRandomFactory::GRandomFactory()  :
	arraySize_(DEFAULTARRAYSIZE),
	threadsHaveBeenStarted_(false),
	n01Threads_(DEFAULT01PRODUCERTHREADS),
	g01_ (boost::shared_ptr<Gem::Util::GBoundedBufferT<boost::shared_array<double> > >(new Gem::Util::GBoundedBufferT<boost::shared_array<double> > (DEFAULTFACTORYBUFFERSIZE)))
{
	boost::mutex::scoped_lock lk(factory_creation_mutex_);
	if(multiple_call_trap_ > 0) {
		std::ostringstream error;
		error << "Error in GRandomFactory::GRandomFactory():" << std::endl
		         << "Class has been instantiated before." << std::endl
		         << "and may be instantiated only once" << std::endl;
		throw(Gem::GenEvA::geneva_error_condition(error.str()));
	}
	else {
		multiple_call_trap_++;
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

#ifdef DEBUG
	std::cout << "GRandomFactory has terminated" << std::endl;
#endif /* DEBUG */
}

/*************************************************************************/
/**
 * Allows to globally set the size of random number arrays. Note that this function
 * will throw if  an array size of 0 was requested. Calling this function from
 * multipe threads is not recommended, as the new array size will have
 * no effects on the existing items in the buffer, and so changing the array
 * size from different locations will lead to confusion.
 *
 * @param arraySize The desired size of the array
 */
void GRandomFactory::setArraySize(const std::size_t& arraySize) {
	if(arraySize == 0) {
		std::ostringstream error;
		error << "In GRandomFactory::setArraySize(): Error" << std::endl
		          << "Requested array size is 0: " << arraySize << std::endl;
		throw(Gem::GenEvA::geneva_error_condition(error.str()));
	}

	{
		boost::mutex::scoped_lock lk(arraySizeMutex_);
		arraySize_ = arraySize;
	}
}

/*************************************************************************/
/**
 * Allows to retrieve the current value of the arraySize_ variable. Note that
 * its value may change after retrieval.
 *
 * @return The current value of the arraySize_ variable
 */
std::size_t GRandomFactory::getCurrentArraySize() const {
	std::size_t result;

	{
		boost::mutex::scoped_lock lk(arraySizeMutex_);
		result = arraySize_;
	}

	return result;
}

/*************************************************************************/
/**
 * Retrieves the size of the random buffer
 *
 * @return The size of the random buffer
 */
std::size_t GRandomFactory::getBufferSize() const {
	return DEFAULTFACTORYBUFFERSIZE;
}

/*************************************************************************/
/**
 * Provides users with an interface to setting the global seed variable.
 * Note that this function will have no effect once a seed has been set.
 * If not set by the user, GSeed will set the value upon first invocation.
 *
 * @param seed The desired initial value of the global seed
 */
void GRandomFactory::setSeed(const boost::uint32_t& seed) const {
	GRandomFactory::setSeed_(seed); // Static, private member function
}

/*************************************************************************/
/**
 * Retrieval of the current value of the globalSeed variable
 * @return The currentl value of the global seed
 */
boost::uint32_t GRandomFactory::getSeed() const {
	return GRandomFactory::getSeed_(); // Static, private member function
}

/*************************************************************************/
/**
 * Checks whether the global seed has already been initialized
 *
 * @return A boolean indicating whether the seed has already been initialized
 */
bool GRandomFactory::checkSeedIsInitialized() const {
	// Make sure we have sole access to global variables
	boost::mutex::scoped_lock lk(global_seed_mutex_);
	return seedInitialized;
}

/*************************************************************************/
/**
 * A static, private member function that allows to set the global
 * seed once. After the first invocation there will be no further effect.
 *
 * @param seed The initial value of the global seed
 */
void GRandomFactory::setSeed_(const boost::uint32_t& seed) {
	// Make sure we have sole access to global variables
	boost::mutex::scoped_lock lk(global_seed_mutex_);
	if(seedInitialized) return; // Do nothing after the first invocation
	globalSeed = seed; // Set the seed as requested;
	seedInitialized=true; // Let the audience know
}

/*************************************************************************/
/**
 * Retrieval of the _current_ value of the global seed
 */
boost::uint32_t GRandomFactory::getSeed_() {
	// Make sure we have sole access to global variables
	boost::mutex::scoped_lock lk(global_seed_mutex_);
	return globalSeed;
}

/*************************************************************************/
/**
 * Sets the number of producer threads for this factory.
 *
 * @param n01Threads
 */
void GRandomFactory::setNProducerThreads(const boost::uint16_t& n01Threads)
{
	// Do some error checking
	if (n01Threads == 0)	{  // We need at least 1 thread
		std::ostringstream error;
		error << "In GRandomFactory::setNProducerThreads(): Error" << std::endl
		          << "Requested 0 threads: " << n01Threads << std::endl;
		throw(Gem::GenEvA::geneva_error_condition(error.str()));
	}

	// Threads might already be running, so we need to restrict access
	{
		boost::mutex::scoped_lock lk(thread_creation_mutex_);
		if(threadsHaveBeenStarted_) {
			if (n01Threads > n01Threads_) { // start new 01 threads
				for (boost::uint16_t i = n01Threads_; i < n01Threads; i++) {
					boost::uint32_t seed_ =  GRandomFactory::GSeed();
					producer_threads_01_.create_thread(boost::bind(&GRandomFactory::producer01, this, seed_));
				}
			} else if (n01Threads < n01Threads_) { // We need to remove threads
				producer_threads_01_.remove_last(n01Threads_ - n01Threads);
			}
		}

		n01Threads_ = n01Threads;
	}
}

/*************************************************************************/
/**
 * When objects need a new container [0,1[ of random numbers with the current
 * default size, they call this function. Note that calling threads are responsible
 * for catching the boost::thread_interrupted exception.
 *
 * @return A packet of new [0,1[ random numbers
 */
boost::shared_array<double> GRandomFactory::new01Container() {
	// Start the producer threads upon first access to this function
	if(!threadsHaveBeenStarted_) {
		boost::mutex::scoped_lock lk(thread_creation_mutex_);
		if(!threadsHaveBeenStarted_) {
			startProducerThreads();
			threadsHaveBeenStarted_=true;
		}
	}

	boost::shared_array<double> result; // empty

	try {
		g01_->pop_back(&result, boost::posix_time::milliseconds(DEFAULTFACTORYGETWAIT));
	} catch (Gem::Util::gem_util_condition_time_out&) {
		// nothing - our way of signaling a time out
		// is to return an empty boost::shared_ptr
	}

	return result;
}

/*************************************************************************/
/**
 * This static function returns a seed based on a global counter
 *
 * @return A seed based on the current time
 */
boost::uint32_t GRandomFactory::GSeed(){
	if(!seedInitialized) GRandomFactory::setSeed_(DEFAULTSEED);

	// Make sure we have sole access to global variables
	boost::mutex::scoped_lock lk(global_seed_mutex_);

	if(globalSeed >= std::numeric_limits<boost::uint32_t>::max() - (GLOBALSEEDINCREMENT+1)) globalSeed = DEFAULTSEED;
	else (globalSeed+=GLOBALSEEDINCREMENT); // "7" chosen randomly -- no special meaning

	return globalSeed;
}

/*************************************************************************/
/**
 * This function starts the threads needed for the production of random numbers
 */
void GRandomFactory::startProducerThreads()  {
	for (boost::uint16_t i = 0; i < n01Threads_; i++) {
		// thread() doesn't throw, and no exceptions are listed in the documentation
		// for the create_thread() function, so we assume it doesn't throw.
		boost::uint32_t seed_ = GRandomFactory::GSeed();
		producer_threads_01_.create_thread(boost::bind(&GRandomFactory::producer01, this, seed_));
#ifdef DEBUG
	std::cout << "Started producer thread " << i << " in GRandomFactory with seed " << seed_ << std::endl;
#endif /* DEBUG */
	}
}

/*************************************************************************/
/**
 * The production of [0,1[ random numbers takes place here. As this function
 * is called in a thread, it may not throw under any circumstance. Exceptions
 * could otherwise go unnoticed. Hence this function has a possibly confusing
 * setup. Note that we do not use the global logger, as we do not want to risk
 * accessing a singleton past its possible destruction. The current value of the
 * arraySize_ variable is stored in the random number array, so it is possible
 * to change the size later.
 *
 * @param seed The seed for our local random number generator
 */
void GRandomFactory::producer01(boost::uint32_t seed)  {
	std::size_t localArraySize;

	try {
		boost::lagged_fibonacci607 lf(seed);

		while (true) {
			if(boost::this_thread::interruption_requested()) break;

			{
				// Retrieve the current array size. Reading this variable should
				// be an atomic operation and should thus not affect thread-safety.
				boost::mutex::scoped_lock lk(arraySizeMutex_);
				localArraySize = arraySize_;
			}

			// we want to store both the array size and the random numbers
			boost::shared_array<double> p(new double[localArraySize + 1]);

			 // Faster access during the fill procedure
			// Note that we own the only instance of this pointer at this point
			double *p_raw = p.get();
			p_raw[0] = static_cast<double>(localArraySize);

			for (std::size_t i = 1; i <= arraySize_; i++) {
#ifdef DEBUG
				double value = lf();
				assert(value>=0. && value<1.);
				p_raw[i]=value;
#else
				p_raw[i] = lf();
#endif /* DEBUG */
			}

			// Get a local copy of g01_, so its object does not get deleted involuntarily  when the singleton exits.
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
				  << "Caught std::invalid_argument exception with message"  << std::endl
				  << e.what() << std::endl;

		std::terminate();
	} catch (boost::thread_resource_error&) {
		std::cerr << "In GRandomFactory::producer01(): Error!" << std::endl
				  << "Caught boost::thread_resource_error exception which"  << std::endl
				  << "likely indicates that a mutex could not be locked."  << std::endl;

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
