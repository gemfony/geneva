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

// Standard includes go here
#include <cstdlib>
#include <random>

// Boost includes go here

namespace Gem {
namespace Hap {

/******************************************************************************/
// Some typedefs for the seed manager and random factory
using mersenne_twister = std::mt19937;
using initial_seed_type = std::mt19937::result_type;
using seed_type = initial_seed_type;
using lagged_fibonacci = std::subtract_with_carry_engine<uint_fast64_t, 48, 5, 12>; // ranlux48_base

/******************************************************************************/
// Some constants needed for the random number generation

#ifdef GENEVA_HAP_RANDOM_FACTORY_DEFAULT_ARRAY_SIZE
	const std::size_t   DEFAULTARRAYSIZE = GENEVA_HAP_RANDOM_FACTORY_DEFAULT_ARRAY_SIZE; ///< Default size of the random number array
#else
	const std::size_t   DEFAULTARRAYSIZE = 1000; ///< Default size of the random number array
#endif /* GHAP_DEFAULT_ARRAY_SIZE */

#ifdef GENEVA_HAP_RANDOM_FACTORY_DEFAULT_BUFFER_SIZE
	const std::size_t   DEFAULTFACTORYBUFFERSIZE = GENEVA_HAP_RANDOM_FACTORY_DEFAULT_BUFFER_SIZE; ///< Default size of the underlying buffer
#else
	const std::size_t   DEFAULTFACTORYBUFFERSIZE = Gem::Common::DEFAULTBUFFERSIZE; ///< Default size of the underlying buffer
#endif /* GENEVA_HAP_RANDOM_FACTORY_DEFAULT_BUFFER_SIZE */

#ifdef GENEVA_HAP_RANDOM_FACTORY_DEFAULT_PUT_WAIT
	const std::uint16_t DEFAULTFACTORYPUTWAIT = GENEVA_HAP_RANDOM_FACTORY_DEFAULT_PUT_WAIT; ///< waiting time in milliseconds
#else
	const std::uint16_t DEFAULTFACTORYPUTWAIT = 200; ///< waiting time in milliseconds
#endif /* GENEVA_HAP_RANDOM_FACTORY_DEFAULT_PUT_WAIT */

#ifdef GENEVA_HAP_RANDOM_FACTORY_DEFAULT_GET_WAIT
	const std::uint16_t DEFAULTFACTORYGETWAIT = GENEVA_HAP_RANDOM_FACTORY_DEFAULT_GET_WAIT; ///< waiting time in milliseconds
#else
	const std::uint16_t DEFAULTFACTORYGETWAIT = 200; ///< waiting time in milliseconds
#endif /* GENEVA_HAP_RANDOM_FACTORY_DEFAULT_GET_WAIT */

#ifdef GENEVA_HAP_RANDOM_FACTORY_SEEDQUEUE_PUT_WAIT
	const std::uint16_t DEFAULTSEEDQUEUEPUTWAIT = GENEVA_HAP_RANDOM_FACTORY_SEEDQUEUE_PUT_WAIT; ///< waiting time for seeding queue in milliseconds
#else
	const std::uint16_t DEFAULTSEEDQUEUEPUTWAIT = 200; ///< waiting time for seeding queue in milliseconds
#endif /* GENEVA_HAP_RANDOM_FACTORY_SEEDQUEUE_PUT_WAIT */

#ifdef GENEVA_HAP_RANDOM_FACTORY_SEED_VECTOR_SIZE
	const std::size_t   DEFAULTSEEDVECTORSIZE = GENEVA_HAP_RANDOM_FACTORY_SEED_VECTOR_SIZE; ///< The size of the seeding vector
#else
	const std::size_t   DEFAULTSEEDVECTORSIZE = 2000; ///< The size of the seeding vector
#endif /* GENEVA_HAP_RANDOM_FACTORY_SEED_VECTOR_SIZE */

/******************************************************************************/
/**
 * The number of threads that simultaneously produce [0,1[ random numbers
 */
const std::uint16_t DEFAULT01PRODUCERTHREADS = 2;

/******************************************************************************/
/**
 * The maximum value of std::int32_t, converted to a double value. This is
 * needed to scale the output of std::minstd_rand0 to a maximum value of 1.
 */
const double rnr_max = static_cast<double>(boost::numeric::bounds<std::minstd_rand0::result_type>::highest());

/******************************************************************************/
/**
 * This seed will be used as the global setting if the seed hasn't
 * been set manually and could not be determined in a random way (e.g.
 * by reading from /dev/urandom). The chosen value follows a setting
 * in boost's mersenne twister library.
 */
const std::uint32_t DEFAULTSTARTSEED = 5489;

/******************************************************************************/
/**
 * This value specifies the number of seeds in the queue
 */
const std::size_t DEFAULTSEEDQUEUESIZE = 1000;

/******************************************************************************/
/**
 * The minimal size of the double buffer in the GRandomFactoryT
 */
const std::size_t MINDOUBLEBUFFERSIZE = 10000;

/******************************************************************************/

} /* namespace Hap */
} /* namespace Gem */

