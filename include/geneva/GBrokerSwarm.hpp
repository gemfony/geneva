/**
 * @file GBrokerSwarm.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

// Standard headers go here

#include <sstream>
#include <algorithm>
#include <vector>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/greg_serialize.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>

#ifndef GBROKEREA_HPP_
#define GBROKEREA_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "courtier/GBufferPortT.hpp"
#include "GSwarmPersonalityTraits.hpp"
#include "GSwarm.hpp"
#include "GIndividual.hpp"

namespace Gem {
namespace Geneva {

/**********************************************************************************/
/**
 * The default factor applied to the turn-around time
 * of the first individual in the first generation. Used to
 * find a suitable timeout-value for following individuals.
 */
const boost::uint32_t DEFAULTSWARMWAITFACTOR = 20;

/**
 * The default maximum value of the wait factor used during automatic
 * adaption of the waitFactor_ variable. If set to 0, no automatic
 * adaption will take place.
 */
const boost::uint32_t DEFAULTSWARMMAXWAITFACTOR = 0;

/**
 * The default allowed time in seconds for the first individual
 * in generation 0 to return. Set it to 0 to disable this timeout.
 */
const std::string DEFAULTSWARMFIRSTTIMEOUT = EMPTYDURATION; // defined in GEvolutionaryAlgorithm

/**
 * The default number of milliseconds before the broker times out
 */
const boost::uint32_t DEFAULTSWARMLOOPMSEC = 20;

/**********************************************************************************/



/**********************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GBROKEREA_HPP_ */
