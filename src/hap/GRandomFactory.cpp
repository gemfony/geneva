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
   if(!mt_ptr_) { // double lock pattern
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
 * Allows recycling of partially used packages. The first entry of the package
 * signifies the first "unused" random number. Note that this function may
 * delete its argument if it cannot be added to the buffer.
 *
 * @param r A pointer to a partially used work package
 * @param current_pos The first position in the array that holds unused random numbers
 */
void GRandomFactory::returnPartiallyUsedPackage(double *r, std::size_t current_pos) {
   // We try to add the item to the r01_ queue, until a timeout occurs.
   // Once this is the case we delete the package, so we do not overflow
   // with recycled packages
   recyclingBin *rb_ptr;
   try{
      rb_ptr = new recyclingBin();

      rb_ptr->r = r;
      rb_ptr->current_pos = current_pos;

      // std::cout << "Returning package with " << DEFAULTARRAYSIZE-current_pos << " ( " << DEFAULTARRAYSIZE << " / " << current_pos << ") remaining numbers" << std::endl;
      r01_.push_front(rb_ptr, boost::posix_time::milliseconds(DEFAULTFACTORYGETWAIT));
   } catch (Gem::Common::condition_time_out&) { // No luck buffer is full. Delete the recycling bin
      delete [] rb_ptr->r;
      delete rb_ptr;
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
		boost::mutex::scoped_lock lk(thread_creation_mutex_);
		if(threadsHaveBeenStarted_.load()) {
			if (n01Threads_local > n01Threads_) { // start new 01 threads
				for (boost::uint16_t i = n01Threads_; i < n01Threads_local; i++) {
					producer_threads_01_.create_thread(
                  boost::bind(
                     &GRandomFactory::producer01
                     , this
                     , this->getSeed()
                  )
					);
				}
			} else if (n01Threads_local < n01Threads_) { // We need to remove threads
			   // remove_last will internally call "interrupt" for these threads
			   producer_threads_01_.remove_last(n01Threads_ - n01Threads_local);
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
double * GRandomFactory::new01Container() {
	// Start the producer threads upon first access to this function
	if(!threadsHaveBeenStarted_.load()) {
		boost::mutex::scoped_lock lk(thread_creation_mutex_);
		if(!threadsHaveBeenStarted_.load()) {
			startProducerThreads();
			threadsHaveBeenStarted_=true;
		}
	}

	double *result = (double *)NULL; // empty
	try {
		p01_.pop_back(result, boost::posix_time::milliseconds(DEFAULTFACTORYGETWAIT));
	} catch (Gem::Common::condition_time_out&) {
      // nothing - our way of signaling a time out
      // is to return an empty boost::shared_ptr
	   result = (double *)NULL;
	}

	return result;
}

/******************************************************************************/
/**
 * This function starts the threads needed for the production of random numbers
 */
void GRandomFactory::startProducerThreads()  {
	for (boost::uint16_t i = 0; i < n01Threads_; i++) {
		producer_threads_01_.create_thread(
         boost::bind(
               &GRandomFactory::producer01
               , this
               , this->getSeed()
         )
		);
	}
}

/******************************************************************************/
/**
 * The production of [0,1[ random numbers takes place here. As this function
 * is called in a thread, it may only throw using Genevas mechanisms. Exceptions
 * could otherwise go unnoticed. Hence this function has a possibly confusing
 * setup.
 *
 * TODO: Check occurances of DEFAULTFACTORYGETWAIT and DEFAULTFACTORYPUTWAIT for consistency
 * TODO: Rename current_pos to pristine_pos
 * TODO: Rewrite again with boost::array
 *
 * @param seed A seed for our local random number generator
 */
void GRandomFactory::producer01(boost::uint32_t seed)  {
	try {
		lagged_fibonacci lf(seed);

		double *p = (double *)NULL;
		while(!boost::this_thread::interruption_requested()) {
		   if((double *)NULL == p) {
		      // First we try to retrieve a "recycled" item from the r01_ buffer. If this
		      // fails (likely because the buffer is empty), we create a new item
	         try {
	            recyclingBin *rb_ptr;
	            r01_.pop_back(rb_ptr, boost::posix_time::milliseconds(DEFAULTFACTORYGETWAIT));

	            // If we reach this line, we have successfully retrieved a recycled container.
	            // First do some error-checking
#ifdef DEBUG
	            if(!rb_ptr) {
	               glogger
	               << "In RandomFactory::producer01(): Error!" << std::endl
	               << "Got empty recycling pointer" << std::endl
	               << GEXCEPTION;
	            }

#endif /* DEBUG */

	            // We now need to freshen it up
	            p = rb_ptr->r;
	            for(std::size_t pos=0; pos<rb_ptr->current_pos; pos++) {
	               p[pos]=lf();
	            }

	            // Finally get rid of the now "empty" rb_ptr
	            if(rb_ptr) delete rb_ptr;
	         } catch (Gem::Common::condition_time_out&) { // O.k., so we need to create a new container
	            p = new double[DEFAULTARRAYSIZE];
	            for (std::size_t pos = 0; pos < DEFAULTARRAYSIZE; pos++) {
	               p[pos] = lf();
	            }
	         }
		   }

			// Thanks to the following call, thread creation will be mostly idle if the buffer is full
		   try {
	         p01_.push_front(p, boost::posix_time::milliseconds(DEFAULTFACTORYPUTWAIT));
	         p = (double *)NULL; // Make sure a new data item is produced in the next iteration
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
