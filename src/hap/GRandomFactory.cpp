/**
 * @file GRandomFactory.cpp
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

#include "hap/GRandomFactory.hpp"

namespace Gem {
namespace Hap {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization with the number of entries in the buffer
 * @param binSize The size of the random buffer
 * @param lf A reference to an external random number generator
 */
random_container::random_container(
   const std::size_t& binSize
   , lagged_fibonacci& lf
)
   : current_pos_(0)
   , binSize_(binSize)
   , r_((double *)NULL)
{
   try {
      r_ = new double[binSize_];
      for(std::size_t pos=0; pos<binSize_; pos++) {
         r_[pos]=lf();
      }
   } catch(const std::bad_alloc& e) {
      // This will propagate the exception to the global error handler so it can be logged
      glogger
      << "In random_container::random_container(const std::size_t&): Error!" << std::endl
      << "std::bad_alloc caught with message" << std::endl
      << e.what() << std::endl
      << GEXCEPTION;
   } catch(...) {
      // This will propagate the exception to the global error handler so it can be logged
      glogger
      << "In random_container::random_container(const std::size_t&): Error!" << std::endl
      << "unknown exception caught" << std::endl
      << GEXCEPTION;
   }
}

/******************************************************************************/
/**
 * The destructor -- gets rid of the random buffer r
 */
random_container::~random_container() {
   if(r_) {
      delete [] r_;
   }
}

/******************************************************************************/
/**
 * Returns the size of the buffer
 */
std::size_t random_container::size() const {
   return binSize_;
}

/******************************************************************************/
/**
 * Returns the current position
 */
std::size_t random_container::getCurrentPosition() const {
   return current_pos_;
}

/******************************************************************************/
/**
 * Replaces "used" random numbers by new numbers and resets the current_pos_
 * pointer.
 */
void random_container::refresh(lagged_fibonacci& lf) {
   for(std::size_t pos=0; pos<current_pos_; pos++) {
      r_[pos]=lf();
   }
   current_pos_ = 0;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization of static data members
 */
boost::uint16_t Gem::Hap::GRandomFactory::multiple_call_trap_ = 0;
boost::mutex Gem::Hap::GRandomFactory::factory_creation_mutex_;

/******************************************************************************/
/**
 * The standard constructor, which seeds the random number generator and checks
 * that this class is instantiated only once.
 */
GRandomFactory::GRandomFactory()
	: finalized_(false)
	, threadsHaveBeenStarted_(false)
	, n01Threads_(boost::numeric_cast<boost::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULT01PRODUCERTHREADS)))
	, p01_ (DEFAULTFACTORYBUFFERSIZE)
   , r01_ (DEFAULTFACTORYBUFFERSIZE)
   , startSeed_(boost::numeric_cast<initial_seed_type>(this->nondet_rng()))
{
   // Check whether enough entropy is available. Warn, if this is not the case
   if(0. == nondet_rng.entropy()) {
      glogger
      << "In GSeedManager::GSeedManager(): Error!" << std::endl
      << "Source of non-deterministic random numbers" << std::endl
      << "has entropy 0." << std::endl
      << GWARNING;
   }

	boost::mutex::scoped_lock lk(factory_creation_mutex_);
	if(multiple_call_trap_ > 0) {
	   std::cerr
	   << "Error in GRandomFactory::GRandomFactory():" << std::endl
	   << "Class has been instantiated before." << std::endl
	   << "and may be instantiated only once" << std::endl;
	   std::terminate();
	}
	else {
		multiple_call_trap_++;
	}
}

/******************************************************************************/
/**
 * The destructor. All work is done in the finalize() function.
 */
GRandomFactory::~GRandomFactory() {
	// Make sure the finalization code is executed
	// (if this hasn't happened already). Calling
	// finalize() multiple times is safe.
	finalize();
}

/******************************************************************************/
/**
 * Initializes the factory. This function does nothing at this time. Its
 * only purpose is to control initialization of the factory in the singleton.
 */
void GRandomFactory::init() { /* nothing */ }

/******************************************************************************/
/**
 * Finalization code for the GRandomFactory. All threads are given the
 * interrupt signal. Then we wait for them to join us. This function will
 * only once perform useful work and will return immediately when called a
 * second time. It can thus be called as often as you wish.
 */
void GRandomFactory::finalize() {
	// Only allow one finalization action to be carried out
	if(finalized_) return;

	producer_threads_01_.interrupt_all(); // doesn't throw
	producer_threads_01_.join_all();
	finalized_ = true; // Let the audience know
}

/******************************************************************************/
/**
 * Allows to retrieve the size of random number arrays
 *
 * @return The current value of the arraySize_ variable
 */
std::size_t GRandomFactory::getCurrentArraySize() const {
   return DEFAULTARRAYSIZE;
}

