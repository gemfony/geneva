/**
 * @file
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

#pragma once

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
#include <array>
#include <algorithm>

// Boost headers go here
#include <boost/utility.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/cast.hpp>

// Geneva headers go here

#include "common/GBoundedBufferT.hpp"
#include "common/GExceptions.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GSingletonT.hpp"
#include "common/GThreadGroup.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
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
class random_container {
	 friend class GRandomFactory; // Needed so we can prevent construction of containers outside of the factory

public:
	/***************************************************************************/
	// Deleted constructors and assignment operators

	random_container() = delete; ///< The default constructor -- intentionally private and undefined
	random_container(const random_container &) = delete; ///< The copy constructor -- intentionally private and undefined
	random_container(random_container&&) = delete; ///< The move constructor -- intentionally private and undefined
	random_container &operator=(const random_container &) = delete; ///< intentionally private and undefined
	random_container &operator=(random_container &&) = delete; ///< Intentionally private and undefined

	 /***************************************************************************/
	 /** @brief The destructor */
	 ~random_container() = default;

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
	 bool empty() const {
		 return (m_current_pos >= DEFAULTARRAYSIZE);
	 }

	 /***************************************************************************/
	 /**
	  * Returns the next random number from the package
	  */
	 G_BASE_GENERATOR::result_type next() {
#ifdef DEBUG
		 if(empty()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In random_container::next(): Error!" << std::endl
					 << "Invalid m_current_pos: " << m_current_pos << " / " << DEFAULTARRAYSIZE << std::endl
			 );
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
	 ) {
		 try {
			 std::generate(m_r.begin(), m_r.end(), [&](){ return rng(); });
		 } catch (const std::bad_alloc &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In random_container::random_container(T_RNG&): Error!" << std::endl
					 << "std::bad_alloc caught with message" << std::endl
					 << e.what() << std::endl
			 );
		 } catch (...) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In random_container::random_container(T_RNG&): Error!" << std::endl
					 << "unknown exception caught" << std::endl
			 );
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Replaces "used" random numbers by new numbers and resets the current_pos_
	  * pointer. T_RNG must be one of the standard C++1x-generators
	  */
	 void refresh(G_BASE_GENERATOR &rng) {
		 std::generate(m_r.begin(), m_r.begin() + m_current_pos, [&](){ return rng(); });
		 m_current_pos = 0;
	 }
	 /***************************************************************************/

	 std::size_t m_current_pos = 0; ///< The current position in the array
	 std::array<G_BASE_GENERATOR::result_type, DEFAULTARRAYSIZE> m_r{}; ///< Holds the actual random numbers
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

	 /***************************************************************************/
	 // Prevent copying and moving

	 GRandomFactory(GRandomFactory const &) = delete;
	 GRandomFactory(GRandomFactory &&) = delete;
	 GRandomFactory& operator=(GRandomFactory const &) = delete;
	 GRandomFactory& operator=(GRandomFactory &&) = delete;

	 /***************************************************************************/

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
	 G_API_HAP void returnUsedPackage(std::unique_ptr<random_container>&&);

private:
	 /** @brief The production of [0,1[ random numbers takes place here */
	 void producer(std::uint32_t seed);

	 std::atomic<bool> m_finalized = ATOMIC_VAR_INIT(false);
	 std::atomic<bool> m_threads_started = ATOMIC_VAR_INIT(false); ///< Indicates whether threads were already started
	 std::atomic<bool> m_threads_stop_requested = ATOMIC_VAR_INIT(false); ///< Indicates whether all threads were requested to stop
	 std::atomic<std::uint16_t> m_n_producer_threads = ATOMIC_VAR_INIT(DEFAULT01PRODUCERTHREADS); ///< The number of threads used to produce random numbers

	 Gem::Common::GThreadGroup m_producer_threads; ///< A thread group that holds [0,1[ producer threads

	 /** @brief A bounded buffer holding the random number packages */
	 Gem::Common::GBoundedBufferT<std::unique_ptr<random_container>,DEFAULTFACTORYBUFFERSIZE> m_p_fresh_bfr; // Note: Absolutely needs to be defined after the thread group !!!
	 /** @brief A bounded buffer holding random number packages ready for recycling */
	 Gem::Common::GBoundedBufferT<std::unique_ptr<random_container>,DEFAULTFACTORYBUFFERSIZE> m_p_ret_bfr;

	 static std::atomic<bool> m_multiple_call_trap; ///< Trap to catch multiple instantiations of this class -- this is mostly for debugging purposes

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
			 , m_nondet_rng()
			 , m_nondet_rng()
			 , m_nondet_rng()
			 , m_nondet_rng()
			 , m_nondet_rng()
			 , m_nondet_rng()
			 , m_nondet_rng()
			 , m_nondet_rng()
		 };

	 mutable std::mutex m_seeding_mutex; ///< Regulates start-up of the seeding process
	 std::vector<seed_type> m_seed_collection = std::vector<seed_type>(DEFAULTSEEDVECTORSIZE); ///< Holds pre-calculated seeds
	 std::vector<seed_type>::const_iterator m_seed_cit = m_seed_collection.begin(); ///< Iterators over the seedCollection_
	 std::atomic<bool> m_seeding_has_started = ATOMIC_VAR_INIT(false);
};

} /* namespace Hap */
} /* namespace Gem */

/******************************************************************************/
/**
 * A single, global random number factory is created as a singleton.
 */
#define GRANDOMFACTORY       Gem::Common::GSingletonT<Gem::Hap::GRandomFactory>::Instance(0)
#define GRANDOMFACTORY_RESET Gem::Common::GSingletonT<Gem::Hap::GRandomFactory>::Instance(1)

