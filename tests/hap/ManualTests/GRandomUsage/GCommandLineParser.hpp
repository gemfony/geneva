/**
 * @file GCommandLineParser.hpp
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

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Boost headers go here
#include <boost/cstdint.hpp>
#include <boost/program_options.hpp>

#ifndef GCOMMANDLINEPARSER_HPP_
#define GCOMMANDLINEPARSER_HPP_

namespace Gem {
namespace Tests {

// Default settings
const std::size_t DEFAULTNENTRIES=20000;
const boost::uint16_t DEFAULTNPRODUCERTHREADS=10;
const bool DEFAULTVERBOSE=true;
const boost::uint16_t DEFAULTRNRPRODUCTIONMODE=0;

namespace po = boost::program_options;

bool parseCommandLine(int argc, char **argv,
					  std::size_t& nEntries,
					  boost::uint16_t& nProducerThreads,
					  boost::uint16_t& rnrProductionMode,
					  bool& verbose);

} /* namespace Tests */
} /* namespace Gem */

#endif /* GCOMMANDLINEPARSER_HPP_ */
