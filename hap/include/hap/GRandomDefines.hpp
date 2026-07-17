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
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard includes go here
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <random>
#include <thread>

// Geneva headers go here
#include "common/GCommonEnums.hpp"  // for DEFAULTBUFFERSIZE
#include "common/concurrency/GQueueCommon.hpp"  // for QueueBackend

namespace Gem::Hap {

/******************************************************************************/
// Some typedefs for the seed manager and random factory
using mersenne_twister = std::mt19937;
using initial_seed_type = std::mt19937::result_type;
using seed_type = initial_seed_type;
using lagged_fibonacci = std::subtract_with_carry_engine<uint_fast64_t, 48, 5, 12>; // ranlux48_base

/******************************************************************************/
// Some constants needed for the random number generation

#ifdef GENEVA_HAP_RANDOM_FACTORY_DEFAULT_ARRAY_SIZE
const std::size_t DEFAULTARRAYSIZE =
    GENEVA_HAP_RANDOM_FACTORY_DEFAULT_ARRAY_SIZE; ///< Default size of the random number array
#else
constexpr std::size_t DEFAULTARRAYSIZE = 10000; ///< Default size of the random number array
#endif /* GHAP_DEFAULT_ARRAY_SIZE */

#ifdef GENEVA_HAP_RANDOM_FACTORY_DEFAULT_BUFFER_SIZE
const std::size_t DEFAULTFACTORYBUFFERSIZE =
    GENEVA_HAP_RANDOM_FACTORY_DEFAULT_BUFFER_SIZE; ///< Default size of the underlying buffer
#else
const std::size_t DEFAULTFACTORYBUFFERSIZE =
    Gem::Common::DEFAULTBUFFERSIZE; ///< Default size of the underlying buffer
#endif /* GENEVA_HAP_RANDOM_FACTORY_DEFAULT_BUFFER_SIZE */

// Selects the MPMC-queue backend the random factory's package buffers use. The default is the
// std::deque-backed queue, so production behaviour is unchanged; defining
// GENEVA_HAP_FACTORY_QUEUE_PREALLOCATED at configure time switches them to the preallocated ring
// buffer (a drop-in, interface-identical alternative -- see Gem::Common::Concurrency::GMPMCQueueT).
#ifdef GENEVA_HAP_FACTORY_QUEUE_PREALLOCATED
constexpr Gem::Common::Concurrency::QueueBackend FACTORYQUEUEBACKEND = Gem::Common::Concurrency::QueueBackend::Preallocated;
#else
constexpr Gem::Common::Concurrency::QueueBackend FACTORYQUEUEBACKEND = Gem::Common::Concurrency::QueueBackend::Deque;
#endif /* GENEVA_HAP_FACTORY_QUEUE_PREALLOCATED */

#ifdef GENEVA_HAP_RANDOM_FACTORY_DEFAULT_PUT_WAIT
const std::uint16_t DEFAULTFACTORYPUTWAIT =
    GENEVA_HAP_RANDOM_FACTORY_DEFAULT_PUT_WAIT; ///< waiting time in milliseconds
#else
constexpr std::uint16_t DEFAULTFACTORYPUTWAIT = 200; ///< waiting time in milliseconds
#endif /* GENEVA_HAP_RANDOM_FACTORY_DEFAULT_PUT_WAIT */

#ifdef GENEVA_HAP_RANDOM_FACTORY_DEFAULT_GET_WAIT
const std::uint16_t DEFAULTFACTORYGETWAIT =
    GENEVA_HAP_RANDOM_FACTORY_DEFAULT_GET_WAIT; ///< waiting time in milliseconds
#else
constexpr std::uint16_t DEFAULTFACTORYGETWAIT = 200; ///< waiting time in milliseconds
#endif /* GENEVA_HAP_RANDOM_FACTORY_DEFAULT_GET_WAIT */

#ifdef GENEVA_HAP_RANDOM_FACTORY_SEEDQUEUE_PUT_WAIT
const std::uint16_t DEFAULTSEEDQUEUEPUTWAIT =
    GENEVA_HAP_RANDOM_FACTORY_SEEDQUEUE_PUT_WAIT; ///< waiting time for seeding queue in milliseconds
#else
constexpr std::uint16_t DEFAULTSEEDQUEUEPUTWAIT =
    200; ///< waiting time for seeding queue in milliseconds
#endif /* GENEVA_HAP_RANDOM_FACTORY_SEEDQUEUE_PUT_WAIT */

#ifdef GENEVA_HAP_RANDOM_FACTORY_SEED_VECTOR_SIZE
const std::size_t DEFAULTSEEDVECTORSIZE =
    GENEVA_HAP_RANDOM_FACTORY_SEED_VECTOR_SIZE; ///< The size of the seeding vector
#else
constexpr std::size_t DEFAULTSEEDVECTORSIZE = 2000; ///< The size of the seeding vector
#endif /* GENEVA_HAP_RANDOM_FACTORY_SEED_VECTOR_SIZE */

/******************************************************************************/
/**
 * The fallback number of threads that simultaneously produce [0,1[ random numbers when the core
 * count cannot be determined, and the floor for the hardware-derived count (see
 * autoProducerThreadCount()).
 */
constexpr std::uint16_t DEFAULT01PRODUCERTHREADS = 2;

/** @brief Divisor applied to hardware_concurrency() when auto-sizing the RNG producer pool. */
constexpr unsigned int PRODUCERTHREADS_HW_DIVISOR = 4;
/** @brief Upper clamp on the auto-sized producer count (bounds fresh-queue lock contention). */
constexpr std::uint16_t MAXAUTOPRODUCERTHREADS = 8;

/******************************************************************************/
/**
 * @brief The producer-thread count used when an automatic ("0") count is requested.
 *
 * A request of 0 producer threads means "size the producer pool to the hardware" (mirroring the
 * "0 == auto" convention elsewhere in Geneva). Random-number production is cheap and the producers
 * block on the bounded fresh-package buffer once it is full, so an idle surplus costs little; the
 * hardware count is nevertheless scaled by @c PRODUCERTHREADS_HW_DIVISOR and clamped to
 * <tt>[DEFAULT01PRODUCERTHREADS, MAXAUTOPRODUCERTHREADS]</tt> to bound contention on the single
 * fresh-package queue. If @c std::thread::hardware_concurrency() cannot determine the core count
 * (returns 0), the fixed @c DEFAULT01PRODUCERTHREADS is used.
 *
 * @return A hardware-derived producer-thread count, clamped to <tt>[DEFAULT01PRODUCERTHREADS, MAXAUTOPRODUCERTHREADS]</tt>
 */
inline std::uint16_t autoProducerThreadCount() {
    const unsigned int hw = std::thread::hardware_concurrency();
    if(hw == 0) { return DEFAULT01PRODUCERTHREADS; }
    const unsigned int scaled = std::max<unsigned int>(DEFAULT01PRODUCERTHREADS, hw / PRODUCERTHREADS_HW_DIVISOR);
    return static_cast<std::uint16_t>(std::min<unsigned int>(scaled, MAXAUTOPRODUCERTHREADS));
}

/******************************************************************************/
/**
 * This seed will be used as the global setting if the seed hasn't
 * been set manually and could not be determined in a random way (e.g.
 * by reading from /dev/urandom). The chosen value follows a setting
 * in boost's mersenne twister library.
 */
constexpr std::uint32_t DEFAULTSTARTSEED = 5489;

/******************************************************************************/
/**
 * This value specifies the number of seeds in the queue
 */
constexpr std::size_t DEFAULTSEEDQUEUESIZE = 1000;

/******************************************************************************/
/**
 * The minimal size of the double buffer in the GRandomFactoryT
 */
constexpr std::size_t MINDOUBLEBUFFERSIZE = 10000;

/******************************************************************************/

} /* namespace Gem::Hap */
