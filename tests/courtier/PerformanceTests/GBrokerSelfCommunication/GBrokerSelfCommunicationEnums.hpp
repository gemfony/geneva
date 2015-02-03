/**
 * @file GBrokerSelfCommunicationEnums.hpp
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

// For Microsoft-compatible compilers
#include "common/GWindowsDefines.hpp"

// Standard headers go here
#include <string>
#include <sstream>
#include <ostream>
#include <istream>

// Boost headers go here
#include <boost/cstdint.hpp>
#include <boost/cast.hpp>

#ifndef GBROKERSELFCOMMUNICATIONENUMS_HPP_
#define GBROKERSELFCOMMUNICATIONENUMS_HPP_

namespace Gem {
namespace Courtier {
namespace Tests {

/**
 * This enum defines the available execution modes of the GBrokerSelfCommunication
 * example
 */
enum GBSCModes {
	SERIAL = 0
	, INTERNALNETWORKING = 1
	, NETWORKING = 2
	, MULTITHREADING = 3
	, THREADANDINTERNALNETWORKING = 4
	, THREAEDANDNETWORKING = 5
};

const GBSCModes MAXGBSCMODES = THREAEDANDNETWORKING;

/** @brief Puts a Gem::Courtier::Tests::GBSCModes into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Courtier::Tests::GBSCModes&);

/** @brief Reads a Gem::Courtier::Tests::GBSCModes item from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Courtier::Tests::GBSCModes&);

} /* namespace Tests */
} /* namespace Courtier */
} /* namespace Gem */

#endif /* GBROKERSELFCOMMUNICATIONENUMS_HPP_ */
