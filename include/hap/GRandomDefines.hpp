/**
 * @file
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