/******************************************************************************/
/**
 * Retrieves the size of the random buffer, i.e. the array holding the random
 * number packages.
 *
 * @return The size of the random buffer
 */
std::size_t GRandomFactory::getBufferSize() const {
	return DEFAULTFACTORYBUFFERSIZE;
}

/******************************************************************************/
/**
 * Provides users with an interface to set the initial seed for the seed
 * generator. Note that this function will have no effect once seeding has started.
 * A boolean will be returned that indicates whether the function has had
 * an effect, i.e. whether the seed could be set. If not set by the user, the seed manager
 * will start upon first retrieval of a seed and will then try to acquire a seed
 * automatically.
 *
 * @param seed The desired initial value of the global seed
 * @return A boolean indicating whether the seed could be set
 */
bool GRandomFactory::setStartSeed(const initial_seed_type& initial_seed) {
   // Determine whether the seed-generator has already been initialized. If not, start it
   boost::upgrade_lock<boost::shared_mutex> sm_lck(seedingMutex_);
   if(!mt_ptr_) { // double check locking pattern
      boost::upgrade_to_unique_lock< boost::shared_mutex > unique_lck(sm_lck); // exclusive access
      if(!mt_ptr_) {
         mt_ptr_ = boost::shared_ptr<mersenne_twister>(new mersenne_twister(boost::numeric_cast<seed_type>(initial_seed)));
         startSeed_ = initial_seed;
         return true;
      }
   }

   return false;
}

/******************************************************************************/
/**
 * Retrieval of the value of the global startSeed_ variable
 *
 * @return The value of the global start seed
 */
initial_seed_type GRandomFactory::getStartSeed() const {
   boost::shared_lock<boost::shared_mutex> sm_lck(seedingMutex_);
   return startSeed_;
}

/******************************************************************************/
/**
 * Checks whether the seeding process has already started
 *
 * @return A boolean indicating whether seeding has already been initialized
 */
bool GRandomFactory::checkSeedingIsInitialized() const {
   boost::shared_lock<boost::shared_mutex> sm_lck(seedingMutex_);
   if(mt_ptr_) {
      return true;
   }
   return false;
}

/******************************************************************************/
/**
 * This function returns a random number from a pseudo random number generator
 * that has (usually -- depends on the system) been seeded from a non-deterministic
 * source (unless the user has set a seed manually). The function will initialize
 * the seeding process, if this hasn't happened yet.
 *
 * @return A seed taken from a local random number generator
 */
boost::uint32_t GRandomFactory::getSeed(){
   { // Determine whether the seed-generator has already been initialized. If not, start it
      boost::upgrade_lock<boost::shared_mutex> sm_lck(seedingMutex_);
      if(!mt_ptr_) { // double lock pattern
         boost::upgrade_to_unique_lock< boost::shared_mutex > unique_lck(sm_lck); // exclusive access
         if(!mt_ptr_) {
            mt_ptr_ = boost::shared_ptr<mersenne_twister>(new mersenne_twister(boost::numeric_cast<seed_type>(startSeed_)));
         }
      }
   }

   boost::unique_lock<boost::shared_mutex> sm_lck(seedingMutex_);
	return (*mt_ptr_)();
}

/******************************************************************************/
/**
 * Allows recycling of partially used packages. Note that this function may
 * delete its argument if it cannot be added to the buffer.
 *
 * @param r A pointer to a partially used work package
 * @param current_pos The first position in the array that holds unused random numbers
 */
void GRandomFactory::returnUsedPackage(boost::shared_ptr<random_container> p) {
   // We try to add the item to the r01_ queue, until a timeout occurs.
   // Once this is the case we delete the package, so we do not overflow
   // with recycled packages
   try{
      r01_.push_front(p, boost::posix_time::milliseconds(DEFAULTFACTORYPUTWAIT));
   } catch (Gem::Common::condition_time_out&) { // No luck buffer is full. Delete the recycling bin
      p.reset();
   }
}

/******************************************************************************/
/**
 * Sets the number of producer threads for this factory.
 *
 * @param n01Threads
 */
void GRandomFactory::setNProducerThreads(const boost::uint16_t& n01Threads)
{
	// Make a suggestion for the number of threads, if requested
   boost::uint16_t n01Threads_local =
         (n01Threads>0)?n01Threads:(boost::numeric_cast<boost::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULT01PRODUCERTHREADS)));

	// Threads might already be running, so we need to regulate access
	{
		boost::unique_lock<boost::mutex> lk(thread_creation_mutex_);
		if(threadsHaveBeenStarted_.load()) {
			if (n01Threads_local > n01Threads_.load()) { // start new 01 threads
				for (boost::uint16_t i = n01Threads_.load(); i < n01Threads_local; i++) {
					producer_threads_01_.create_thread(
					   [this](){ this->producer01(this->getSeed()); }
					);
				}
			} else if (n01Threads_local < n01Threads_.load()) { // We need to remove threads
			   // remove_last will internally call "interrupt" for these threads
			   producer_threads_01_.remove_last(n01Threads_.load() - n01Threads_local);
			}
		}

		n01Threads_ = n01Threads_local;
	}
}

