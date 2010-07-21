/**
 * @file GRandomDefines.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Hap library, Gemfony scientific's library of
 * random number routines.
 *
 * Hap is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Hap is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Hap library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Hap, visit
 * http://www.gemfony.com .
 */

// Standard includes go here
#include <cstdlib>

#ifndef GRANDOMDEFINES_HPP_
#define GRANDOMDEFINES_HPP_

/****************************************************************************/
// Some constants needed for the random number generation

const std::size_t DEFAULTARRAYSIZE = 1000; ///< Default size of the random number array
const std::size_t DEFAULTFACTORYBUFFERSIZE = 400; ///< Default size of the underlying buffer
const boost::uint16_t DEFAULTFACTORYPUTWAIT = 5; ///< waiting time in milliseconds
const boost::uint16_t DEFAULTFACTORYGETWAIT = 5; ///< waiting time in milliseconds
const boost::uint16_t DEFAULTSEEDQUEUEPUTWAIT = 50; ///< waiting time for seeding queue in mulliseconds

/****************************************************************************/
/**
 * The number of threads that simultaneously produce [0,1[ random numbers
 */
const boost::uint16_t DEFAULT01PRODUCERTHREADS = 4;

/****************************************************************************/
/**
 * The maximum value of boost::int32_t, converted to a double value. This is
 * needed to scale the output of boost::rand48 to a maximum value of 1.
 */
const double rnr_max = static_cast<double>(std::numeric_limits<boost::int32_t>::max());

/****************************************************************************/
/**
 * This seed will be used as the global setting if the seed hasn't
 * been set manually and could not be determined in a random way (e.g.
 * by reading from /dev/urandom). The chosen value follows a setting
 * in boost's mersenne twister library.
 */
const boost::uint32_t DEFAULTSTARTSEED=5489;

/****************************************************************************/
/**
 * This value specifies the number of seeds in the queue
 */
const std::size_t DEFAULTSEEDQUEUESIZE=2000;

/****************************************************************************/
/**
 * The minimal size of the double buffer in the GRandomFactoryT
 */
const std::size_t MINDOUBLEBUFFERSIZE=10000;

/****************************************************************************/

#endif /* GRANDOMDEFINES_HPP_ */
