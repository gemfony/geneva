/**
 * @file GRandomFactory.hpp
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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here

#include <atomic>
#include <cstdlib>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cassert>
#include <limits>
#include <random>
#include <thread>
#include <mutex>

// Boost headers go here
#include <boost/utility.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/cast.hpp>


#ifndef GRANDOMFACTORY_HPP_
#define GRANDOMFACTORY_HPP_


// Geneva headers go here

#include "common/GBoundedBufferT.hpp"
#include "common/GExceptions.hpp"
#include "common/GSingletonT.hpp"
#include "common/GStdThreadGroup.hpp"
#include "common/GHelperFunctions.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "hap/GRandomDefines.hpp"

/******************************************************************************/

namespace Gem {
namespace Hap {

using G_BASE_GENERATOR = std::mt19937;

class GRandomFactory; // Forward declaration, so we can make random_container constructor private

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This struct holds and creates random number containers to be transmitted to GRandomT
 * via a buffer. It does minimal error checking as it is meant for internal usage only,
 * and excessive error checking on the code might have strong performance implications.
 * None of the functions in this class is thread-safe (in the sense of being usable
 * concurrently from multiple threads).
 */
class random_container
	: private boost::noncopyable
{
	 friend class GRandomFactory; // Needed so we can prevent construction of containers outside of the factory

public:
	 /***************************************************************************/
	 /** @brief The destructor */
	 ~random_container()
	 { /* nothing */ }

	 /***************************************************************************/
	 /** @brief Returns the size of the buffer */
	 std::size_t size() const {
		 return DEFAULTARRAYSIZE;
	 }

	 /***************************************************************************/
	 /** @brief Returns the current position */
	 std::size_t getCurrentPosition() const {
		 return m_current_pos;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to check whether the buffer has run empty
	  */
	 bool empty() {
		 return (m_current_pos >= DEFAULTARRAYSIZE);
	 }

	 /***************************************************************************/
	 /**
	  * Returns the next random number from the package
	  */
	 G_BASE_GENERATOR::result_type next() {
#ifdef DEBUG
		 if(empty()) {
         glogger
         << "In random_container::next(): Error!" << std::endl
         << "Invalid m_current_pos: " << m_current_pos << " / " << DEFAULTARRAYSIZE << std::endl
         << GEXCEPTION;
      }
#endif

		 return m_r[m_current_pos++];
	 }

private:
	 /***************************************************************************/
	 /**
	  * Initialization with the number of entries in the buffer
	  *
	  * @param rng A reference to an external random number generator
	  */
	 explicit random_container(
		 G_BASE_GENERATOR &rng
	 )
		 : m_current_pos(0)
	 {
		 try {
			 for (std::size_t pos = 0; pos < DEFAULTARRAYSIZE; pos++) {
				 m_r[pos] = rng();
			 }
		 } catch (const std::bad_alloc &e) {
			 // This will propagate the exception to the global error handler so it can be logged
			 glogger
			 << "In random_container::random_container(T_RNG&): Error!" << std::endl
			 << "std::bad_alloc caught with message" << std::endl
			 << e.what() << std::endl
			 << GEXCEPTION;
		 } catch (...) {
			 // This will propagate the exception to the global error handler so it can be logged
			 glogger
			 << "In random_container::random_container(T_RNG&): Error!" << std::endl
			 << "unknown exception caught" << std::endl
			 << GEXCEPTION;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Replaces "used" random numbers by new numbers and resets the current_pos_
	  * pointer. T_RNG must be one of the standard C++1x-generators
	  */
	 void refresh(G_BASE_GENERATOR &rng) {
		 for (std::size_t pos = 0; pos < m_current_pos; pos++) {
			 m_r[pos] = rng();
		 }
		 m_current_pos = 0;
	 }
	 /***************************************************************************/

	 random_container() = delete; ///< The default constructor -- intentionally private and undefined
	 random_container(const random_container &) = delete; ///< The copy constructor -- intentionally private and undefined
	 random_container(random_container&&) = delete; ///< The move constructor -- intentionally private and undefined
	 const random_container &operator=(const random_container &) = delete; ///< intentionally private and undefined
	 const random_container &operator=(random_container &&) = delete; ///< Intentionally private and undefined

	 std::size_t m_current_pos; ///< The current position in the array

	 G_BASE_GENERATOR::result_type m_r[DEFAULTARRAYSIZE]; ///< Holds the actual random numbers
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Past implementations of random numbers for the Geneva library showed a
 * particular bottle neck in the random number generation. Every GObject
 * had its own random number generator, and seeding was very expensive.
 * We thus now produce floating point numbers in the range [0,1[ in a separate
 * thread in this class and calculate other numbers from this in the GRandom class.
 * A second thread is responsible for the creation of gaussian random numbers.
 * This circumvents the necessity to seed the generator over and over again and
 * allows us to get rid of a dependency on the MersenneTwister library. We are now
 * using a generator from the boost library instead, so users need to download fewer
 * libraries to use the Geneva library.
 *
 * This class produces packets of random numbers and stores them in bounded buffers.
 * Clients can retrieve packets of random numbers, while separate threads keep
 * filling the buffer up.
 *
 * The implementation currently uses the lagged fibonacci generator. According to
 * http://www.boost.org/doc/libs/1_35_0/libs/random/random-performance.html this is
 * the fastest generator amongst all of Boost's generators. It is the author's belief that
 * the "quality" of random numbers is of less concern in evolutionary algorithms, as the
 * geometry of the quality surface adds to the randomness. The original boost random
 * number generators have now been replaced by std::random generators, which are
 * modelled after their boost-equivalents.
 */
class GRandomFactory {
public:
	 /** @brief The default constructor */
	 G_API_HAP GRandomFactory();
	 /** @brief The destructor */
	 G_API_HAP ~GRandomFactory();

	 /** @brief Initialization code for the GRandomFactory */
	 G_API_HAP void init();
	 /** @brief Finalization code for the GRandomFactory */
	 G_API_HAP void finalize();

	 /** @brief Sets the number of producer threads for this factory. */
	 G_API_HAP void setNProducerThreads(const std::uint16_t &);

	 /** @brief Allows to retrieve the size of the array */
	 G_API_HAP std::size_t getCurrentArraySize() const;

	 /** @brief Allows to retrieve the size of the buffer */
	 G_API_HAP std::size_t getBufferSize() const;

	 /** @brief Delivers a new [0,1[ random number container with the current standard size to clients */
	 G_API_HAP std::unique_ptr <random_container> getNewRandomContainer();
	 /** @brief Retrieval of a new seed for external or internal random number generators */
	 G_API_HAP seed_type getSeed();

	 /** @brief Allows recycling of partially used packages */
	 G_API_HAP void returnUsedPackage(std::unique_ptr<random_container>);

private:
	 /***************************************************************************/
	 // Prevent copying
	 GRandomFactory(const GRandomFactory&) = delete;
	 GRandomFactory(const GRandomFactory&&) = delete;
	 const GRandomFactory& operator=(const GRandomFactory&) = delete;
	 const GRandomFactory& operator=(const GRandomFactory&&) = delete;

	 /** @brief The production of [0,1[ random numbers takes place here */
	 void producer(std::uint32_t seed);

	 bool m_finalized = false;
	 std::atomic<bool> m_threads_started; ///< Indicates whether threads were already started
	 std::atomic<bool> m_threads_stop_requested; ///< Indicates whether all threads were requested to stop
	 std::atomic<std::uint16_t> m_n_producer_threads; ///< The number of threads used to produce random numbers
	 Gem::Common::GStdThreadGroup m_producer_threads; ///< A thread group that holds [0,1[ producer threads

	 /** @brief A bounded buffer holding the random number packages */
	 Gem::Common::GBoundedBufferT<std::unique_ptr<random_container>,DEFAULTFACTORYBUFFERSIZE> m_p_fresh_bfr; // Note: Absolutely needs to be defined after the thread group !!!
	 /** @brief A bounded buffer holding random number packages ready for recycling */
	 Gem::Common::GBoundedBufferT<std::unique_ptr<random_container>,DEFAULTFACTORYBUFFERSIZE> m_p_ret_bfr;

	 static std::uint16_t m_multiple_call_trap; ///< Trap to catch multiple instantiations of this class
	 static std::mutex m_factory_creation_mutex; ///< Synchronization of access to multiple_call_trap in constructor

	 mutable std::mutex m_thread_creation_mutex; ///< Synchronization of access to the threads_started_ variable

	 std::random_device m_nondet_rng; ///< Source of non-deterministic random numbers
	 std::seed_seq m_seed_seq ///< A seeding sequence
		 = {
			 m_nondet_rng()
			 , m_nondet_rng()
			 , m_nondet_rng()
			 , m_nondet_rng()
			 , m_nondet_rng()
			 , m_nondet_rng()
			 , m_nondet_rng()
			 , m_nondet_rng()
		 };

	 mutable std::mutex m_seeding_mutex; ///< Regulates start-up of the seeding process
	 std::vector<seed_type> m_seed_collection; ///< Holds pre-calculated seeds
	 std::vector<seed_type>::const_iterator m_seed_cit; ///< Iterators over the seedCollection_
	 bool m_seeding_has_started=false;
};

} /* namespace Hap */
} /* namespace Gem */

/******************************************************************************/
/**
 * A single, global random number factory is created as a singleton.
 */
#define GRANDOMFACTORY      Gem::Common::GSingletonT<Gem::Hap::GRandomFactory>::Instance(0)
#define RESETGRANDOMFACTORY Gem::Common::GSingletonT<Gem::Hap::GRandomFactory>::Instance(1)


#endif /* GRANDOMFACTORY_HPP_ */