/******************************************************************************/
/**
 * When objects need a new container [0,1[ of random numbers with the current
 * default size, they call this function.
 *
 * @return A packet of new [0,1[ random numbers
 */
boost::shared_ptr<random_container> GRandomFactory::new01Container() {
	// Start the producer threads upon first access to this function
	if(!threadsHaveBeenStarted_.load()) {
		boost::unique_lock<boost::mutex> tc_lk(thread_creation_mutex_);
		if(!threadsHaveBeenStarted_.load()) { // double checked locking pattern
		   //---------------------------------------------------------
		   for (boost::uint16_t i = 0; i < n01Threads_.load(); i++) {
		      producer_threads_01_.create_thread(
               [this](){ this->producer01(this->getSeed()); }
		      );
		   }
         //---------------------------------------------------------

			threadsHaveBeenStarted_=true;
		}
	}

	boost::shared_ptr<random_container> p; // empty
	try {
		p01_.pop_back(p, boost::posix_time::milliseconds(DEFAULTFACTORYGETWAIT));
	} catch (Gem::Common::condition_time_out&) {
      // nothing - our way of signaling a time out
      // is to return an empty boost::shared_ptr
	   p = boost::shared_ptr<random_container>();
	}

	return p;
}

/******************************************************************************/
/**
 * The production of [0,1[ random numbers takes place here. As this function
 * is called in a thread, it may only throw using Genevas mechanisms. Exceptions
 * could otherwise go unnoticed. Hence this function has a possibly confusing
 * setup.
 *
 * @param seed A seed for our local random number generator
 */
void GRandomFactory::producer01(boost::uint32_t seed)  {
	try {
		lagged_fibonacci lf(seed);
		boost::shared_ptr<random_container> p;

		while(!boost::this_thread::interruption_requested()) {
		   if(!p) { // buffer is still empty
		      // First we try to retrieve a "recycled" item from the r01_ buffer. If this
		      // fails (likely because the buffer is empty), we create a new item instead
		      try {
		         r01_.pop_back(p, boost::posix_time::milliseconds(DEFAULTFACTORYGETWAIT));

		         // If we reach this line, we have successfully retrieved a recycled container.
		         // First do some error-checking
#ifdef DEBUG
		         if(!p) {
		            glogger
		            << "In RandomFactory::producer01(): Error!" << std::endl
		            << "Got empty recycling pointer" << std::endl
		            << GEXCEPTION;
		         }

#endif /* DEBUG */

		         // Finally we replace "used" random numbers with new ones
		         p->refresh(lf);

		      } catch (Gem::Common::condition_time_out&) { // O.k., so we need to create a new container
		         p = boost::shared_ptr<random_container>(new random_container(DEFAULTARRAYSIZE, lf));
		      }
		   }

		   // Thanks to the following call, thread creation will be mostly idle if the buffer is full
		   try {
		      // Put the bufffer in the queue
		      p01_.push_front(p, boost::posix_time::milliseconds(DEFAULTFACTORYPUTWAIT));
		      // Reset the shared_ptr so the next pointer may be created
		      p.reset();
		   } catch (Gem::Common::condition_time_out&) {
		      continue; // Try again, if we didn't succeed
		   }
		}
	} catch (boost::thread_interrupted&) { // Not an error
		return; // We're done
	} catch (std::bad_alloc& e) {
	   glogger
	   << "In GRandomFactory::producer01(): Error!" << std::endl
		<< "Caught std::bad_alloc exception with message"
		<< std::endl << e.what() << std::endl
		<< GEXCEPTION;
	} catch (std::invalid_argument& e) {
		glogger
		<< "In GRandomFactory::producer01(): Error!" << std::endl
		<< "Caught std::invalid_argument exception with message"  << std::endl
		<< e.what() << std::endl
		<< GEXCEPTION;
	} catch (boost::thread_resource_error&) {
		glogger
      << "In GRandomFactory::producer01(): Error!" << std::endl
      << "Caught boost::thread_resource_error exception which"  << std::endl
      << "likely indicates that a mutex could not be locked."  << std::endl
      << GEXCEPTION;
	} catch (...) {
		glogger
		<< "In GRandomFactory::producer01(): Error!" << std::endl
		<< "Caught unkown exception." << std::endl
		<< GEXCEPTION;
	}
}

/******************************************************************************/

} /* namespace Hap */
} /* namespace Gem */
