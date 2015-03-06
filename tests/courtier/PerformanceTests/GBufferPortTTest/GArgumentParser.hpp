/**
 * @file GArgumentParser.hpp
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
#include <iostream>
#include <fstream>

// Boost headers go here
#include <boost/cstdint.hpp>
#include <boost/program_options.hpp>

#ifndef GARGUMENTPARSER_HPP_
#define GARGUMENTPARSER_HPP_

// Geneva headers go here

namespace Gem {
namespace Courtier {
namespace Tests {

namespace po = boost::program_options;

/********************************************************************************************/
// Default settings
const boost::uint32_t DEFAULTNPRODUCTIONCYLCESAP = 10000;
const std::size_t DEFAULTNCONTAINERENTRIESAP = 100;
const long DEFAULTPUTTIMEOUTMSAP = 1000;
const long DEFAULTGETTIMEOUTMSAP = 1000;
const std::size_t DEFAULTMAXPUTTIMEOUTS = 100;
const std::size_t DEFAULTMAXGETTIMEOUTS = 100;

/********************************************************************************************/

bool parseCommandLine(
	int
  , char **
  , boost::uint32_t& nProductionCycles
  , std::size_t& nContainerEntries
  , long& putTimeoutMS
  , long& getTimeoutMS
  , std::size_t& maxPutTimeouts
  , std::size_t& maxGetTimeouts
);

/********************************************************************************************/

} /* namespace Tests */
} /* namespace Courtier */
} /* namespace Gem */

#endif /* GARGUMENTPARSER_HPP_ */
