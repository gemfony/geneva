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

// For Microsoft-compatible compilers
#include "common/GWindowsDefines.hpp"

// Standard headers go here
#include <iostream>
#include <fstream>

// Boost headers go here
#include <boost/cstdint.hpp>
#include <boost/program_options.hpp>

#ifndef GARGUMENTPARSER_HPP_
#define GARGUMENTPARSER_HPP_

namespace Gem {
namespace Common {
namespace Tests {

namespace po = boost::program_options;

/********************************************************************************************/
// Default settings
const std::string DEFAULTRESULTFILEAP = "result.C";
const std::size_t DEFAULTNPRODUCERSAP = 4;
const std::size_t DEFAULTNCONSUMERSAP = 4;
const long DEFAULTTIMEOUTMS = 10000; // 0.01 s
const long DEFAULTMAXRANDOMDELAYMS = 0; // 0.01
const std::size_t DEFAULTNITEMS = 10000;
const long DEFAULTSTARTDELAY = 1000000;
const bool DEFAULTSTARTATONCE = true;

/********************************************************************************************/

bool parseCommandLine(
		int argc
	  , char **argv
	  , std::string& resultFile
	  , std::size_t& nProducers
	  , std::size_t& nItems
	  , std::size_t& nConsumers
	  , long& timeoutMS
	  , long& maxRandomDelayMS
	  , long& startDelayMS
	  , bool& startAtOnce
);

/********************************************************************************************/

} /* namespace Tests */
} /* namespace Common */
} /* namespace Gem */

#endif /* GARGUMENTPARSER_HPP_ */
